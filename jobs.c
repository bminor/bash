/* The thing that makes children, remembers them, and contains wait loops. */

/* This file works with both POSIX and BSD systems. */

/* Copyright (C) 1989, 1992 Free Software Foundation, Inc.

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

/* Something that can be ignored. */
#define IGNORE_ARG (char *)0

#include "config.h"

#if !defined (JOB_CONTROL)
#include "nojobs.c"
#else /* JOB_CONTROL */

#include "bashtypes.h"
#include "trap.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>

#if !defined (USG) || defined (HAVE_RESOURCE)
#include <sys/time.h>
#endif /* USG */

#if !defined (_POSIX_VERSION)
#  if defined (HAVE_RESOURCE)
#    include <sys/resource.h>
#  endif
#endif /* _POSIX_VERSION */

#include <sys/file.h>
#include "filecntl.h"
#include <sys/ioctl.h>
#include <sys/param.h>

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

/* Terminal handling stuff, to save and restore tty state. */
#define NEW_TTY_DRIVER

/* Define this if your output is getting swallowed.  It's a no-op on
   machines with the termio or termios tty drivers. */
/* #define DRAIN_OUTPUT */

#if defined (_POSIX_VERSION) && !defined (TERMIOS_MISSING)
#  undef NEW_TTY_DRIVER
#  define TERMIOS_TTY_DRIVER
#  if defined (sun) && !defined (_POSIX_SOURCE)	/* XXX - SunOS4, SunOS5? */
#    define _POSIX_SOURCE
#  endif
#else /* !_POSIX_VERSION */
#  if defined (USG) || defined (hpux) || defined (Xenix) || defined (sgi)
#    undef NEW_TTY_DRIVER
#    define TERMIO_TTY_DRIVER
#  endif /* USG | hpux | Xenix | sgi */
#endif /* !_POSIX_VERSION */

/* Include the right header file for the specific type of terminal
   handler installed on this system. */
#if defined (NEW_TTY_DRIVER)
#include <sgtty.h>
#endif

#if defined (TERMIO_TTY_DRIVER)
#include <termio.h>
#endif

#if defined (TERMIOS_TTY_DRIVER)
/* For Sun workstations we undefine a couple of defines so that
   the inclusion of termios.h won't cause complaints. */
#  if defined (SunOS4)
#    undef ECHO
#    undef NOFLSH
#    undef TOSTOP
#  endif /* SunOS4 */
#  include <termios.h>
#endif /* TERMIOS_TTY_DRIVER */

/* For the TIOCGPGRP and TIOCSPGRP ioctl parameters on HP-UX */

#if defined (hpux) && !defined (TERMIOS_TTY_DRIVER)
#  include <bsdtty.h>
#endif /* hpux && !TERMIOS_TTY_DRIVER */

#include "bashansi.h"
#include "shell.h"
#include "jobs.h"

#include "builtins/builtext.h"
#include "builtins/common.h"

/* Not all systems declare errno in errno.h... and some systems #define it! */
#if !defined (errno)
extern int errno;
#endif /* !errno */

/* Variables used here but defined in other files. */
extern int interactive, interactive_shell, asynchronous_notification;
extern int subshell_environment;
extern int posixly_correct, no_symbolic_links, shell_level;
extern int interrupt_immediately, last_command_exit_value;
extern int loop_level, breaking;
extern Function *this_shell_builtin;
extern char *shell_name, *this_command_name;
extern sigset_t top_level_mask;

/* The array of known jobs. */
JOB **jobs = (JOB **)NULL;

/* The number of slots currently allocated to JOBS. */
int job_slots = 0;

/* The number of additional slots to allocate when we run out. */
#define JOB_SLOTS 5

/* The controlling tty for this shell. */
int shell_tty = -1;

/* The shell's process group. */
pid_t shell_pgrp = NO_PID;

/* The terminal's process group. */
pid_t terminal_pgrp = NO_PID;

/* The process group of the shell's parent. */
pid_t original_pgrp = NO_PID;

/* The process group of the pipeline currently being made. */
pid_t pipeline_pgrp = (pid_t)0;

#if defined (PGRP_PIPE)
/* Pipes which each shell uses to communicate with the process group leader
   until all of the processes in a pipeline have been started.  Then the
   process leader is allowed to continue. */
int pgrp_pipe[2] = { -1, -1 };
#endif
      
/* The job which is current; i.e. the one that `%+' stands for. */
int current_job = NO_JOB;

/* The previous job; i.e. the one that `%-' stands for. */
int previous_job = NO_JOB;

/* Last child made by the shell.  */
pid_t last_made_pid = NO_PID;

/* Pid of the last asynchronous child. */
pid_t last_asynchronous_pid = NO_PID;

/* The pipeline currently being built. */
PROCESS *the_pipeline = (PROCESS *)NULL;

/* If this is non-zero, do job control. */
int job_control = 1;

/* Call this when you start making children. */
int already_making_children = 0;

/* Functions local to this file. */
static sighandler flush_child ();
static int waitchld();
static PROCESS *find_pipeline ();
static char *current_working_directory ();
static char *job_working_directory ();
static pid_t last_pid ();
static int set_new_line_discipline (), map_over_jobs (), last_running_job ();
static int most_recent_job_in_state (), last_stopped_job (), find_job ();
static void notify_of_job_status (), cleanup_dead_jobs (), discard_pipeline ();
static void add_process (), set_current_job (), reset_current ();
static void pretty_print_job ();
static void mark_dead_jobs_as_notified ();
#if defined (PGRP_PIPE)
static void pipe_read (), pipe_close ();
#endif

static int waiting_for_job, sigchld;

/* Set this to non-zero whenever you don't want the jobs list to change at
   all: no jobs deleted and no status change notifications.  This is used,
   for example, when executing SIGCHLD traps, which may run arbitrary
   commands. */
static int freeze_jobs_list;

#if !defined (_POSIX_VERSION)

/* These are definitions to map POSIX 1003.1 functions onto existing BSD
   library functions and system calls. */
#define setpgid(pid, pgrp)	setpgrp (pid, pgrp)
#define tcsetpgrp(fd, pgrp)	ioctl ((fd), TIOCSPGRP, &(pgrp))

pid_t
tcgetpgrp (fd)
     int fd;
{
  pid_t pgrp;

  /* ioctl will handle setting errno correctly. */
  if (ioctl (fd, TIOCGPGRP, &pgrp) < 0)
    return (-1);
  return (pgrp);
}

/* Perform OPERATION on NEWSET, perhaps leaving information in OLDSET. */
sigprocmask (operation, newset, oldset)
     int operation, *newset, *oldset;
{
  int old, new;

  if (newset)
    new = *newset;
  else
    new = 0;

  switch (operation)
    {
    case SIG_BLOCK:
      old = sigblock (new);
      break;

    case SIG_SETMASK:
      sigsetmask (new);
      break;

    default:
      internal_error ("Bad code in jobs.c: sigprocmask");
    }

  if (oldset)
    *oldset = old;
}
#endif /* !_POSIX_VERSION */

/* Return the working directory for the current process.  Unlike
   job_working_directory, this does not call malloc (), nor do any
   of the functions it calls.  This is so that it can safely be called
   from a signal handler. */
static char *
current_working_directory ()
{
  char *dir;
  static char d[MAXPATHLEN];

  dir = get_string_value ("PWD");

  if (!dir && the_current_working_directory && no_symbolic_links)
    dir = the_current_working_directory;

  if (!dir)
    {
      dir = getwd (d);
      if (dir)
	dir = d;
    }

  if (!dir)
    return ("<unknown>");
  else
    return (dir);
}

/* Return the working directory for the current process. */
static char *
job_working_directory ()
{
  char *dir;

  dir = get_string_value ("PWD");
  if (dir)
    return (savestring (dir));

  dir = get_working_directory ("job-working-directory");
  if (dir)
    return (dir);

  return (savestring ("<unknown>"));
}

void
making_children ()
{
  if (already_making_children)
    return;

  already_making_children = 1;
  start_pipeline ();
}

void
stop_making_children ()
{
  already_making_children = 0;
}

void
cleanup_the_pipeline ()
{
  if (the_pipeline)
    {
      discard_pipeline (the_pipeline);
      the_pipeline = (PROCESS *)NULL;
    }
}

/* Start building a pipeline.  */
void
start_pipeline ()
{
  if (the_pipeline)
    {
      discard_pipeline (the_pipeline);
      the_pipeline = (PROCESS *)NULL;
      pipeline_pgrp = 0;
#if defined (PGRP_PIPE)
      pipe_close (pgrp_pipe);
#endif
    }

#if defined (PGRP_PIPE)
  if (job_control)
    {
      if (pipe (pgrp_pipe) == -1)
	internal_error ("start_pipeline: pgrp pipe");
    }
#endif
}

/* Stop building a pipeline.  Install the process list in the job array.
   This returns the index of the newly installed job.
   DEFERRED is a command structure to be executed upon satisfactory
   execution exit of this pipeline. */
int
stop_pipeline (async, deferred)
     int async;
     COMMAND *deferred;
{
  register int i, j;
  JOB *newjob = (JOB *)NULL;
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);

#if defined (PGRP_PIPE)
  /* The parent closes the process group synchronization pipe. */
  pipe_close (pgrp_pipe);
#endif
    
  cleanup_dead_jobs ();

  if (!job_slots)
    {
      jobs =
	(JOB **)xmalloc ((job_slots = JOB_SLOTS) * sizeof (JOB *));

      /* Now blank out these new entries. */
      for (i = 0; i < job_slots; i++)
	jobs[i] = (JOB *)NULL;
    }

  /* Scan from the last slot backward, looking for the next free one. */
  if (interactive)
    {
      for (i = job_slots; i; i--)
	if (jobs[i - 1])
	  break;
    }
  else
    {
      /* If we're not interactive, we don't need to monotonically increase
	 the job number (in fact, we don't care about the job number at all),
	 so we can simply scan for the first free slot.  This helps to keep
	 us from continuously reallocating the jobs array when running
	 certain kinds of shell loops, and saves time spent searching. */
      for (i = 0; i < job_slots; i++)
	if (!jobs[i])
	  break;
    }

  /* Do we need more room? */
  if (i == job_slots)
    {
      job_slots += JOB_SLOTS;
      jobs = (JOB **)xrealloc (jobs, ((1 + job_slots) * sizeof (JOB *)));

      for (j = i; j < job_slots; j++)
	jobs[j] = (JOB *)NULL;
    }

  /* Add the current pipeline to the job list. */
  if (the_pipeline)
    {
      register PROCESS *p;

      newjob = (JOB *)xmalloc (sizeof (JOB));

      for (p = the_pipeline; p->next != the_pipeline; p = p->next);
      p->next = (PROCESS *)NULL;
      newjob->pipe = REVERSE_LIST (the_pipeline, PROCESS *);
      for (p = newjob->pipe; p->next; p = p->next);
      p->next = newjob->pipe;

      the_pipeline = (PROCESS *)NULL;
      newjob->pgrp = pipeline_pgrp;
      pipeline_pgrp = 0;

      newjob->flags = 0;

      /* Flag to see if in another pgrp. */
      if (job_control)
	newjob->flags |= J_JOBCONTROL;

      /* Set the state of this pipeline. */
      {
	register PROCESS *p = newjob->pipe;
	register int any_alive = 0;
	register int any_stopped = 0;

	do
	  {
	    any_alive |= p->running;
	    any_stopped |= WIFSTOPPED (p->status);
	    p = p->next;
	  }
	while (p != newjob->pipe);

	if (any_alive)
	  {
	    newjob->state = JRUNNING;
	  }
	else
	  {
	    if (any_stopped)
	      newjob->state = JSTOPPED;
	    else
	      newjob->state = JDEAD;
	  }
      }

      newjob->wd = job_working_directory ();
      newjob->deferred = deferred;

      jobs[i] = newjob;
    }

  if (async)
    {
      if (newjob)
	newjob->flags &= ~J_FOREGROUND;
      reset_current ();
    }
  else
    {
      if (newjob)
	{
	  newjob->flags |= J_FOREGROUND;
	  /*
	   *		!!!!! NOTE !!!!!  (chet@ins.cwru.edu)
	   *
	   * The currently-accepted job control wisdom says to set the
	   * terminal's process group n+1 times in an n-step pipeline:
	   * once in the parent and once in each child.  This is where
	   * the parent gives it away.
	   *
	   */
	  if (job_control && newjob->pgrp)
	    give_terminal_to (newjob->pgrp);
	}
    }

  stop_making_children ();
  UNBLOCK_CHILD (oset);
  return (current_job);
}

/* Delete all DEAD jobs that the user had received notification about. */
static void
cleanup_dead_jobs ()
{
  register int i;
  sigset_t set, oset;

  if (!job_slots || freeze_jobs_list)
    return;

  BLOCK_CHILD (set, oset);

  for (i = 0; i < job_slots; i++)
    if (jobs[i] && JOBSTATE (i) == JDEAD && (jobs[i]->flags & J_NOTIFIED))
      delete_job (i);

  UNBLOCK_CHILD (oset);
}

/* Delete the job at INDEX from the job list.  Must be called
   with SIGCHLD blocked. */
void
delete_job (job_index)
     int job_index;
{
  register JOB *temp;

  if (freeze_jobs_list)
    return;

  temp = jobs[job_index];
  if (job_index == current_job || job_index == previous_job)
    reset_current ();

  jobs[job_index] = (JOB *)NULL;

  free (temp->wd);
  discard_pipeline (temp->pipe);

  if (temp->deferred)
    dispose_command (temp->deferred);

  free (temp);
}

/* Get rid of the data structure associated with a process chain. */
static void
discard_pipeline (chain)
     register PROCESS *chain;
{
  register PROCESS *this, *next;

  this = chain;
  do
    {
      next = this->next;
      if (this->command)
	free (this->command);
      free (this);
      this = next;
    }
  while (this != chain);
}

/* Add this process to the chain being built in the_pipeline.
   NAME is the command string that will be exec'ed later.
   PID is the process id of the child. */
static void
add_process (name, pid)
     char *name;
     pid_t pid;
{
  PROCESS *t = (PROCESS *)xmalloc (sizeof (PROCESS));

  t->next = the_pipeline;
  t->pid = pid;
  WSTATUS (t->status) = 0;
  t->running = 1;
  t->command = name;
  the_pipeline = t;

  if (!(t->next))
    t->next = t;
  else
    {
      register PROCESS *p = t->next;

      while (p->next != t->next)
	p = p->next;
      p->next = t;
    }
}

#if 0
/* Take the last job and make it the first job.  Must be called with
   SIGCHLD blocked. */
rotate_the_pipeline ()
{
  PROCESS *p;

  if (the_pipeline->next == the_pipeline)
    return;
  for (p = the_pipeline; p->next != the_pipeline; p = p->next)
    ;
  the_pipeline = p;
}

/* Reverse the order of the processes in the_pipeline.  Must be called with
   SIGCHLD blocked. */
reverse_the_pipeline ()
{
  PROCESS *p, *n;

  if (the_pipeline->next == the_pipeline)
    return;

  for (p = the_pipeline; p->next != the_pipeline; p = p->next)
    ;
  p->next = (PROCESS *)NULL;

  n = REVERSE_LIST (the_pipeline, PROCESS *);

  the_pipeline = n;
  for (p = the_pipeline; p->next; p = p->next)
    ;
  p->next = the_pipeline;
}
#endif

/* Map FUNC over the list of jobs.  If FUNC returns non-zero,
   then it is time to stop mapping, and that is the return value
   for map_over_jobs.  FUNC is called with a JOB, arg1, arg2,
   and INDEX. */
static int
map_over_jobs (func, arg1, arg2)
     Function *func;
     int arg1, arg2;
{
  register int i;
  int result;
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);
  result = 0;

  for (i = 0; i < job_slots; i++)
    {
      if (jobs[i])
	{
	  result = (*func)(jobs[i], arg1, arg2, i);
	  if (result)
	    break;
	}
    }

  UNBLOCK_CHILD (oset);
  return (result);
}

/* Cause all the jobs in the current pipeline to exit. */
void
terminate_current_pipeline ()
{
  if (pipeline_pgrp && pipeline_pgrp != shell_pgrp)
    {
      killpg (pipeline_pgrp, SIGTERM);
      killpg (pipeline_pgrp, SIGCONT);
    }
}

/* Cause all stopped jobs to exit. */
void
terminate_stopped_jobs ()
{
  register int i;

  for (i = 0; i < job_slots; i++)
    {
      if (jobs[i] && (JOBSTATE (i) == JSTOPPED))
	{
	  killpg (jobs[i]->pgrp, SIGTERM);
	  killpg (jobs[i]->pgrp, SIGCONT);
	}
    }
}

/* Cause all jobs, running or stopped, to receive a hangup signal. */
void
hangup_all_jobs ()
{
  register int i;

  for (i = 0; i < job_slots; i++)
    {
      if (jobs[i])
	{
	  killpg (jobs[i]->pgrp, SIGHUP);
	  if (JOBSTATE (i) == JSTOPPED)
	    killpg (jobs[i]->pgrp, SIGCONT);
	}
    }
}

void
kill_current_pipeline ()
{
  stop_making_children ();
  start_pipeline ();
}

/* Return the pipeline that PID belongs to.  Note that the pipeline
   doesn't have to belong to a job.  Must be called with SIGCHLD blocked. */
static PROCESS *
find_pipeline (pid)
     pid_t pid;
{
  int job;

  /* See if this process is in the pipeline that we are building. */
  if (the_pipeline)
    {
      register PROCESS *p = the_pipeline;

      do
	{
	  /* Return it if we found it. */
	  if (p->pid == pid)
	    return (p);

	  p = p->next;
	}
      while (p != the_pipeline);
    }

  job = find_job (pid);

  if (job == NO_JOB)
    return ((PROCESS *)NULL);
  else
    return (jobs[job]->pipe);
}

/* Return the job index that PID belongs to, or NO_JOB if it doesn't
   belong to any job.  Must be called with SIGCHLD blocked. */
static int
find_job (pid)
     pid_t pid;
{
  register int i;
  register PROCESS *p;

  for (i = 0; i < job_slots; i++)
    {
      if (jobs[i])
	{
	  p = jobs[i]->pipe;

	  do
	    {
	      if (p->pid == pid)
		return (i);

	      p = p->next;
	    }
	  while (p != jobs[i]->pipe);
	}
    }

  return (NO_JOB);
}

/* Print descriptive information about the job with leader pid PID. */
void
describe_pid (pid)
     pid_t pid;
{
  int job;
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);

  job = find_job (pid);

  if (job != NO_JOB)
    printf ("[%d] %d\n", job + 1, pid);
  else
    programming_error ("describe_pid: No such pid (%d)!\n", pid);

  UNBLOCK_CHILD (oset);
}

/* This is the way to print out information on a job if you
   know the index.  FORMAT is:

    JLIST_NORMAL)   [1]+ Running	   emacs
    JLIST_LONG  )   [1]+ 2378 Running      emacs
    -1	  )   [1]+ 2378	      emacs

    JLIST_NORMAL)   [1]+ Stopped	   ls | more
    JLIST_LONG  )   [1]+ 2369 Stopped      ls
			 2367	    | more
    JLIST_PID_ONLY)
	Just list the pid of the process group leader (really
	the process group).
    JLIST_CHANGED_ONLY)
	Use format JLIST_NORMAL, but list only jobs about which
	the user has not been notified. */
static void
pretty_print_job (job_index, format, stream)
     int job_index, format;
     FILE *stream;
{
  register PROCESS *p, *first, *last;
  int name_padding;
  char retcode_name_buffer[20];
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);

  /* Format only pid information about the process group leader? */
  if (format == JLIST_PID_ONLY)
    {
      fprintf (stream, "%d\n", jobs[job_index]->pipe->pid);
      UNBLOCK_CHILD (oset);
      return;
    }

  if (format == JLIST_CHANGED_ONLY)
    {
      if (jobs[job_index]->flags & J_NOTIFIED)
	{
	  UNBLOCK_CHILD (oset);
	  return;
	}
      format = JLIST_STANDARD;
    }

  fprintf (stream, "[%d]%c ", job_index + 1,
	   (job_index == current_job) ? '+':
	   (job_index == previous_job) ? '-' : ' ');

  first = last = p = jobs[job_index]->pipe;
  while (last->next != first)
    last = last->next;

  /* We have printed information about this job.  When the job's
     status changes, waitchld () sets the notification flag to 0. */
  jobs[job_index]->flags |= J_NOTIFIED;

  for (;;)
    {
      if (p != first)
	fprintf (stream, format ? "     " : " |");

      if (format)
	fprintf (stream, "%5d", p->pid);

      fprintf (stream, " ");

      if (format > -1)
	{
	  PROCESS *show = format ? p : last;
	  char *temp = "Done";

	  if (JOBSTATE (job_index) == JSTOPPED && !format)
	    temp = "Stopped";

	  if (JOBSTATE (job_index) == JRUNNING)
	    temp = "Running";
	  else
	    {
	      if (WIFSTOPPED (show->status))
		temp = strsignal (WSTOPSIG (show->status));
	      else if (WIFSIGNALED (show->status))
		temp = strsignal (WTERMSIG (show->status));
	      else if (WIFEXITED (show->status))
		{
		  int exit_status;

		  temp = retcode_name_buffer;
		  exit_status = WEXITSTATUS (show->status);

		  if (!exit_status)
		    strcpy (temp, "Done");
		  else if (posixly_correct)
		    sprintf (temp, "Done(%d)", exit_status);
		  else
		    sprintf (temp, "Exit %d", exit_status);
		}
	      else
		temp = "Unknown status";
	    }

	  if (p != first)
	    {
	      if (format)
		{
		  if (show->running == first->running &&
		      WSTATUS (show->status) == WSTATUS (first->status))
		    temp = "";
		}
	      else
		temp = (char *)NULL;
	    }

	  if (temp)
	    {
	      int templ = strlen (temp);
	      fprintf (stream, "%s", temp);

	      if (templ)
		name_padding = LONGEST_SIGNAL_DESC - templ;
	      else
		name_padding = LONGEST_SIGNAL_DESC - 2; /* strlen ("| ") */

	      fprintf (stream, "%*s", name_padding, "");

	      if ((WIFSTOPPED (show->status) == 0) && (WIFCORED (show->status)))
		fprintf (stream, "(core dumped) ");
	    }
	}

      if (p != first && format)
	fprintf (stream, "| ");

      if (p->command)
	fprintf (stream, "%s", p->command);

      if (p == last) 
	{
	  char *wd = current_working_directory ();

	  if (JOBSTATE (job_index) == JRUNNING &&
	      !(jobs[job_index]->flags & J_FOREGROUND))
	    fprintf (stream, " &");

	  if (strcmp (wd, jobs[job_index]->wd) != 0)
	    fprintf (stream,
		     "  (wd: %s)", polite_directory_format (jobs[job_index]->wd));
	}

      if (format || (p == last))
	fprintf (stream, "\r\n");

      if (p == last)
	break;
      p = p->next;
    }

  fflush (stream);
  UNBLOCK_CHILD (oset);
}

int
list_one_job (job, format, ignore, job_index)
     JOB *job;
     int format, ignore, job_index;
{
  pretty_print_job (job_index, format, stdout);
  return (0);
}

/* List jobs.  If FORMAT is non-zero, then the long form of the information
   is printed, else just a short version. */
void
list_jobs (format)
     int format;
{
  cleanup_dead_jobs ();
  map_over_jobs (list_one_job, format, (int)IGNORE_ARG);
}

/* Fork, handling errors.  Returns the pid of the newly made child, or 0.
   COMMAND is just for remembering the name of the command; we don't do
   anything else with it.  ASYNC_P says what to do with the tty.  If
   non-zero, then don't give it away. */
pid_t
make_child (command, async_p)
     char *command;
     int async_p;
{
  sigset_t set, oset;
  pid_t pid;

  sigemptyset (&set);
  sigaddset (&set, SIGCHLD);
  sigaddset (&set, SIGINT);
  sigemptyset (&oset);
  sigprocmask (SIG_BLOCK, &set, &oset);

  making_children ();

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
  if ((pid = fork ()) < 0)
    {
      internal_error ("fork: %s", strerror (errno));

      /* Kill all of the processes in the current pipeline. */
      terminate_current_pipeline ();

      /* Discard the current pipeline, if any. */
      if (the_pipeline)
	kill_current_pipeline ();

      throw_to_top_level ();	/* Reset signals, etc. */
    }

  if (pid == 0)
    {
      /* In the child.  Give this child the right process group, set the
	 signals to the default state for a new process. */
      pid_t mine = getpid ();

#if defined (BUFFERED_INPUT)
      /* Close default_buffered_input if it's > 0.  We don't close it if it's
	 0 because that's the file descriptor used when redirecting input,
	 and it's wrong to close the file in that case. */
      if (default_buffered_input > 0)
	{
	  close_buffered_fd (default_buffered_input);
	  default_buffered_input = bash_input.location.buffered_fd = -1;
	}
#endif /* BUFFERED_INPUT */

      /* Restore top-level signal mask. */
      sigprocmask (SIG_SETMASK, &top_level_mask, (sigset_t *)NULL);

      if (job_control)
	{
	  /* All processes in this pipeline belong in the same
	     process group. */

	  if (!pipeline_pgrp)	/* Then this is the first child. */
	    pipeline_pgrp = mine;

	  /* Check for running command in backquotes. */
	  if (pipeline_pgrp == shell_pgrp)
	    {
	      set_signal_handler (SIGTSTP, SIG_IGN);
	      set_signal_handler (SIGTTOU, SIG_IGN);
	      set_signal_handler (SIGTTIN, SIG_IGN);
	    }
	  else
	    {
	      set_signal_handler (SIGTSTP, SIG_DFL);
	      set_signal_handler (SIGTTOU, SIG_DFL);
	      set_signal_handler (SIGTTIN, SIG_DFL);
	    }

	  /* Set the process group before trying to mess with the terminal's
	     process group.  This is mandated by POSIX. */
	  /* This is in accordance with the Posix 1003.1 standard,
	     section B.7.2.4, which says that trying to set the terminal
	     process group with tcsetpgrp() to an unused pgrp value (like
	     this would have for the first child) is an error.  Section
	     B.4.3.3, p. 237 also covers this, in the context of job control
	     shells. */
	  if (setpgid (mine, pipeline_pgrp) < 0)
	    internal_error ("child setpgid (%d to %d) error %d: %s\n",
			    mine, pipeline_pgrp, errno, strerror (errno));
#if defined (PGRP_PIPE)
	  if (pipeline_pgrp == mine)
	    {
#endif
	      if (!async_p)
		give_terminal_to (pipeline_pgrp);

#if defined (PGRP_PIPE)
	      pipe_read (pgrp_pipe);
	    }
#endif
	}
      else			/* Without job control... */
	{
	  if (!pipeline_pgrp)
	    pipeline_pgrp = shell_pgrp;

	  /* If these signals are set to SIG_DFL, we encounter the curious
	     situation of an interactive ^Z to a running process *working*
	     and stopping the process, but being unable to do anything with
	     that process to change its state.  On the other hand, if they
	     are set to SIG_IGN, jobs started from scripts do not stop when
	     the shell running the script gets a SIGTSTP and stops. */

	  set_signal_handler (SIGTSTP, SIG_DFL);
	  set_signal_handler (SIGTTOU, SIG_DFL);
	  set_signal_handler (SIGTTIN, SIG_DFL);
	}

#if defined (PGRP_PIPE)
      /* Release the process group pipe, since our call to setpgid ()
	 is done.  The last call to pipe_close is done in stop_pipeline. */
      pipe_close (pgrp_pipe);
#endif /* PGRP_PIPE */

      if (async_p)
	last_asynchronous_pid = getpid ();
    }
  else
    {
      /* In the parent.  Remember the pid of the child just created
	 as the proper pgrp if this is the first child. */

      if (job_control)
	{
	  if (!pipeline_pgrp)
	    {
	      pipeline_pgrp = pid;
	      /* Don't twiddle terminal pgrps in the parent!  This is the bug,
		 not the good thing of twiddling them in the child! */
	      /* give_terminal_to (pipeline_pgrp); */
	    }
	  /* This is done on the recommendation of the Rationale section of
	     the POSIX 1003.1 standard, where it discusses job control and
	     shells.  It is done to avoid possible race conditions. (Ref.
	     1003.1 Rationale, section B.4.3.3, page 236). */
	  setpgid (pid, pipeline_pgrp);
	}
      else
	{
	  if (!pipeline_pgrp)
	    pipeline_pgrp = shell_pgrp;
	}

      /* Place all processes into the jobs array regardless of the
	 state of job_control. */
      add_process (command, pid);

      if (async_p)
	last_asynchronous_pid = pid;

      last_made_pid = pid;

      /* Unblock SIGINT and SIGCHLD. */
      sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
    }

  return (pid);
}

/* When we end a job abnormally, or if we stop a job, we set the tty to the
   state kept in here.  When a job ends normally, we set the state in here
   to the state of the tty. */

#if defined (NEW_TTY_DRIVER)
static struct sgttyb shell_tty_info;
static struct tchars shell_tchars;
static struct ltchars shell_ltchars;
#endif /* NEW_TTY_DRIVER */

#if defined (TERMIO_TTY_DRIVER)
static struct termio shell_tty_info;
#endif /* TERMIO_TTY_DRIVER */

#if defined (TERMIOS_TTY_DRIVER)
static struct termios shell_tty_info;
#endif /* TERMIOS_TTY_DRIVER */

#if defined (NEW_TTY_DRIVER) && defined (DRAIN_OUTPUT)
/* Since the BSD tty driver does not allow us to change the tty modes
   while simultaneously waiting for output to drain and preserving
   typeahead, we have to drain the output ourselves before calling
   ioctl.  We cheat by finding the length of the output queue, and
   using select to wait for an appropriate length of time.  This is
   a hack, and should be labeled as such (it's a hastily-adapted
   mutation of a `usleep' implementation).  It's only reason for
   existing is the flaw in the BSD tty driver. */

static int ttspeeds[] =
{
  0, 50, 75, 110, 134, 150, 200, 300, 600, 1200,
  1800, 2400, 4800, 9600, 19200, 38400
};

static void
draino (fd, ospeed)
     int fd, ospeed;
{
  register int delay = ttspeeds[ospeed];
  int n;

  if (!delay)
    return;

  while ((ioctl (fd, TIOCOUTQ, &n) == 0) && n)
    {
      if (n > (delay / 100))
	{
	  struct timeval tv;

	  n *= 10;		/* 2 bits more for conservativeness. */
	  tv.tv_sec = n / delay;
	  tv.tv_usec = ((n % delay) * 1000000) / delay;
	  select (fd, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv);
	}
      else
	break;
    }
}
#endif /* NEW_TTY_DRIVER && DRAIN_OUTPUT */

/* Return the fd from which we are actually getting input. */
#define input_tty() (shell_tty != -1) ? shell_tty : fileno (stderr)

/* Fill the contents of shell_tty_info with the current tty info. */
get_tty_state ()
{
  int tty = input_tty ();

  if (tty != -1)
    {
#if defined (NEW_TTY_DRIVER)
      ioctl (tty, TIOCGETP, &shell_tty_info);
      ioctl (tty, TIOCGETC, &shell_tchars);
      ioctl (tty, TIOCGLTC, &shell_ltchars);
#endif /* NEW_TTY_DRIVER */

#if defined (TERMIO_TTY_DRIVER)
      ioctl (tty, TCGETA, &shell_tty_info);
#endif /* TERMIO_TTY_DRIVER */

#if defined (TERMIOS_TTY_DRIVER)
      if (tcgetattr (tty, &shell_tty_info) < 0)
	{
#if 0
	  /* Only print an error message if we're really interactive at
	     this time. */
	  if (interactive)
	    internal_error ("[%d: %d] tcgetattr: %s",
			    getpid (), shell_level, strerror (errno));
#endif
	  return -1;
	}
#endif /* TERMIOS_TTY_DRIVER */
    }
  return 0;
}

/* Make the current tty use the state in shell_tty_info. */
set_tty_state ()
{
  int tty = input_tty ();

  if (tty != -1)
    {
#if defined (NEW_TTY_DRIVER)
#  if defined (DRAIN_OUTPUT)
      draino (tty, shell_tty_info.sg_ospeed);
#  endif /* DRAIN_OUTPUT */
      ioctl (tty, TIOCSETN, &shell_tty_info);
      ioctl (tty, TIOCSETC, &shell_tchars);
      ioctl (tty, TIOCSLTC, &shell_ltchars);
#endif /* NEW_TTY_DRIVER */

#if defined (TERMIO_TTY_DRIVER)
      ioctl (tty, TCSETAW, &shell_tty_info);
#endif /* TERMIO_TTY_DRIVER */

#if defined (TERMIOS_TTY_DRIVER)
      if (tcsetattr (tty, TCSADRAIN, &shell_tty_info) < 0)
	{
	  /* Only print an error message if we're really interactive at
	     this time. */
	  if (interactive)
	    internal_error ("[%d: %d] tcsetattr: %s",
			    getpid (), shell_level, strerror (errno));
	  return -1;
	}
#endif /* TERMIOS_TTY_DRIVER */
    }
  return 0;
}

/* Given an index into the jobs array JOB, return the pid of the last
   process in that job's pipeline.  This is the one whose exit status
   counts. */
static pid_t
last_pid (job)
     int job;
{
  register PROCESS *p;
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);

  p = jobs[job]->pipe;
  while (p->next != jobs[job]->pipe)
    p = p->next;

  UNBLOCK_CHILD (oset);
  return (p->pid);
}

/* Wait for a particular child of the shell to finish executing.
   This low-level function prints an error message if PID is not
   a child of this shell.  It returns -1 if it fails, or 0 if not. */
int
wait_for_single_pid (pid)
     pid_t pid;
{
  register PROCESS *child;

  {
    sigset_t set, oset;

    BLOCK_CHILD (set, oset);
    child = find_pipeline (pid);
    UNBLOCK_CHILD (oset);
  }

  if (!child)
    {
      report_error ("wait: pid %d is not a child of this shell", pid);
      return (127);
    }

  return (wait_for (pid));
}

/* Wait for all of the backgrounds of this shell to finish. */
void
wait_for_background_pids ()
{
  while (1)
    {
      register int i, count = 0;
      sigset_t set, oset;

      BLOCK_CHILD (set, oset);

      for (i = 0; i < job_slots; i++)
	if (jobs[i] && (JOBSTATE (i) == JRUNNING) &&
	    (jobs[i]->flags & J_FOREGROUND) == 0)
	  {
	    count++;
	    break;
	  }

      if (!count)
	{
	  UNBLOCK_CHILD (oset);
	  break;
	}

      for (i = 0; i < job_slots; i++)
	if (jobs[i] && (JOBSTATE (i) == JRUNNING) &&
	    (jobs[i]->flags & J_FOREGROUND) == 0)
	  {
	    pid_t pid = last_pid (i);
	    UNBLOCK_CHILD (oset);
	    QUIT;
	    wait_for_single_pid (pid);
	    break;
	  }
    }
}

/* Make OLD_SIGINT_HANDLER the SIGINT signal handler. */
#define INVALID_SIGNAL_HANDLER (SigHandler *)wait_for_background_pids
static SigHandler *old_sigint_handler = INVALID_SIGNAL_HANDLER;

static void
restore_sigint_handler ()
{
  if (old_sigint_handler != INVALID_SIGNAL_HANDLER)
    {
      set_signal_handler (SIGINT, old_sigint_handler);
      old_sigint_handler = INVALID_SIGNAL_HANDLER;
    }
}

static int wait_sigint_received = 0;

/* Handle SIGINT while we are waiting for children in a script to exit.
   The `wait' builtin should be interruptible, but all others should be
   effectively ignored (i.e. not cause the shell to exit). */
static sighandler
wait_sigint_handler (sig)
     int sig;
{
  if (interrupt_immediately ||
      (this_shell_builtin && this_shell_builtin == wait_builtin))
    {
      last_command_exit_value = EXECUTION_FAILURE;
      restore_sigint_handler ();
      interrupt_state++;
      QUIT;
    }

  wait_sigint_received = 1;	/* XXX - should this be interrupt_state? */
  /* Otherwise effectively ignore the SIGINT and allow the running job to
     be killed. */
#if !defined (VOID_SIGHANDLER)
  return (0);
#endif /* !VOID_SIGHANDLER */
}

static int
process_exit_status (status)
     WAIT status;
{
  if (WIFSIGNALED (status))
    return (128 + WTERMSIG (status));
  else if (!WIFSTOPPED (status))
    return (WEXITSTATUS (status));
  else
    return (EXECUTION_SUCCESS);
}

/* Wait for pid (one of our children) to terminate, then
   return the termination state. */
int
wait_for (pid)
     pid_t pid;
{
  int job, termination_state;
  register PROCESS *child;
  sigset_t set, oset;

  /* In the case that this code is interrupted, and we longjmp () out of it,
     we are relying on the code in throw_to_top_level () to restore the
     top-level signal mask. */
  BLOCK_CHILD (set, oset);

  /* Ignore interrupts while waiting for a job run without job control
     to finish.  We don't want the shell to exit if an interrupt is
     received, only if one of the jobs run is killed via SIGINT.  If
     job control is not set, the job will be run in the same pgrp as 
     the shell, and the shell will see any signals the job gets. */

  /* This is possibly a race condition -- should it go in stop_pipeline? */
  wait_sigint_received = 0;
  if (!job_control)
    old_sigint_handler = set_signal_handler (SIGINT, wait_sigint_handler);

  termination_state = last_command_exit_value;

  /* If we say wait_for (), then we have a record of this child somewhere.
     If this child and all of its peers are not running, then don't
     sigpause (), since there is no need to. */
 wait_loop:

  /* If the shell is interactive, and job control is disabled, see if the
     foreground process has died due to SIGINT and jump out of the wait
     loop if it has.  waitchld has already restored the old SIGINT
     signal handler. */
  if (interactive && !job_control)
    QUIT;

  child = find_pipeline (pid);

  if (!child)
    {
      give_terminal_to (shell_pgrp);
      UNBLOCK_CHILD (oset);
      programming_error ("wait_for: No record of pid %d", pid);
    }

  /* If this child is part of a job, then we are really waiting for the
     job to finish.  Otherwise, we are waiting for the child to finish. */

  job = find_job (pid);

  if (job != NO_JOB)
    {
      register int job_state = 0, any_stopped = 0;
      register PROCESS *p = jobs[job]->pipe;

      do
	{
	  job_state |= p->running;
	  if (!p->running)
	    any_stopped |= WIFSTOPPED (p->status);
	  p = p->next;
	}
      while (p != jobs[job]->pipe);

      if (job_state == 0)
	{
	  if (any_stopped)
	    jobs[job]->state = JSTOPPED;
	  else
	    jobs[job]->state = JDEAD;
	}
    }

  if (child->running || ((job != NO_JOB) && (JOBSTATE (job) == JRUNNING)))
    {
#if defined (WAITPID_BROKEN)	/* SCOv4 */
      sigset_t suspend_set;
      sigemptyset (&suspend_set);
      sigsuspend (&suspend_set);
#else /* !WAITPID_BROKEN */
#  if defined (MUST_UNBLOCK_CHILD)	/* SCO */
      struct sigaction act, oact;
      sigset_t nullset, chldset;

      sigemptyset (&nullset);
      sigemptyset (&chldset);
      sigprocmask (SIG_SETMASK, &nullset, &chldset);
      act.sa_handler = SIG_DFL;
      sigemptyset (&act.sa_mask);
      sigemptyset (&oact.sa_mask);
      act.sa_flags = 0;
      sigaction (SIGCHLD, &act, &oact);
#  endif
      waiting_for_job = 1;
      waitchld (0);
      waiting_for_job = 0;
#  if defined (MUST_UNBLOCK_CHILD)
      sigaction (SIGCHLD, &oact, (struct sigaction *)NULL);
      sigprocmask (SIG_SETMASK, &chldset, (sigset_t *)NULL);
#  endif
#endif /* !WAITPID_BROKEN */
      goto wait_loop;
    }

  /* The exit state of the command is either the termination state of the
     child, or the termination state of the job.  If a job, the status
     of the last child in the pipeline is the significant one. */

  if (job != NO_JOB)
    {
      register PROCESS *p = jobs[job]->pipe;

      while (p->next != jobs[job]->pipe)
	p = p->next;
      termination_state = process_exit_status (p->status);
    }
  else
    termination_state = process_exit_status (child->status);

  if (job == NO_JOB || (jobs[job]->flags & J_JOBCONTROL))
    give_terminal_to (shell_pgrp);

  /* If the command did not exit cleanly, or the job is just
     being stopped, then reset the tty state back to what it
     was before this command.  Reset the tty state and notify
     the user of the job termination only if the shell is
     interactive.  Clean up any dead jobs in either case. */
  if (job != NO_JOB)
    {
      if (interactive_shell && !subshell_environment)
	{
	  if (WIFSIGNALED (child->status) || WIFSTOPPED (child->status))
	    set_tty_state ();
	  else
	    get_tty_state ();

	  /* If job control is enabled, the job was started with job
	     control, the job was the foreground job, and it was killed
	     by SIGINT, then print a newline to compensate for the kernel
	     printing the ^C without a trailing newline. */
	  if (job_control && (jobs[job]->flags & J_JOBCONTROL) &&
		(jobs[job]->flags & J_FOREGROUND) && 
		WIFSIGNALED (child->status) &&
		WTERMSIG (child->status) == SIGINT)
	    {
	      /* If SIGINT is not trapped, set the interrupt state if in a
	         loop so the loop will be broken.  If not in a loop, print
	         the newline that the kernel does not. */
	      if (signal_is_trapped (SIGINT) == 0)
		{
		  if (loop_level)
		    interrupt_state++;
		  else
		    {
		      putchar ('\n');
		      fflush (stdout);
		    }
		}
	    }

	  notify_and_cleanup ();
	}
      else
	{
	  /* If this job is dead, and the shell is not interactive, make
	     sure we turn on the notify bit so we don't get an unwanted
	     message about the job's termination, and so delete_job really
	     clears the slot in the jobs table. */
	  if (JOBSTATE(job) == JDEAD)
	    jobs[job]->flags |= J_NOTIFIED;
	  cleanup_dead_jobs ();
	}
    }
    
  UNBLOCK_CHILD (oset);

  /* Restore the original SIGINT signal handler before we return. */
  restore_sigint_handler ();

  return (termination_state);
}

/* Wait for the last process in the pipeline for JOB. */
int
wait_for_job (job)
     int job;
{
  pid_t pid = last_pid (job);
  return (wait_for (pid));
}

/* Print info about dead jobs, and then delete them from the list
   of known jobs.  This does not actually delete jobs when the
   shell is not interactive, because the dead jobs are not marked
   as notified. */
void
notify_and_cleanup ()
{
  if (freeze_jobs_list)
    return;

  if (interactive)
    notify_of_job_status ();

  cleanup_dead_jobs ();
}

/* Make dead jobs disappear from the jobs array without notification.
   This is used when the shell is not interactive. */
void
reap_dead_jobs ()
{
  mark_dead_jobs_as_notified ();
  cleanup_dead_jobs ();
}

/* Return the next closest (chronologically) job to JOB which is in
   STATE.  STATE can be JSTOPPED, JRUNNING.  NO_JOB is returned if
   there is no next recent job. */
static int
most_recent_job_in_state (job, state)
     int job;
     JOB_STATE state;
{
  register int i, result;
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);
  result = NO_JOB;

  for (i = job - 1; i >= 0; i--)
    {
      if (jobs[i])
	{
	  if (JOBSTATE (i) == state)
	    {
	      result = i;
	      break;
	    }
	}
    }
  UNBLOCK_CHILD (oset);
  return (result);
}

/* Return the newest *stopped* job older than JOB, or NO_JOB if not
   found. */
static int
last_stopped_job (job)
     int job;
{
  return (most_recent_job_in_state (job, JSTOPPED));
}

/* Return the newest *running* job older than JOB, or NO_JOB if not
   found. */
static int
last_running_job (job)
     int job;
{
  return (most_recent_job_in_state (job, JRUNNING));
}

/* Make JOB be the current job, and make previous be useful.  Must be
   called with SIGCHLD blocked. */
static void
set_current_job (job)
     int job;
{
  int candidate = NO_JOB;

  if (current_job != job)
    {
      previous_job = current_job;
      current_job = job;
    }

  /* First choice for previous_job is the old current_job. */
  if (previous_job != current_job &&
      previous_job != NO_JOB &&
      jobs[previous_job] &&
      JOBSTATE (previous_job) == JSTOPPED)
    return;

  /* Second choice:  Newest stopped job that is older than
     the current job. */
  if (JOBSTATE (current_job) == JSTOPPED)
    {
      candidate = last_stopped_job (current_job);

      if (candidate != NO_JOB)
	{
	  previous_job = candidate;
	  return;
	}
    }

  /* If we get here, there is either only one stopped job, in which case it is
     the current job and the previous job should be set to the newest running
     job, or there are only running jobs and the previous job should be set to
     the newest running job older than the current job.  We decide on which
     alternative to use based on whether or not JOBSTATE(current_job) is
     JSTOPPED. */

  if (JOBSTATE (current_job) == JRUNNING)
    candidate = last_running_job (current_job);
  else
    candidate = last_running_job (job_slots);

  if (candidate != NO_JOB)
    {
      previous_job = candidate;
      return;
    }

  /* There is only a single job, and it is both `+' and `-'. */
  previous_job = current_job;
}

/* Make current_job be something useful, if it isn't already. */

/* Here's the deal:  The newest non-running job should be `+', and the
   next-newest non-running job should be `-'.  If there is only a single
   stopped job, the previous_job is the newest non-running job.  If there
   are only running jobs, the newest running job is `+' and the
   next-newest running job is `-'.  Must be called with SIGCHLD blocked. */
static void
reset_current ()
{
  int candidate = NO_JOB;

  if (current_job != NO_JOB &&
      job_slots && jobs[current_job] &&
      JOBSTATE (current_job) == JSTOPPED)
    {
      candidate = current_job;
    }
  else
    {
      /* First choice: the previous job! */
      if (previous_job != NO_JOB && jobs[previous_job] &&
	  JOBSTATE (previous_job) == JSTOPPED)
	candidate = previous_job;

      /* Second choice: the most recently stopped job. */
      if (candidate == NO_JOB)
	candidate = last_stopped_job (job_slots);

      if (candidate == NO_JOB)
	{
	  /* Third choice: the newest running job. */
	  candidate = last_running_job (job_slots);
	}
    }

  /* If we found a job to use, then use it.  Otherwise, there
     are no jobs period. */
  if (candidate != NO_JOB)
    set_current_job (candidate);
  else
    current_job = previous_job = NO_JOB;
}

/* Start a job.  FOREGROUND if non-zero says to do that.  Otherwise,
   start the job in the background.  JOB is a zero-based index into
   JOBS.  Returns -1 if it is unable to start a job, and the return
   status of the job otherwise. */
int
start_job (job, foreground)
     int job, foreground;
{
  register PROCESS *p;
  int already_running;
  sigset_t set, oset;
  char *wd;
#if defined (NEW_TTY_DRIVER)
  static struct sgttyb save_stty;
#endif

#if defined (TERMIO_TTY_DRIVER)
  static struct termio save_stty;
#endif

#if defined (TERMIOS_TTY_DRIVER)
  static struct termios save_stty;
#endif

  BLOCK_CHILD (set, oset);
  already_running = (JOBSTATE (job) == JRUNNING);

  if (JOBSTATE (job) == JDEAD)
    {
      report_error ("%s: job has terminated", this_command_name);
      UNBLOCK_CHILD (oset);
      return (-1);
    }

  if (!foreground && already_running)
    {
      report_error ("%s: bg background job?", this_command_name);
      UNBLOCK_CHILD (oset);
      return (-1);
    }

  wd = current_working_directory ();

  /* You don't know about the state of this job.  Do you? */
  jobs[job]->flags &= ~J_NOTIFIED;

  if (foreground)
    {
      set_current_job (job);
      jobs[job]->flags |= J_FOREGROUND;
    }

  /* Tell the outside world what we're doing. */
  p = jobs[job]->pipe;

  if (!foreground)
    fprintf (stderr, "[%d]%c ", job + 1,
	   (job == current_job) ? '+': ((job == previous_job) ? '-' : ' '));

  do
    {
      fprintf (stderr, "%s%s",
	       p->command ? p->command : "",
	       p->next != jobs[job]->pipe? " | " : "");
      p = p->next;
    }
  while (p != jobs[job]->pipe);

  if (!foreground)
    fprintf (stderr, " &");

  if (strcmp (wd, jobs[job]->wd) != 0)
    fprintf (stderr, "	(wd: %s)", polite_directory_format (jobs[job]->wd));

  fprintf (stderr, "\n");

  /* Run the job. */
  if (!already_running)
    {
      /* Each member of the pipeline is now running. */
      p = jobs[job]->pipe;

      do
	{
	  if (WIFSTOPPED (p->status))
	    p->running = 1;
	  p = p->next;
	}
      while (p != jobs[job]->pipe);

      /* This means that the job is running. */
      JOBSTATE (job) = JRUNNING;
    }

  /* Save the tty settings before we start the job in the foreground. */
  if (foreground)
    {
      get_tty_state ();
      save_stty = shell_tty_info;
    }

  /* Give the terminal to this job. */
  if (foreground)
    {
      if (jobs[job]->flags & J_JOBCONTROL)
	give_terminal_to (jobs[job]->pgrp);
    }
  else
    jobs[job]->flags &= ~J_FOREGROUND;

  /* If the job is already running, then don't bother jump-starting it. */
  if (!already_running)
    {
      jobs[job]->flags |= J_NOTIFIED;
      killpg (jobs[job]->pgrp, SIGCONT);
    }

  UNBLOCK_CHILD (oset);

  if (foreground)
    {
      pid_t pid = last_pid (job);
      int s = wait_for (pid);

      shell_tty_info = save_stty;
      set_tty_state ();
      return (s);
    }
  else
    {
      BLOCK_CHILD (set, oset);
      reset_current ();
      UNBLOCK_CHILD (oset);
      return (0);
    }
}

/* Give PID SIGNAL.  This determines what job the pid belongs to (if any).
   If PID does belong to a job, and the job is stopped, then CONTinue the
   job after giving it SIGNAL.  Returns -1 on failure.  If GROUP is non-null,
   then kill the process group associated with PID. */
int
kill_pid (pid, sig, group)
     pid_t pid;
     int sig, group;
{
  register PROCESS *p;
  int job, result = EXECUTION_SUCCESS;
  sigset_t set, oset;

  BLOCK_CHILD (set, oset);
  p = find_pipeline (pid);
  job = find_job (pid);

  if (group)
    {
      if (job != NO_JOB)
	{
	  jobs[job]->flags &= ~J_NOTIFIED;

	  /* Kill process in backquotes or one started without job control? */
	  if (jobs[job]->pgrp == shell_pgrp)
	    {
	      p = jobs[job]->pipe;

	      do
		{
		  kill (p->pid, sig);
		  if (p->running == 0 && (sig == SIGTERM || sig == SIGHUP))
		    kill (p->pid, SIGCONT);
		  p = p->next;
		}
	      while (p != jobs[job]->pipe);
	    }
	  else
	    {
	      result = killpg (jobs[job]->pgrp, sig);
	      if (p && (JOBSTATE (job) == JSTOPPED) &&
		  (sig == SIGTERM || sig == SIGHUP))
		killpg (jobs[job]->pgrp, SIGCONT);
	    }
	}
      else
	result = killpg (pid, sig);
    }
  else
    result = kill (pid, sig);

  UNBLOCK_CHILD (oset);
  return (result);
}

/* Take care of system dependencies that must be handled when waiting for
   children.  The arguments to the WAITPID macro match those to the Posix.1
   waitpid() function. */

#if defined (Ultrix) && defined (mips) && defined (_POSIX_VERSION)
#  define WAITPID(pid, statusp, options) \
	wait3 ((union wait *)statusp, options, (struct rusage *)0)
#else
#  if defined (_POSIX_VERSION)
#    define WAITPID(pid, statusp, options) \
	waitpid ((pid_t)pid, statusp, options)
#  else
#    if defined (hpux)
#      define WAITPID(pid, statusp, options) \
	wait3 (statusp, options, (int *)0)
#    else
#      define WAITPID(pid, statusp, options) \
	wait3 (statusp, options, (struct rusage *)0)
#    endif /* !hpux */
#  endif /* !_POSIX_VERSION */
#endif /* !(Ultrix && mips && _POSIX_VERSION) */

/* If the system needs it, REINSTALL_SIGCHLD_HANDLER will reinstall the
   handler for SIGCHLD. */

#if defined (hpux) && !defined (_POSIX_VERSION)
#  define REINSTALL_SIGCHLD_HANDLER signal (SIGCHLD, flush_child)
#else
#  define REINSTALL_SIGCHLD_HANDLER
#endif /* !hpux || _POSIX_VERSION */

/* Flush_child () flushes at least one of the children that we are waiting for.
   It gets run when we have gotten a SIGCHLD signal, and stops when there
   aren't any children terminating any more.  If SIG is 0, this is to be a
   blocking wait for a single child. */
static sighandler
flush_child (sig)
     int sig;
{
  REINSTALL_SIGCHLD_HANDLER;
  sigchld++;
  if (waiting_for_job == 0)
    waitchld (sig);

#if !defined (VOID_SIGHANDLER)
  return (0);
#endif /* VOID_SIGHANDLER */
}
  
static int
waitchld (s)
     int s;
{
  WAIT status;
  PROCESS *child;
  pid_t pid;
  int call_set_current = 0, last_stopped_job = NO_JOB;
  int children_exited = 0, flag;

  do
    {
      flag = WUNTRACED;
      if (sigchld || s)
	flag |= WNOHANG;
      pid = WAITPID (-1, &status, flag);
      if (sigchld && (flag & WNOHANG))
        sigchld--;

      if (pid > 0)
	{
	  /* Locate our PROCESS for this pid. */
	  child = find_pipeline (pid);

	  /* It is not an error to have a child terminate that we did
	     not have a record of.  This child could have been part of
	     a pipeline in backquote substitution. */
	  if (child)
	    {
	      int job = find_job (pid);

	      while (child->pid != pid)
		child = child->next;

	      /* Remember status, and fact that process is not running. */
	      child->status = status;
	      child->running = 0;

	      if (job != NO_JOB)
		{
		  int job_state = 0;
		  int any_stopped = 0;
		  int any_tstped = 0;

		  child = jobs[job]->pipe;
		  jobs[job]->flags &= ~J_NOTIFIED;

		  /* If all children are not running, but any of them is
		     stopped, then the job is stopped, not dead. */
		  do
		   {
		      job_state |= child->running;
		      if (!child->running)
		        {
			  any_stopped |= WIFSTOPPED (child->status);
			  any_tstped |= interactive && job_control &&
					WIFSTOPPED (child->status) &&
					WSTOPSIG (child->status) == SIGTSTP;
		        }
		      child = child->next;
		    }
		  while (child != jobs[job]->pipe);

		  if (job_state == 0)
		    {
		      if (any_stopped)
			{
			  jobs[job]->state = JSTOPPED;
			  jobs[job]->flags &= ~J_FOREGROUND;
			  call_set_current++;
			  last_stopped_job = job;
			  /* Suspending a job in a loop from the keyboard
			     breaks out of all active loops. */
			  if (any_tstped && loop_level)
			    breaking = loop_level;
			}
		      else
			{
			  jobs[job]->state = JDEAD;

			  if (job == last_stopped_job)
			    last_stopped_job = NO_JOB;

			  /* If the foreground job is killed by SIGINT when
			     job control is not active, we need to perform
			     some special handling. */
			  /* The check of wait_sigint_received is a way to
			     determine if the SIGINT came from the keyboard
			     (in which case the shell has already seen it,
			     and wait_sigint_received is non-zero, because
			     keyboard signals are sent to process groups)
			     or via kill(2) to the foreground process by
			     another process (or itself).  If the shell did
			     receive the SIGINT, it needs to perform normal
			     SIGINT processing. */
			  if ((WTERMSIG (jobs[job]->pipe->status) == SIGINT) &&
			      (jobs[job]->flags & J_FOREGROUND) &&
			      (jobs[job]->flags & J_JOBCONTROL) == 0 &&
			      wait_sigint_received)
			    {
			      wait_sigint_received = 0;

			      /* If SIGINT is trapped, set the exit status so
				 that the trap handler can see it. */
			      if (signal_is_trapped (SIGINT))
				last_command_exit_value = process_exit_status
				  (jobs[job]->pipe->status);

			      /* If the signal is trapped, let the trap handler
				 get it no matter what and simply return if
				 the trap handler returns.
			         maybe_call_trap_handler may cause dead jobs
				 to be removed from the job table because of
				 a call to execute_command.  Watch out for
				 this. */
			      if (maybe_call_trap_handler (SIGINT) == 0 &&
			          old_sigint_handler != INVALID_SIGNAL_HANDLER)
				{
				  /* wait_sigint_handler () has already
				     seen SIGINT and allowed the wait
				     builtin to jump out.  We need to
				     call the original SIGINT handler. */
				  SigHandler *temp_handler;
				  temp_handler = old_sigint_handler;
				  restore_sigint_handler ();
				  if (temp_handler != SIG_IGN)
				    (*temp_handler) (SIGINT);
				}
			    }
			}
		    }
		}
	    }
	  /* If we have caught a child, and a trap was set for SIGCHLD, then
	     bump up the count of the number of children that have exited,
	     so we know how many times to call it. */
	  children_exited++;
	}
    }
  while ((s || sigchld) && pid > (pid_t)0);

  /* If a job was running and became stopped, then set the current
     job.  Otherwise, don't change a thing. */
  if (call_set_current)
    if (last_stopped_job != NO_JOB)
      set_current_job (last_stopped_job);
    else
      reset_current ();

  /* Call a SIGCHLD trap handler for each child that exits, if one is set. */
  if (job_control && signal_is_trapped (SIGCHLD) &&
      trap_list[SIGCHLD] != (char *)IGNORE_SIG)
    {
      char *trap_command;

      /* Turn off the trap list during the call to parse_and_execute ()
	 to avoid potentially infinite recursive calls.  Preserve the
	 values of last_command_exit_value, last_made_pid, and the_pipeline
	 around the execution of the trap commands. */
      trap_command = savestring (trap_list[SIGCHLD]);

      begin_unwind_frame ("SIGCHLD trap");
      unwind_protect_int (last_command_exit_value);
      unwind_protect_int (last_made_pid);
      unwind_protect_int (interrupt_immediately);
      unwind_protect_int (freeze_jobs_list);
      unwind_protect_pointer (the_pipeline);

      /* We have to add the commands this way because they will be run
	 in reverse order of adding.  We don't want maybe_set_sigchld_trap ()
	 to reference freed memory. */
      add_unwind_protect ((Function *)xfree, trap_command);
      add_unwind_protect ((Function *)maybe_set_sigchld_trap, trap_command);

      the_pipeline = (PROCESS *)NULL;
      restore_default_signal (SIGCHLD);
      freeze_jobs_list = 1;
      while (children_exited--)
	{
	  interrupt_immediately = 1;
	  parse_and_execute (savestring (trap_command), "trap", -1);
	}

      run_unwind_frame ("SIGCHLD trap");
    }

  /* We have successfully recorded the useful information about this process
     that has just changed state.  If we notify asynchronously, and the job
     that this process belongs to is no longer running, then notify the user
     of that fact now. */
  if (asynchronous_notification && interactive)
    notify_of_job_status ();

}

/* Function to call when you want to notify people of changes
   in job status.  This prints out all jobs which are pending
   notification to stderr, and marks those printed as already
   notified, thus making them candidates for cleanup. */
static void
notify_of_job_status ()
{
  register int job, termsig;
  char *dir;
  sigset_t set, oset;

  sigemptyset (&set);
  sigaddset (&set, SIGCHLD);
  sigaddset (&set, SIGTTOU);
  sigemptyset (&oset);
  sigprocmask (SIG_BLOCK, &set, &oset);

  dir = (char *)NULL;

  for (job = 0; job < job_slots; job++)
    {
      if (jobs[job] && (jobs[job]->flags & J_NOTIFIED) == 0)
	{
	  WAIT s;

	  s = jobs[job]->pipe->status;
	  termsig = WTERMSIG (s);

	  /* If job control is disabled, don't print the status messages.
	     Mark dead jobs as notified so that they get cleaned up. */
	  if (!job_control)
	    {
	      if (JOBSTATE (job) == JDEAD)
		jobs[job]->flags |= J_NOTIFIED;
	      continue;
	    }

	  switch (JOBSTATE (job))
	    {
	      /* Print info on jobs that are running in the background,
		 and on foreground jobs that were killed by anything
		 except SIGINT. */

	    case JDEAD:

	      if (jobs[job]->flags & J_FOREGROUND)
		{
		  if (termsig && WIFSIGNALED (s) && termsig != SIGINT)
		    {
		      fprintf (stderr, "%s", strsignal (termsig));

		      if (WIFCORED (s))
			fprintf (stderr, " (core dumped)");

		      fprintf (stderr, "\n");
		    }
		}
	      else
		{
		  if (!dir)
		    dir = current_working_directory ();
		  pretty_print_job (job, 0, stderr);
		  if (dir && strcmp (dir, jobs[job]->wd) != 0)
		    fprintf (stderr,
			     "(wd now: %s)\n", polite_directory_format (dir));
		}

	      jobs[job]->flags |= J_NOTIFIED;
	      break;

	    case JSTOPPED:
	      fprintf (stderr, "\n");
	      if (!dir)
		dir = current_working_directory ();
	      pretty_print_job (job, 0, stderr);
	      if (dir && (strcmp (dir, jobs[job]->wd) != 0))
		fprintf (stderr,
			 "(wd now: %s)\n", polite_directory_format (dir));
	      jobs[job]->flags |= J_NOTIFIED;
	      break;

	    case JRUNNING:
	    case JMIXED:
	      break;

	    default:
	      programming_error ("notify_of_job_status");
	    }
	}
    }
  sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
}

/* getpgrp () varies between systems.  Even systems that claim to be
   Posix.1 compatible lie sometimes (Ultrix, SunOS4, apollo). */
#if defined (_POSIX_VERSION) && !defined (BSD_GETPGRP)
#  define getpgid(p) getpgrp ()
#else
#  define getpgid(p) getpgrp (p)
#endif /* !_POSIX_VERSION || BSD_GETPGRP */

/* Initialize the job control mechanism, and set up the tty stuff. */
initialize_jobs ()
{
  shell_pgrp = getpgid (0);

  if (shell_pgrp == -1)
    {
      internal_error ("initialize_jobs: getpgrp failed: %s", strerror (errno));
      exit (1);
    }

  /* We can only have job control if we are interactive?
     I guess that makes sense. */

  if (!interactive)
    {
      job_control = 0;
      original_pgrp = NO_PID;
    }
  else
    {
      /* Make sure that we are using the new line discipline. */

      /* Get our controlling terminal.  If job_control is set, or
	 interactive is set, then this is an interactive shell no
	 matter what. */
      shell_tty = dup (fileno (stderr));

      /* Find the highest unused file descriptor we can. */
      {
	int ignore, nds = getdtablesize ();

	if (nds <= 0)
	  nds = 20;
	else if (nds > 256)
	  nds = 256;

	while (--nds > 3)
	  {
	    if (fcntl (nds, F_GETFD, &ignore) == -1)
	      break;
	  }

	if (nds && shell_tty != nds && (dup2 (shell_tty, nds) != -1))
	  {
	    if (shell_tty != fileno (stderr))
	      close (shell_tty);
	    shell_tty = nds;
	  }
      }

#if defined (RLOGIN_PGRP_BUG)
      /* Compensate for a bug in systems that compiled the BSD
	 /usr/etc/rlogind with DEBUG defined, like NeXT and Alliant. */
      if (shell_pgrp == 0)
	{
	  shell_pgrp = getpid ();
	  setpgid (0, shell_pgrp);
	  tcsetpgrp (shell_tty, shell_pgrp);
	}
#endif /* RLOGIN_PGRP_BUG */

      while ((terminal_pgrp = tcgetpgrp (shell_tty)) != -1)
	{
	  if (shell_pgrp != terminal_pgrp)
	    {
	      SigHandler *old_ttin = (SigHandler *)set_signal_handler (SIGTTIN, SIG_DFL);
	      kill (0, SIGTTIN);
	      set_signal_handler (SIGTTIN, old_ttin);
	      continue;
	    }
	  break;
	}

      if (set_new_line_discipline (shell_tty) < 0)
	{
	  internal_error ("initialize_jobs: line discipline: %s",
			  strerror (errno));
	  job_control = 0;
	}
      else
	{
	  original_pgrp = shell_pgrp;
	  shell_pgrp = getpid ();

	  if ((original_pgrp != shell_pgrp) && (setpgid (0, shell_pgrp) < 0))
	    {
	      internal_error ("initialize_jobs: setpgid: %s", strerror (errno));
	      shell_pgrp = original_pgrp;
	    }

	  job_control = 1;
	  if (give_terminal_to (shell_pgrp) < 0)		/* XXX */
	    /* job_control = 0 */;				/* XXX */
	}
      if (job_control == 0)
        internal_error ("no job control in this shell");	/* XXX */
    }

  if (shell_tty != fileno (stderr))
    SET_CLOSE_ON_EXEC (shell_tty);

  set_signal_handler (SIGCHLD, flush_child);

  change_flag ('m', job_control ? '-' : '+');

  if (interactive)
    get_tty_state ();
  return job_control;
}

/* Set the line discipline to the best this system has to offer.
   Return -1 if this is not possible. */
static int
set_new_line_discipline (tty)
     int tty;
{
#if defined (NEW_TTY_DRIVER)
  int ldisc;

  if (ioctl (tty, TIOCGETD, &ldisc) < 0)
    return (-1);

  if (ldisc != NTTYDISC)
    {
      ldisc = NTTYDISC;

      if (ioctl (tty, TIOCSETD, &ldisc) < 0)
	return (-1);
    }
  return (0);
#endif /* NEW_TTY_DRIVER */

#if defined (TERMIO_TTY_DRIVER)
#  if defined (NTTYDISC)
  if (ioctl (tty, TCGETA, &shell_tty_info) < 0)
    return (-1);

  if (shell_tty_info.c_line != NTTYDISC)
    {
      shell_tty_info.c_line = NTTYDISC;
      if (ioctl (tty, TCSETAW, &shell_tty_info) < 0)
	return (-1);
    }
#  endif /* NTTYDISC */
  return (0);
#endif /* TERMIO_TTY_DRIVER */

#if defined (TERMIOS_TTY_DRIVER)
#  if defined (TERMIOS_LDISC)
  if (tcgetattr (tty, &shell_tty_info) < 0)
    return (-1);

  if (shell_tty_info.c_line != NTTYDISC)
    {
      shell_tty_info.c_line = NTTYDISC;
      if (tcsetattr (tty, TCSADRAIN, &shell_tty_info) < 0)
	return (-1);
    }
#  endif /* TERMIOS_LDISC */
  return (0);
#endif /* TERMIOS_TTY_DRIVER */

#if !defined (NEW_TTY_DRIVER) && !defined (TERMIO_TTY_DRIVER) && !defined (TERMIOS_TTY_DRIVER)
  return (-1);
#endif
}

static SigHandler *old_tstp, *old_ttou, *old_ttin;
static SigHandler *old_cont = (SigHandler *)SIG_DFL;
static sighandler stop_signal_handler (), cont_signal_handler ();

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
  if ((ioctl (shell_tty, TIOCGWINSZ, &win) == 0) &&
      win.ws_row > 0 && win.ws_col > 0)
    {
#if defined (aixpc)
      shell_tty_info.c_winsize = win;	/* structure copying */
#endif
      set_lines_and_columns (win.ws_row, win.ws_col);
    }
}
#endif /* !READLINE && TIOCGWINSZ && SIGWINCH */

/* Setup this shell to handle C-C, etc. */
void
initialize_job_signals ()
{
  if (interactive)
    {
      set_signal_handler (SIGINT, sigint_sighandler);
      set_signal_handler (SIGTSTP, SIG_IGN);
      set_signal_handler (SIGTTOU, SIG_IGN);
      set_signal_handler (SIGTTIN, SIG_IGN);
#if !defined (READLINE) && defined (TIOCGWINSZ) && defined (SIGWINCH)
      old_winch = set_signal_handler (SIGWINCH, sigwinch_sighandler);
#endif /* !READLINE && TIOCGWINSZ && SIGWINCH */
    }
  else if (job_control)
    {
      old_tstp = set_signal_handler (SIGTSTP, stop_signal_handler);
      old_ttou = set_signal_handler (SIGTTOU, stop_signal_handler);
      old_ttin = set_signal_handler (SIGTTIN, stop_signal_handler);
    }
  /* Leave these things alone for non-interactive shells without job
     control. */
}

/* Here we handle CONT signals. */
static sighandler
cont_signal_handler (sig)
     int sig;
{
  initialize_job_signals ();
  set_signal_handler (SIGCONT, old_cont);
  kill (getpid (), SIGCONT);

#if !defined (VOID_SIGHANDLER)
  return (0);
#endif /* VOID_SIGHANDLER */
}

/* Here we handle stop signals while we are running not as a login shell. */
static sighandler
stop_signal_handler (sig)
     int sig;
{
  set_signal_handler (SIGTSTP, old_tstp);
  set_signal_handler (SIGTTOU, old_ttou);
  set_signal_handler (SIGTTIN, old_ttin);

  old_cont = set_signal_handler (SIGCONT, cont_signal_handler);

  give_terminal_to (shell_pgrp);

  kill (getpid (), sig);

#if !defined (VOID_SIGHANDLER)
  return (0);
#endif /* VOID_SIGHANDLER */
}

/* Give the terminal to PGRP.  */
give_terminal_to (pgrp)
     pid_t pgrp;
{
  sigset_t set, oset;
  int r = 0;

  if (job_control)
    {
      sigemptyset (&set);
      sigaddset (&set, SIGTTOU);
      sigaddset (&set, SIGTTIN);
      sigaddset (&set, SIGTSTP);
      sigaddset (&set, SIGCHLD);
      sigemptyset (&oset);
      sigprocmask (SIG_BLOCK, &set, &oset);

      if (tcsetpgrp (shell_tty, pgrp) < 0)
	{
	  /* Maybe we should print an error message? */
/*	  internal_error ("tcsetpgrp(%d) failed: pid %d to pgrp %d: %s",
	    shell_tty, getpid(), pgrp, strerror (errno)); */
	  r = -1;
	}
      else
	terminal_pgrp = pgrp;

      sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
    }

  return r;
}

/* Clear out any jobs in the job array.  This is intended to be used by
   children of the shell, who should not have any job structures as baggage
   when they start executing (forking subshells for parenthesized execution
   and functions with pipes are the two that spring to mind). */
static void
delete_all_jobs ()
{
  register int i;
  sigset_t set, oset;

  if (job_slots)
    {
      BLOCK_CHILD (set, oset);

      if (job_slots)
	{
	  current_job = previous_job = NO_JOB;

	  for (i = 0; i < job_slots; i++)
	  if (jobs[i] != (JOB *) NULL)
	    delete_job (i);

	  free ((char *)jobs);
	  job_slots = 0;
	}

      UNBLOCK_CHILD (oset);
    }
}

/* Mark all dead jobs as notified, so delete_job () cleans them out
   of the job table properly. */
static void
mark_dead_jobs_as_notified ()
{
  register int i;
  sigset_t set, oset;

  if (job_slots)
    {
      BLOCK_CHILD (set, oset);

      for (i = 0; i < job_slots; i++)
	if (jobs[i] && JOBSTATE (i) == JDEAD)
	  jobs[i]->flags |= J_NOTIFIED;

      UNBLOCK_CHILD (oset);
    }
}

/* Allow or disallow job control to take place.  Returns the old value
   of job_control. */
int
set_job_control (arg)
     int arg;
{
  int old;

  old = job_control;
  job_control = arg;
  return (old);
}

/* Turn off all traces of job control.  This is run by children of the shell
   which are going to do shellsy things, like wait (), etc. */
void
without_job_control ()
{
  stop_making_children ();
  start_pipeline ();
  delete_all_jobs ();
  set_job_control (0);
}

/* If this shell is interactive, terminate all stopped jobs and
   restore the original terminal process group.  This is done
   before the `exec' builtin calls shell_execve. */
void
end_job_control ()
{
  if (interactive_shell)		/* XXX - should it be interactive? */
    {
      terminate_stopped_jobs ();

      if (original_pgrp >= 0)
	give_terminal_to (original_pgrp);
    }

  if (original_pgrp >= 0)
    setpgid (0, original_pgrp);
}

/* Restart job control by closing shell tty and reinitializing.  This is
   called after an exec fails in an interactive shell and we do not exit. */
void
restart_job_control ()
{
  if (shell_tty != -1)
    close (shell_tty);
  initialize_jobs ();
}

/* Set the handler to run when the shell receives a SIGCHLD signal. */
void
set_sigchld_handler ()
{
  set_signal_handler (SIGCHLD, flush_child);
}

#if defined (PGRP_PIPE)
/* Read from the read end of a pipe.  This is how the process group leader
   blocks until all of the processes in a pipeline have been made. */
static void
pipe_read (pp)
     int *pp;
{
  char ch;

  if (pp[1] >= 0)
    {
      close (pp[1]);
      pp[1] = -1;
    }

  if (pp[0] >= 0)
    {
      while (read (pp[0], &ch, 1) == -1 && errno == EINTR)
	continue;
    }
}

/* Close the read and write ends of PP, an array of file descriptors. */
static void
pipe_close (pp)
     int *pp;
{
  if (pp[0] >= 0)
    close (pp[0]);

  if (pp[1] >= 0)
    close (pp[1]);

  pp[0] = pp[1] = -1;
}

/* Functional interface closes our local-to-job-control pipes. */
close_pgrp_pipe ()
{
  pipe_close (pgrp_pipe);
}

#endif /* PGRP_PIPE */

#endif /* JOB_CONTROL */
