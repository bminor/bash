/* jobs.h -- structures and stuff used by the jobs.c file. */

/* Copyright (C) 1993 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#if !defined (__JOBS_H__)
#  define __JOBS_H__

#include "quit.h"
#include "siglist.h"

#include "stdc.h"

/* Defines controlling the fashion in which jobs are listed. */
#define JLIST_STANDARD 0
#define JLIST_LONG     1
#define JLIST_PID_ONLY 2
#define JLIST_CHANGED_ONLY 3

#if defined (HAVE_WAIT_H)
#  include <sys/wait.h>
#else /* !HAVE_WAIT_H */

#  include "bash_endian.h"

#  if !defined (_POSIX_VERSION)
#    if defined (LITTLE_ENDIAN)
union wait
  {
    int	w_status;		/* used in syscall */

    /* Terminated process status. */
    struct
      {
	unsigned short
	  w_Termsig  : 7,	/* termination signal */
	  w_Coredump : 1,	/* core dump indicator */
	  w_Retcode  : 8,	/* exit code if w_termsig==0 */
	  w_Fill1    : 16;	/* high 16 bits unused */
      } w_T;

    /* Stopped process status.  Returned
       only for traced children unless requested
       with the WUNTRACED option bit. */
    struct
      {
	unsigned short
	  w_Stopval : 8,	/* == W_STOPPED if stopped */
	  w_Stopsig : 8,	/* actually zero on XENIX */
	  w_Fill2   : 16;	/* high 16 bits unused */
      } w_S;
  };

#    else  /* !LITTLE_ENDIAN */

/* This is for big-endian machines like the IBM RT, HP 9000, or Sun-3 */

union wait
  {
    int	w_status;		/* used in syscall */

    /* Terminated process status. */
    struct
      {
	unsigned short w_Fill1    : 16;	/* high 16 bits unused */
	unsigned       w_Retcode  : 8;	/* exit code if w_termsig==0 */
	unsigned       w_Coredump : 1;	/* core dump indicator */
	unsigned       w_Termsig  : 7;	/* termination signal */
      } w_T;

    /* Stopped process status.  Returned
       only for traced children unless requested
       with the WUNTRACED option bit. */
    struct
      {
	unsigned short w_Fill2   : 16;	/* high 16 bits unused */
	unsigned       w_Stopsig : 8;	/* signal that stopped us */
	unsigned       w_Stopval : 8;	/* == W_STOPPED if stopped */
      } w_S;
  };

#    endif /* !LITTLE_ENDIAN */

#    define w_termsig  w_T.w_Termsig
#    define w_coredump w_T.w_Coredump
#    define w_retcode  w_T.w_Retcode
#    define w_stopval  w_S.w_Stopval
#    define w_stopsig  w_S.w_Stopsig

/* Note that sys/wait.h defines these for Posix systems. */
#    define WSTOPPED 0177
#    define WIFSTOPPED(x) (((x) . w_stopval) == WSTOPPED)
#    define WIFEXITED(x) ((! (WIFSTOPPED (x))) && (((x) . w_termsig) == 0))
#    define WIFSIGNALED(x) ((! (WIFSTOPPED (x))) && (((x) . w_termsig) != 0))
#  endif /* !_POSIX_VERSION */
#endif  /* !HAVE_WAIT_H */

/* How to get the status of a job.  For Posix, this is just an
   int, but for other systems we have to crack the union wait. */
#if !defined (_POSIX_VERSION)
#  define pid_t int
typedef union wait WAIT;
#  define WSTATUS(t)  (t.w_status)
#else /* _POSIX_VERSION */
typedef int WAIT;
#  define WSTATUS(t)  (t)
#endif /* _POSIX_VERSION */

/* Make sure that parameters to wait3 are defined. */
#if !defined (WNOHANG)
#  define WNOHANG 1
#  define WUNTRACED 2
#endif /* WNOHANG */

/* More Posix P1003.1 definitions.  In the POSIX versions, the parameter is
   passed as an `int', in the non-POSIX version, as `union wait'. */
#if defined (_POSIX_VERSION)

#  if !defined (WSTOPSIG)
#    define WSTOPSIG(s)       ((s) >> 8)
#  endif /* !WSTOPSIG */

#  if !defined (WTERMSIG)
#    define WTERMSIG(s)	      ((s) & 0177)
#  endif /* !WTERMSIG */

#  if !defined (WEXITSTATUS)
#    define WEXITSTATUS(s)    ((s) >> 8)
#  endif /* !WEXITSTATUS */

#  if !defined (WIFSTOPPED)
#    define WIFSTOPPED(s)     (((s) & 0177) == 0177)
#  endif /* !WIFSTOPPED */

#  if !defined (WIFEXITED)
#    define WIFEXITED(s)      (((s) & 0377) == 0)
#  endif /* !WIFEXITED */

#  if !defined (WIFSIGNALED)
#    define WIFSIGNALED(s)    (!WIFSTOPPED(s) && !WIFEXITED(s))
#  endif /* !WIFSIGNALED */

#  if !defined (WIFCORED)
#    define WIFCORED(s)       ((s) & 0200)
#  endif /* !WIFCORED */

#else /* !_POSIX_VERSION */

#  if !defined (WSTOPSIG)
#    define WSTOPSIG(s)	      ((s).w_stopsig)
#  endif /* !WSTOPSIG */

#  if !defined (WTERMSIG)
#    define WTERMSIG(s)	      ((s).w_termsig)
#  endif /* !WTERMSIG */

#  if !defined (WEXITSTATUS)
#    define WEXITSTATUS(s)    ((s).w_retcode)
#  endif /* !WEXITSTATUS */

#  if !defined (WIFCORED)
#    define WIFCORED(s)       ((s).w_coredump)
#  endif /* !WIFCORED */

#endif /* !_POSIX_VERSION */

/* I looked it up.  For pretty_print_job ().  The real answer is 24. */
#define LONGEST_SIGNAL_DESC 24

/* We keep an array of jobs.  Each entry in the array is a linked list
   of processes that are piped together.  The first process encountered is
   the group leader. */

/* Each child of the shell is remembered in a STRUCT PROCESS.  A chain of
   such structures is a pipeline.  The chain is circular. */
typedef struct process {
  struct process *next;	/* Next process in the pipeline.  A circular chain. */
  pid_t pid;		/* Process ID. */
  WAIT status;		/* The status of this command as returned by wait. */
  int running;		/* Non-zero if this process is running. */
  char *command;	/* The particular program that is running. */
} PROCESS;

/* A description of a pipeline's state. */
typedef enum { JRUNNING, JSTOPPED, JDEAD, JMIXED } JOB_STATE;
#define JOBSTATE(job) (jobs[(job)]->state)

/* Values for the FLAGS field in the JOB struct below. */
#define J_FOREGROUND 0x01 /* Non-zero if this is running in the foreground.  */
#define J_NOTIFIED   0x02 /* Non-zero if already notified about job state.   */
#define J_JOBCONTROL 0x04 /* Non-zero if this job started under job control. */

typedef struct job {
  char *wd;	   /* The working directory at time of invocation. */
  PROCESS *pipe;   /* The pipeline of processes that make up this job. */
  pid_t pgrp;	   /* The process ID of the process group (necessary). */
  JOB_STATE state; /* The state that this job is in. */
  int flags;	   /* Flags word: J_NOTIFIED, J_FOREGROUND, or J_JOBCONTROL. */
#if defined (JOB_CONTROL)
  COMMAND *deferred;	/* Commands that will execute when this job is done. */
#endif /* JOB_CONTROL */
} JOB;

#define NO_JOB  -1	/* An impossible job array index. */
#define DUP_JOB -2	/* A possible return value for get_job_spec (). */

/* A value which cannot be a process ID. */
#define NO_PID (pid_t)-1

#if !defined (_POSIX_VERSION) && !defined (sigmask)
#  define sigmask(x) (1 << ((x)-1))
#endif /* !POSIX && !sigmask */

#if !defined (SIGABRT)
#  define SIGABRT SIGIOT
#endif /* !SIGABRT */

#if !defined (SIGCHLD)
#  define SIGCHLD SIGCLD
#endif /* !SIGCHLD */

#if !defined (_POSIX_VERSION)
#  if !defined (SIG_BLOCK)
#    define SIG_BLOCK 2
#    define SIG_SETMASK 3
#  endif /* SIG_BLOCK */

/* Type of a signal set. */
#  define sigset_t int

/* Make sure there is nothing inside the signal set. */
#  define sigemptyset(set) (*(set) = 0)

/* Initialize the signal set to hold all signals. */
#  define sigfillset(set) (*set) = sigmask (NSIG) - 1

/* Add SIG to the contents of SET. */
#  define sigaddset(set, sig) *(set) |= sigmask (sig)

/* Delete SIG from signal set SET. */
#  define sigdelset(set, sig) *(set) &= ~sigmask (sig)

/* Is SIG a member of the signal set SET? */
#  define sigismember(set, sig) ((*(set) & sigmask (sig)) != 0)

/* Suspend the process until the reception of one of the signals
   not present in SET. */
#  define sigsuspend(set) sigpause (*(set))
#endif /* !_POSIX_VERSION */

/* These definitions are used both in POSIX and non-POSIX implementations. */

#define BLOCK_SIGNAL(sig, nvar, ovar) \
  sigemptyset (&nvar); \
  sigaddset (&nvar, sig); \
  sigemptyset (&ovar); \
  sigprocmask (SIG_BLOCK, &nvar, &ovar)

#if defined (_POSIX_VERSION)
#  define BLOCK_CHILD(nvar, ovar) \
	BLOCK_SIGNAL (SIGCHLD, nvar, ovar)
#  define UNBLOCK_CHILD(ovar) \
	sigprocmask (SIG_SETMASK, &ovar, (sigset_t *) NULL)
#else /* !_POSIX_VERSION */
#  define BLOCK_CHILD(nvar, ovar) ovar = sigblock (sigmask (SIGCHLD))
#  define UNBLOCK_CHILD(ovar) sigsetmask (ovar)
#endif /* !_POSIX_VERSION */

/* System calls. */
#if !defined (SunOS5) && !defined (USGr4_2) && !defined (__BSD_4_4__)
extern pid_t fork (), getpid (), getpgrp ();
#endif /* !SunOS5 && !USGr4_2 && !__BSD_4_4__ */

/* Stuff from the jobs.c file. */
extern pid_t  original_pgrp, shell_pgrp, pipeline_pgrp;
extern pid_t last_made_pid, last_asynchronous_pid;
extern int current_job, previous_job;
extern int asynchronous_notification;
extern JOB **jobs;
extern int job_slots;

extern void making_children __P((void));
extern void stop_making_children __P((void));
extern void cleanup_the_pipeline __P((void));
extern void start_pipeline __P((void));
extern int stop_pipeline __P((int, COMMAND *));
extern void delete_job __P((int));

extern void terminate_current_pipeline __P((void));
extern void terminate_stopped_jobs __P((void));
extern void hangup_all_jobs __P((void));
extern void kill_current_pipeline __P((void));

#if defined (__STDC__) && defined (pid_t)
extern void describe_pid __P((int));
#else
extern void describe_pid __P((pid_t));
#endif

extern int list_one_job __P((JOB *, int, int, int));
extern void list_jobs __P((int));

extern pid_t make_child __P((char *, int));
extern int get_tty_state __P((void));
extern int set_tty_state __P((void));

extern int wait_for_single_pid __P((pid_t));
extern void wait_for_background_pids __P((void));
extern int wait_for __P((pid_t));
extern int wait_for_job __P((int));

extern void notify_and_cleanup __P((void));
extern void reap_dead_jobs __P((void));
extern int start_job __P((int, int));
extern int kill_pid __P((pid_t, int, int));
extern int initialize_jobs __P((void));
extern void initialize_job_signals __P((void));
extern int give_terminal_to __P((pid_t));

extern int set_job_control __P((int));
extern void without_job_control __P((void));
extern void end_job_control __P((void));
extern void restart_job_control __P((void));
extern void set_sigchld_handler __P((void));

#if defined (JOB_CONTROL)
extern int job_control;
#endif

#endif /* __JOBS_H__ */
