/* The thing that makes children, remembers them, and contains wait loops. */

/* This file works under BSD, System V, minix, and Posix systems. */

/* Copyright (C) 1987, 1989, 1992 Free Software Foundation, Inc.

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
#include <signal.h>
#include <setjmp.h>
#include <errno.h>

#include "config.h"
#include "command.h"
#include "general.h"
#include "filecntl.h"
#include "jobs.h"
#include "externs.h"
#include "error.h"

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

#if !defined (USG) && !defined (_POSIX_VERSION)
#  include <sgtty.h>
#else
#  if defined (_POSIX_VERSION)
#    include <termios.h>
#  else
#    include <termio.h>
#    if !defined (AIXRT)
#      include <sys/ttold.h>
#    endif /* !AIXRT */
#  endif /* !POSIX_VERSION */
#endif /* USG && _POSIX_VERSION */

#if !defined (SIGABRT)
#  define SIGABRT SIGIOT
#endif /* !SIGABRT */

#if defined (USG) || defined (_POSIX_VERSION)
#  define killpg(pg, sig)		kill(-(pg),(sig))
#endif /* USG || _POSIX_VERSION */

#if defined (USG)
#  define siginterrupt(sig, code)
#endif /* USG */

#if defined (_POSIX_VERSION)
#  define WAITPID(pid, statusp, options) waitpid (pid, statusp, options)
#else
#  define WAITPID(pid, statusp, options) wait (statusp)
#endif /* !_POSIX_VERSION */

#if !defined (errno)
extern int errno;
#endif /* !errno */

extern int interactive, interactive_shell, login_shell;
extern int subshell_environment;
extern int last_command_exit_value;
#if defined (_POSIX_VERSION)
extern sigset_t top_level_mask;
#endif

pid_t last_made_pid = NO_PID;
pid_t last_asynchronous_pid = NO_PID;

/* Call this when you start making children. */
int already_making_children = 0;

#if defined (_POSIX_VERSION)
static void reap_zombie_children ();
#endif

struct proc_status {
  pid_t pid;
  int status;	/* Exit status of PID or 128 + fatal signal number */
};

static struct proc_status *pid_list = (struct proc_status *)NULL;
static int pid_list_size = 0;

#define PROC_BAD -1
#define PROC_STILL_ALIVE -2

/* Allocate new, or grow existing PID_LIST. */
static void
alloc_pid_list ()
{
  register int i;
  int old = pid_list_size;

  pid_list_size += 10;
  pid_list = (struct proc_status *)
    xrealloc (pid_list, pid_list_size * sizeof (struct proc_status));

  /* None of the newly allocated slots have process id's yet. */
  for (i = old; i < pid_list_size; i++)
    pid_list[i].pid = NO_PID;  
}

/* Return the offset within the PID_LIST array of an empty slot.  This can
   create new slots if all of the existing slots are taken. */
static int
find_proc_slot ()
{
  register int i;

  for (i = 0; i < pid_list_size; i++)
    if (pid_list[i].pid == NO_PID)
      return (i);

  if (i == pid_list_size)
    alloc_pid_list ();

  return (i);
}

/* Return the offset within the PID_LIST array of a slot containing PID,
   or the value NO_PID if the pid wasn't found. */
static int
find_index_by_pid (pid)
     pid_t pid;
{
  register int i;

  for (i = 0; i < pid_list_size; i++)
    if (pid_list[i].pid == pid)
      return (i);

  return (NO_PID);
}

/* Return the status of PID as looked up in the PID_LIST array.  A
   return value of PROC_BAD indicates that PID wasn't found. */
static int
find_status_by_pid (pid)
     pid_t pid;
{
  int i;

  i = find_index_by_pid (pid);
  if (i == NO_PID)
    return (PROC_BAD);
  return (pid_list[i].status);
}

/* Give PID the status value STATUS in the PID_LIST array. */
static void
set_pid_status (pid, status)
     pid_t pid;
     WAIT status;
{
  int slot;

  slot = find_index_by_pid (pid);
  if (slot == NO_PID)
    return;

  if (WIFSIGNALED (status))
    pid_list[slot].status = 128 + WTERMSIG (status);
  else
    pid_list[slot].status = WEXITSTATUS (status);
}

static void
add_pid (pid)
     pid_t pid;
{
  int slot;

  slot = find_proc_slot ();
  pid_list[slot].pid = pid;
  pid_list[slot].status = PROC_STILL_ALIVE;
}

int
cleanup_dead_jobs ()
{
  register int i;

#if defined (_POSIX_VERSION)
  reap_zombie_children ();
#endif

  for (i = 0; i < pid_list_size; i++)
    if (pid_list[i].status != PROC_STILL_ALIVE)
      pid_list[i].pid = NO_PID;
}

/* Initialize the job control mechanism, and set up the tty stuff. */
initialize_jobs ()
{
  get_tty_state ();
}

#if !defined (READLINE) && defined (TIOCGWINSZ) && defined (SIGWINCH)
static SigHandler *old_winch;

static sighandler
sigwinch_sighandler (sig)
     int sig;
{
  struct winsize win;

#if defined (USG) && !defined (_POSIX_VERSION)
  set_signal_handler (SIGWINCH, sigwinch_sighandler);
#endif /* USG && !_POSIX_VERSION */
  if ((ioctl (0, TIOCGWINSZ, &win) == 0) &&
      win.ws_row > 0 && win.ws_col > 0)
    set_lines_and_columns (win.ws_row, win.ws_col);
}
#endif /* !READLINE && TIOCGWINSZ && SIGWINCH */

/* Setup this shell to handle C-C, etc. */
void
initialize_job_signals ()
{
  set_signal_handler (SIGINT, sigint_sighandler);
#if !defined (READLINE) && defined (TIOCGWINSZ) && defined (SIGWINCH)
  set_signal_handler (SIGWINCH, sigwinch_sighandler);
#endif /* !READLINE && TIOCGWINSZ && SIGWINCH */

  /* If this is a login shell we don't wish to be disturbed by
     stop signals. */
  if (login_shell)
    {
#if defined (SIGTSTP)
      set_signal_handler (SIGTSTP, SIG_IGN);
      set_signal_handler (SIGTTOU, SIG_IGN);
      set_signal_handler (SIGTTIN, SIG_IGN);
#endif
    }
}

#if defined (_POSIX_VERSION)
/* Collect the status of all zombie children so that their system
   resources can be deallocated. */
static void
reap_zombie_children ()
{
#if defined (WNOHANG)
  pid_t pid;
  WAIT status;

  while ((pid = waitpid (-1, (int *)&status, WNOHANG)) > 0)
    set_pid_status (pid, status);
#endif /* WNOHANG */
}
#endif /* _POSIX_VERSION */

/* Fork, handling errors.  Returns the pid of the newly made child, or 0.
   COMMAND is just for remembering the name of the command; we don't do
   anything else with it.  ASYNC_P says what to do with the tty.  If
   non-zero, then don't give it away. */
pid_t
make_child (command, async_p)
     char *command;
     int async_p;
{
  pid_t pid;
#if defined (_POSIX_VERSION)
  int retry = 1;
#endif /* _POSIX_VERSION */

  /* Discard saved memory. */
  if (command)  
    free (command);

  start_pipeline ();

#if defined (BUFFERED_INPUT)
  /* If default_buffered_input is active, we are reading a script.  If
     the command is asynchronous, we have already duplicated /dev/null
     as fd 0, but have not changed the buffered stream corresponding to
     the old fd 0.  We don't want to sync the stream in this case. */
  if (default_buffered_input != -1 &&
      (!async_p || default_buffered_input > 0))
    sync_buffered_stream (default_buffered_input);
#endif /* BUFFERED_INPUT */

  /* Create the child, handle severe errors. */
#if defined (_POSIX_VERSION)
  retry_fork:
#endif /* _POSIX_VERSION */

  if ((pid = fork ()) < 0)
    {
#if defined (_POSIX_VERSION)
      /* Posix systems with a non-blocking waitpid () system call available
	 get another chance after zombies are reaped. */
      if (errno == EAGAIN && retry)
	{
	  reap_zombie_children ();
	  retry = 0;
	  goto retry_fork;
	}
#endif /* _POSIX_VERSION */

      internal_error ("fork: %s", strerror (errno));

      throw_to_top_level ();
    }
 
  if (pid == 0)
    {
#if defined (BUFFERED_INPUT)
      if (default_buffered_input > 0)
	{
          close_buffered_fd (default_buffered_input);
          default_buffered_input = bash_input.location.buffered_fd = -1;
	}
#endif /* BUFFERED_INPUT */

#if defined (_POSIX_VERSION)
      /* Restore top-level signal mask. */
      sigprocmask (SIG_SETMASK, &top_level_mask, (sigset_t *)NULL);
#endif

      /* Ignore INT and QUIT in asynchronous children. */
      if (async_p)
	{
#if 0
	  /* This now done by setup_async_signals (). */
	  set_signal_handler (SIGINT, SIG_IGN);
	  set_signal_handler (SIGQUIT, SIG_IGN);
#endif
	  last_asynchronous_pid = getpid ();
	}

#if defined (SIGTSTP)
      set_signal_handler (SIGTSTP, SIG_DFL);
      set_signal_handler (SIGTTIN, SIG_DFL);
      set_signal_handler (SIGTTOU, SIG_DFL);
#endif
    }
  else
    {
      /* In the parent. */

      last_made_pid = pid;

      if (async_p)
	last_asynchronous_pid = pid;

      add_pid (pid);
    }
  return (pid);
}

/* Wait for a single pid (PID) and return its exit status. */
wait_for_single_pid (pid)
     pid_t pid;
{
  pid_t got_pid;
  WAIT status;
  int pstatus;

  pstatus = find_status_by_pid (pid);

  if (pstatus == PROC_BAD)
    return (127);

  if (pstatus != PROC_STILL_ALIVE)
    return (pstatus);

  siginterrupt (SIGINT, 1);
  while ((got_pid = WAITPID (pid, &status, 0)) != pid)
    {
      if (got_pid < 0)
	{
	  if (errno != EINTR && errno != ECHILD)
	    {
	      siginterrupt (SIGINT, 0);
	      file_error ("wait");
	    }
	  break;
	}
      else if (got_pid > 0)
        set_pid_status (got_pid, status);
    }

  set_pid_status (got_pid, status);
  siginterrupt (SIGINT, 0);
  QUIT;

  if (WIFSIGNALED (status))
    return (128 + WTERMSIG (status));
  else
    return (WEXITSTATUS (status));
}

/* Wait for all of the shell's children to exit. */
void
wait_for_background_pids ()
{
  pid_t got_pid;
  WAIT status;

  /* If we aren't using job control, we let the kernel take care of the
     bookkeeping for us.  wait () will return -1 and set errno to ECHILD 
     when there are no more unwaited-for child processes on both
     4.2 BSD-based and System V-based systems. */

  siginterrupt (SIGINT, 1);

  /* Wait for ECHILD */
  while ((got_pid = WAITPID (-1, &status, 0)) != -1)
    set_pid_status (got_pid, status);

  if (errno != EINTR && errno != ECHILD)
    {
      siginterrupt (SIGINT, 0);
      file_error("wait");
    }

  siginterrupt (SIGINT, 0);
  QUIT;
}

/* Handle SIGINT while we are waiting for children in a script to exit.
   All interrupts are effectively ignored by the shell, but allowed to
   kill a running job. */
static sighandler
wait_sigint_handler (sig)
     int sig;
{
#if 0
  /* Run a trap handler if one has been defined. */
  maybe_call_trap_handler (sig);
#endif

#if !defined (VOID_SIGHANDLER)
  return (0);
#endif /* !VOID_SIGHANDLER */
}

/* Wait for pid (one of our children) to terminate.  This is called only
   by the execution code in execute_cmd.c. */
int
wait_for (pid)
     pid_t pid;
{
  int return_val, pstatus;
  pid_t got_pid;
  WAIT status;
  SigHandler *old_sigint_handler;

  pstatus = find_status_by_pid (pid);

  if (pstatus == PROC_BAD)
    return (0);

  if (pstatus != PROC_STILL_ALIVE)
    return (pstatus);

  /* If we are running a script, ignore SIGINT while we're waiting for
     a child to exit.  The loop below does some of this, but not all. */
  if (!interactive_shell)
    old_sigint_handler = set_signal_handler (SIGINT, wait_sigint_handler);

  while ((got_pid = WAITPID (-1, &status, 0)) != pid) /* XXX was pid now -1 */
    {
      if (got_pid < 0 && errno == ECHILD)
	{
#if !defined (_POSIX_VERSION)
	  status.w_termsig = status.w_retcode = 0;
#else
	  status = 0;
#endif /* _POSIX_VERSION */
	  break;
	}
      else if (got_pid < 0 && errno != EINTR)
	programming_error ("got errno %d while waiting for %d", errno, pid);
      else if (got_pid > 0)
	set_pid_status (got_pid, status);
    }

  set_pid_status (got_pid, status);

#if defined (_POSIX_VERSION)
  if (got_pid >= 0)
    reap_zombie_children ();
#endif /* _POSIX_VERSION */

  if (!interactive_shell)
    {
      set_signal_handler (SIGINT, old_sigint_handler);
      /* If the job exited because of SIGINT, make sure the shell acts as if
	 it had received one also. */
      if (WIFSIGNALED (status) && (WTERMSIG (status) == SIGINT))
	{
	  if (maybe_call_trap_handler (SIGINT) == 0)
	    (*old_sigint_handler) (SIGINT);
	}
    }
	    
  /* Default return value. */
  /* ``a full 8 bits of status is returned'' */
  if (WIFSIGNALED (status))
    return_val = 128 + WTERMSIG (status);
  else
    return_val = WEXITSTATUS (status);
                            
  if (!WIFSTOPPED (status) && WIFSIGNALED (status) &&
      (WTERMSIG (status) != SIGINT))
    {
      fprintf (stderr, "%s", strsignal (WTERMSIG (status)));
      if (WIFCORED (status))
	fprintf (stderr, " (core dumped)");
      fprintf (stderr, "\n");
    }

  if (interactive_shell && !subshell_environment)
    {
      if (WIFSIGNALED (status) || WIFSTOPPED (status))
	set_tty_state ();
      else
	get_tty_state ();
    }

  return (return_val);
}

/* Give PID SIGNAL.  This determines what job the pid belongs to (if any).
   If PID does belong to a job, and the job is stopped, then CONTinue the
   job after giving it SIGNAL.  Returns -1 on failure.  If GROUP is non-null,
   then kill the process group associated with PID. */
int
kill_pid (pid, signal, group)
     pid_t pid;
     int signal, group;
{
  int result;

  if (group)
    result = killpg (pid, signal);
  else
    result = kill (pid, signal);

  return (result);
}

#if defined (_POSIX_VERSION)
static struct termios shell_tty_info;
#else
#  if defined (USG)
static struct termio shell_tty_info;
#  else
static struct sgttyb shell_tty_info;
#  endif /* USG */
#endif /* _POSIX_VERSION */

static int got_tty_state = 0;

/* Fill the contents of shell_tty_info with the current tty info. */
get_tty_state ()
{
  int tty = open ("/dev/tty", O_RDONLY);
  if (tty != -1)
    {
#if defined (_POSIX_VERSION)
      tcgetattr (tty, &shell_tty_info);
#else
#  if defined (USG)
      ioctl (tty, TCGETA, &shell_tty_info);
#  else
      ioctl (tty, TIOCGETP, &shell_tty_info);
#  endif
#endif
      close (tty);
      got_tty_state = 1;
    }
}

/* Make the current tty use the state in shell_tty_info. */
set_tty_state ()
{
  int tty = open ("/dev/tty", O_RDONLY);
  if (tty != -1)
    {
      if (!got_tty_state)
	{
	  close (tty);
	  return;
	}
#if defined (_POSIX_VERSION)
      tcsetattr (tty, TCSADRAIN, &shell_tty_info);
#else
#  if defined (USG)
      ioctl (tty, TCSETAW, &shell_tty_info);  /* Wait for output, no flush */
#  else
      ioctl (tty, TIOCSETN, &shell_tty_info);
#  endif
#endif
      close (tty);
    }
}

/* Give the terminal to PGRP.  */
give_terminal_to (pgrp)
     pid_t pgrp;
{
}

/* Stop a pipeline. */
stop_pipeline (async, ignore)
     int async;
     COMMAND *ignore;
{
  already_making_children = 0;
}

void
start_pipeline ()
{
  already_making_children = 1;
}

/* Print descriptive information about the job with leader pid PID. */
void
describe_pid (pid)
     pid_t pid;
{
  fprintf (stderr, "%d\n", (int) pid);
}
