/* The thing that makes children, remembers them, and contains wait loops. */

/* This file works under BSD, System V, minix, and Posix systems.  It does
   not implement job control. */

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

#include "config.h"

#include "bashtypes.h"
#include "filecntl.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <signal.h>
#include <errno.h>

#include "bashjmp.h"

#include "command.h"
#include "general.h"
#include "jobs.h"
#include "externs.h"
#include "sig.h"
#include "error.h"
#include "bashtty.h"

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

#if defined (TERMIOS_TTY_DRIVER)
#  include <termios.h>
#else
#  if defined (TERMIO_TTY_DRIVER)
#    include <termio.h>
#  else
#    include <sgtty.h>
#  endif /* !TERMIO_TTY_DRIVER */
#endif /* TERMIOS_TTY_DRIVER */

#if !defined (STRUCT_WINSIZE_IN_SYS_IOCTL)
/* For struct winsize on SCO */
/*   sys/ptem.h has winsize but needs mblk_t from sys/stream.h */
#  if defined (HAVE_SYS_PTEM_H) && defined (TIOCGWINSZ) && defined (SIGWINCH)
#    if defined (HAVE_SYS_STREAM_H)
#      include <sys/stream.h>
#    endif
#    include <sys/ptem.h>
#  endif /* HAVE_SYS_PTEM_H && TIOCGWINSZ && SIGWINCH */
#endif /* !STRUCT_WINSIZE_IN_SYS_IOCTL */

#if defined (_POSIX_VERSION) || !defined (HAVE_KILLPG)
#  define killpg(pg, sig)		kill(-(pg),(sig))
#endif /* USG || _POSIX_VERSION */

#if !defined (HAVE_SIGINTERRUPT)
#  define siginterrupt(sig, code)
#endif /* USG */

#if defined (HAVE_WAITPID)
#  define WAITPID(pid, statusp, options) waitpid (pid, statusp, options)
#else
#  define WAITPID(pid, statusp, options) wait (statusp)
#endif /* !HAVE_WAITPID */

/* Return the fd from which we are actually getting input. */
#define input_tty() (shell_tty != -1) ? shell_tty : fileno (stderr)

#if !defined (errno)
extern int errno;
#endif /* !errno */

#if defined (READLINE)
extern void _rl_set_screen_size (); 
#endif    

extern int interactive, interactive_shell, login_shell;
extern int subshell_environment;
extern int last_command_exit_value;
#if defined (HAVE_POSIX_SIGNALS)
extern sigset_t top_level_mask;
#endif

pid_t last_made_pid = NO_PID;
pid_t last_asynchronous_pid = NO_PID;

/* Call this when you start making children. */
int already_making_children = 0;

/* The controlling tty for this shell. */
int shell_tty = -1;

/* If this is non-zero, $LINES and $COLUMNS are reset after every process
   exits from get_tty_state(). */
int check_window_size;

#if defined (HAVE_WAITPID)
static void reap_zombie_children ();
#endif

struct proc_status {
  pid_t pid;
  int status;	/* Exit status of PID or 128 + fatal signal number */
};

static struct proc_status *pid_list = (struct proc_status *)NULL;
static int pid_list_size;

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

#if defined (HAVE_WAITPID)
  reap_zombie_children ();
#endif

  for (i = 0; i < pid_list_size; i++)
    if (pid_list[i].status != PROC_STILL_ALIVE)
      pid_list[i].pid = NO_PID;
}

/* Initialize the job control mechanism, and set up the tty stuff. */
initialize_job_control (force)
     int force;
{
  shell_tty = fileno (stderr);

  if (interactive)
    get_tty_state ();
}

#if defined (TIOCGWINSZ) && defined (SIGWINCH)
static SigHandler *old_winch = (SigHandler *)SIG_DFL;

static void
get_new_window_size (from_sig)
     int from_sig;
{
  struct winsize win;
  int tty;

  tty = input_tty ();
  if (tty >= 0 && (ioctl (tty, TIOCGWINSZ, &win) == 0) &&
      win.ws_row > 0 && win.ws_col > 0)
    {
#if defined (aixpc)
      shell_tty_info.c_winsize = win;	/* structure copying */
#endif
      set_lines_and_columns (win.ws_row, win.ws_col);
#if defined (READLINE)
      _rl_set_screen_size (win.ws_row, win.ws_col);
#endif
    }
}

static sighandler
sigwinch_sighandler (sig)
     int sig;
{
#if defined (MUST_REINSTALL_SIGHANDLERS)
  set_signal_handler (SIGWINCH, sigwinch_sighandler);
#endif /* MUST_REINSTALL_SIGHANDLERS */
  get_new_window_size (1);
}
#else
static void
get_new_window_size (from_sig)
     int from_sig;
{
}
#endif /* TIOCGWINSZ && SIGWINCH */

void
set_sigwinch_handler ()
{
#if defined (TIOCGWINSZ) && defined (SIGWINCH)
  old_winch = set_signal_handler (SIGWINCH, sigwinch_sighandler);
#endif
}

void
unset_sigwinch_handler ()
{
#if defined (TIOCGWINSZ) && defined (SIGWINCH)
  set_signal_handler (SIGWINCH, old_winch);
#endif
}

/* Setup this shell to handle C-C, etc. */
void
initialize_job_signals ()
{
  set_signal_handler (SIGINT, sigint_sighandler);
  set_sigwinch_handler ();

  /* If this is a login shell we don't wish to be disturbed by
     stop signals. */
  if (login_shell)
    ignore_tty_job_signals ();
}

#if defined (HAVE_WAITPID)
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
#endif
}
#endif /* WAITPID && WNOHANG */

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
#if defined (HAVE_WAITPID)
  int retry = 1;
#endif /* HAVE_WAITPID */

  /* Discard saved memory. */
  if (command)
    free (command);

  start_pipeline ();

#if defined (BUFFERED_INPUT)
  /* If default_buffered_input is active, we are reading a script.  If
     the command is asynchronous, we have already duplicated /dev/null
     as fd 0, but have not changed the buffered stream corresponding to
     the old fd 0.  We don't want to sync the stream in this case. */
  if (default_buffered_input != -1 && (!async_p || default_buffered_input > 0))
    sync_buffered_stream (default_buffered_input);
#endif /* BUFFERED_INPUT */

  /* Create the child, handle severe errors. */
#if defined (HAVE_WAITPID)
  retry_fork:
#endif /* HAVE_WAITPID */

  if ((pid = fork ()) < 0)
    {
#if defined (HAVE_WAITPID)
      /* Posix systems with a non-blocking waitpid () system call available
	 get another chance after zombies are reaped. */
      if (errno == EAGAIN && retry)
	{
	  reap_zombie_children ();
	  retry = 0;
	  goto retry_fork;
	}
#endif /* HAVE_WAITPID */

      sys_error ("fork");

      throw_to_top_level ();
    }

  if (pid == 0)
    {
#if defined (BUFFERED_INPUT)
      unset_bash_input (0);
#endif /* BUFFERED_INPUT */

#if defined (HAVE_POSIX_SIGNALS)
      /* Restore top-level signal mask. */
      sigprocmask (SIG_SETMASK, &top_level_mask, (sigset_t *)NULL);
#endif

      /* Ignore INT and QUIT in asynchronous children. */
      if (async_p)
	last_asynchronous_pid = getpid ();

      default_tty_job_signals ();
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

void
ignore_tty_job_signals ()
{
#if defined (SIGTSTP)
  set_signal_handler (SIGTSTP, SIG_IGN);
  set_signal_handler (SIGTTIN, SIG_IGN);
  set_signal_handler (SIGTTOU, SIG_IGN);
#endif
}

void
default_tty_job_signals ()
{
#if defined (SIGTSTP)
  set_signal_handler (SIGTSTP, SIG_DFL);
  set_signal_handler (SIGTTIN, SIG_DFL);
  set_signal_handler (SIGTTOU, SIG_DFL);
#endif
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
    {
      internal_error ("wait: pid %d is not a child of this shell", pid);
      return (127);
    }

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
	      sys_error ("wait");
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
      sys_error("wait");
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

  SIGRETURN (0);
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
	programming_error ("wait_for(%d): %s", pid, strerror(errno));
      else if (got_pid > 0)
	set_pid_status (got_pid, status);
    }

  set_pid_status (got_pid, status);

#if defined (HAVE_WAITPID)
  if (got_pid >= 0)
    reap_zombie_children ();
#endif /* HAVE_WAITPID */

  if (interactive_shell == 0)
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

#if !defined (DONT_REPORT_SIGPIPE)
  if ((WIFSTOPPED (status) == 0) && WIFSIGNALED (status) &&
	(WTERMSIG (status) != SIGINT))
#else
  if ((WIFSTOPPED (status) == 0) && WIFSIGNALED (status) &&
	(WTERMSIG (status) != SIGINT) && (WTERMSIG (status) != SIGPIPE))
#endif
    {
      fprintf (stderr, "%s", strsignal (WTERMSIG (status)));
      if (WIFCORED (status))
	fprintf (stderr, " (core dumped)");
      fprintf (stderr, "\n");
    }

  if (interactive_shell && subshell_environment == 0)
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

#if defined (TERMIOS_TTY_DRIVER)
static struct termios shell_tty_info;
#else /* !TERMIOS_TTY_DRIVER */
#  if defined (TERMIO_TTY_DRIVER)
static struct termio shell_tty_info;
#  else
static struct sgttyb shell_tty_info;
#  endif /* !TERMIO_TTY_DRIVER */
#endif /* !TERMIOS_TTY_DRIVER */

static int got_tty_state;

/* Fill the contents of shell_tty_info with the current tty info. */
get_tty_state ()
{
  int tty;

  tty = input_tty ();
  if (tty != -1)
    {
#if defined (TERMIOS_TTY_DRIVER)
      tcgetattr (tty, &shell_tty_info);
#else
#  if defined (TERMIO_TTY_DRIVER)
      ioctl (tty, TCGETA, &shell_tty_info);
#  else
      ioctl (tty, TIOCGETP, &shell_tty_info);
#  endif
#endif
      got_tty_state = 1;
      if (check_window_size)
	get_new_window_size (0);
    }
}

/* Make the current tty use the state in shell_tty_info. */
set_tty_state ()
{
  int tty;

  tty = input_tty ();
  if (tty != -1)
    {
      if (got_tty_state == 0)
	return;

#if defined (TERMIOS_TTY_DRIVER)
      tcsetattr (tty, TCSADRAIN, &shell_tty_info);
#else
#  if defined (TERMIO_TTY_DRIVER)
      ioctl (tty, TCSETAW, &shell_tty_info);  /* Wait for output, no flush */
#  else
      ioctl (tty, TIOCSETN, &shell_tty_info);
#  endif
#endif
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

int
get_job_by_pid (pid, block)
     pid_t pid;
     int block;
{
  int i;

  i = find_index_by_pid (pid);
  return ((i == NO_PID) ? PROC_BAD : i);
}

/* Print descriptive information about the job with leader pid PID. */
void
describe_pid (pid)
     pid_t pid;
{
  fprintf (stderr, "%d\n", (int) pid);
}

void
unfreeze_jobs_list ()
{
}
