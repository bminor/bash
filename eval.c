/* eval.c -- reading and evaluating commands.

   Copyright (C) 1996 Free Software Foundation, Inc.

   This file is part of GNU Bash.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY.  No author or distributor accepts responsibility to
   anyone for the consequences of using it or for whether it serves
   any particular purpose or works at all, unless he says so in
   writing.  Refer to the GNU Emacs General Public License for full
   details.

   Everyone is granted permission to copy, modify and redistribute
   Bash, but only under the conditions described in the GNU General
   Public License.  A copy of this license is supposed to have been
   given to you along with GNU Emacs so you can know your rights and
   responsibilities.  It should be in a file named COPYING.

   Among other things, the copyright notice and this notice must be
   preserved on all copies. */

#include "config.h"

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdio.h>

#include "shell.h"
#include "flags.h"
#include "trap.h"

#include "builtins/common.h"

#include "input.h"
#include "execute_cmd.h"

#if defined (HISTORY)
#  include "bashhist.h"
#endif

extern int yyparse ();

extern int EOF_reached;
extern int indirection_level, interactive, interactive_shell;
extern int subshell_environment, running_under_emacs;
extern int last_command_exit_value, stdin_redir;
extern int need_here_doc;
extern int current_command_number, current_command_line_count, line_number;
extern char *ps1_prompt, **prompt_string_pointer;
extern int expand_aliases;

/* Read and execute commands until EOF is reached.  This assumes that
   the input source has already been initialized. */
int
reader_loop ()
{
  int our_indirection_level;
  COMMAND *current_command = (COMMAND *)NULL;

  our_indirection_level = ++indirection_level;

  while (EOF_Reached == 0)
    {
      int code;

      code = setjmp (top_level);

#if defined (PROCESS_SUBSTITUTION)
      unlink_fifo_list ();
#endif /* PROCESS_SUBSTITUTION */

      if (interactive_shell && signal_is_ignored (SIGINT) == 0)
	set_signal_handler (SIGINT, sigint_sighandler);

      if (code != NOT_JUMPED)
	{
	  indirection_level = our_indirection_level;

	  switch (code)
	    {
	      /* Some kind of throw to top_level has occured. */
	    case FORCE_EOF:
	    case EXITPROG:
	      current_command = (COMMAND *)NULL;
	      EOF_Reached = EOF;
	      goto exec_done;

	    case DISCARD:
	      last_command_exit_value = 1;
	      if (subshell_environment)
		{
		  current_command = (COMMAND *)NULL;
		  EOF_Reached = EOF;
		  goto exec_done;
		}
	      /* Obstack free command elements, etc. */
	      if (current_command)
		{
		  dispose_command (current_command);
		  current_command = (COMMAND *)NULL;
		}
	      break;

	    default:
	      command_error ("reader_loop", CMDERR_BADJUMP, code, 0);
	    }
	}

      executing = 0;
      dispose_used_env_vars ();

#if (defined (ultrix) && defined (mips)) || defined (C_ALLOCA)
      /* Attempt to reclaim memory allocated with alloca (). */
      (void) alloca (0);
#endif

      if (read_command () == 0)
	{
	  if (interactive_shell == 0 && read_but_dont_execute)
	    {
	      last_command_exit_value = EXECUTION_SUCCESS;
	      dispose_command (global_command);
	      global_command = (COMMAND *)NULL;
	    }
	  else if (current_command = global_command)
	    {
	      global_command = (COMMAND *)NULL;
	      current_command_number++;

	      executing = 1;
	      stdin_redir = 0;
	      execute_command (current_command);

	    exec_done:
	      if (current_command)
	        {
		  dispose_command (current_command);
		  current_command = (COMMAND *)NULL;
	        }

	      QUIT;
	    }
	}
      else
	{
	  /* Parse error, maybe discard rest of stream if not interactive. */
	  if (interactive == 0)
	    EOF_Reached = EOF;
	}
      if (just_one_command)
	EOF_Reached = EOF;
    }
  indirection_level--;
  return (last_command_exit_value);
}

static sighandler
alrm_catcher(i)
     int i;
{
  printf ("\007timed out waiting for input: auto-logout\n");
  jump_to_top_level (EXITPROG);
  SIGRETURN (0);
}

/* Send an escape sequence to emacs term mode to tell it the
   current working directory. */
static void
send_pwd_to_eterm ()
{
  char *pwd;

  pwd = get_string_value ("PWD");
  if (pwd == 0)
    pwd = get_working_directory ("eterm");
  fprintf (stderr, "\032/%s\n", pwd);
}

/* Call the YACC-generated parser and return the status of the parse.
   Input is read from the current input stream (bash_input).  yyparse
   leaves the parsed command in the global variable GLOBAL_COMMAND.
   This is where PROMPT_COMMAND is executed. */
int
parse_command ()
{
  int r;
  char *command_to_execute;

  need_here_doc = 0;
  run_pending_traps ();

  /* Allow the execution of a random command just before the printing
     of each primary prompt.  If the shell variable PROMPT_COMMAND
     is set then the value of it is the command to execute. */
  if (interactive && bash_input.type != st_string)
    {
      command_to_execute = get_string_value ("PROMPT_COMMAND");
      if (command_to_execute)
	execute_prompt_command (command_to_execute);

      if (running_under_emacs == 2)
	send_pwd_to_eterm ();	/* Yuck */
    }

  current_command_line_count = 0;
  r = yyparse ();

  if (need_here_doc)
    gather_here_documents ();

  return (r);
}

/* Read and parse a command, returning the status of the parse.  The command
   is left in the globval variable GLOBAL_COMMAND for use by reader_loop.
   This is where the shell timeout code is executed. */
int
read_command ()
{
  SHELL_VAR *tmout_var;
  int tmout_len, result;
  SigHandler *old_alrm;

  prompt_string_pointer = &ps1_prompt;
  global_command = (COMMAND *)NULL;

  /* Only do timeouts if interactive. */
  tmout_var = (SHELL_VAR *)NULL;
  tmout_len = 0;

  if (interactive)
    {
      tmout_var = find_variable ("TMOUT");
      old_alrm = (SigHandler *)NULL;

      if (tmout_var && tmout_var->value)
	{
	  tmout_len = atoi (tmout_var->value);
	  if (tmout_len > 0)
	    {
	      old_alrm = set_signal_handler (SIGALRM, alrm_catcher);
	      alarm (tmout_len);
	    }
	}
    }

  QUIT;

  current_command_line_count = 0;
  result = parse_command ();

  if (interactive && tmout_var && (tmout_len > 0))
    {
      alarm(0);
      set_signal_handler (SIGALRM, old_alrm);
    }

  return (result);
}

/* Take a string and run it through the shell parser, returning the
   resultant word list.  Used by compound array assignment. */
WORD_LIST *
parse_string_to_word_list (s, whom)
     char *s, *whom;
{
  WORD_LIST *wl;
  COMMAND *saved_global;
#if defined (HISTORY)
  int old_remember_on_history, old_history_expansion_inhibited;
#endif

#if defined (HISTORY)
  old_remember_on_history = remember_on_history;
#  if defined (BANG_HISTORY)
  old_history_expansion_inhibited = history_expansion_inhibited;
#  endif
  bash_history_disable ();
#endif

  push_stream (1);

  saved_global = global_command;
  global_command = (COMMAND *)0;

  with_input_from_string (s, whom);
  if (parse_command () != 0 || global_command == 0 || global_command->type != cm_simple)
    {
      if (global_command)
	dispose_command (global_command);
      wl = (WORD_LIST *)NULL;
    }
  else
    {
      wl = global_command->value.Simple->words;
      free (global_command->value.Simple);
      free (global_command);
    }

  global_command = saved_global;

  pop_stream ();

#if defined (HISTORY)
  remember_on_history = old_remember_on_history;
#  if defined (BANG_HISTORY)
  history_expansion_inhibited = old_history_expansion_inhibited;
#  endif /* BANG_HISTORY */
#endif /* HISTORY */

  return (wl);
}
