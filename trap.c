/* trap.c -- Not the trap command, but useful functions for manipulating
   those objects.  The trap command is in builtins/trap.def. */

/* Copyright (C) 1987, 1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 1, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <stdio.h>

#include "bashtypes.h"
#include "trap.h"

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#include "shell.h"
#include "signames.h"

/* Flags which describe the current handling state of a signal. */
#define SIG_INHERITED   0x0	/* Value inherited from parent. */
#define SIG_TRAPPED     0x1	/* Currently trapped. */
#define SIG_HARD_IGNORE 0x2	/* Signal was ignored on shell entry. */
#define SIG_SPECIAL     0x4	/* Treat this signal specially. */
#define SIG_NO_TRAP     0x8	/* Signal cannot be trapped. */
#define SIG_INPROGRESS  0x10	/* Signal handler currently executing. */
#define SIG_CHANGED     0x20	/* Trap value changed in trap handler. */
#define SIG_IGNORED     0x40	/* The signal is currently being ignored. */

/* An array of such flags, one for each signal, describing what the
   shell will do with a signal. */
static int sigmodes[NSIG];

static void change_signal (), restore_signal ();

/* Variables used here but defined in other files. */
extern int interactive_shell, interactive;
extern int interrupt_immediately;
extern int last_command_exit_value;

/* The list of things to do originally, before we started trapping. */
SigHandler *original_signals[NSIG];

/* For each signal, a slot for a string, which is a command to be
   executed when that signal is recieved.  The slot can also contain
   DEFAULT_SIG, which means do whatever you were going to do before
   you were so rudely interrupted, or IGNORE_SIG, which says ignore
   this signal. */
char *trap_list[NSIG];

/* A bitmap of signals received for which we have trap handlers. */
int pending_traps[NSIG];

/* A value which can never be the target of a trap handler. */
#define IMPOSSIBLE_TRAP_HANDLER (SigHandler *)initialize_traps

void
initialize_traps ()
{
  register int i;

  trap_list[0] = (char *)NULL;
  sigmodes[0] = SIG_INHERITED;	/* On EXIT trap handler. */

  for (i = 1; i < NSIG; i++)
    {
      pending_traps[i] = 0;
      trap_list[i] = (char *)DEFAULT_SIG;
      sigmodes[i] = SIG_INHERITED;
      original_signals[i] = IMPOSSIBLE_TRAP_HANDLER;
    }

  /* Show which signals are treated specially by the shell. */
#if defined (SIGCHLD)
  original_signals[SIGCHLD] = (SigHandler *) set_signal_handler (SIGCHLD, SIG_DFL);
  set_signal_handler (SIGCHLD, original_signals[SIGCHLD]);
  sigmodes[SIGCHLD] |= (SIG_SPECIAL | SIG_NO_TRAP);
#endif /* SIGCHLD */

  original_signals[SIGINT] =
    (SigHandler *) set_signal_handler (SIGINT, SIG_DFL);
  set_signal_handler (SIGINT, original_signals[SIGINT]);
  sigmodes[SIGINT] |= SIG_SPECIAL;

  original_signals[SIGQUIT] =
    (SigHandler *) set_signal_handler (SIGQUIT, SIG_DFL);
  set_signal_handler (SIGQUIT, original_signals[SIGQUIT]);
  sigmodes[SIGQUIT] |= SIG_SPECIAL;

  if (interactive)
    {
      original_signals[SIGTERM] = (SigHandler *)set_signal_handler (SIGTERM, SIG_DFL);
      set_signal_handler (SIGTERM, original_signals[SIGTERM]);
      sigmodes[SIGTERM] |= SIG_SPECIAL;
    }
}

/* Return the print name of this signal. */
char *
signal_name (sig)
     int sig;
{
  if (sig >= NSIG || sig < 0)
    return ("bad signal number");
  else
    return (signal_names[sig]);
}

/* Turn a string into a signal number, or a number into
   a signal number.  If STRING is "2", "SIGINT", or "INT",
   then (int)2 is returned.  Return NO_SIG if STRING doesn't
   contain a valid signal descriptor. */
int
decode_signal (string)
     char *string;
{
  int sig;

  if (sscanf (string, "%d", &sig) == 1)
    {
      if (sig < NSIG && sig >= 0)
	return (sig);
      else
	return (NO_SIG);
    }

  for (sig = 0; sig < NSIG; sig++)
    if (STREQ (string, signal_names[sig]) ||
	STREQ (string, &(signal_names[sig])[3]))
      return (sig);

  return (NO_SIG);
}

/* Non-zero when we catch a trapped signal. */
static int catch_flag = 0;

#if !defined (USG) && !defined (USGr4)
#define HAVE_BSD_SIGNALS
#endif

void
run_pending_traps ()
{
  register int sig;
  int old_exit_value;

  if (catch_flag == 0)		/* simple optimization */
    return;

  catch_flag = 0;

  /* Preserve $? when running trap. */
  old_exit_value = last_command_exit_value;

  for (sig = 1; sig < NSIG; sig++)
    {
      /* XXX this could be made into a counter by using
         while (pending_traps[sig]--) instead of the if statement. */
      if (pending_traps[sig])
	{
#if defined (_POSIX_VERSION)
	  sigset_t set, oset;

	  sigemptyset (&set);
	  sigemptyset (&oset);

	  sigaddset (&set, sig);
	  sigprocmask (SIG_BLOCK, &set, &oset);
#else
#  if defined (HAVE_BSD_SIGNALS)
	  int oldmask = sigblock (sigmask (sig));
#  endif
#endif /* POSIX_VERSION */

	  if (sig == SIGINT)
	    {
	      run_interrupt_trap ();
	      interrupt_state = 0;
	    }
	  else
	    parse_and_execute (savestring (trap_list[sig]), "trap", 0);

	  pending_traps[sig] = 0;

#if defined (_POSIX_VERSION)
	  sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
#else
#  if defined (HAVE_BSD_SIGNALS)
	  sigsetmask (oldmask);
#  endif
#endif /* POSIX_VERSION */
	}
    }

  last_command_exit_value = old_exit_value;
}

sighandler
trap_handler (sig)
     int sig;
{
  if ((sig >= NSIG) ||
      (trap_list[sig] == (char *)DEFAULT_SIG) ||
      (trap_list[sig] == (char *)IGNORE_SIG))
    programming_error ("trap_handler: Bad signal %d", sig);
  else
    {
#if defined (USG) && !defined (HAVE_BSD_SIGNALS) && !defined (_POSIX_VERSION)
      set_signal_handler (sig, trap_handler);
#endif /* USG && !HAVE_BSD_SIGNALS && !_POSIX_VERSION */

      catch_flag = 1;
      pending_traps[sig]++;

      if (interrupt_immediately)
	run_pending_traps ();
    }
#if !defined (VOID_SIGHANDLER)
  return (0);
#endif /* VOID_SIGHANDLER */
}

#if defined (JOB_CONTROL) && defined (SIGCHLD)
/* Make COMMAND_STRING be executed when SIGCHLD is caught. */
void
set_sigchld_trap (command_string)
     char *command_string;
{
  void set_signal ();

  set_signal (SIGCHLD, command_string);
}

/* Make COMMAND_STRING be executed when SIGCHLD is caught iff the current
   SIGCHLD trap handler is DEFAULT_SIG. */
void
maybe_set_sigchld_trap (command_string)
     char *command_string;
{
  void set_signal ();

  if ((sigmodes[SIGCHLD] & SIG_TRAPPED) == 0)
    set_signal (SIGCHLD, command_string);
}
#endif /* JOB_CONTROL && SIGCHLD */

static void
set_sigint_trap (command)
     char *command;
{
  void set_signal ();

  set_signal (SIGINT, command);
}

/* Reset the SIGINT handler so that subshells that are doing `shellsy'
   things, like waiting for command substitution or executing commands
   in explicit subshells ( ( cmd ) ), can catch interrupts properly. */
SigHandler *
set_sigint_handler ()
{
  if (sigmodes[SIGINT] & SIG_HARD_IGNORE)
    return ((SigHandler *)SIG_IGN);

  else if (sigmodes[SIGINT] & SIG_IGNORED)
    return ((SigHandler *)set_signal_handler (SIGINT, SIG_IGN));
    
  else if (sigmodes[SIGINT] & SIG_TRAPPED)
    return ((SigHandler *)set_signal_handler (SIGINT, trap_handler));

  /* The signal is not trapped, so set the handler to the shell's special
     interrupt handler. */
  else if (interactive)	/* XXX - was interactive_shell */
    return (set_signal_handler (SIGINT, sigint_sighandler));
  else
    return (set_signal_handler (SIGINT, termination_unwind_protect));
}

/* Set SIG to call STRING as a command. */
void
set_signal (sig, string)
     int sig;
     char *string;
{
  /* A signal ignored on entry to the shell cannot be trapped or reset, but
     no error is reported when attempting to do so.  -- Posix.2 */
  if (sigmodes[sig] & SIG_HARD_IGNORE)
    return;

  /* Make sure we have original_signals[sig] if the signal has not yet
     been trapped. */
  if ((sigmodes[sig] & SIG_TRAPPED) == 0)
    {
      /* If we aren't sure of the original value, check it. */
      if (original_signals[sig] == IMPOSSIBLE_TRAP_HANDLER)
	{
	  original_signals[sig] = (SigHandler *)set_signal_handler (sig, SIG_DFL);
	  set_signal_handler (sig, original_signals[sig]);
	}

      /* Signals ignored on entry to the shell cannot be trapped or reset. */
      if (original_signals[sig] == SIG_IGN)
	{
	  sigmodes[sig] |= SIG_HARD_IGNORE;
	  return;
	}
    }

  /* Only change the system signal handler if SIG_NO_TRAP is not set.
     The trap command string is changed in either case.  The shell signal
     handlers for SIGINT and SIGCHLD run the user specified traps in an
     environment in which it is safe to do so. */
  if ((sigmodes[sig] & SIG_NO_TRAP) == 0)
    {
      set_signal_handler (sig, SIG_IGN);
      change_signal (sig, savestring (string));
      set_signal_handler (sig, trap_handler);
    }
  else
    change_signal (sig, savestring (string));
}

static void
free_trap_command (sig)
     int sig;
{
  if ((sigmodes[sig] & SIG_TRAPPED) && trap_list[sig] &&
      (trap_list[sig] != (char *)IGNORE_SIG) &&
      (trap_list[sig] != (char *)DEFAULT_SIG) &&
      (trap_list[sig] != (char *)IMPOSSIBLE_TRAP_HANDLER))
    free (trap_list[sig]);
}
     
/* If SIG has a string assigned to it, get rid of it.  Then give it
   VALUE. */
static void
change_signal (sig, value)
     int sig;
     char *value;
{
  free_trap_command (sig);
  trap_list[sig] = value;

  sigmodes[sig] |= SIG_TRAPPED;
  if (value == (char *)IGNORE_SIG)
    sigmodes[sig] |= SIG_IGNORED;
  else
    sigmodes[sig] &= ~SIG_IGNORED;
  if (sigmodes[sig] & SIG_INPROGRESS)
    sigmodes[sig] |= SIG_CHANGED;
}

#define GET_ORIGINAL_SIGNAL(sig) \
  if (sig && sig < NSIG && original_signals[sig] == IMPOSSIBLE_TRAP_HANDLER) \
    get_original_signal (sig)

static void
get_original_signal (sig)
     int sig;
{
  /* If we aren't sure the of the original value, then get it. */
  if (original_signals[sig] == (SigHandler *)IMPOSSIBLE_TRAP_HANDLER)
    {
      original_signals[sig] =
	(SigHandler *) set_signal_handler (sig, SIG_DFL);
      set_signal_handler (sig, original_signals[sig]);

      /* Signals ignored on entry to the shell cannot be trapped. */
      if (original_signals[sig] == SIG_IGN)
	sigmodes[sig] |= SIG_HARD_IGNORE;
    }
}

/* Restore the default action for SIG; i.e., the action the shell
   would have taken before you used the trap command.  This is called
   from trap_builtin (), which takes care to restore the handlers for
   the signals the shell treats specially. */
void
restore_default_signal (sig)
     int sig;
{
  if (sig == 0)
    {
      free_trap_command (sig);
      trap_list[sig] = (char *)NULL;
      sigmodes[sig] &= ~SIG_TRAPPED;
      return;
    }

  GET_ORIGINAL_SIGNAL (sig);

  /* A signal ignored on entry to the shell cannot be trapped or reset, but
     no error is reported when attempting to do so.  Thanks Posix.2. */
  if (sigmodes[sig] & SIG_HARD_IGNORE)
    return;

  /* If we aren't trapping this signal, don't bother doing anything else. */
  if (!(sigmodes[sig] & SIG_TRAPPED))
    return;

  /* Only change the signal handler for SIG if it allows it. */
  if (!(sigmodes[sig] & SIG_NO_TRAP))
    set_signal_handler (sig, original_signals[sig]);

  /* Change the trap command in either case. */
  change_signal (sig, (char *)DEFAULT_SIG);

  /* Mark the signal as no longer trapped. */
  sigmodes[sig] &= ~SIG_TRAPPED;
}

/* Make this signal be ignored. */
void
ignore_signal (sig)
     int sig;
{
  GET_ORIGINAL_SIGNAL (sig);

  /* A signal ignored on entry to the shell cannot be trapped or reset.
     No error is reported when the user attempts to do so.
     Thanks to Posix.2. */
  if (sigmodes[sig] & SIG_HARD_IGNORE)
    return;

  /* If already trapped and ignored, no change necessary. */
  if ((sigmodes[sig] & SIG_TRAPPED) && (trap_list[sig] == (char *)IGNORE_SIG))
    return;

  /* Only change the signal handler for SIG if it allows it. */
  if (!(sigmodes[sig] & SIG_NO_TRAP))
    set_signal_handler (sig, SIG_IGN);

  /* Change the trap command in either case. */
  change_signal (sig, (char *)IGNORE_SIG);
}

/* Handle the calling of "trap 0".  The only sticky situation is when
   the command to be executed includes an "exit".  This is why we have
   to provide our own place for top_level to jump to. */
int
run_exit_trap ()
{
  int old_exit_value;

  old_exit_value = last_command_exit_value;

  /* Run the trap only if signal 0 is trapped and not ignored. */
  if ((sigmodes[0] & SIG_TRAPPED) &&
      (trap_list[0] != (char *)IGNORE_SIG) &&
      (sigmodes[0] & SIG_INPROGRESS) == 0)
    {
      char *trap_command;
      int code;

      trap_command= savestring (trap_list[0]);
      sigmodes[0] &= ~SIG_TRAPPED;
      sigmodes[0] |= SIG_INPROGRESS;

      code = setjmp (top_level);

      if (code == 0)
	parse_and_execute (trap_command, "trap", 0);
      else if (code == EXITPROG)
	return (last_command_exit_value);
      else
	return (old_exit_value);
    }

  return (old_exit_value);
}

/* Set the handler signal SIG to the original and free any trap
   command associated with it. */     
static void
restore_signal (sig)
     int sig;
{
  set_signal_handler (sig, original_signals[sig]);
  change_signal (sig, (char *)DEFAULT_SIG);
  sigmodes[sig] &= ~SIG_TRAPPED;
}

/* Free all the allocated strings in the list of traps and reset the trap
   values to the default. */
void
free_trap_strings ()
{
  register int i;

  for (i = 0; i < NSIG; i++)
    {
      free_trap_command (i);
      trap_list[i] = (char *)DEFAULT_SIG;
      sigmodes[i] &= ~SIG_TRAPPED;
    }
}

/* Reset the handler for SIG to the original value. */
static void
reset_signal (sig)
     int sig;
{
  set_signal_handler (sig, original_signals[sig]);
}

/* Reset the handlers for all trapped signals to the values they had when
   the shell was started. */
void
reset_signal_handlers ()
{
  register int i;

  if (sigmodes[0] & SIG_TRAPPED)
    {
      free_trap_command (0);
      trap_list[0] = (char *)NULL;
      sigmodes[0] &= ~SIG_TRAPPED;
    }

  for (i = 1; i < NSIG; i++)
    {
      if (sigmodes[i] & SIG_SPECIAL)
	reset_signal (i);
      else if (sigmodes[i] & SIG_TRAPPED)
	{
	  if (trap_list[i] == (char *)IGNORE_SIG)
	    set_signal_handler (i, SIG_IGN);
	  else
	    reset_signal (i);
	}
    }
}

/* Reset all trapped signals to their original values.  Signals set to be
   ignored with trap '' SIGNAL should be ignored, so we make sure that they
   are.  Called by child processes after they are forked. */
void
restore_original_signals ()
{
  register int i;

  reset_terminating_signals ();		/* in shell.c */

  if (sigmodes[0] & SIG_TRAPPED)
    {
      free_trap_command (0);
      trap_list[0] = (char *)NULL;
      sigmodes[0] &= ~SIG_TRAPPED;
    }

  for (i = 1; i < NSIG; i++)
    {
      if (sigmodes[i] & SIG_SPECIAL)
	restore_signal (i);
      else if (sigmodes[i] & SIG_TRAPPED)
	{
	  if (trap_list[i] == (char *)IGNORE_SIG)
	    set_signal_handler (i, SIG_IGN);
	  else
	    restore_signal (i);
	}
    }
}

/* Run a trap set on SIGINT.  This is called from throw_to_top_level (), and
   declared here to localize the trap functions. */
void
run_interrupt_trap ()
{
  char *command, *saved_command;
  int old_exit_value;

  /* Run the interrupt trap if SIGINT is trapped and not ignored, and if
     we are not currently running in the interrupt trap handler. */
  if ((sigmodes[SIGINT] & SIG_TRAPPED) &&
      (trap_list[SIGINT] != (char *)IGNORE_SIG) &&
      (trap_list[SIGINT] != (char *)IMPOSSIBLE_TRAP_HANDLER) &&
      ((sigmodes[SIGINT] & SIG_INPROGRESS) == 0))
    {
      saved_command = trap_list[SIGINT];
      sigmodes[SIGINT] |= SIG_INPROGRESS;
      sigmodes[SIGINT] &= ~SIG_CHANGED;

      command = savestring (saved_command);

      old_exit_value = last_command_exit_value;
      parse_and_execute (command, "interrupt trap", 0);
      last_command_exit_value = old_exit_value;

      sigmodes[SIGINT] &= ~SIG_INPROGRESS;

      if (sigmodes[SIGINT] & SIG_CHANGED)
	{
	  free (saved_command);
	  sigmodes[SIGINT] &= ~SIG_CHANGED;
	}
    }
}

/* If a trap handler exists for signal SIG, then call it; otherwise just
   return failure. */
int
maybe_call_trap_handler (sig)
     int sig;
{
  /* Call the trap handler for SIG if the signal is trapped and not ignored. */
  if ((sigmodes[sig] & SIG_TRAPPED) &&
      (trap_list[sig] != (char *)IGNORE_SIG))
    {
      switch (sig)
	{
	case SIGINT:
	  run_interrupt_trap ();
	  break;
	case 0:
	  run_exit_trap ();
	  break;
	default:
	  trap_handler (sig);
	  break;
	}
      return (1);
    }
  else
    return (0);
}

int
signal_is_trapped (sig)
     int sig;
{
  return (sigmodes[sig] & SIG_TRAPPED);
}

int
signal_is_special (sig)
     int sig;
{
  return (sigmodes[sig] & SIG_SPECIAL);
}

int
signal_is_ignored (sig)
     int sig;
{
  return (sigmodes[sig] & SIG_IGNORED);
}

void
set_signal_ignored (sig)
     int sig;
{
  sigmodes[sig] |= SIG_HARD_IGNORE;
  original_signals[sig] = SIG_IGN; 
}
