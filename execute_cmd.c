/* execute_command.c -- Execute a COMMAND structure. */

/* Copyright (C) 1987,1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */
#if defined (AIX) && defined (RISC6000) && !defined (__GNUC__)
  #pragma alloca
#endif /* AIX && RISC6000 && !__GNUC__ */

#include <stdio.h>
#include <ctype.h>
#include "bashtypes.h"
#include <sys/file.h>
#include "filecntl.h"
#include "posixstat.h"
#include <signal.h>

#if !defined (SIGABRT)
#define SIGABRT SIGIOT
#endif

#include <sys/param.h>
#include <errno.h>

#if !defined (errno)
extern int errno;
#endif

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#include "shell.h"
#include "y.tab.h"
#include "flags.h"
#include "hash.h"
#include "jobs.h"
#include "execute_cmd.h"

#include "sysdefs.h"
#include "builtins/common.h"
#include "builtins/builtext.h"	/* list of builtins */

#include <glob/fnmatch.h>
#include <tilde/tilde.h>

#if defined (BUFFERED_INPUT)
#  include "input.h"
#endif

extern int posixly_correct;
extern int breaking, continuing, loop_level;
extern int interactive, interactive_shell, login_shell;
extern int parse_and_execute_level;
extern int command_string_index, variable_context, line_number;
extern int dot_found_in_search;
extern char **temporary_env, **function_env, **builtin_env;
extern char *the_printed_command, *shell_name;
extern pid_t last_command_subst_pid;
extern Function *last_shell_builtin, *this_shell_builtin;
extern jmp_buf top_level, subshell_top_level;
extern int subshell_argc;
extern char **subshell_argv, **subshell_envp;
extern int already_making_children;

extern int getdtablesize ();
extern int close ();

/* Static functions defined and used in this file. */
static void close_pipes (), do_piping (), execute_disk_command ();
static void execute_subshell_builtin_or_function ();
static void cleanup_redirects (), cleanup_func_redirects (), bind_lastarg ();
static void add_undo_close_redirect (), add_exec_redirect ();
static int do_redirection_internal (), do_redirections ();
static int expandable_redirection_filename (), execute_shell_script ();
static int execute_builtin_or_function (), add_undo_redirect ();
static char *find_user_command_internal (), *find_user_command_in_path ();

/* The line number that the currently executing function starts on. */
static int function_line_number = 0;

/* Set to 1 if fd 0 was the subject of redirection to a subshell. */
static int stdin_redir = 0;

/* The name of the command that is currently being executed.
   `test' needs this, for example. */
char *this_command_name;

struct stat SB;		/* used for debugging */

static REDIRECTEE rd;

/* For catching RETURN in a function. */
int return_catch_flag = 0;
int return_catch_value;
jmp_buf return_catch;

/* The value returned by the last synchronous command. */
int last_command_exit_value = 0;

/* The list of redirections to perform which will undo the redirections
   that I made in the shell. */
REDIRECT *redirection_undo_list = (REDIRECT *)NULL;

/* The list of redirections to perform which will undo the internal
   redirections performed by the `exec' builtin.  These are redirections
   that must be undone even when exec discards redirection_undo_list. */
REDIRECT *exec_redirection_undo_list = (REDIRECT *)NULL;

/* Non-zero if we have just forked and are currently running in a subshell
   environment. */
int subshell_environment = 0;

struct fd_bitmap *current_fds_to_close = (struct fd_bitmap *)NULL;

#define FD_BITMAP_DEFAULT_SIZE 32
/* Functions to allocate and deallocate the structures used to pass
   information from the shell to its children about file descriptors
   to close. */
struct fd_bitmap *
new_fd_bitmap (size)
     long size;
{
  struct fd_bitmap *ret;

  ret = (struct fd_bitmap *)xmalloc (sizeof (struct fd_bitmap));

  ret->size = size;

  if (size)
    {
      ret->bitmap = xmalloc (size);
      bzero (ret->bitmap, size);
    }
  else
    ret->bitmap = (char *)NULL;
  return (ret);
}

void
dispose_fd_bitmap (fdbp)
     struct fd_bitmap *fdbp;
{
  FREE (fdbp->bitmap);
  free (fdbp);
}

void
close_fd_bitmap (fdbp)
     struct fd_bitmap *fdbp;
{
  register int i;

  if (fdbp)
    {
      for (i = 0; i < fdbp->size; i++)
	if (fdbp->bitmap[i])
	  {
	    close (i);
	    fdbp->bitmap[i] = 0;
	  }
    }
}

/* Execute the command passed in COMMAND.  COMMAND is exactly what
   read_command () places into GLOBAL_COMMAND.  See "command.h" for the
   details of the command structure.

   EXECUTION_SUCCESS or EXECUTION_FAILURE are the only possible
   return values.  Executing a command with nothing in it returns
   EXECUTION_SUCCESS. */
execute_command (command)
     COMMAND *command;
{
  struct fd_bitmap *bitmap;
  int result;

  current_fds_to_close = (struct fd_bitmap *)NULL;
  bitmap = new_fd_bitmap (FD_BITMAP_DEFAULT_SIZE);
  begin_unwind_frame ("execute-command");
  add_unwind_protect (dispose_fd_bitmap, (char *)bitmap);

  /* Just do the command, but not asynchronously. */
  result = execute_command_internal (command, 0, NO_PIPE, NO_PIPE, bitmap);

  dispose_fd_bitmap (bitmap);
  discard_unwind_frame ("execute-command");

#if defined (PROCESS_SUBSTITUTION)
  unlink_fifo_list ();
#endif /* PROCESS_SUBSTITUTION */

  return (result);
}

/* Return 1 if TYPE is a shell control structure type. */
static int
shell_control_structure (type)
     enum command_type type;
{
  switch (type)
    {
    case cm_for:
#if defined (SELECT_COMMAND)
    case cm_select:
#endif
    case cm_case:
    case cm_while:
    case cm_until:
    case cm_if:
    case cm_group:
      return (1);

    default:
      return (0);
    }
}

/* A function to use to unwind_protect the redirection undo list
   for loops. */
static void
cleanup_redirects (list)
     REDIRECT *list;
{
  do_redirections (list, 1, 0, 0);
  dispose_redirects (list);
}

/* Function to unwind_protect the redirections for functions and builtins. */
static void
cleanup_func_redirects (list)
     REDIRECT *list;
{
  do_redirections (list, 1, 0, 0);
}

static void
dispose_exec_redirects ()
{
  if (exec_redirection_undo_list)
    {
      dispose_redirects (exec_redirection_undo_list);
      exec_redirection_undo_list = (REDIRECT *)NULL;
    }
}

#if defined (JOB_CONTROL)
/* A function to restore the signal mask to its proper value when the shell
   is interrupted or errors occur while creating a pipeline. */
static int
restore_signal_mask (set)
     sigset_t set;
{
  return (sigprocmask (SIG_SETMASK, &set, (sigset_t *)NULL));
}
#endif /* JOB_CONTROL */

/* A debugging function that can be called from gdb, for instance. */
void
open_files ()
{
  register int i;
  int f, fd_table_size;

  fd_table_size = getdtablesize ();

  fprintf (stderr, "pid %d open files:", getpid ());
  for (i = 3; i < fd_table_size; i++)
    {
      if ((f = fcntl (i, F_GETFD, 0)) != -1)
	fprintf (stderr, " %d (%s)", i, f ? "close" : "open");
    }
  fprintf (stderr, "\n");
}

#define DESCRIBE_PID(pid) if (interactive) describe_pid (pid)

/* Execute the command passed in COMMAND, perhaps doing it asynchrounously.
   COMMAND is exactly what read_command () places into GLOBAL_COMMAND.
   ASYNCHROUNOUS, if non-zero, says to do this command in the background.
   PIPE_IN and PIPE_OUT are file descriptors saying where input comes
   from and where it goes.  They can have the value of NO_PIPE, which means
   I/O is stdin/stdout.
   FDS_TO_CLOSE is a list of file descriptors to close once the child has
   been forked.  This list often contains the unusable sides of pipes, etc.

   EXECUTION_SUCCESS or EXECUTION_FAILURE are the only possible
   return values.  Executing a command with nothing in it returns
   EXECUTION_SUCCESS. */
execute_command_internal (command, asynchronous, pipe_in, pipe_out, 
			  fds_to_close)
     COMMAND *command;
     int asynchronous;
     int pipe_in, pipe_out;
     struct fd_bitmap *fds_to_close;
{
  int exec_result = EXECUTION_SUCCESS;
  int invert, ignore_return;
  REDIRECT *my_undo_list, *exec_undo_list;

  if (!command || breaking || continuing)
    return (EXECUTION_SUCCESS);

  run_pending_traps ();

  invert = (command->flags & CMD_INVERT_RETURN) != 0;

  /* If a command was being explicitly run in a subshell, or if it is
     a shell control-structure, and it has a pipe, then we do the command
     in a subshell. */

  if ((command->flags & CMD_WANT_SUBSHELL) ||
      (command->flags & CMD_FORCE_SUBSHELL)  ||
      (shell_control_structure (command->type) &&
       (pipe_out != NO_PIPE || pipe_in != NO_PIPE || asynchronous)))
    {
      pid_t paren_pid;

      /* Fork a subshell, turn off the subshell bit, turn off job
	 control and call execute_command () on the command again. */
      paren_pid = make_child (savestring (make_command_string (command)),
			      asynchronous);
      if (paren_pid == 0)
	{
	  int user_subshell, return_code, function_value;

	  /* Cancel traps, in trap.c. */
	  restore_original_signals ();
	  if (asynchronous)
	    setup_async_signals ();

#if defined (JOB_CONTROL)
	  set_sigchld_handler ();
#endif /* JOB_CONTROL */

	  set_sigint_handler ();

	  user_subshell = (command->flags & CMD_WANT_SUBSHELL) != 0;
	  command->flags &= ~(CMD_FORCE_SUBSHELL | CMD_WANT_SUBSHELL | CMD_INVERT_RETURN);

	  /* If a command is asynchronous in a subshell (like ( foo ) & or
	     the special case of an asynchronous GROUP command where the
	     the subshell bit is turned on down in case cm_group: below), 
	     turn off `asynchronous', so that two subshells aren't spawned.

	     This seems semantically correct to me.  For example, 
	     ( foo ) & seems to say ``do the command `foo' in a subshell
	     environment, but don't wait for that subshell to finish'',
	     and "{ foo ; bar } &" seems to me to be like functions or
	     builtins in the background, which executed in a subshell
	     environment.  I just don't see the need to fork two subshells. */

	  /* Don't fork again, we are already in a subshell.  A `doubly
	     async' shell is not interactive, however. */
	  if (asynchronous)
	    {
#if defined (JOB_CONTROL)
	      /* If a construct like ( exec xxx yyy ) & is given while job
		 control is active, we want to prevent exec from putting the
		 subshell back into the original process group, carefully
		 undoing all the work we just did in make_child. */
	      original_pgrp = -1;
#endif /* JOB_CONTROL */
	      interactive_shell = 0;
	      asynchronous = 0;
	    }

	  /* Subshells are neither login nor interactive. */
	  login_shell = interactive = 0;

	  subshell_environment = 1;

#if defined (JOB_CONTROL)
	  /* Delete all traces that there were any jobs running.  This is
	     only for subshells. */
	  without_job_control ();
#endif /* JOB_CONTROL */
	  do_piping (pipe_in, pipe_out);

	  /* If this is a user subshell, set a flag if stdin was redirected.
	     This is used later to decide whether to redirect fd 0 to
	     /dev/null for async commands in the subshell.  This adds more
	     sh compatibility, but I'm not sure it's the right thing to do. */
	  if (user_subshell)
	    {
	      REDIRECT *r;

	      for (r = command->redirects; r; r = r->next)
		switch (r->instruction)
		  {
		  case r_input_direction:
		  case r_inputa_direction:
		  case r_input_output:
		  case r_reading_until:
		  case r_deblank_reading_until:
		    stdin_redir++;
		    break;
		  case r_duplicating_input:
		  case r_duplicating_input_word:
		  case r_close_this:
		    if (r->redirector == 0)
		      stdin_redir++;
		    break;
		  }
	    }

	  if (fds_to_close)
	    close_fd_bitmap (fds_to_close);

	  /* Do redirections, then dispose of them before recursive call. */
	  if (command->redirects)
	    {
	      if (do_redirections (command->redirects, 1, 0, 0) != 0)
		exit (EXECUTION_FAILURE);

	      dispose_redirects (command->redirects);
	      command->redirects = (REDIRECT *)NULL;
	    }

	  /* If this is a simple command, tell execute_disk_command that it
	     might be able to get away without forking and simply exec.
	     This means things like ( sleep 10 ) will only cause one fork. */
	  if (user_subshell && command->type == cm_simple)
	    {
	      command->flags |= CMD_NO_FORK;
	      command->value.Simple->flags |= CMD_NO_FORK;
	    }

	  /* If we're inside a function while executing this subshell, we
	     need to handle a possible `return'. */
	  function_value = 0;
	  if (return_catch_flag)
	    function_value =  setjmp (return_catch);

	  if (function_value)
	    return_code = return_catch_value;
	  else
	    return_code = execute_command_internal
	      (command, asynchronous, NO_PIPE, NO_PIPE, fds_to_close);

	  /* If we were explicitly placed in a subshell with (), we need
	     to do the `shell cleanup' things, such as running traps[0]. */
	  if (user_subshell && signal_is_trapped (0))
	    {
	      last_command_exit_value = return_code;
	      return_code = run_exit_trap ();
	    }

	  exit (return_code);
	}
      else
	{
	  close_pipes (pipe_in, pipe_out);

#if defined (PROCESS_SUBSTITUTION) && defined (HAVE_DEV_FD)
	  unlink_fifo_list ();
#endif
	  /* If we are part of a pipeline, and not the end of the pipeline,
	     then we should simply return and let the last command in the
	     pipe be waited for.  If we are not in a pipeline, or are the
	     last command in the pipeline, then we wait for the subshell 
	     and return its exit status as usual. */
	  if (pipe_out != NO_PIPE)
	    return (EXECUTION_SUCCESS);

	  stop_pipeline (asynchronous, (COMMAND *)NULL);

	  if (!asynchronous)
	    {
	      last_command_exit_value = wait_for (paren_pid);

	      /* If we have to, invert the return value. */
	      if (invert)
		{
		  if (last_command_exit_value == EXECUTION_SUCCESS)
		    return (EXECUTION_FAILURE);
		  else
		    return (EXECUTION_SUCCESS);
		}
	      else
		return (last_command_exit_value);
	    }
	  else
	    {
	      DESCRIBE_PID (paren_pid);

	      run_pending_traps ();

	      return (EXECUTION_SUCCESS);
	    }
	}
    }

  /* Handle WHILE FOR CASE etc. with redirections.  (Also '&' input
     redirection.)  */
  if (do_redirections (command->redirects, 1, 1, 0) != 0)
    {
      cleanup_redirects (redirection_undo_list);
      redirection_undo_list = (REDIRECT *)NULL;
      dispose_exec_redirects ();
      return (EXECUTION_FAILURE);
    }

  if (redirection_undo_list)
    {
      my_undo_list = (REDIRECT *)copy_redirects (redirection_undo_list);
      dispose_redirects (redirection_undo_list);
      redirection_undo_list = (REDIRECT *)NULL;
    }
  else
    my_undo_list = (REDIRECT *)NULL;

  if (exec_redirection_undo_list)
    {
      exec_undo_list = (REDIRECT *)copy_redirects (exec_redirection_undo_list);
      dispose_redirects (exec_redirection_undo_list);
      exec_redirection_undo_list = (REDIRECT *)NULL;
    }
  else
    exec_undo_list = (REDIRECT *)NULL;

  if (my_undo_list || exec_undo_list)
    begin_unwind_frame ("loop_redirections");

  if (my_undo_list)
    add_unwind_protect ((Function *)cleanup_redirects, my_undo_list);

  if (exec_undo_list)
    add_unwind_protect ((Function *)dispose_redirects, exec_undo_list);

  ignore_return = (command->flags & CMD_IGNORE_RETURN) != 0;

  QUIT;

  switch (command->type)
    {
    case cm_for:
      if (ignore_return)
	command->value.For->flags |= CMD_IGNORE_RETURN;
      exec_result = execute_for_command (command->value.For);
      break;

#if defined (SELECT_COMMAND)
    case cm_select:
      if (ignore_return)
	command->value.Select->flags |= CMD_IGNORE_RETURN;
      exec_result = execute_select_command (command->value.Select);
      break;
#endif

    case cm_case:
      if (ignore_return)
	command->value.Case->flags |= CMD_IGNORE_RETURN;
      exec_result = execute_case_command (command->value.Case);
      break;

    case cm_while:
      if (ignore_return)
	command->value.While->flags |= CMD_IGNORE_RETURN;
      exec_result = execute_while_command (command->value.While);
      break;

    case cm_until:
      if (ignore_return)
	command->value.While->flags |= CMD_IGNORE_RETURN;
      exec_result = execute_until_command (command->value.While);
      break;

    case cm_if:
      if (ignore_return)
	command->value.If->flags |= CMD_IGNORE_RETURN;
      exec_result = execute_if_command (command->value.If);
      break;

    case cm_group:

      /* This code can be executed from either of two paths: an explicit
	 '{}' command, or via a function call.  If we are executed via a
	 function call, we have already taken care of the function being
	 executed in the background (down there in execute_simple_command ()),
	 and this command should *not* be marked as asynchronous.  If we
	 are executing a regular '{}' group command, and asynchronous == 1,
	 we must want to execute the whole command in the background, so we
	 need a subshell, and we want the stuff executed in that subshell
	 (this group command) to be executed in the foreground of that
	 subshell (i.e. there will not be *another* subshell forked).

	 What we do is to force a subshell if asynchronous, and then call
	 execute_command_internal again with asynchronous still set to 1,
	 but with the original group command, so the printed command will
	 look right.

	 The code above that handles forking off subshells will note that
	 both subshell and async are on, and turn off async in the child
	 after forking the subshell (but leave async set in the parent, so
	 the normal call to describe_pid is made).  This turning off
	 async is *crucial*; if it is not done, this will fall into an
	 infinite loop of executions through this spot in subshell after
	 subshell until the process limit is exhausted. */

      if (asynchronous)
	{
	  command->flags |= CMD_FORCE_SUBSHELL;
	  exec_result =
	    execute_command_internal (command, 1, pipe_in, pipe_out,
				      fds_to_close);
	}
      else
	{
	  if (ignore_return && command->value.Group->command)
	    command->value.Group->command->flags |= CMD_IGNORE_RETURN;
	  exec_result =
	    execute_command_internal (command->value.Group->command,
				      asynchronous, pipe_in, pipe_out,
				      fds_to_close);
	}
      break;

    case cm_simple:
      {
	/* We can't rely on this variable retaining its value across a
	   call to execute_simple_command if a longjmp occurs as the
	   result of a `return' builtin.  This is true for sure with gcc. */
	pid_t last_pid = last_made_pid;

	if (ignore_return && command->value.Simple)
	  command->value.Simple->flags |= CMD_IGNORE_RETURN;
	exec_result =
	  execute_simple_command (command->value.Simple, pipe_in, pipe_out,
				  asynchronous, fds_to_close);

	/* The temporary environment should be used for only the simple
	   command immediately following its definition. */
	dispose_used_env_vars ();

#if (defined (Ultrix) && defined (mips)) || !defined (HAVE_ALLOCA)
	/* Reclaim memory allocated with alloca () on machines which
	   may be using the alloca emulation code. */
	(void) alloca (0);
#endif /* (Ultrix && mips) || !HAVE_ALLOCA */

	/* If we forked to do the command, then we must wait_for ()
	   the child. */

	/* XXX - this is something to watch out for if there are problems
	   when the shell is compiled without job control. */
	if (already_making_children && pipe_out == NO_PIPE &&
	    last_pid != last_made_pid)
	  {
	    stop_pipeline (asynchronous, (COMMAND *)NULL);

	    if (asynchronous)
	      {
		DESCRIBE_PID (last_made_pid);
	      }
	    else
#if !defined (JOB_CONTROL)
	      /* Do not wait for asynchronous processes started from
		 startup files. */
	    if (last_made_pid != last_asynchronous_pid)
#endif
	    /* When executing a shell function that executes other
	       commands, this causes the last simple command in
	       the function to be waited for twice. */
	      exec_result = wait_for (last_made_pid);
	  }
      }

      if (!ignore_return && exit_immediately_on_error && !invert &&
	  (exec_result != EXECUTION_SUCCESS))
	{
	  last_command_exit_value = exec_result;
	  run_pending_traps ();
	  longjmp (top_level, EXITPROG);
	}

      break;

    case cm_connection:
      switch (command->value.Connection->connector)
	{
	  /* Do the first command asynchronously. */
	case '&':
	  {
	    COMMAND *tc = command->value.Connection->first;
	    REDIRECT *rp;

	    if (!tc)
	      break;

	    rp = tc->redirects;

	    if (ignore_return && tc)
	      tc->flags |= CMD_IGNORE_RETURN;

	    /* If this shell was compiled without job control support, if
	       the shell is not running interactively, if we are currently
	       in a subshell via `( xxx )', or if job control is not active
	       then the standard input for an asynchronous command is
	       forced to /dev/null. */
#if defined (JOB_CONTROL)
	    if ((!interactive_shell || subshell_environment || !job_control) &&
		!stdin_redir)
#else
	    if (!stdin_redir)
#endif /* JOB_CONTROL */
	    {
	      REDIRECT *tr;

	      rd.filename = make_word ("/dev/null");
	      tr = make_redirection (0, r_inputa_direction, rd);
	      tr->next = tc->redirects;
	      tc->redirects = tr;
	    }

	    exec_result = execute_command_internal
	      (tc, 1, pipe_in, pipe_out, fds_to_close);

#if defined (JOB_CONTROL)
	    if ((!interactive_shell || subshell_environment || !job_control) &&
		!stdin_redir)
#else
	    if (!stdin_redir)
#endif /* JOB_CONTROL */
	    {
	      /* Remove the redirection we added above.  It matters,
		 especially for loops, which call execute_command ()
		 multiple times with the same command. */
	      REDIRECT *tr, *tl;

	      tr = tc->redirects;
	      do
		{
		  tl = tc->redirects;
		  tc->redirects = tc->redirects->next;
		}
	      while (tc->redirects && tc->redirects != rp);

	      tl->next = (REDIRECT *)NULL;
	      dispose_redirects (tr);
	    }

	    {
	      register COMMAND *second;

	      second = command->value.Connection->second;

	      if (second)
		{
		  if (ignore_return)
		    second->flags |= CMD_IGNORE_RETURN;

		  exec_result = execute_command_internal
		    (second, asynchronous, pipe_in, pipe_out, fds_to_close);
		}
	    }
	  }
	  break;

	case ';':
	  /* Just call execute command on both of them. */
	  if (ignore_return)
	    {
	      if (command->value.Connection->first)
		command->value.Connection->first->flags |= CMD_IGNORE_RETURN;
	      if (command->value.Connection->second)
		command->value.Connection->second->flags |= CMD_IGNORE_RETURN;
	    }
	  QUIT;
	  execute_command (command->value.Connection->first);
	  QUIT;
	  exec_result =
	    execute_command_internal (command->value.Connection->second,
				      asynchronous, pipe_in, pipe_out,
				      fds_to_close);
	  break;

	case '|':
	  {
	    int prev, fildes[2], new_bitmap_size, dummyfd;
	    COMMAND *cmd;
	    struct fd_bitmap *fd_bitmap;

#if defined (JOB_CONTROL)
	    sigset_t set, oset;
	    BLOCK_CHILD (set, oset);
#endif /* JOB_CONTROL */

	    prev = pipe_in;
	    cmd = command;

	    while (cmd &&
		   cmd->type == cm_connection &&
		   cmd->value.Connection &&
		   cmd->value.Connection->connector == '|')
	      {
		/* Make a pipeline between the two commands. */
		if (pipe (fildes) < 0)
		  {
		    report_error ("pipe error: %s", strerror (errno));
#if defined (JOB_CONTROL)
		    terminate_current_pipeline ();
		    kill_current_pipeline ();
#endif /* JOB_CONTROL */
		    last_command_exit_value = EXECUTION_FAILURE;
		    /* The unwind-protects installed below will take care
		       of closing all of the open file descriptors. */
		    throw_to_top_level ();
		  }
		else
		  {
		    /* Here is a problem: with the new file close-on-exec
		       code, the read end of the pipe (fildes[0]) stays open
		       in the first process, so that process will never get a
		       SIGPIPE.  There is no way to signal the first process
		       that it should close fildes[0] after forking, so it
		       remains open.  No SIGPIPE is ever sent because there
		       is still a file descriptor open for reading connected
		       to the pipe.  We take care of that here.  This passes
		       around a bitmap of file descriptors that must be
		       closed after making a child process in
		       execute_simple_command. */

		    /* We need fd_bitmap to be at least as big as fildes[0].
		       If fildes[0] is less than fds_to_close->size, then
		       use fds_to_close->size. */
		    if (fildes[0] < fds_to_close->size)
		      new_bitmap_size = fds_to_close->size;
		    else
		      new_bitmap_size = fildes[0] + 8;

		    fd_bitmap = new_fd_bitmap (new_bitmap_size);

		    /* Now copy the old information into the new bitmap. */
		    xbcopy ((char *)fds_to_close->bitmap,
			    (char *)fd_bitmap->bitmap, fds_to_close->size);

		    /* And mark the pipe file descriptors to be closed. */
		    fd_bitmap->bitmap[fildes[0]] = 1;

		    /* In case there are pipe or out-of-processes errors, we
		       want all these file descriptors to be closed when
		       unwind-protects are run, and the storage used for the
		       bitmaps freed up. */
		    begin_unwind_frame ("pipe-file-descriptors");
		    add_unwind_protect (dispose_fd_bitmap, fd_bitmap);
		    add_unwind_protect (close_fd_bitmap, fd_bitmap);
		    if (prev >= 0)
		      add_unwind_protect (close, prev);
		    dummyfd = fildes[1];
		    add_unwind_protect (close, dummyfd);

#if defined (JOB_CONTROL)
		    add_unwind_protect (restore_signal_mask, oset);
#endif /* JOB_CONTROL */

		    if (ignore_return && cmd->value.Connection->first)
		      cmd->value.Connection->first->flags |=
			CMD_IGNORE_RETURN;
		    execute_command_internal
		      (cmd->value.Connection->first, asynchronous, prev,
		       fildes[1], fd_bitmap);

		    if (prev >= 0)
		      close (prev);
		    
		    prev = fildes[0];
		    close (fildes[1]);

		    dispose_fd_bitmap (fd_bitmap);
		    discard_unwind_frame ("pipe-file-descriptors");
		  }
		cmd = cmd->value.Connection->second;
	      }

	    /* Now execute the rightmost command in the pipeline.  */
	    if (ignore_return && cmd)
	      cmd->flags |= CMD_IGNORE_RETURN;
	    exec_result =
	      execute_command_internal
		(cmd, asynchronous, prev, pipe_out, fds_to_close);

	    if (prev >= 0)
	      close (prev);

#if defined (JOB_CONTROL)
	    UNBLOCK_CHILD (oset);
#endif
	  }
	  break;

	case AND_AND:
	case OR_OR:
	  if (asynchronous)
	    {
	      /* If we have something like `a && b &' or `a || b &', run the
	         && or || stuff in a subshell.  Force a subshell and just call
		 execute_command_internal again.  Leave asynchronous on
		 so that we get a report from the parent shell about the
		 background job. */
	      command->flags |= CMD_FORCE_SUBSHELL;
	      exec_result = execute_command_internal (command, 1, pipe_in,
			      pipe_out, fds_to_close);
	      break;
	    }

	  /* Execute the first command.  If the result of that is successful
	     and the connector is AND_AND, or the result is not successful
	     and the connector is OR_OR, then execute the second command,
	     otherwise return. */

	  if (command->value.Connection->first)
	    command->value.Connection->first->flags |= CMD_IGNORE_RETURN;

	  exec_result = execute_command (command->value.Connection->first);
	  QUIT;
	  if (((command->value.Connection->connector == AND_AND) && 
	       (exec_result == EXECUTION_SUCCESS)) ||
	      ((command->value.Connection->connector == OR_OR) &&
	       (exec_result != EXECUTION_SUCCESS)))
	    {
	      if (ignore_return && command->value.Connection->second)
		command->value.Connection->second->flags |=
		  CMD_IGNORE_RETURN;

	      exec_result =
		execute_command (command->value.Connection->second);
	    }
	  break;

	default:
	  programming_error ("Bad connector `%d'!",
			     command->value.Connection->connector);
	  longjmp (top_level, DISCARD);
	  break;
	}
      break;

    case cm_function_def:
      exec_result = intern_function (command->value.Function_def->name,
				     command->value.Function_def->command);
      break;

    default:
      programming_error
	("execute_command: Bad command type `%d'!", command->type);
    }

  if (my_undo_list)
    {
      do_redirections (my_undo_list, 1, 0, 0);
      dispose_redirects (my_undo_list);
    }

  if (exec_undo_list)
    dispose_redirects (exec_undo_list);

  if (my_undo_list || exec_undo_list)
    discard_unwind_frame ("loop_redirections");

  /* Invert the return value if we have to */
  if (invert)
    {
      if (exec_result == EXECUTION_SUCCESS)
	exec_result = EXECUTION_FAILURE;
      else
	exec_result = EXECUTION_SUCCESS;
    }

  last_command_exit_value = exec_result;
  run_pending_traps ();
  return (last_command_exit_value);
}

#if defined (JOB_CONTROL)
#  define REAP() \
	do \
	  { \
	    if (!interactive_shell) \
	      reap_dead_jobs (); \
	  } \
	while (0)
#else /* !JOB_CONTROL */
#  define REAP() \
	do \
	  { \
	    if (!interactive_shell) \
	      cleanup_dead_jobs (); \
	  } \
	while (0)
#endif /* !JOB_CONTROL */


/* Execute a FOR command.  The syntax is: FOR word_desc IN word_list;
   DO command; DONE */
execute_for_command (for_command)
     FOR_COM *for_command;
{
  /* I just noticed that the Bourne shell leaves word_desc bound to the
     last name in word_list after the FOR statement is done.  This seems
     wrong to me; I thought that the variable binding should be lexically
     scoped, i.e., only would last the duration of the FOR command.  This
     behaviour can be gotten by turning on the lexical_scoping switch. */

  register WORD_LIST *releaser, *list;
  char *identifier;
  SHELL_VAR *old_value = (SHELL_VAR *)NULL; /* Remember the old value of x. */
  int retval = EXECUTION_SUCCESS;

  if (check_identifier (for_command->name, 1) == 0)
    return (EXECUTION_FAILURE);

  loop_level++;
  identifier = for_command->name->word;

  list = releaser = expand_words_no_vars (for_command->map_list);

  begin_unwind_frame ("for");
  add_unwind_protect (dispose_words, releaser);

  if (lexical_scoping)
    {
      old_value = copy_variable (find_variable (identifier));
      if (old_value)
	add_unwind_protect (dispose_variable, old_value);
    }

  if (for_command->flags & CMD_IGNORE_RETURN)
    for_command->action->flags |= CMD_IGNORE_RETURN;

  while (list)
    {
      QUIT;
      bind_variable (identifier, list->word->word);
      execute_command (for_command->action);
      retval = last_command_exit_value;
      REAP ();
      QUIT;

      if (breaking)
	{
	  breaking--; 
	  break;
	}

      if (continuing)
	{
	  continuing--;
	  if (continuing)
	    break;
	}

      list = list->next;
    }

  loop_level--;

  if (lexical_scoping)
    {
      if (!old_value)
	makunbound (identifier, shell_variables);
      else
	{
	  SHELL_VAR *new_value;

	  new_value = bind_variable (identifier, value_cell(old_value));
	  new_value->attributes = old_value->attributes;
	  dispose_variable (old_value);
	}
    }

  dispose_words (releaser);
  discard_unwind_frame ("for");
  return (retval);
}

#if defined (SELECT_COMMAND)
static int LINES, COLS, tabsize;

#define RP_SPACE ") "
#define RP_SPACE_LEN 2

/* XXX - does not handle numbers > 1000000 at all. */
#define NUMBER_LEN(s) \
((s < 10) ? 1 \
	  : ((s < 100) ? 2 \
		      : ((s < 1000) ? 3 \
				   : ((s < 10000) ? 4 \
						 : ((s < 100000) ? 5 \
								: 6)))))

static int
print_index_and_element (len, ind, list)
      int len, ind;
      WORD_LIST *list;
{
  register WORD_LIST *l;
  register int i;

  if (list == 0)
    return (0);
  i = ind;
  l = list;
  while (l && --i)
    l = l->next;
  fprintf (stderr, "%*d%s%s", len, ind, RP_SPACE, l->word->word);
  return (STRLEN (l->word->word));
}

static void
indent (from, to)
     int from, to;
{
  while (from < to)
    {
      if ((to / tabsize) > (from / tabsize))
	{
	  putc ('\t', stderr);
	  from += tabsize - from % tabsize;
	}
      else
	{
	  putc (' ', stderr);
	  from++;
	}
    }
}

static void
print_select_list (list, list_len, max_elem_len, indices_len)
     WORD_LIST *list;
     int list_len, max_elem_len, indices_len;
{
  int ind, row, elem_len, pos, cols, rows;
  int first_column_indices_len, other_indices_len;

  if (list == 0)
    {
      putc ('\n', stderr);
      return;
    }

  cols = COLS / max_elem_len;
  if (cols == 0)
    cols = 1;
  rows = list_len ? list_len / cols + (list_len % cols != 0) : 1;
  cols = list_len ? list_len / rows + (list_len % rows != 0) : 1;

  if (rows == 1)
    {
      rows = cols;
      cols = 1;
    }

  first_column_indices_len = NUMBER_LEN (rows);
  other_indices_len = indices_len;

  for (row = 0; row < rows; row++)
    {
      ind = row;
      pos = 0;
      while (1)
	{
	  indices_len = (pos == 0) ? first_column_indices_len : other_indices_len;
	  elem_len = print_index_and_element (indices_len, ind + 1, list);
	  elem_len += indices_len + RP_SPACE_LEN;
	  ind += rows;
	  if (ind >= list_len)
	    break;
	  indent (pos + elem_len, pos + max_elem_len);
	  pos += max_elem_len;
	}
      putc ('\n', stderr);
    }
}

/* Print the elements of LIST, one per line, preceded by an index from 1 to
   LIST_LEN.  Then display PROMPT and wait for the user to enter a number.
   If the number is between 1 and LIST_LEN, return that selection.  If EOF
   is read, return a null string.  If a blank line is entered, the loop is
   executed again. */
static char *
select_query (list, list_len, prompt)
     WORD_LIST *list;
     int list_len;
     char *prompt;
{
  int max_elem_len, indices_len, len, reply;
  WORD_LIST *l;
  char *repl_string, *t;

  t = get_string_value ("LINES");
  LINES = (t && *t) ? atoi (t) : 24;
  t = get_string_value ("COLUMNS");
  COLS =  (t && *t) ? atoi (t) : 80;

#if 0
  t = get_string_value ("TABSIZE");
  tabsize = (t && *t) ? atoi (t) : 8;
  if (tabsize <= 0)
    tabsize = 8;
#else
  tabsize = 8;
#endif

  max_elem_len = 0;
  for (l = list; l; l = l->next)
    {
      len = STRLEN (l->word->word);
      if (len > max_elem_len)
        max_elem_len = len;
    }
  indices_len = NUMBER_LEN (list_len);
  max_elem_len += indices_len + RP_SPACE_LEN + 2;

  while (1)
    {
      print_select_list (list, list_len, max_elem_len, indices_len);
      printf ("%s", prompt);
      fflush (stdout);
      QUIT;

      if (read_builtin ((WORD_LIST *)NULL) == EXECUTION_FAILURE)
	{
	  putchar ('\n');
	  return ((char *)NULL);
	}
      repl_string = get_string_value ("REPLY");
      if (*repl_string == 0)
	continue;
      reply = atoi (repl_string);
      if (reply < 1 || reply > list_len)
	return "";

      l = list;
      while (l && --reply)
        l = l->next;
      return (l->word->word);
    }
}

/* Execute a SELECT command.  The syntax is:
   SELECT word IN list DO command_list DONE
   Only `break' or `return' in command_list will terminate
   the command. */
execute_select_command (select_command)
     SELECT_COM *select_command;
{
  WORD_LIST *releaser, *list;
  char *identifier, *ps3_prompt, *selection;
  int retval, list_len, return_val;
#if 0
  SHELL_VAR *old_value = (SHELL_VAR *)0;
#endif


  retval = EXECUTION_SUCCESS;

  if (check_identifier (select_command->name, 1) == 0)
    return (EXECUTION_FAILURE);

  loop_level++;
  identifier = select_command->name->word;

  /* command and arithmetic substitution, parameter and variable expansion,
     word splitting, pathname expansion, and quote removal. */
  list = releaser = expand_words_no_vars (select_command->map_list);
  list_len = list_length (list);
  if (list == 0 || list_len == 0)
    {
      if (list)
	dispose_words (list);
      return (EXECUTION_SUCCESS);
    }

  begin_unwind_frame ("select");
  add_unwind_protect (dispose_words, releaser);

#if 0
  if (lexical_scoping)
    {
      old_value = copy_variable (find_variable (identifier));
      if (old_value)
	add_unwind_protect (dispose_variable, old_value);
    }
#endif

  if (select_command->flags & CMD_IGNORE_RETURN)
    select_command->action->flags |= CMD_IGNORE_RETURN;

  unwind_protect_int (return_catch_flag);
  unwind_protect_jmp_buf (return_catch);
  return_catch_flag++;

  while (1)
    {
      ps3_prompt = get_string_value ("PS3");
      if (!ps3_prompt)
	ps3_prompt = "#? ";

      QUIT;
      selection = select_query (list, list_len, ps3_prompt);
      QUIT;
      if (selection == 0)
	break;
      else
	bind_variable (identifier, selection);

      return_val = setjmp (return_catch);

      if (return_val)
        {
	  retval = return_catch_value;
	  break;
        }
      else
        retval = execute_command (select_command->action);

      REAP ();
      QUIT;

      if (breaking)
	{
	  breaking--;
	  break;
	}
    }

  loop_level--;

#if 0
  if (lexical_scoping)
    {
      if (!old_value)
	makunbound (identifier, shell_variables);
      else
	{
	  SHELL_VAR *new_value;

	  new_value = bind_variable (identifier, value_cell(old_value));
	  new_value->attributes = old_value->attributes;
	  dispose_variable (old_value);
	}
    }
#endif

  run_unwind_frame ("select");
  return (retval);
}
#endif /* SELECT_COMMAND */

/* Execute a CASE command.  The syntax is: CASE word_desc IN pattern_list ESAC.
   The pattern_list is a linked list of pattern clauses; each clause contains
   some patterns to compare word_desc against, and an associated command to
   execute. */
execute_case_command (case_command)
     CASE_COM *case_command;
{
  register WORD_LIST *list;
  WORD_LIST *wlist;
  PATTERN_LIST *clauses;
  char *word;
  int retval;

  /* Posix.2 specifies that the WORD is tilde expanded. */
  if (member ('~', case_command->word->word))
    {
      word = tilde_expand (case_command->word->word);
      free (case_command->word->word);
      case_command->word->word = word;
    }

  wlist = expand_word_no_split (case_command->word, 0);
  clauses = case_command->clauses;
  word = (wlist) ? string_list (wlist) : savestring ("");
  retval = EXECUTION_SUCCESS;

  begin_unwind_frame ("case");
  add_unwind_protect (dispose_words, wlist);
  add_unwind_protect ((Function *)xfree, word);

  while (clauses)
    {
      QUIT;
      list = clauses->patterns;
      while (list)
	{
	  char *pattern;
	  WORD_LIST *es;
	  int match;

	  /* Posix.2 specifies to tilde expand each member of the pattern
	     list. */
	  if (member ('~', list->word->word))
	    {
	      char *expansion = tilde_expand (list->word->word);
	      free (list->word->word);
	      list->word->word = expansion;
	    }

	  es = expand_word_leave_quoted (list->word, 0);

	  if (es && es->word && es->word->word && *(es->word->word))
	    pattern = quote_string_for_globbing (es->word->word, 1);
	  else
	    pattern = savestring ("");

	  /* Since the pattern does not undergo quote removal (as per
	     Posix.2, section 3.9.4.3), the fnmatch () call must be able
	     to recognize backslashes as escape characters. */
	  match = (fnmatch (pattern, word, 0) != FNM_NOMATCH);
	  free (pattern);

	  dispose_words (es);

	  if (match)
	    {
	      if (clauses->action && 
		  (case_command->flags & CMD_IGNORE_RETURN))
		clauses->action->flags |= CMD_IGNORE_RETURN;
	      execute_command (clauses->action);
	      retval = last_command_exit_value;
	      goto exit_command;
	    }

	  list = list->next;
	  QUIT;
	}

      clauses = clauses->next;
    }

 exit_command:
  dispose_words (wlist);
  free (word);  
  discard_unwind_frame ("case");

  return (retval);
}

#define CMD_WHILE 0
#define CMD_UNTIL 1

/* The WHILE command.  Syntax: WHILE test DO action; DONE.
   Repeatedly execute action while executing test produces
   EXECUTION_SUCCESS. */
execute_while_command (while_command)
     WHILE_COM *while_command;
{
  return (execute_while_or_until (while_command, CMD_WHILE));
}

/* UNTIL is just like WHILE except that the test result is negated. */
execute_until_command (while_command)
     WHILE_COM *while_command;
{
  return (execute_while_or_until (while_command, CMD_UNTIL));
}

/* The body for both while and until.  The only difference between the
   two is that the test value is treated differently.  TYPE is
   CMD_WHILE or CMD_UNTIL.  The return value for both commands should
   be EXECUTION_SUCCESS if no commands in the body are executed, and
   the status of the last command executed in the body otherwise. */
execute_while_or_until (while_command, type)
     WHILE_COM *while_command;
     int type;
{
  int return_value, body_status;

  body_status = EXECUTION_SUCCESS;
  loop_level++;

  while_command->test->flags |= CMD_IGNORE_RETURN;
  if (while_command->flags & CMD_IGNORE_RETURN)
    while_command->action->flags |= CMD_IGNORE_RETURN;

  while (1)
    {
      return_value = execute_command (while_command->test);
      REAP ();

      if (type == CMD_WHILE && return_value != EXECUTION_SUCCESS)
	break;
      if (type == CMD_UNTIL && return_value == EXECUTION_SUCCESS)
	break;

      QUIT;
      body_status = execute_command (while_command->action);
      QUIT;

      if (breaking)
	{
	  breaking--;
	  break;
	}

      if (continuing)
	{
	  continuing--;
	  if (continuing)
	    break;
	}
    }
  loop_level--;

  return (body_status);
}

/* IF test THEN command [ELSE command].
   IF also allows ELIF in the place of ELSE IF, but
   the parser makes *that* stupidity transparent. */
execute_if_command (if_command)
     IF_COM *if_command;
{
  int return_value;

  if_command->test->flags |= CMD_IGNORE_RETURN;
  return_value = execute_command (if_command->test);

  if (return_value == EXECUTION_SUCCESS)
    {
      QUIT;
      if (if_command->true_case && (if_command->flags & CMD_IGNORE_RETURN))
	  if_command->true_case->flags |= CMD_IGNORE_RETURN;
      return (execute_command (if_command->true_case));
    }
  else
    {
      QUIT;

      if (if_command->false_case && (if_command->flags & CMD_IGNORE_RETURN))
	if_command->false_case->flags |= CMD_IGNORE_RETURN;

      return (execute_command (if_command->false_case));
    }
}

static void
bind_lastarg (arg)
     char *arg;
{
  SHELL_VAR *var;

  if (!arg)
    arg = "";
  var = bind_variable ("_", arg);
  var->attributes &= ~att_exported;
}

/* The meaty part of all the executions.  We have to start hacking the
   real execution of commands here.  Fork a process, set things up,
   execute the command. */
execute_simple_command (simple_command, pipe_in, pipe_out, async, fds_to_close)
     SIMPLE_COM *simple_command;
     int pipe_in, pipe_out, async;
     struct fd_bitmap *fds_to_close;
{
  WORD_LIST *words, *lastword;
  char *command_line, *lastarg;
  int first_word_quoted, result;
  pid_t old_last_command_subst_pid;

  result = EXECUTION_SUCCESS;

  /* If we're in a function, update the pseudo-line-number information. */
  if (variable_context)
    line_number = simple_command->line - function_line_number;

  /* Remember what this command line looks like at invocation. */
  command_string_index = 0;
  print_simple_command (simple_command);
  command_line = (char *)alloca (1 + strlen (the_printed_command));
  strcpy (command_line, the_printed_command);

  first_word_quoted =
    simple_command->words ? simple_command->words->word->quoted : 0;

  old_last_command_subst_pid = last_command_subst_pid;

  /* If we are re-running this as the result of executing the `command'
     builtin, do not expand the command words a second time. */
  if ((simple_command->flags & CMD_INHIBIT_EXPANSION) == 0)
    {
      current_fds_to_close = fds_to_close;
      words = expand_words (simple_command->words);
      current_fds_to_close = (struct fd_bitmap *)NULL;
    }
  else
    words = copy_word_list (simple_command->words);

  lastarg = (char *)NULL;

  /* It is possible for WORDS not to have anything left in it.
     Perhaps all the words consisted of `$foo', and there was
     no variable `$foo'. */
  if (words)
    {
      Function *builtin;
      SHELL_VAR *func;

      begin_unwind_frame ("simple-command");

      if (echo_command_at_execute)
	{
	  char *line = string_list (words);

	  if (line && *line)
	    fprintf (stderr, "%s%s\n", indirection_level_string (), line);

	  FREE (line);
	}

      if (simple_command->flags & CMD_NO_FUNCTIONS)
	func = (SHELL_VAR *)NULL;
      else
	func = find_function (words->word->word);

      add_unwind_protect (dispose_words, words);

      QUIT;

      /* Bind the last word in this command to "$_" after execution. */
      for (lastword = words; lastword->next; lastword = lastword->next);
      lastarg = lastword->word->word;

#if defined (JOB_CONTROL)
      /* Is this command a job control related thing? */
      if (words->word->word[0] == '%')
	{
	  int result;

	  if (async)
	    this_command_name = "bg";
	  else
	    this_command_name = "fg";

	  last_shell_builtin = this_shell_builtin;
	  this_shell_builtin = builtin_address (this_command_name);
	  result = (*this_shell_builtin) (words);
	  goto return_result;
	}

      /* One other possiblilty.  The user may want to resume an existing job.
	 If they do, find out whether this word is a candidate for a running
	 job. */
      {
	char *auto_resume_value = get_string_value ("auto_resume");

	if (auto_resume_value &&
	    !first_word_quoted &&
	    !words->next &&
	    words->word->word[0] &&
	    !simple_command->redirects &&
	    pipe_in == NO_PIPE &&
	    pipe_out == NO_PIPE &&
	    !async)
	  {
	    char *word = words->word->word;
	    register int i;
	    int wl, cl, exact, substring, match, started_status;
	    register PROCESS *p;

	    exact = STREQ (auto_resume_value, "exact");
	    substring = STREQ (auto_resume_value, "substring");
	    wl = strlen (word);
	    for (i = job_slots - 1; i > -1; i--)
	      {
		if (!jobs[i] || (JOBSTATE (i) != JSTOPPED))
		  continue;

		p = jobs[i]->pipe;
		do
		  {
		    if (exact)
		      {
		        cl = strlen (p->command);
		        match = STREQN (p->command, word, cl);
		      }
		    else if (substring)
		      match = strindex (p->command, word) != (char *)0;
		    else
		      match = STREQN (p->command, word, wl);

		    if (match == 0)
		      {
		        p = p->next;
		        continue;
		      }

		    run_unwind_frame ("simple-command");
		    last_shell_builtin = this_shell_builtin;
		    this_shell_builtin = builtin_address ("fg");

		    started_status = start_job (i, 1);

		    if (started_status < 0)
		      return (EXECUTION_FAILURE);
		    else
		      return (started_status);
		  }
		while (p != jobs[i]->pipe);
	      }
	  }
      }
#endif /* JOB_CONTROL */

      /* Remember the name of this command globally. */
      this_command_name = words->word->word;

      QUIT;

      /* This command could be a shell builtin or a user-defined function.
	 If so, and we have pipes, then fork a subshell in here.  Else, just
	 do the command. */

      if (func)
	builtin = (Function *)NULL;
      else
	builtin = find_shell_builtin (this_command_name);

      last_shell_builtin = this_shell_builtin;
      this_shell_builtin = builtin;

      if (builtin || func)
	{
	  if ((pipe_in != NO_PIPE) || (pipe_out != NO_PIPE) || async)
	    {
	      if (make_child (savestring (command_line), async) == 0)
		{
		  /* Cancel traps, in trap.c. */
		  restore_original_signals ();

		  if (async)
		    setup_async_signals ();

		  execute_subshell_builtin_or_function
		    (words, simple_command->redirects, builtin, func,
		     pipe_in, pipe_out, async, fds_to_close,
		     simple_command->flags);
		}
	      else
		{
		  close_pipes (pipe_in, pipe_out);
#if defined (PROCESS_SUBSTITUTION) && defined (HAVE_DEV_FD)
		  unlink_fifo_list ();
#endif
		  goto return_result;
		}
	    }
	  else
	    {
	      result = execute_builtin_or_function
		(words, builtin, func, simple_command->redirects, fds_to_close,
		 simple_command->flags);

	      goto return_result;
	    }
	}

      execute_disk_command (words, simple_command->redirects, command_line,
			    pipe_in, pipe_out, async, fds_to_close,
			    (simple_command->flags & CMD_NO_FORK));

      goto return_result;
    }
  else if (pipe_in != NO_PIPE || pipe_out != NO_PIPE || async)
    {
      /* We have a null command, but we really want a subshell to take
	 care of it.  Just fork, do piping and redirections, and exit. */
      if (make_child (savestring (""), async) == 0)
	{
	  /* Cancel traps, in trap.c. */
	  restore_original_signals ();

	  do_piping (pipe_in, pipe_out);

	  subshell_environment = 1;

	  if (do_redirections (simple_command->redirects, 1, 0, 0) == 0)
	    exit (EXECUTION_SUCCESS);
	  else
	    exit (EXECUTION_FAILURE);
	}
      else
	{
	  close_pipes (pipe_in, pipe_out);
#if defined (PROCESS_SUBSTITUTION) && defined (HAVE_DEV_FD)
	  unlink_fifo_list ();
#endif
	  result = EXECUTION_SUCCESS;
	  goto return_result;
	}
    }
  else
    {
      /* Even if there aren't any command names, pretend to do the
	 redirections that are specified.  The user expects the side
	 effects to take place.  If the redirections fail, then return
	 failure.  Otherwise, if a command substitution took place while
	 expanding the command or a redirection, return the value of that
	 substitution.  Otherwise, return EXECUTION_SUCCESS. */

      if (do_redirections (simple_command->redirects, 0, 0, 0) != 0)
	result = EXECUTION_FAILURE;
      else if (old_last_command_subst_pid != last_command_subst_pid)
	result = last_command_exit_value;
      else
	result = EXECUTION_SUCCESS;
    }

 return_result:
  bind_lastarg (lastarg);
  /* The unwind-protect frame is set up only if WORDS is not empty. */
  if (words)
    run_unwind_frame ("simple-command");
  return (result);
}

static int
execute_builtin (builtin, words, flags, subshell)
     Function *builtin;
     WORD_LIST *words;
     int flags, subshell;
{
  int old_e_flag = exit_immediately_on_error;
  int result;

  /* The eval builtin calls parse_and_execute, which does not know about
     the setting of flags, and always calls the execution functions with
     flags that will exit the shell on an error if -e is set.  If the
     eval builtin is being called, and we're supposed to ignore the exit
     value of the command, we turn the -e flag off ourselves, then
     restore it when the command completes. */
  if (subshell == 0 && builtin == eval_builtin && (flags & CMD_IGNORE_RETURN))
    {
      begin_unwind_frame ("eval_builtin");
      unwind_protect_int (exit_immediately_on_error);
      exit_immediately_on_error = 0;
    }

  /* The temporary environment for a builtin is supposed to apply to
     all commands executed by that builtin.  Currently, this is a
     problem only with the `source' builtin. */
  if (builtin == source_builtin)
    {
      if (subshell == 0)
	begin_unwind_frame ("builtin_env");

      if (temporary_env)
	{
	  builtin_env = copy_array (temporary_env);
	  if (subshell == 0)
	    add_unwind_protect (dispose_builtin_env, (char *)NULL);
	  dispose_used_env_vars ();
	}
#if 0
      else
	builtin_env = (char **)NULL;
#endif
    }

  result = ((*builtin) (words->next));

  if (subshell == 0 && builtin == source_builtin)
    {
      dispose_builtin_env ();
      discard_unwind_frame ("builtin_env");
    }

  if (subshell == 0 && builtin == eval_builtin && (flags & CMD_IGNORE_RETURN))
    {
      exit_immediately_on_error += old_e_flag;
      discard_unwind_frame ("eval_builtin");
    }

  return (result);
}

/* XXX -- why do we need to set up unwind-protects for the case where
   subshell == 1 at all? */
static int
execute_function (var, words, flags, fds_to_close, async, subshell)
     SHELL_VAR *var;
     WORD_LIST *words;
     int flags, subshell, async;
     struct fd_bitmap *fds_to_close;
{
  int return_val, result;
  COMMAND *tc, *fc;

  tc = (COMMAND *)copy_command (function_cell (var));
  if (tc && (flags & CMD_IGNORE_RETURN))
    tc->flags |= CMD_IGNORE_RETURN;

  if (subshell)
    begin_unwind_frame ("subshell_function_calling");
  else
    begin_unwind_frame ("function_calling");

  if (subshell == 0)
    {
      push_context ();
      add_unwind_protect (pop_context, (char *)NULL);
      unwind_protect_int (line_number);
    }
  else
    unwind_protect_int (variable_context);

  unwind_protect_int (loop_level);
  unwind_protect_int (return_catch_flag);
  unwind_protect_jmp_buf (return_catch);
  add_unwind_protect (dispose_command, (char *)tc);

  /* The temporary environment for a function is supposed to apply to
     all commands executed within the function body. */
  if (temporary_env)
    {
      function_env = copy_array (temporary_env);
      add_unwind_protect (dispose_function_env, (char *)NULL);
      dispose_used_env_vars ();
    }
#if 0
  else
    function_env = (char **)NULL;
#endif

  /* Note the second argument of "1", meaning that we discard
     the current value of "$*"!  This is apparently the right thing. */
  remember_args (words->next, 1);

  line_number = function_line_number = tc->line;

  if (subshell)
    {
#if defined (JOB_CONTROL)
      stop_pipeline (async, (COMMAND *)NULL);
#endif
      if (tc->type == cm_group)
	fc = tc->value.Group->command;
      else
	fc = tc;

      if (fc && (flags & CMD_IGNORE_RETURN))
	fc->flags |= CMD_IGNORE_RETURN;

      variable_context++;
    }
  else
    fc = tc;

  return_catch_flag++;
  return_val = setjmp (return_catch);

  if (return_val)
    result = return_catch_value;
  else
    result = execute_command_internal (fc, 0, NO_PIPE, NO_PIPE, fds_to_close);

  if (subshell)
    run_unwind_frame ("subshell_function_calling");
  else
    run_unwind_frame ("function_calling");

  return (result);
}

/* Execute a shell builtin or function in a subshell environment.  This
   routine does not return; it only calls exit().  If BUILTIN is non-null,
   it points to a function to call to execute a shell builtin; otherwise
   VAR points at the body of a function to execute.  WORDS is the arguments
   to the command, REDIRECTS specifies redirections to perform before the
   command is executed. */
static void
execute_subshell_builtin_or_function (words, redirects, builtin, var,
				      pipe_in, pipe_out, async, fds_to_close,
				      flags)
     WORD_LIST *words;
     REDIRECT *redirects;
     Function *builtin;
     SHELL_VAR *var;
     int pipe_in, pipe_out, async;
     struct fd_bitmap *fds_to_close;
     int flags;
{
  /* A subshell is neither a login shell nor interactive. */
  login_shell = interactive = 0;

  subshell_environment = 1;

  maybe_make_export_env ();

#if defined (JOB_CONTROL)
  /* Eradicate all traces of job control after we fork the subshell, so
     all jobs begun by this subshell are in the same process group as
     the shell itself. */

  /* Allow the output of `jobs' to be piped. */
  if (builtin == jobs_builtin && !async &&
      (pipe_out != NO_PIPE || pipe_in != NO_PIPE))
    kill_current_pipeline ();
  else
    without_job_control ();

  set_sigchld_handler ();
#endif /* JOB_CONTROL */

  set_sigint_handler ();

  do_piping (pipe_in, pipe_out);

  if (fds_to_close)
    close_fd_bitmap (fds_to_close);

  if (do_redirections (redirects, 1, 0, 0) != 0)
    exit (EXECUTION_FAILURE);

  if (builtin)
    {
      int result;

      /* Give builtins a place to jump back to on failure,
	 so we don't go back up to main(). */
      result = setjmp (top_level);

      if (result == EXITPROG)
	exit (last_command_exit_value);
      else if (result)
	exit (EXECUTION_FAILURE);
      else
	exit (execute_builtin (builtin, words, flags, 1));
    }
  else
    {
      exit (execute_function (var, words, flags, fds_to_close, async, 1));
    }
}

/* Execute a builtin or function in the current shell context.  If BUILTIN
   is non-null, it is the builtin command to execute, otherwise VAR points
   to the body of a function.  WORDS are the command's arguments, REDIRECTS
   are the redirections to perform.  FDS_TO_CLOSE is the usual bitmap of
   file descriptors to close.

   If BUILTIN is exec_builtin, the redirections specified in REDIRECTS are
   not undone before this function returns. */
static int
execute_builtin_or_function (words, builtin, var, redirects,
			     fds_to_close, flags)
     WORD_LIST *words;
     Function *builtin;
     SHELL_VAR *var;
     REDIRECT *redirects;
     struct fd_bitmap *fds_to_close;
     int flags;
{
  int result = EXECUTION_FAILURE;
  REDIRECT *saved_undo_list;

  if (do_redirections (redirects, 1, 1, 0) != 0)
    {
      cleanup_redirects (redirection_undo_list);
      redirection_undo_list = (REDIRECT *)NULL;
      dispose_exec_redirects ();
      return (EXECUTION_FAILURE);
    }

  saved_undo_list = redirection_undo_list;

  /* Calling the "exec" builtin changes redirections forever. */
  if (builtin == exec_builtin)
    {
      dispose_redirects (saved_undo_list);
      saved_undo_list = exec_redirection_undo_list;
      exec_redirection_undo_list = (REDIRECT *)NULL;
    }
  else
    dispose_exec_redirects ();

  if (saved_undo_list)
    {
      begin_unwind_frame ("saved redirects");
      add_unwind_protect (cleanup_func_redirects, (char *)saved_undo_list);
      add_unwind_protect (dispose_redirects, (char *)saved_undo_list);
    }

  redirection_undo_list = (REDIRECT *)NULL;

  if (builtin)
    result = execute_builtin (builtin, words, flags, 0);
  else
    result = execute_function (var, words, flags, fds_to_close, 0, 0);

  if (saved_undo_list)
    {
      redirection_undo_list = saved_undo_list;
      discard_unwind_frame ("saved redirects");
    }

  if (redirection_undo_list)
    {
      do_redirections (redirection_undo_list, 1, 0, 0);
      dispose_redirects (redirection_undo_list);
      redirection_undo_list = (REDIRECT *)NULL;
    }

  return (result);
}

void
setup_async_signals ()
{
#if defined (JOB_CONTROL)
  if (job_control == 0)
#endif
    {
      set_signal_handler (SIGINT, SIG_IGN);
      set_signal_ignored (SIGINT);
      set_signal_handler (SIGQUIT, SIG_IGN);
      set_signal_ignored (SIGQUIT);
    }
}

/* Execute a simple command that is hopefully defined in a disk file
   somewhere.

   1) fork ()
   2) connect pipes
   3) look up the command
   4) do redirections
   5) execve ()
   6) If the execve failed, see if the file has executable mode set.
   If so, and it isn't a directory, then execute its contents as
   a shell script.

   Note that the filename hashing stuff has to take place up here,
   in the parent.  This is probably why the Bourne style shells
   don't handle it, since that would require them to go through
   this gnarly hair, for no good reason.  */
static void
execute_disk_command (words, redirects, command_line, pipe_in, pipe_out,
		      async, fds_to_close, nofork)
     WORD_LIST *words;
     REDIRECT *redirects;
     char *command_line;
     int pipe_in, pipe_out, async;
     struct fd_bitmap *fds_to_close;
     int nofork;	/* Don't fork, just exec, if no pipes */
{
  register char *pathname;
  char *hashed_file, *command, **args;
  int pid, temp_path;
  SHELL_VAR *path;

  pathname = words->word->word;
#if defined (RESTRICTED_SHELL)
  if (restricted && strchr (pathname, '/'))
    {
      report_error ("%s: restricted: cannot specify `/' in command names",
		    pathname);
      last_command_exit_value = EXECUTION_FAILURE;
      return;
    }
#endif /* RESTRICTED_SHELL */

  hashed_file = command = (char *)NULL;

  /* If PATH is in the temporary environment for this command, don't use the
     hash table to search for the full pathname. */
  temp_path = 0;
  path = find_tempenv_variable ("PATH");
  if (path)
    temp_path = 1;

  /* Don't waste time trying to find hashed data for a pathname
     that is already completely specified. */

  if (!path && !absolute_program (pathname))
    hashed_file = find_hashed_filename (pathname);

  /* If a command found in the hash table no longer exists, we need to
     look for it in $PATH.  Thank you Posix.2.  This forces us to stat
     every command found in the hash table.  It seems pretty stupid to me,
     so I am basing it on the presence of POSIXLY_CORRECT. */

  if (hashed_file && posixly_correct)
    {
      int st;

      st = file_status (hashed_file);
      if ((st ^ (FS_EXISTS | FS_EXECABLE)) != 0)
	{
	  remove_hashed_filename (pathname);
	  hashed_file = (char *)NULL;
	}
    }

  if (hashed_file)
    command = savestring (hashed_file);
  else if (absolute_program (pathname))
    /* A command containing a slash is not looked up in PATH or saved in
       the hash table. */
    command = savestring (pathname);
  else
    {
      command = find_user_command (pathname);
      if (command && !hashing_disabled && !temp_path)
	remember_filename (pathname, command, dot_found_in_search, 1);
    }

  maybe_make_export_env ();

  if (command)
    put_command_name_into_env (command);

  /* We have to make the child before we check for the non-existance
     of COMMAND, since we want the error messages to be redirected. */
  /* If we can get away without forking and there are no pipes to deal with,
     don't bother to fork, just directly exec the command. */
  if (nofork && pipe_in == NO_PIPE && pipe_out == NO_PIPE)
    pid = 0;
  else
    pid = make_child (savestring (command_line), async);

  if (pid == 0)
    {
      int old_interactive;

      /* Cancel traps, in trap.c. */
      restore_original_signals ();

      /* restore_original_signals may have undone the work done
         by make_child to ensure that SIGINT and SIGQUIT are ignored
         in asynchronous children. */
      if (async)
	setup_async_signals ();

      do_piping (pipe_in, pipe_out);

      /* Execve expects the command name to be in args[0].  So we
	 leave it there, in the same format that the user used to
	 type it in. */
      args = make_word_array (words);

      if (async)
	{
	  old_interactive = interactive;
	  interactive = 0;
	}

      subshell_environment = 1;

      /* This functionality is now provided by close-on-exec of the
	 file descriptors manipulated by redirection and piping.
	 Some file descriptors still need to be closed in all children
	 because of the way bash does pipes; fds_to_close is a 
	 bitmap of all such file descriptors. */
      if (fds_to_close)
	close_fd_bitmap (fds_to_close);

      if (redirects && (do_redirections (redirects, 1, 0, 0) != 0))
	{
#if defined (PROCESS_SUBSTITUTION)
	  /* Try to remove named pipes that may have been created as the
	     result of redirections. */
	  unlink_fifo_list ();
#endif /* PROCESS_SUBSTITUTION */
	  exit (EXECUTION_FAILURE);
	}

      if (async)
	interactive = old_interactive;

      if (!command)
	{
	  report_error ("%s: command not found", args[0]);
	  exit (EX_NOTFOUND);	/* Posix.2 says the exit status is 127 */
	}

      exit (shell_execve (command, args, export_env));
    }
  else
    {
      /* Make sure that the pipes are closed in the parent. */
      close_pipes (pipe_in, pipe_out);
#if defined (PROCESS_SUBSTITUTION) && defined (HAVE_DEV_FD)
      unlink_fifo_list ();
#endif
      FREE (command);
    }
}

/* If the operating system on which we're running does not handle
   the #! executable format, then help out.  SAMPLE is the text read
   from the file, SAMPLE_LEN characters.  COMMAND is the name of
   the script; it and ARGS, the arguments given by the user, will
   become arguments to the specified interpreter.  ENV is the environment
   to pass to the interpreter.

   The word immediately following the #! is the interpreter to execute.
   A single argument to the interpreter is allowed. */
static int
execute_shell_script (sample, sample_len, command, args, env)
     unsigned char *sample;
     int sample_len;
     char *command;
     char **args, **env;
{
  register int i;
  char *execname, *firstarg;
  int start, size_increment, larry;

  /* Find the name of the interpreter to exec. */
  for (i = 2; whitespace (sample[i]) && i < sample_len; i++)
    ;

  for (start = i;
       !whitespace (sample[i]) && sample[i] != '\n' && i < sample_len;
       i++)
    ;

  execname = xmalloc (1 + (i - start));
  strncpy (execname, (char *) (sample + start), i - start);
  execname[i - start] = '\0';
  size_increment = 1;

  /* Now the argument, if any. */
  firstarg = (char *)NULL;
  for (start = i;
       whitespace (sample[i]) && sample[i] != '\n' && i < sample_len;
       i++)
    ;

  /* If there is more text on the line, then it is an argument for the
     interpreter. */
  if (i < sample_len && sample[i] != '\n' && !whitespace (sample[i]))
    {
      for (start = i;
	   !whitespace (sample[i]) && sample[i] != '\n' && i < sample_len;
	   i++)
	;
      firstarg = xmalloc (1 + (i - start));
      strncpy (firstarg, (char *)(sample + start), i - start);
      firstarg[i - start] = '\0';

      size_increment = 2;
    }

  larry = array_len (args) + size_increment;

  args = (char **)xrealloc ((char *)args, (1 + larry) * sizeof (char *));

  for (i = larry - 1; i; i--)
    args[i] = args[i - size_increment];

  args[0] = execname;
  if (firstarg)
    {
      args[1] = firstarg;
      args[2] = command;
    }
  else
    args[1] = command;

  args[larry] = (char *)NULL;

  return (shell_execve (execname, args, env));
}

/* Call execve (), handling interpreting shell scripts, and handling
   exec failures. */
int
shell_execve (command, args, env)
     char *command;
     char **args, **env;
{
#if defined (isc386) && defined (_POSIX_SOURCE)
  __setostype (0);		/* Turn on USGr3 semantics. */
  execve (command, args, env);
  __setostype (1);		/* Turn the POSIX semantics back on. */
#else
  execve (command, args, env);
#endif /* !(isc386 && _POSIX_SOURCE) */

  /* If we get to this point, then start checking out the file.
     Maybe it is something we can hack ourselves. */
  {
    struct stat finfo;

    if (errno != ENOEXEC)
      {
	if ((stat (command, &finfo) == 0) &&
	    (S_ISDIR (finfo.st_mode)))
	  report_error ("%s: is a directory", args[0]);
	else
	  file_error (command);

	return (EX_NOEXEC);	/* XXX Posix.2 says that exit status is 126 */
      }
    else
      {
	/* This file is executable.
	   If it begins with #!, then help out people with losing operating
	   systems.  Otherwise, check to see if it is a binary file by seeing
	   if the first line (or up to 30 characters) are in the ASCII set.
	   Execute the contents as shell commands. */
	int larray = array_len (args) + 1;
	int i, should_exec = 0;

	{
	  int fd = open (command, O_RDONLY);
	  if (fd != -1)
	    {
	      unsigned char sample[80];
	      int sample_len = read (fd, &sample[0], 80);

	      close (fd);

	      if (sample_len == 0)
		return (EXECUTION_SUCCESS);

	      /* Is this supposed to be an executable script?
	         If so, the format of the line is "#! interpreter [argument]".
		 A single argument is allowed.  The BSD kernel restricts
		 the length of the entire line to 32 characters (32 bytes
		 being the size of the BSD exec header), but we allow 80
		 characters. */

	      if (sample_len > 0 && sample[0] == '#' && sample[1] == '!')
		return (execute_shell_script
			(sample, sample_len, command, args, env));
	      else if ((sample_len != -1) &&
		       check_binary_file (sample, sample_len))
		{
		  report_error ("%s: cannot execute binary file", command);
		  return (EX_BINARY_FILE);
		}
	    }
	}
#if defined (JOB_CONTROL)
	/* Forget about the way that job control was working. We are
	   in a subshell. */
	without_job_control ();
#endif /* JOB_CONTROL */
#if defined (ALIAS)
	/* Forget about any aliases that we knew of.  We are in a subshell. */
	delete_all_aliases ();
#endif /* ALIAS */

#if defined (JOB_CONTROL)
	set_sigchld_handler ();
#endif /* JOB_CONTROL */
	set_sigint_handler ();

	/* Insert the name of this shell into the argument list. */
	args = (char **)xrealloc ((char *)args, (1 + larray) * sizeof (char *));

	for (i = larray - 1; i; i--)
	  args[i] = args[i - 1];

	args[0] = shell_name;
	args[1] = command;
	args[larray] = (char *)NULL;

	if (args[0][0] == '-')
	  args[0]++;

	if (should_exec)
	  {
	    struct stat finfo;

#if defined (isc386) && defined (_POSIX_SOURCE)
	    __setostype (0);	/* Turn on USGr3 semantics. */
	    execve (shell_name, args, env);
	    __setostype (1);	/* Turn the POSIX semantics back on. */
#else
	    execve (shell_name, args, env);
#endif /* isc386 && _POSIX_SOURCE */

	    /* Oh, no!  We couldn't even exec this! */
	    if ((stat (args[0], &finfo) == 0) && (S_ISDIR (finfo.st_mode)))
	      report_error ("%s: is a directory", args[0]);
	    else
	      file_error (args[0]);

	    return (EXECUTION_FAILURE);
	  }
	else
	  {
	    subshell_argc = larray;
	    subshell_argv = args;
	    subshell_envp = env;
	    longjmp (subshell_top_level, 1);
	  }
      }
  }
}

#if defined (PROCESS_SUBSTITUTION)
/* Currently unused */
void
close_all_files ()
{
  register int i, fd_table_size;

  fd_table_size = getdtablesize ();
  if (fd_table_size > 256)	/* clamp to a reasonable value */
  	fd_table_size = 256;

  for (i = 3; i < fd_table_size; i++)
    close (i);
}
#endif /* PROCESS_SUBSTITUTION */

static void
close_pipes (in, out)
     int in, out;
{
  if (in >= 0)
    close (in);
  if (out >= 0)
    close (out);
}

/* Redirect input and output to be from and to the specified pipes.
   NO_PIPE and REDIRECT_BOTH are handled correctly. */
static void
do_piping (pipe_in, pipe_out)
     int pipe_in, pipe_out;
{
  if (pipe_in != NO_PIPE)
    {
      if (dup2 (pipe_in, 0) < 0)
	internal_error ("cannot duplicate fd %d to fd 0: %s",
			pipe_in, strerror (errno));
      if (pipe_in > 0)
        close (pipe_in);
    }
  if (pipe_out != NO_PIPE)
    {
      if (pipe_out != REDIRECT_BOTH)
	{
	  if (dup2 (pipe_out, 1) < 0)
	    internal_error ("cannot duplicate fd %d to fd 1: %s",
			    pipe_out, strerror (errno));
	  if (pipe_out == 0 || pipe_out > 1)
	    close (pipe_out);
	}
      else
	dup2 (1, 2);
    }
}

#define AMBIGUOUS_REDIRECT  -1
#define NOCLOBBER_REDIRECT  -2
#define RESTRICTED_REDIRECT -3	/* Only can happen in restricted shells. */

/* Perform the redirections on LIST.  If FOR_REAL, then actually make
   input and output file descriptors, otherwise just do whatever is
   neccessary for side effecting.  INTERNAL says to remember how to
   undo the redirections later, if non-zero.  If SET_CLEXEC is non-zero,
   file descriptors opened in do_redirection () have their close-on-exec
   flag set. */
static int
do_redirections (list, for_real, internal, set_clexec)
     REDIRECT *list;
     int for_real, internal, set_clexec;
{
  register int error;
  register REDIRECT *temp = list;

  if (internal)
    {
      if (redirection_undo_list)
	{
	  dispose_redirects (redirection_undo_list);
	  redirection_undo_list = (REDIRECT *)NULL;
	}
      if (exec_redirection_undo_list)
	dispose_exec_redirects ();
    }

  while (temp)
    {
      error = do_redirection_internal (temp, for_real, internal, set_clexec);

      if (error)
	{
	  char *filename;

	  if (expandable_redirection_filename (temp))
	    {
	      if (posixly_correct && !interactive_shell)
	        disallow_filename_globbing++;
	      filename = redirection_expand (temp->redirectee.filename);
	      if (posixly_correct && !interactive_shell)
	        disallow_filename_globbing--;

	      if (!filename)
		filename = savestring ("");
	    }
	  else
	    filename = itos (temp->redirectee.dest);

	  switch (error)
	    {
	    case AMBIGUOUS_REDIRECT:
	      report_error ("%s: Ambiguous redirect", filename);
	      break;

	    case NOCLOBBER_REDIRECT:
	      report_error ("%s: Cannot clobber existing file", filename);
	      break;

#if defined (RESTRICTED_SHELL)
	    case RESTRICTED_REDIRECT:
	      report_error ("%s: output redirection restricted", filename);
	      break;
#endif /* RESTRICTED_SHELL */

	    default:
	      report_error ("%s: %s", filename, strerror (error));
	      break;
	    }

	  free (filename);
	  return (error);
	}

      temp = temp->next;
    }
  return (0);
}

/* Return non-zero if the redirection pointed to by REDIRECT has a
   redirectee.filename that can be expanded. */
static int
expandable_redirection_filename (redirect)
     REDIRECT *redirect;
{
  int result;

  switch (redirect->instruction)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:
    case r_input_output:
    case r_output_force:
    case r_duplicating_input_word:
    case r_duplicating_output_word:
      result = 1;
      break;

    default:
      result = 0;
    }
  return (result);
}

/* Expand the word in WORD returning a string.  If WORD expands to
   multiple words (or no words), then return NULL. */
char *
redirection_expand (word)
     WORD_DESC *word;
{
  char *result;
  WORD_LIST *tlist1, *tlist2;

  tlist1 = make_word_list (copy_word (word), (WORD_LIST *)NULL);
  tlist2 = expand_words_no_vars (tlist1);
  dispose_words (tlist1);

  if (!tlist2 || tlist2->next)
    {
      /* We expanded to no words, or to more than a single word.
	 Dispose of the word list and return NULL. */
      if (tlist2)
	dispose_words (tlist2);
      return ((char *)NULL);
    }
  result = string_list (tlist2);
  dispose_words (tlist2);
  return (result);
}

/* Do the specific redirection requested.  Returns errno in case of error.
   If FOR_REAL is zero, then just do whatever is neccessary to produce the
   appropriate side effects.   REMEMBERING, if non-zero, says to remember
   how to undo each redirection.  If SET_CLEXEC is non-zero, then
   we set all file descriptors > 2 that we open to be close-on-exec.  */
static int
do_redirection_internal (redirect, for_real, remembering, set_clexec)
     REDIRECT *redirect;
     int for_real, remembering, set_clexec;
{
  WORD_DESC *redirectee = redirect->redirectee.filename;
  int redir_fd = redirect->redirectee.dest;
  int fd, redirector = redirect->redirector;
  char *redirectee_word;
  enum r_instruction ri = redirect->instruction;
  REDIRECT *new_redirect;

  if (ri == r_duplicating_input_word || ri == r_duplicating_output_word)
    {
      /* We have [N]>&WORD or [N]<&WORD.  Expand WORD, then translate
	 the redirection into a new one and continue. */
      redirectee_word = redirection_expand (redirectee);

      if (redirectee_word[0] == '-' && redirectee_word[1] == '\0')
	{
	  rd.dest = 0L;
	  new_redirect = make_redirection (redirector, r_close_this, rd);
	}
      else if (all_digits (redirectee_word))
	{
	  if (ri == r_duplicating_input_word)
	    {
	      rd.dest = atol (redirectee_word);
	      new_redirect = make_redirection (redirector, r_duplicating_input, rd);
	    }
	  else
	    {
	      rd.dest = atol (redirectee_word);
	      new_redirect = make_redirection (redirector, r_duplicating_output, rd);
	    }
	}
      else if (ri == r_duplicating_output_word && redirector == 1)
	{
	  if (!posixly_correct)
	    {
	      rd.filename = make_word (redirectee_word);
	      new_redirect = make_redirection (1, r_err_and_out, rd);
	    }
	  else
	    new_redirect = copy_redirect (redirect);
	}
      else
	{
	  free (redirectee_word);
	  return (AMBIGUOUS_REDIRECT);
	}

      free (redirectee_word);

      /* Set up the variables needed by the rest of the function from the
	 new redirection. */
      if (new_redirect->instruction == r_err_and_out)
	{
	  char *alloca_hack;

	  /* Copy the word without allocating any memory that must be
	     explicitly freed. */
	  redirectee = (WORD_DESC *)alloca (sizeof (WORD_DESC));
	  xbcopy ((char *)new_redirect->redirectee.filename,
		 (char *)redirectee, sizeof (WORD_DESC));

	  alloca_hack = (char *)
	    alloca (1 + strlen (new_redirect->redirectee.filename->word));
	  redirectee->word = alloca_hack;
	  strcpy (redirectee->word, new_redirect->redirectee.filename->word);
	}
      else
	/* It's guaranteed to be an integer, and shouldn't be freed. */
	redirectee = new_redirect->redirectee.filename;

      redir_fd = new_redirect->redirectee.dest;
      redirector = new_redirect->redirector;
      ri = new_redirect->instruction;

      /* Overwrite the flags element of the old redirect with the new value. */
      redirect->flags = new_redirect->flags;
      dispose_redirects (new_redirect);
    }

  switch (ri)
    {
    case r_output_direction:
    case r_appending_to:
    case r_input_direction:
    case r_inputa_direction:
    case r_err_and_out:		/* command &>filename */
    case r_input_output:
    case r_output_force:

      if (posixly_correct && !interactive_shell)
	disallow_filename_globbing++;
      redirectee_word = redirection_expand (redirectee);
      if (posixly_correct && !interactive_shell)
	disallow_filename_globbing--;
      
      if (!redirectee_word)
	return (AMBIGUOUS_REDIRECT);

#if defined (RESTRICTED_SHELL)
      if (restricted && (ri == r_output_direction ||
			 ri == r_input_output ||
			 ri == r_err_and_out ||
			 ri == r_appending_to ||
			 ri == r_output_force))
	{
	  free (redirectee_word);
	  return (RESTRICTED_REDIRECT);
	}
#endif /* RESTRICTED_SHELL */

      /* If we are in noclobber mode, you are not allowed to overwrite
	 existing files.  Check first. */
      if (noclobber && (ri == r_output_direction ||
		      ri == r_input_output ||
		      ri == r_err_and_out))
	{
	  struct stat finfo;
	  int stat_result;

	  stat_result = stat (redirectee_word, &finfo);

	  if ((stat_result == 0) && (S_ISREG (finfo.st_mode)))
	    {
	      free (redirectee_word);
	      return (NOCLOBBER_REDIRECT);
	    }

	  /* If the file was not present, make sure we open it exclusively
	     so that if it is created before we open it, our open will fail. */
	  if (stat_result != 0)
	    redirect->flags |= O_EXCL;

	  fd = open (redirectee_word, redirect->flags, 0666);

	  if ((fd < 0) && (errno == EEXIST))
	    {
	      free (redirectee_word);
	      return (NOCLOBBER_REDIRECT);
	    }
	}
      else
	{
	  fd = open (redirectee_word, redirect->flags, 0666);
#if defined (AFS_CREATE_BUG)
	  if ((fd < 0) && (errno == EACCES))
	    fd = open (redirectee_word, (redirect->flags & ~O_CREAT), 0666);
#endif /* AFS_CREATE_BUG */
	}
      free (redirectee_word);

      if (fd < 0)
	return (errno);

      if (for_real)
	{
	  if (remembering)
	    /* Only setup to undo it if the thing to undo is active. */
	    if ((fd != redirector) && (fcntl (redirector, F_GETFD, 0) != -1))
	      add_undo_redirect (redirector);
	    else
	      add_undo_close_redirect (redirector);

#if defined (BUFFERED_INPUT)
	  check_bash_input (redirector);
#endif

	  if ((fd != redirector) && (dup2 (fd, redirector) < 0))
	    return (errno);

#if defined (BUFFERED_INPUT)
	  /* Do not change the buffered stream for an implicit redirection
	     of /dev/null to fd 0 for asynchronous commands without job
	     control (r_inputa_direction). */
	  if (ri == r_input_direction || ri == r_input_output)
	    duplicate_buffered_stream (fd, redirector);
#endif /* BUFFERED_INPUT */

	  /*
	   * If we're remembering, then this is the result of a while, for
	   * or until loop with a loop redirection, or a function/builtin
	   * executing in the parent shell with a redirection.  In the
	   * function/builtin case, we want to set all file descriptors > 2
	   * to be close-on-exec to duplicate the effect of the old
	   * for i = 3 to NOFILE close(i) loop.  In the case of the loops,
	   * both sh and ksh leave the file descriptors open across execs.
	   * The Posix standard mentions only the exec builtin.
	   */
	  if (set_clexec && (redirector > 2))
	    SET_CLOSE_ON_EXEC (redirector);
	}

      if (fd != redirector)
	{
#if defined (BUFFERED_INPUT)
	  if (ri == r_input_direction || ri == r_inputa_direction ||
	      ri == r_input_output)
	    close_buffered_fd (fd);
	  else
#endif /* !BUFFERED_INPUT */
	    close (fd);		/* Don't close what we just opened! */
	}

      /* If we are hacking both stdout and stderr, do the stderr
	 redirection here. */
      if (ri == r_err_and_out)
	{
	  if (for_real)
	    {
	      if (remembering)
		add_undo_redirect (2);
	      if (dup2 (1, 2) < 0)
		return (errno);
	    }
	}
      break;

    case r_reading_until:
    case r_deblank_reading_until:
      /* REDIRECTEE is a pointer to a WORD_DESC containing the text of
	 the new input.  Place it in a temporary file. */
      if (redirectee)
	{
	  char filename[40];
	  pid_t pid = getpid ();

	  /* Make the filename for the temp file. */
	  sprintf (filename, "/tmp/t%d-sh", pid);

	  fd = open (filename, O_TRUNC | O_WRONLY | O_CREAT, 0666);
	  if (fd < 0)
	    return (errno);

	  errno = 0;		/* XXX */
	  if (redirectee->word)
	    {
	      char *document;
	      int document_len;

	      /* Expand the text if the word that was specified had
		 no quoting.  The text that we expand is treated
		 exactly as if it were surrounded by double quotes. */

	      if (redirectee->quoted)
		{
		  document = redirectee->word;
		  document_len = strlen (document);
		  /* Set errno to something reasonable if the write fails. */
		  if (write (fd, document, document_len) < document_len)
		    {
		      if (errno == 0)
			errno = ENOSPC;
		      close (fd);
		      return (errno);
		    }
		}
	      else
		{
		  WORD_LIST *tlist;
		  tlist = expand_string (redirectee->word, Q_HERE_DOCUMENT);
		  if (tlist)
		    {
		      int fd2;
		      FILE *fp;
		      register WORD_LIST *t;

		      /* Try using buffered I/O (stdio) and writing a word
			 at a time, letting stdio do the work of buffering
			 for us rather than managing our own strings.  Most
			 stdios are not particularly fast, however -- this
			 may need to be reconsidered later. */
		      if ((fd2 = dup (fd)) < 0 ||
			  (fp = fdopen (fd2, "w")) == NULL)
			{
			  if (fd2 >= 0)
			    close (fd2);
			  close (fd);
			  return (errno);
			}
		      errno = 0;	/* XXX */
		      for (t = tlist; t; t = t->next)
			{
			  /* This is essentially the body of
			     string_list_internal expanded inline. */
			  document = t->word->word;
			  document_len = strlen (document);
			  if (t != tlist)
			    putc (' ', fp);	/* separator */
			  fwrite (document, document_len, 1, fp);
			  if (ferror (fp))
			    {
			      if (errno == 0)
				errno = ENOSPC;
			      break;
			    }
			}
		      fclose (fp);
		      dispose_words (tlist);
		    }
		}
	    }	  

	  close (fd);
	  if (errno)
	    return (errno);

	  /* Make the document really temporary.  Also make it the input. */
	  fd = open (filename, O_RDONLY, 0666);

	  if (unlink (filename) < 0 || fd < 0)
	    {
	      if (fd >= 0)
	        close (fd);
	      return (errno);
	    }

	  if (for_real)
	    {
	      if (remembering)
		/* Only setup to undo it if the thing to undo is active. */
		if ((fd != redirector) && (fcntl (redirector, F_GETFD, 0) != -1))
		  add_undo_redirect (redirector);
		else
		  add_undo_close_redirect (redirector);

#if defined (BUFFERED_INPUT)
	      check_bash_input (redirector);
#endif
	      if (dup2 (fd, redirector) < 0)
		{
		  close (fd);
		  return (errno);
		}

#if defined (BUFFERED_INPUT)
	      duplicate_buffered_stream (fd, redirector);
#endif

	      if (set_clexec && (redirector > 2))
		SET_CLOSE_ON_EXEC (redirector);
	    }

#if defined (BUFFERED_INPUT)
	  close_buffered_fd (fd);
#else
	  close (fd);
#endif
	}
      break;

    case r_duplicating_input:
    case r_duplicating_output:
      if (for_real && (redir_fd != redirector))
	{
	  if (remembering)
	    /* Only setup to undo it if the thing to undo is active. */
	    if (fcntl (redirector, F_GETFD, 0) != -1)
	      add_undo_redirect (redirector);
	    else
	      add_undo_close_redirect (redirector);

#if defined (BUFFERED_INPUT)
	  check_bash_input (redirector);
#endif
	  /* This is correct.  2>&1 means dup2 (1, 2); */
	  if (dup2 (redir_fd, redirector) < 0)
	    return (errno);

#if defined (BUFFERED_INPUT)
	  if (ri == r_duplicating_input)
	    duplicate_buffered_stream (redir_fd, redirector);
#endif /* BUFFERED_INPUT */

	  /* First duplicate the close-on-exec state of redirectee.  dup2
	     leaves the flag unset on the new descriptor, which means it
	     stays open.  Only set the close-on-exec bit for file descriptors
	     greater than 2 in any case, since 0-2 should always be open
	     unless closed by something like `exec 2<&-'. */
	  /* if ((already_set || set_unconditionally) && (ok_to_set))
		set_it () */
	  if (((fcntl (redir_fd, F_GETFD, 0) == 1) || set_clexec) &&
	       (redirector > 2))
	    SET_CLOSE_ON_EXEC (redirector);
	}
      break;

    case r_close_this:
      if (for_real)
	{
	  if (remembering && (fcntl (redirector, F_GETFD, 0) != -1))
	    add_undo_redirect (redirector);

#if defined (BUFFERED_INPUT)
	  check_bash_input (redirector);
	  close_buffered_fd (redirector);
#else /* !BUFFERED_INPUT */
	  close (redirector);
#endif /* !BUFFERED_INPUT */
	}
      break;
    }
  return (0);
}

#define SHELL_FD_BASE	10

/* Remember the file descriptor associated with the slot FD,
   on REDIRECTION_UNDO_LIST.  Note that the list will be reversed
   before it is executed.  Any redirections that need to be undone
   even if REDIRECTION_UNDO_LIST is discarded by the exec builtin
   are also saved on EXEC_REDIRECTION_UNDO_LIST. */
static int
add_undo_redirect (fd)
     int fd;
{
  int new_fd, clexec_flag;
  REDIRECT *new_redirect, *closer;

  new_fd = fcntl (fd, F_DUPFD, SHELL_FD_BASE);

  if (new_fd < 0)
    {
      file_error ("redirection error");
      return (-1);
    }
  else
    {
      REDIRECT *dummy_redirect;

      clexec_flag = fcntl (fd, F_GETFD, 0);

      rd.dest = 0L;
      closer = make_redirection (new_fd, r_close_this, rd);
      dummy_redirect = copy_redirects (closer);

      rd.dest = (long)new_fd;
      new_redirect = make_redirection (fd, r_duplicating_output, rd);
      new_redirect->next = closer;

      closer->next = redirection_undo_list;
      redirection_undo_list = new_redirect;

      /* Save redirections that need to be undone even if the undo list
	 is thrown away by the `exec' builtin. */
      add_exec_redirect (dummy_redirect);

      /* File descriptors used only for saving others should always be
	 marked close-on-exec.  Unfortunately, we have to preserve the
	 close-on-exec state of the file descriptor we are saving, since
	 fcntl (F_DUPFD) sets the new file descriptor to remain open
	 across execs.  If, however, the file descriptor whose state we
	 are saving is <= 2, we can just set the close-on-exec flag,
	 because file descriptors 0-2 should always be open-on-exec,
	 and the restore above in do_redirection() will take care of it. */
      if (clexec_flag || fd < 3)
	SET_CLOSE_ON_EXEC (new_fd);
    }
  return (0);
}

/* Set up to close FD when we are finished with the current command
   and its redirections. */
static void
add_undo_close_redirect (fd)
     int fd;
{
  REDIRECT *closer;

  rd.dest = 0L;
  closer = make_redirection (fd, r_close_this, rd);
  closer->next = redirection_undo_list;
  redirection_undo_list = closer;
}

static void
add_exec_redirect (dummy_redirect)
     REDIRECT *dummy_redirect;
{
  dummy_redirect->next = exec_redirection_undo_list;
  exec_redirection_undo_list = dummy_redirect;
}

intern_function (name, function)
     WORD_DESC *name;
     COMMAND *function;
{
  SHELL_VAR *var;

  if (!check_identifier (name, posixly_correct))
    return (EXECUTION_FAILURE);

  var = find_function (name->word);
  if (var && readonly_p (var))
    {
      report_error ("%s: readonly function", var->name);
      return (EXECUTION_FAILURE);
    }

  bind_function (name->word, function);
  return (EXECUTION_SUCCESS);
}

#define u_mode_bits(x) (((x) & 0000700) >> 6)
#define g_mode_bits(x) (((x) & 0000070) >> 3)
#define o_mode_bits(x) (((x) & 0000007) >> 0)
#define X_BIT(x) ((x) & 1)

/* Return some flags based on information about this file.
   The EXISTS bit is non-zero if the file is found.
   The EXECABLE bit is non-zero the file is executble.
   Zero is returned if the file is not found. */
int
file_status (name)
     char *name;
{
  struct stat finfo;
  static int user_id = -1;

  /* Determine whether this file exists or not. */
  if (stat (name, &finfo) < 0)
    return (0);

  /* If the file is a directory, then it is not "executable" in the
     sense of the shell. */
  if (S_ISDIR (finfo.st_mode))
    return (FS_EXISTS);

#if defined (AFS)
  /* We have to use access(2) to determine access because AFS does not
     support Unix file system semantics.  This may produce wrong
     answers for non-AFS files when ruid != euid.  I hate AFS. */
  if (access (name, X_OK) == 0)
    return (FS_EXISTS | FS_EXECABLE);
  else
    return (FS_EXISTS);
#else /* !AFS */

  /* Find out if the file is actually executable.  By definition, the
     only other criteria is that the file has an execute bit set that
     we can use. */
  if (user_id == -1)
    user_id = current_user.euid;

  /* Root only requires execute permission for any of owner, group or
     others to be able to exec a file. */
  if (user_id == 0)
    {
      int bits;

      bits = (u_mode_bits (finfo.st_mode) |
	      g_mode_bits (finfo.st_mode) |
	      o_mode_bits (finfo.st_mode));

      if (X_BIT (bits))
	return (FS_EXISTS | FS_EXECABLE);
    }

  /* If we are the owner of the file, the owner execute bit applies. */
  if (user_id == finfo.st_uid && X_BIT (u_mode_bits (finfo.st_mode)))
    return (FS_EXISTS | FS_EXECABLE);

  /* If we are in the owning group, the group permissions apply. */
  if (group_member (finfo.st_gid) && X_BIT (g_mode_bits (finfo.st_mode)))
    return (FS_EXISTS | FS_EXECABLE);

  /* If `others' have execute permission to the file, then so do we,
     since we are also `others'. */
  if (X_BIT (o_mode_bits (finfo.st_mode)))
    return (FS_EXISTS | FS_EXECABLE);
  else
    return (FS_EXISTS);
#endif /* !AFS */
}

/* Return non-zero if FILE exists and is executable.
   Note that this function is the definition of what an
   executable file is; do not change this unless YOU know
   what an executable file is. */
int
executable_file (file)
     char *file;
{
  return (file_status (file) & FS_EXECABLE);
}

/* DOT_FOUND_IN_SEARCH becomes non-zero when find_user_command ()
   encounters a `.' as the directory pathname while scanning the
   list of possible pathnames; i.e., if `.' comes before the directory
   containing the file of interest. */
int dot_found_in_search = 0;

/* Locate the executable file referenced by NAME, searching along
   the contents of the shell PATH variable.  Return a new string
   which is the full pathname to the file, or NULL if the file
   couldn't be found.  If a file is found that isn't executable,
   and that is the only match, then return that. */
char *
find_user_command (name)
     char *name;
{
  return (find_user_command_internal (name, FS_EXEC_PREFERRED));
}

/* Locate the file referenced by NAME, searching along the contents
   of the shell PATH variable.  Return a new string which is the full
   pathname to the file, or NULL if the file couldn't be found.  This
   returns the first file found. */
char *
find_path_file (name)
     char *name;
{
  return (find_user_command_internal (name, FS_EXISTS));
}

static char *
find_user_command_internal (name, flags)
     char *name;
     int flags;
{
  char *path_list;
  SHELL_VAR *var;

  /* Search for the value of PATH in both the temporary environment, and
     in the regular list of variables. */
  if (var = find_variable_internal ("PATH", 1))
    path_list = value_cell (var);
  else
    path_list = (char *)NULL;

  if (path_list == 0 || *path_list == '\0')
    return (savestring (name));

  return (find_user_command_in_path (name, path_list, flags));
}

/* Return the next element from PATH_LIST, a colon separated list of
   paths.  PATH_INDEX_POINTER is the address of an index into PATH_LIST;
   the index is modified by this function.
   Return the next element of PATH_LIST or NULL if there are no more. */
static char *
get_next_path_element (path_list, path_index_pointer)
     char *path_list;
     int *path_index_pointer;
{
  char *path;

  path = extract_colon_unit (path_list, path_index_pointer);

  if (!path)
    return (path);

  if (!*path)
    {
      free (path);
      path = savestring (".");
    }

  return (path);
}

char *
user_command_matches (name, flags, state)
     char *name;
     int flags, state;
{
  register int i;
  char *path_list;
  int  path_index;
  char *path_element;
  char *match;
  static char **match_list = NULL;
  static int match_list_size = 0;
  static int match_index = 0;

  if (!state)
    {
      /* Create the list of matches. */
      if (!match_list)
	{
	  match_list =
	    (char **) xmalloc ((match_list_size = 5) * sizeof(char *));

	  for (i = 0; i < match_list_size; i++)
	    match_list[i] = 0;
	}

      /* Clear out the old match list. */
      for (i = 0; i < match_list_size; i++)
	match_list[i] = NULL;

      /* We haven't found any files yet. */
      match_index = 0;

      path_list = get_string_value ("PATH");
      path_index = 0;

      while (path_list && path_list[path_index])
	{
	  path_element = get_next_path_element (path_list, &path_index);

	  if (!path_element)
	    break;

	  match = find_user_command_in_path (name, path_element, flags);

	  free (path_element);

	  if (!match)
	    continue;

	  if (match_index + 1 == match_list_size)
	    match_list = (char **)xrealloc
	      (match_list, ((match_list_size += 10) + 1) * sizeof (char *));
	  match_list[match_index++] = match;
	  match_list[match_index] = (char *)NULL;
	}

      /* We haven't returned any strings yet. */
      match_index = 0;
    }

  match = match_list[match_index];

  if (match)
    match_index++;

  return (match);
}

/* Return 1 if PATH1 and PATH2 are the same file.  This is kind of
   expensive.  If non-NULL STP1 and STP2 point to stat structures
   corresponding to PATH1 and PATH2, respectively. */
int
same_file (path1, path2, stp1, stp2)
     char *path1, *path2;
     struct stat *stp1, *stp2;
{
  struct stat st1, st2;

  if (stp1 == NULL)
    {
      if (stat (path1, &st1) != 0)
	return (0);
      stp1 = &st1;
    }

  if (stp2 == NULL)
    {
      if (stat (path2, &st2) != 0)
	return (0);
      stp2 = &st2;
    }

  return ((stp1->st_dev == stp2->st_dev) && (stp1->st_ino == stp2->st_ino));
}

/* Turn PATH, a directory, and NAME, a filename, into a full pathname.
   This allocates new memory and returns it. */
static char *
make_full_pathname (path, name, name_len)
     char *path, *name;
     int name_len;
{
  char *full_path;
  int path_len;

  path_len = strlen (path);
  full_path = xmalloc (2 + path_len + name_len);
  strcpy (full_path, path);
  full_path[path_len] = '/';
  strcpy (full_path + path_len + 1, name);
  return (full_path);
}
  
/* This does the dirty work for find_path_file () and find_user_command ().
   NAME is the name of the file to search for.
   PATH_LIST is a colon separated list of directories to search.
   FLAGS contains bit fields which control the files which are eligible.
   Some values are:
      FS_EXEC_ONLY:		The file must be an executable to be found.
      FS_EXEC_PREFERRED:	If we can't find an executable, then the
				the first file matching NAME will do.
      FS_EXISTS:		The first file found will do.
*/
static char *
find_user_command_in_path (name, path_list, flags)
     char *name;
     char *path_list;
     int flags;
{
  char *full_path, *path, *file_to_lose_on;
  int status, path_index, name_len;
  struct stat finfo;

  name_len = strlen (name);

  /* The file name which we would try to execute, except that it isn't
     possible to execute it.  This is the first file that matches the
     name that we are looking for while we are searching $PATH for a
     suitable one to execute.  If we cannot find a suitable executable
     file, then we use this one. */
  file_to_lose_on = (char *)NULL;

  /* We haven't started looking, so we certainly haven't seen
     a `.' as the directory path yet. */
  dot_found_in_search = 0;

  if (absolute_program (name))
    {
      full_path = xmalloc (1 + name_len);
      strcpy (full_path, name);

      status = file_status (full_path);

      /* If the file doesn't exist, quit now. */
      if (!(status & FS_EXISTS))
	{
	  free (full_path);
	  return ((char *)NULL);
	}

      /* If we only care about whether the file exists or not, return
	 this filename. */
      if (flags & FS_EXISTS)
	return (full_path);

      /* Otherwise, maybe we care about whether this file is executable.
	 If it is, and that is what we want, return it. */
      if ((flags & FS_EXEC_ONLY) && (status & FS_EXECABLE))
	return (full_path);
      else
	{
	  free (full_path);
	  return ((char *)NULL);
	}
    }

  /* Find out the location of the current working directory. */
  stat (".", &finfo);

  path_index = 0;
  while (path_list && path_list[path_index])
    {
      /* Allow the user to interrupt out of a lengthy path search. */
      QUIT;

      path = get_next_path_element (path_list, &path_index);

      if (!path)
	break;

      if (*path == '~')
	{
	  char *t = tilde_expand (path);
	  free (path);
	  path = t;
	}

      /* Remember the location of "." in the path, in all its forms
	 (as long as they begin with a `.', e.g. `./.') */
      if (!dot_found_in_search && (*path == '.') &&
	  same_file (".", path, &finfo, (struct stat *)NULL))
	dot_found_in_search = 1;

      full_path = make_full_pathname (path, name, name_len);
      free (path);

      status = file_status (full_path);

      if (!(status & FS_EXISTS))
	goto next_file;

      /* The file exists.  If the caller simply wants the first file,
	 here it is. */
      if (flags & FS_EXISTS)
	return (full_path);

       /* If the file is executable, then it satisfies the cases of
	  EXEC_ONLY and EXEC_PREFERRED.  Return this file unconditionally. */
      if (status & FS_EXECABLE)
	{
	  FREE (file_to_lose_on);

	  return (full_path);
	}

      /* The file is not executable, but it does exist.  If we prefer
	 an executable, then remember this one if it is the first one
	 we have found. */
      if (flags & FS_EXEC_PREFERRED)
	{
	  if (!file_to_lose_on)
	    file_to_lose_on = savestring (full_path);
	}

    next_file:
      free (full_path);
    }

  /* We didn't find exactly what the user was looking for.  Return
     the contents of FILE_TO_LOSE_ON which is NULL when the search
     required an executable, or non-NULL if a file was found and the
     search would accept a non-executable as a last resort. */
  return (file_to_lose_on);
}

/* Given a string containing units of information separated by colons,
   return the next one pointed to by (P_INDEX), or NULL if there are no more.
   Advance (P_INDEX) to the character after the colon. */
char *
extract_colon_unit (string, p_index)
     char *string;
     int *p_index;
{
  int i, start;

  i = *p_index;

  if (!string || (i >= (int)strlen (string)))
    return ((char *)NULL);

  /* Each call to this routine leaves the index pointing at a colon if
     there is more to the path.  If I is > 0, then increment past the
     `:'.  If I is 0, then the path has a leading colon.  Trailing colons
     are handled OK by the `else' part of the if statement; an empty
     string is returned in that case. */
  if (i && string[i] == ':')
    i++;

  start = i;

  while (string[i] && string[i] != ':') i++;

  *p_index = i;

  if (i == start)
    {
      if (string[i])
	(*p_index)++;

      /* Return "" in the case of a trailing `:'. */
      return (savestring (""));
    }
  else
    {
      char *value;

      value = xmalloc (1 + i - start);
      strncpy (value, string + start, i - start);
      value [i - start] = '\0';

      return (value);
    }
}

/* Return non-zero if the characters from SAMPLE are not all valid
   characters to be found in the first line of a shell script.  We
   check up to the first newline, or SAMPLE_LEN, whichever comes first.
   All of the characters must be printable or whitespace. */

#if !defined (isspace)
#define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\f')
#endif

#if !defined (isprint)
#define isprint(c) (isletter(c) || digit(c) || ispunct(c))
#endif

int
check_binary_file (sample, sample_len)
     unsigned char *sample;
     int sample_len;
{
  register int i;

  for (i = 0; i < sample_len; i++)
    {
      if (sample[i] == '\n')
	break;

      if (!isspace (sample[i]) && !isprint (sample[i]))
	return (1);
    }
  return (0);
}
