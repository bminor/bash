/* shell.c -- GNU's idea of the POSIX shell specification.

   This file is part of Bash, the Bourne Again SHell.  Bash is free
   software; no one can prevent you from reading the source code, or
   giving it to someone else.  This file is copyrighted under the GNU
   General Public License, which can be found in the file called
   COPYING.

   Copyright (C) 1988, 1991 Free Software Foundation, Inc.

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
   preserved on all copies.

  Birthdate:
  Sunday, January 10th, 1988.
  Initial author: Brian Fox
*/
#define INSTALL_DEBUG_MODE

#include "config.h"

#include "bashtypes.h"
#ifndef _MINIX
#  include <sys/file.h>
#endif
#include "posixstat.h"
#include "bashansi.h"
#include <stdio.h>
#include <signal.h>
#include <errno.h>
#include "filecntl.h"
#include <pwd.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "shell.h"
#include "flags.h"
#include "trap.h"
#include "mailcheck.h"
#include "builtins.h"
#include "builtins/common.h"

#if defined (JOB_CONTROL)
#include "jobs.h"
#endif /* JOB_CONTROL */

#include "input.h"
#include "execute_cmd.h"
#include "findcmd.h"

#if defined (HISTORY)
#  include "bashhist.h"
#  include <readline/history.h>
#endif

#include <tilde/tilde.h>
#include <glob/fnmatch.h>

#if !defined (HAVE_GETPW_DECLS)
extern struct passwd *getpwuid ();
#endif /* !HAVE_GETPW_DECLS */

#if !defined (errno)
extern int errno;
#endif

extern char *dist_version, *release_status;
extern int patch_level, build_version;
extern int subshell_environment;
extern int last_command_exit_value;
extern int line_number;
extern char *primary_prompt, *secondary_prompt;
extern int expand_aliases;
extern char *this_command_name;
extern int array_needs_making;

/* Non-zero means that this shell has already been run; i.e. you should
   call shell_reinitialize () if you need to start afresh. */
int shell_initialized = 0;

COMMAND *global_command = (COMMAND *)NULL;

/* Information about the current user. */
struct user_info current_user =
{
  -1, -1, -1, -1, (char *)NULL, (char *)NULL, (char *)NULL
};

/* The current host's name. */
char *current_host_name = (char *)NULL;

/* Non-zero means that this shell is a login shell.
   Specifically:
   0 = not login shell.
   1 = login shell from getty (or equivalent fake out)
  -1 = login shell from "-login" flag.
  -2 = both from getty, and from flag.
 */
int login_shell = 0;

/* Non-zero means that at this moment, the shell is interactive.  In
   general, this means that the shell is at this moment reading input
   from the keyboard. */
int interactive = 0;

/* Non-zero means that the shell was started as an interactive shell. */
int interactive_shell = 0;

/* Non-zero means to send a SIGHUP to all jobs when an interactive login
   shell exits. */
int hup_on_exit = 0;

/* Tells what state the shell was in when it started:
	0 = non-interactive shell script
	1 = interactive
	2 = -c command
   This is a superset of the information provided by interactive_shell.
*/
int startup_state = 0;

/* Special debugging helper. */
int debugging_login_shell = 0;

/* The environment that the shell passes to other commands. */
char **shell_environment;

/* Non-zero when we are executing a top-level command. */
int executing = 0;

/* The number of commands executed so far. */
int current_command_number = 1;

/* Non-zero is the recursion depth for commands. */
int indirection_level = 0;

/* The name of this shell, as taken from argv[0]. */
char *shell_name = (char *)NULL;

/* time in seconds when the shell was started */
time_t shell_start_time;

/* Are we running in an emacs shell window? */
int running_under_emacs;

/* The name of the .(shell)rc file. */
static char *bashrc_file = "~/.bashrc";

/* Non-zero means to act more like the Bourne shell on startup. */
static int act_like_sh;

/* Non-zero if this shell is being run by `su'. */
static int su_shell;

/* Non-zero if we have already expanded and sourced $ENV. */
static int sourced_env;

/* Is this shell running setuid? */
static int running_setuid;

/* Values for the long-winded argument names. */
static int debugging;			/* Do debugging things. */
static int no_rc;			/* Don't execute ~/.bashrc */
static int no_profile;			/* Don't execute .profile */
static int do_version;			/* Display interesting version info. */
static int make_login_shell;		/* Make this shell be a `-bash' shell. */
static int want_initial_help;		/* --help option */

int no_line_editing = 0;	/* Don't do fancy line editing. */
int posixly_correct = 0;	/* Non-zero means posix.2 superset. */
int dump_translatable_strings;	/* Dump strings in $"...", don't execute. */
int dump_po_strings;		/* Dump strings in $"..." in po format */
int wordexp_only = 0;		/* Do word expansion only */

/* Some long-winded argument names.  These are obviously new. */
#define Int 1
#define Charp 2
struct {
  char *name;
  int type;
  int *int_value;
  char **char_value;
} long_args[] = {
  { "debug", Int, &debugging, (char **)0x0 },
  { "dump-po-strings", Int, &dump_po_strings, (char **)0x0 },
  { "dump-strings", Int, &dump_translatable_strings, (char **)0x0 },
  { "help", Int, &want_initial_help, (char **)0x0 },
  { "login", Int, &make_login_shell, (char **)0x0 },
  { "noediting", Int, &no_line_editing, (char **)0x0 },
  { "noprofile", Int, &no_profile, (char **)0x0 },
  { "norc", Int, &no_rc, (char **)0x0 },
  { "posix", Int, &posixly_correct, (char **)0x0 },
  { "rcfile", Charp, (int *)0x0, &bashrc_file },
#if defined (RESTRICTED_SHELL)
  { "restricted", Int, &restricted, (char **)0x0 },
#endif
  { "verbose", Int, &echo_input_at_read, (char **)0x0 },
  { "version", Int, &do_version, (char **)0x0 },
  { "wordexp", Int, &wordexp_only, (char **)0x0 },
  { (char *)0x0, Int, (int *)0x0, (char **)0x0 }
};

/* These are extern so execute_simple_command can set them, and then
   longjmp back to main to execute a shell script, instead of calling
   main () again and resulting in indefinite, possibly fatal, stack
   growth. */
procenv_t subshell_top_level;
int subshell_argc;
char **subshell_argv;
char **subshell_envp;

#if defined (BUFFERED_INPUT)
/* The file descriptor from which the shell is reading input. */
int default_buffered_input = -1;
#endif

static int read_from_stdin;		/* -s flag supplied */
static int want_pending_command;	/* -c flag supplied */
static char *local_pending_command;

static FILE *default_input;

static int parse_long_options ();
static int parse_shell_options ();
static void run_startup_files ();
static int bind_args ();
static int open_shell_script ();
static void set_bash_input ();
static int run_one_command ();
static int run_wordexp ();

static int uidget ();
static int isnetconn ();

static void init_interactive (), init_noninteractive ();
static void set_shell_name ();
static void shell_initialize ();
static void shell_reinitialize ();

static void show_shell_usage ();

#ifdef __CYGWIN32__
static void
_cygwin32_check_tmp ()
{
  struct stat sb;

  if (stat ("/tmp", &sb) < 0)
    internal_warning ("could not find /tmp, please create!");
  else
    {
      if (S_ISDIR (sb.st_mode) == 0)
	internal_warning ("/tmp must be a valid directory name");
    }
}
#endif /* __CYGWIN32__ */

int
main (argc, argv, env)
     int argc;
     char **argv, **env;
{
  register int i;
  int code, saverst;
  volatile int locally_skip_execution;
  volatile int arg_index, top_level_arg_index;

  /* Catch early SIGINTs. */
  code = setjmp (top_level);
  if (code)
    exit (2);

  check_dev_tty ();

#ifdef __CYGWIN32__
  _cygwin32_check_tmp ();
#endif

  /* Wait forever if we are debugging a login shell. */
  while (debugging_login_shell);

  set_default_locale ();

  running_setuid = uidget ();

  posixly_correct = (getenv ("POSIXLY_CORRECT") != (char *)NULL) ||
		    (getenv ("POSIX_PEDANTIC") != (char *)NULL);

#if defined (USE_GNU_MALLOC_LIBRARY)
  mcheck (programming_error, (void (*) ())0);
#endif /* USE_GNU_MALLOC_LIBRARY */

  if (setjmp (subshell_top_level))
    {
      argc = subshell_argc;
      argv = subshell_argv;
      env = subshell_envp;
      sourced_env = 0;
    }

  /* Initialize `local' variables for all `invocations' of main (). */
  arg_index = 1;
  local_pending_command = (char *)NULL;
  want_pending_command = locally_skip_execution = read_from_stdin = 0;
  default_input = stdin;
#if defined (BUFFERED_INPUT)
  default_buffered_input = -1;
#endif

  /* Fix for the `infinite process creation' bug when running shell scripts
     from startup files on System V. */
  login_shell = make_login_shell = 0;

  /* If this shell has already been run, then reinitialize it to a
     vanilla state. */
  if (shell_initialized || shell_name)
    {
      /* Make sure that we do not infinitely recurse as a login shell. */
      if (*shell_name == '-')
	shell_name++;

      shell_reinitialize ();
      if (setjmp (top_level))
	exit (2);
    }

  shell_environment = env;
  set_shell_name (argv[0]);
  shell_start_time = NOW;	/* NOW now defined in general.h */

  /* Parse argument flags from the input line. */

  /* Find full word arguments first. */
  arg_index = parse_long_options (argv, arg_index, argc);

  if (want_initial_help)
    {
      show_shell_usage (stdout, 1);
      exit (EXECUTION_SUCCESS);
    }

  if (do_version)
    {
      show_shell_version (1);
      exit (EXECUTION_SUCCESS);
    }

  /* If user supplied the "--login" flag, then set and invert LOGIN_SHELL. */
  if (make_login_shell)
    {
      login_shell++;
      login_shell = -login_shell;
    }

  /* All done with full word options; do standard shell option parsing.*/
  this_command_name = shell_name;	/* for error reporting */
  arg_index = parse_shell_options (argv, arg_index, argc);

  if (dump_po_strings)
    dump_translatable_strings = 1;

  if (dump_translatable_strings)
    read_but_dont_execute = 1;

  if (running_setuid && privileged_mode == 0)
    disable_priv_mode ();

  /* Need to get the argument to a -c option processed in the
     above loop.  The next arg is a command to execute, and the
     following args are $0...$n respectively. */
  if (want_pending_command)
    {
      local_pending_command = argv[arg_index];
      if (local_pending_command == 0)
	{
	  report_error ("option `-c' requires an argument");
	  exit (EX_USAGE);
	}
      arg_index++;
    }
  this_command_name = (char *)NULL;

  /* First, let the outside world know about our interactive status.
     A shell is interactive if the `-i' flag was given, or if all of
     the following conditions are met:
	no -c command
	no arguments remaining or the -s flag given
	standard input is a terminal
	standard output is a terminal
     Refer to Posix.2, the description of the `sh' utility. */

  if (forced_interactive ||		/* -i flag */
      (!local_pending_command &&	/* No -c command and ... */
       wordexp_only == 0 &&		/* No --wordexp and ... */
       ((arg_index == argc) ||		/*   no remaining args or... */
	read_from_stdin) &&		/*   -s flag with args, and */
       isatty (fileno (stdin)) &&	/* Input is a terminal and */
       isatty (fileno (stdout))))	/* output is a terminal. */
    init_interactive ();
  else
    init_noninteractive ();

#define CLOSE_FDS_AT_LOGIN
#if defined (CLOSE_FDS_AT_LOGIN)
  /*
   * Some systems have the bad habit of starting login shells with lots of open
   * file descriptors.  For instance, most systems that have picked up the
   * pre-4.0 Sun YP code leave a file descriptor open each time you call one
   * of the getpw* functions, and it's set to be open across execs.  That
   * means one for login, one for xterm, one for shelltool, etc.
   */
  if (login_shell && interactive_shell)
    {
      for (i = 3; i < 20; i++)
	close (i);
    }
#endif /* CLOSE_FDS_AT_LOGIN */

  /* If we're in a strict Posix.2 mode, turn on interactive comments and
     other Posix.2 things. */
  if (posixly_correct)
    {
      posix_initialize (posixly_correct);
#if defined (READLINE)
      if (interactive_shell)
	posix_readline_initialize (posixly_correct);
#endif
    }

  /* From here on in, the shell must be a normal functioning shell.
     Variables from the environment are expected to be set, etc. */
  shell_initialize ();

  set_default_locale_vars ();

  if (interactive_shell)
    {
      char *term;

      term = getenv ("TERM");
      no_line_editing |= term && (STREQ (term, "emacs"));
      term = getenv ("EMACS");
      running_under_emacs = term ? ((fnmatch ("*term*", term, 0) == 0) ? 2 : 1)
				 : 0;
    }

  top_level_arg_index = arg_index;

  /* Give this shell a place to longjmp to before executing the
     startup files.  This allows users to press C-c to abort the
     lengthy startup. */
  code = setjmp (top_level);
  if (code)
    {
      if (code == EXITPROG)
	exit_shell (last_command_exit_value);
      else
	{
#if defined (JOB_CONTROL)
	  /* Reset job control, since run_startup_files turned it off. */
	  set_job_control (interactive_shell);
#endif
	  locally_skip_execution++;
	}
    }

  arg_index = top_level_arg_index;

  /* Execute the start-up scripts. */

  if (interactive_shell == 0)
    {
      makunbound ("PS1", shell_variables);
      makunbound ("PS2", shell_variables);
      interactive = expand_aliases = 0;
    }
  else
    {
      change_flag ('i', FLAG_ON);
      interactive = 1;
    }

#if defined (RESTRICTED_SHELL)
  /* If the `-r' option is supplied at invocation, make sure that the shell
     is not in restricted mode when running the startup files. */
    saverst = restricted;
    restricted = 0;
#endif

  if (locally_skip_execution == 0 && running_setuid == 0)
    run_startup_files ();

  /* If we are invoked as `sh', turn on Posix mode. */
  if (act_like_sh)
    {
      posix_initialize (posixly_correct = 1);
#if defined (READLINE)
      if (interactive_shell)
        posix_readline_initialize (posixly_correct);
#endif
    }

#if defined (RESTRICTED_SHELL)
  /* Turn on the restrictions after executing the startup files.  This
     means that `bash -r' or `set -r' invoked from a startup file will
     turn on the restrictions after the startup files are executed. */
  restricted = saverst || restricted;
  maybe_make_restricted (shell_name);
#endif /* RESTRICTED_SHELL */

  if (wordexp_only)
    {
      startup_state = 3;
      last_command_exit_value = run_wordexp (argv[arg_index]);
      exit_shell (last_command_exit_value);
    }

  if (local_pending_command)
    {
      arg_index = bind_args (argv, arg_index, argc, 0);

      startup_state = 2;
#if defined (ONESHOT)
      run_one_command (local_pending_command);
      exit_shell (last_command_exit_value);
#else /* ONESHOT */
      with_input_from_string (local_pending_command, "-c");
      goto read_and_execute;
#endif /* !ONESHOT */
    }

  /* Get possible input filename and set up default_buffered_input or
     default_input as appropriate. */
  if (arg_index != argc && read_from_stdin == 0)
    {
      open_shell_script (argv[arg_index]);
      arg_index++;
    }
  else if (interactive == 0)
    /* In this mode, bash is reading a script from stdin, which is a
       pipe or redirected file. */
#if defined (BUFFERED_INPUT)
    default_buffered_input = fileno (stdin);	/* == 0 */
#else
    setbuf (default_input, (char *)NULL);
#endif /* !BUFFERED_INPUT */

  set_bash_input ();

  /* Bind remaining args to $1 ... $n */
  arg_index = bind_args (argv, arg_index, argc, 1);

  /* Do the things that should be done only for interactive shells. */
  if (interactive_shell)
    {
      /* Set up for checking for presence of mail. */
      remember_mail_dates ();
      reset_mail_timer ();

#if defined (HISTORY)
      /* Initialize the interactive history stuff. */
      bash_initialize_history ();
      if (shell_initialized == 0)
	load_history ();
#endif /* HISTORY */

      /* Initialize terminal state for interactive shells after the
	 .bash_profile and .bashrc are interpreted. */
      get_tty_state ();
    }

#if !defined (ONESHOT)
 read_and_execute:
#endif /* !ONESHOT */

  shell_initialized = 1;

  /* Read commands until exit condition. */
  reader_loop ();
  exit_shell (last_command_exit_value);
}

static int
parse_long_options (argv, arg_start, arg_end)
     char **argv;
     int arg_start, arg_end;
{
  int arg_index, longarg, i;
  char *arg_string;

  arg_index = arg_start;
  while ((arg_index != arg_end) && (arg_string = argv[arg_index]) &&
	 (*arg_string == '-'))
    {
      longarg = 0;

      /* Make --login equivalent to -login. */
      if (arg_string[1] == '-' && arg_string[2])
	{
	  longarg = 1;
	  arg_string++;
	}

      for (i = 0; long_args[i].name; i++)
	{
	  if (STREQ (arg_string + 1, long_args[i].name))
	    {
	      if (long_args[i].type == Int)
		*long_args[i].int_value = 1;
	      else if (argv[++arg_index] == 0)
		{
		  report_error ("option `%s' requires an argument",
				long_args[i].name);
		  exit (EX_USAGE);
		}
	      else
		*long_args[i].char_value = argv[arg_index];

	      break;
	    }
	}
      if (long_args[i].name == 0)
	{
	  if (longarg)
	    {
	      report_error ("%s: unrecognized option", argv[arg_index]);
	      show_shell_usage (stderr, 0);
	      exit (EX_USAGE);
	    }
	  break;		/* No such argument.  Maybe flag arg. */
	}

      arg_index++;
    }

  return (arg_index);
}

static int
parse_shell_options (argv, arg_start, arg_end)
     char **argv;
     int arg_start, arg_end;
{
  int arg_index;
  int arg_character, on_or_off, next_arg, i;
  char *o_option, *arg_string;

  arg_index = arg_start;
  while (arg_index != arg_end && (arg_string = argv[arg_index]) &&
	 (*arg_string == '-' || *arg_string == '+'))
    {
      /* There are flag arguments, so parse them. */
      next_arg = arg_index + 1;

      /* A single `-' signals the end of options.  From the 4.3 BSD sh.
	 An option `--' means the same thing; this is the standard
	 getopt(3) meaning. */
      if (arg_string[0] == '-' &&
	   (arg_string[1] == '\0' ||
	     (arg_string[1] == '-' && arg_string[2] == '\0')))
	return (next_arg);

      i = 1;
      on_or_off = arg_string[0];
      while (arg_character = arg_string[i++])
	{
	  switch (arg_character)
	    {
	    case 'c':
	      want_pending_command = 1;
	      break;

	    case 's':
	      read_from_stdin = 1;
	      break;

	    case 'o':
	      o_option = argv[next_arg];
	      if (o_option == 0)
		{
		  list_minus_o_opts (-1, (on_or_off == '-') ? 0 : 1);
		  break;
		}
	      if (set_minus_o_option (on_or_off, o_option) != EXECUTION_SUCCESS)
		exit (EX_USAGE);
	      next_arg++;
	      break;

	    case 'D':
	      dump_translatable_strings = 1;
	      break;

	    default:
	      if (change_flag (arg_character, on_or_off) == FLAG_ERROR)
		{
		  report_error ("%c%c: unrecognized option", on_or_off, arg_character);
		  show_shell_usage (stderr, 0);
		  exit (EX_USAGE);
		}
	    }
	}
      /* Can't do just a simple increment anymore -- what about
	 "bash -abouo emacs ignoreeof -hP"? */
      arg_index = next_arg;
    }

  return (arg_index);
}

/* Exit the shell with status S. */
void
exit_shell (s)
     int s;
{
  /* Do trap[0] if defined.  Allow it to override the exit status
     passed to us. */
  if (signal_is_trapped (0))
    s = run_exit_trap ();

#if defined (PROCESS_SUBSTITUTION)
  unlink_fifo_list ();
#endif /* PROCESS_SUBSTITUTION */

#if defined (HISTORY)
  if (interactive_shell)
    maybe_save_shell_history ();
#endif /* HISTORY */

#if defined (JOB_CONTROL)
  /* If the user has run `shopt -s huponexit', hangup all jobs when we exit
     an interactive login shell.  ksh does this unconditionally. */
  if (interactive_shell && login_shell && hup_on_exit)
    hangup_all_jobs ();

  /* If this shell is interactive, terminate all stopped jobs and
     restore the original terminal process group. */
  end_job_control ();
#endif /* JOB_CONTROL */

  /* Always return the exit status of the last command to our parent. */
  exit (s);
}

/* Source the bash startup files.  If POSIXLY_CORRECT is non-zero, we obey
   the Posix.2 startup file rules:  $ENV is expanded, and if the file it
   names exists, that file is sourced.  The Posix.2 rules are in effect
   for interactive shells only. (section 4.56.5.3) */

/* Execute ~/.bashrc for most shells.  Never execute it if
   ACT_LIKE_SH is set, or if NO_RC is set.

   If the executable file "/usr/gnu/src/bash/foo" contains:

   #!/usr/gnu/bin/bash
   echo hello

   then:

         COMMAND	    EXECUTE BASHRC
         --------------------------------
         bash -c foo		NO
         bash foo		NO
         foo			NO
         rsh machine ls		YES (for rsh, which calls `bash -c')
         rsh machine foo	YES (for shell started by rsh) NO (for foo!)
         echo ls | bash		NO
         login			NO
         bash			YES
*/

static void
execute_env_file (env_file)
      char *env_file;
{
  char *fn;
  WORD_LIST *list;

  if (env_file && *env_file)
    {
      list = expand_string_unsplit (env_file, Q_DOUBLE_QUOTES);
      if (list)
	{
	  fn = string_list (list);
	  dispose_words (list);

	  if (fn && *fn)
	    maybe_execute_file (fn, 1);
	  FREE (fn);
	}
    }
}

static void
run_startup_files ()
{
#if defined (JOB_CONTROL)
  int old_job_control;
#endif
  int sourced_login, run_by_ssh;
  SHELL_VAR *sshvar;

  /* get the rshd/sshd case out of the way first. */
  if (interactive_shell == 0 && no_rc == 0 && login_shell == 0 &&
      act_like_sh == 0 && local_pending_command)
    {
      /* Find out if we were invoked by ssh.  If so, set RUN_BY_SSH to 1. */
      sshvar = find_variable ("SSH_CLIENT");
      if (sshvar)
	{
	  run_by_ssh = 1;
	  /* Now that we've tested the variable, we need to unexport it. */
	  sshvar->attributes &= ~att_exported;
	  array_needs_making = 1;
	}
      else
	run_by_ssh = 0;

      /* If we were run by sshd or we think we were run by rshd, execute
	 ~/.bashrc. */
      if (run_by_ssh || isnetconn (fileno (stdin)))
	{
#ifdef SYS_BASHRC
	  maybe_execute_file (SYS_BASHRC, 1);
#endif
	  maybe_execute_file (bashrc_file, 1);
	  return;
	}
    }

#if defined (JOB_CONTROL)
  /* Startup files should be run without job control enabled. */
  old_job_control = interactive_shell ? set_job_control (0) : 0;
#endif

  sourced_login = 0;

#if defined (NON_INTERACTIVE_LOGIN_SHELLS)
  if (login_shell)
    {
      /* We don't execute .bashrc for login shells. */
      no_rc++;

      /* Execute /etc/profile and one of the personal login shell
	 initialization files. */
      if (no_profile == 0)
	{
	  maybe_execute_file (SYS_PROFILE, 1);

	  if (act_like_sh)	/* sh */
	    maybe_execute_file ("~/.profile", 1);
	  else if ((maybe_execute_file ("~/.bash_profile", 1) == 0) &&
		   (maybe_execute_file ("~/.bash_login", 1) == 0))	/* bash */
	    maybe_execute_file ("~/.profile", 1);
	}

      sourced_login = 1;
    }
#endif /* NON_INTERACTIVE_LOGIN_SHELLS */

  /* A non-interactive shell not named `sh' and not in posix mode reads and
     executes commands from $BASH_ENV.  If `su' starts a shell with `-c cmd'
     and `-su' as the name of the shell, we want to read the startup files.
     No other non-interactive shells read any startup files. */
  if (interactive_shell == 0 && !(su_shell && login_shell))
    {
      if (posixly_correct == 0 && act_like_sh == 0 && privileged_mode == 0 &&
	    sourced_env++ == 0)
	execute_env_file (get_string_value ("BASH_ENV"));
      return;
    }

  /* Interactive shell or `-su' shell. */
  if (posixly_correct == 0)		  /* bash, sh */
    {
      if (login_shell && sourced_login++ == 0)
	{
	  /* We don't execute .bashrc for login shells. */
	  no_rc++;

	  /* Execute /etc/profile and one of the personal login shell
	     initialization files. */
	  if (no_profile == 0)
	    {
	      maybe_execute_file (SYS_PROFILE, 1);

	      if (act_like_sh)	/* sh */
		maybe_execute_file ("~/.profile", 1);
	      else if ((maybe_execute_file ("~/.bash_profile", 1) == 0) &&
		       (maybe_execute_file ("~/.bash_login", 1) == 0))	/* bash */
		maybe_execute_file ("~/.profile", 1);
	    }
	}

      /* bash */
      if (act_like_sh == 0 && no_rc == 0)
	{
#ifdef SYS_BASHRC
	  maybe_execute_file (SYS_BASHRC, 1);
#endif
          maybe_execute_file (bashrc_file, 1);
	}
      /* sh */
      else if (act_like_sh && privileged_mode == 0 && sourced_env++ == 0)
        execute_env_file (get_string_value ("ENV"));
    }
  else		/* bash --posix, sh --posix */
    {
      /* bash and sh */
      if (interactive_shell && privileged_mode == 0 && sourced_env++ == 0)
        execute_env_file (get_string_value ("ENV"));
    }

#if defined (JOB_CONTROL)
  set_job_control (old_job_control);
#endif
}

#if defined (RESTRICTED_SHELL)
/* Perhaps make this shell a `restricted' one, based on NAME.  If the
   basename of NAME is "rbash", then this shell is restricted.  The
   name of the restricted shell is a configurable option, see config.h.
   In a restricted shell, PATH and SHELL are read-only and non-unsettable.
   Do this also if `restricted' is already set to 1; maybe the shell was
   started with -r. */
int
maybe_make_restricted (name)
     char *name;
{
  char *temp;

  temp = base_pathname (shell_name);
  if (restricted || (STREQ (temp, RESTRICTED_SHELL_NAME)))
    {
      set_var_read_only ("PATH");
      set_var_read_only ("SHELL");
      restricted++;
    }
  return (restricted);
}
#endif /* RESTRICTED_SHELL */

/* Fetch the current set of uids and gids and return 1 if we're running
   setuid or setgid. */
static int
uidget ()
{
  uid_t u;

  u = getuid ();
  if (current_user.uid != u)
    {
      FREE (current_user.user_name);
      FREE (current_user.shell);
      FREE (current_user.home_dir);
      current_user.user_name = current_user.shell = current_user.home_dir = (char *)NULL;
    }
  current_user.uid = u;
  current_user.gid = getgid ();
  current_user.euid = geteuid ();
  current_user.egid = getegid ();

  /* See whether or not we are running setuid or setgid. */
  return (current_user.uid != current_user.euid) ||
	   (current_user.gid != current_user.egid);
}

void
disable_priv_mode ()
{
  setuid (current_user.uid);
  setgid (current_user.gid);
  current_user.euid = current_user.uid;
  current_user.egid = current_user.gid;
}

static int
run_wordexp (words)
     char *words;
{
  int code, nw, nb;
  WORD_DESC *w;
  WORD_LIST *wl, *result;

  code = setjmp (top_level);

  if (code != NOT_JUMPED)
    {
      switch (code)
	{
	  /* Some kind of throw to top_level has occured. */
	case FORCE_EOF:
	  return last_command_exit_value = 127;
	case EXITPROG:
	  return last_command_exit_value;
	case DISCARD:
	  return last_command_exit_value = 1;
	default:
	  programming_error ("run_wordexp: bad jump: code %d", code);
	}
    }

  /* Run it through the parser to get a list of words and expand them */
  if (words && *words)
    {
      with_input_from_string (words, "--wordexp");
      if (parse_command () != 0)
        return (126);
      if (global_command == 0)
	{
	  printf ("0\n0\n");
	  return (0);
	}
      if (global_command->type != cm_simple)
        return (126);
      wl = global_command->value.Simple->words;
      result = wl ? expand_words_no_vars (wl) : (WORD_LIST *)0;
    }
  else
    result = (WORD_LIST *)0;

  last_command_exit_value = 0;

  if (result == 0)
    {
      printf ("0\n0\n");
      return (0);
    }

  /* Count up the number of words and bytes, and print them.  Don't count
     the trailing NUL byte. */
  for (nw = nb = 0, wl = result; wl; wl = wl->next)
    {
      nw++;
      nb += strlen (wl->word->word);
    }
  printf ("%u\n%u\n", nw, nb);
  /* Print each word on a separate line.  This will have to be changed when
     the interface to glibc is completed. */
  for (wl = result; wl; wl = wl->next)
    printf ("%s\n", wl->word->word);

  return (0);
}

#if defined (ONESHOT)
/* Run one command, given as the argument to the -c option.  Tell
   parse_and_execute not to fork for a simple command. */
static int
run_one_command (command)
     char *command;
{
  int code;

  code = setjmp (top_level);

  if (code != NOT_JUMPED)
    {
#if defined (PROCESS_SUBSTITUTION)
      unlink_fifo_list ();
#endif /* PROCESS_SUBSTITUTION */
      switch (code)
	{
	  /* Some kind of throw to top_level has occured. */
	case FORCE_EOF:
	  return last_command_exit_value = 127;
	case EXITPROG:
	  return last_command_exit_value;
	case DISCARD:
	  return last_command_exit_value = 1;
	default:
	  programming_error ("run_one_command: bad jump: code %d", code);
	}
    }
   return (parse_and_execute (savestring (command), "-c", SEVAL_NOHIST));
}
#endif /* ONESHOT */

static int
bind_args (argv, arg_start, arg_end, start_index)
     char **argv;
     int arg_start, arg_end, start_index;
{
  register int i;
  WORD_LIST *args;

  for (i = arg_start, args = (WORD_LIST *)NULL; i != arg_end; i++)
    args = make_word_list (make_word (argv[i]), args);
  if (args)
    {
      args = REVERSE_LIST (args, WORD_LIST *);
      if (start_index == 0)	/* bind to $0...$n for sh -c command */
	{
	  /* Posix.2 4.56.3 says that the first argument after sh -c command
	     becomes $0, and the rest of the arguments become $1...$n */
	  shell_name = savestring (args->word->word);
	  FREE (dollar_vars[0]);
	  dollar_vars[0] = savestring (args->word->word);
	  remember_args (args->next, 1);
	}
      else			/* bind to $1...$n for shell script */
        remember_args (args, 1);

      dispose_words (args);
    }

  return (i);
}

void
unbind_args ()
{
  remember_args ((WORD_LIST *)NULL, 1);
}

static int
open_shell_script (script_name)
     char *script_name;
{
  int fd, e;
  char *filename, *path_filename;
  unsigned char sample[80];
  int sample_len;
  struct stat sb;

  free (dollar_vars[0]);
  dollar_vars[0] = savestring (script_name);
  filename = savestring (script_name);

  fd = open (filename, O_RDONLY);
  if ((fd < 0) && (errno == ENOENT) && (absolute_program (filename) == 0))
    {
      e = errno;
      /* If it's not in the current directory, try looking through PATH
	 for it. */
      path_filename = find_path_file (script_name);
      if (path_filename)
	{
	  free (filename);
	  filename = path_filename;
	  fd = open (filename, O_RDONLY);
	}
      else
	errno = e;
    }

  if (fd < 0)
    {
      e = errno;
      file_error (filename);
      exit ((e == ENOENT) ? EX_NOTFOUND : EX_NOINPUT);
    }

  /* Only do this with file descriptors we can seek on. */
  if (lseek (fd, 0L, 1) != -1)
    {
      /* Check to see if the `file' in `bash file' is a binary file
	 according to the same tests done by execute_simple_command (),
	 and report an error and exit if it is. */
      sample_len = read (fd, sample, sizeof (sample));
      if (sample_len < 0)
	{
	  e = errno;
	  if ((fstat (fd, &sb) == 0) && S_ISDIR (sb.st_mode))
	    internal_error ("%s: is a directory", filename);
          else
	    {
	      errno = e;
	      file_error (filename);
	    }
	  exit (EX_NOEXEC);
	}
      else if (sample_len > 0 && (check_binary_file (sample, sample_len)))
	{
	  internal_error ("%s: cannot execute binary file", filename);
	  exit (EX_BINARY_FILE);
	}
      /* Now rewind the file back to the beginning. */
      lseek (fd, 0L, 0);
    }

#if defined (BUFFERED_INPUT)
  default_buffered_input = fd;
#  if 0
  /* This is never executed. */
  if (default_buffered_input == -1)
    {
      file_error (filename);
      exit (EX_NOTFOUND);
    }
#  endif
  SET_CLOSE_ON_EXEC (default_buffered_input);
#else /* !BUFFERED_INPUT */
  /* Open the script.  But try to move the file descriptor to a randomly
     large one, in the hopes that any descriptors used by the script will
      not match with ours. */
  fd = move_to_high_fd (fd, 0, -1);

  default_input = fdopen (fd, "r");

  if (default_input == 0)
    {
      file_error (filename);
      exit (EX_NOTFOUND);
    }

  SET_CLOSE_ON_EXEC (fd);
  if (fileno (default_input) != fd)
    SET_CLOSE_ON_EXEC (fileno (default_input));
#endif /* !BUFFERED_INPUT */

  if (interactive_shell == 0 || isatty (fd) == 0)
    /* XXX - does this really need to be called again here? */
    init_noninteractive ();
  else
    {
      /* I don't believe that this code is ever executed, even in
	 the presence of /dev/fd. */
      dup2 (fd, 0);
      close (fd);
      fd = 0;
#if defined (BUFFERED_INPUT)
      default_buffered_input = 0;
#else
      fclose (default_input);
      default_input = stdin;
#endif
    }
  free (filename);
  return (fd);
}

/* Initialize the input routines for the parser. */
static void
set_bash_input ()
{
  /* Make sure the fd from which we are reading input is not in
     no-delay mode. */
#if defined (BUFFERED_INPUT)
  if (interactive == 0)
    unset_nodelay_mode (default_buffered_input);
  else
#endif /* !BUFFERED_INPUT */
    unset_nodelay_mode (fileno (stdin));

  /* with_input_from_stdin really means `with_input_from_readline' */
  if (interactive && no_line_editing == 0)
    with_input_from_stdin ();
  else
#if defined (BUFFERED_INPUT)
    {
      if (interactive == 0)
	with_input_from_buffered_stream (default_buffered_input, dollar_vars[0]);
      else
	with_input_from_stream (default_input, dollar_vars[0]);
    }
#else /* !BUFFERED_INPUT */
    with_input_from_stream (default_input, dollar_vars[0]);
#endif /* !BUFFERED_INPUT */
}

/* Close the current shell script input source and forget about it.  This is
   extern so execute_cmd.c:initialize_subshell() can call it.  If CHECK_ZERO
   is non-zero, we close default_buffered_input even if it's the standard
   input (fd 0). */
void
unset_bash_input (check_zero)
     int check_zero;
{
#if defined (BUFFERED_INPUT)
  if ((check_zero && default_buffered_input >= 0) ||
      (check_zero == 0 && default_buffered_input > 0))
    {
      close_buffered_fd (default_buffered_input);
      default_buffered_input = bash_input.location.buffered_fd = -1;
    }
#else /* !BUFFERED_INPUT */
  if (default_input)
    {
      fclose (default_input);
      default_input = (FILE *)NULL;
    }
#endif /* !BUFFERED_INPUT */
}
      

#if !defined (PROGRAM)
#  define PROGRAM "bash"
#endif

static void
set_shell_name (argv0)
     char *argv0;
{
  /* Here's a hack.  If the name of this shell is "sh", then don't do
     any startup files; just try to be more like /bin/sh. */
  shell_name = base_pathname (argv0);
  if (*shell_name == '-')
    shell_name++;
  if (shell_name[0] == 's' && shell_name[1] == 'h' && shell_name[2] == '\0')
    act_like_sh++;
  if (shell_name[0] == 's' && shell_name[1] == 'u' && shell_name[2] == '\0')
    su_shell++;

  shell_name = argv0;
  FREE (dollar_vars[0]);
  dollar_vars[0] = savestring (shell_name);

  if (*shell_name == '-')
    {
      shell_name++;
      login_shell++;
    }

  /* A program may start an interactive shell with
	  "execl ("/bin/bash", "-", NULL)".
     If so, default the name of this shell to our name. */
  if (!shell_name || !*shell_name || (shell_name[0] == '-' && !shell_name[1]))
    shell_name = PROGRAM;
}

static void
init_interactive ()
{
  interactive_shell = startup_state = interactive = 1;
  expand_aliases = 1;
}

static void
init_noninteractive ()
{
#if defined (HISTORY)
  bash_history_reinit (0);
#endif /* HISTORY */
  interactive_shell = startup_state = interactive = 0;
  expand_aliases = 0;
  no_line_editing = 1;
#if defined (JOB_CONTROL)
  set_job_control (0);
#endif /* JOB_CONTROL */
}

void
get_current_user_info ()
{
  struct passwd *entry;

  /* Don't fetch this more than once. */
  if (current_user.user_name == 0)
    {
      entry = getpwuid (current_user.uid);
      if (entry)
	{
	  current_user.user_name = savestring (entry->pw_name);
	  current_user.shell = (entry->pw_shell && entry->pw_shell[0])
				? savestring (entry->pw_shell)
				: savestring ("/bin/sh");
	  current_user.home_dir = savestring (entry->pw_dir);
	}
      else
	{
	  current_user.user_name = savestring ("I have no name!");
	  current_user.shell = savestring ("/bin/sh");
	  current_user.home_dir = savestring ("/");
	}
      endpwent ();
    }
}

/* Do whatever is necessary to initialize the shell.
   Put new initializations in here. */
static void
shell_initialize ()
{
  char hostname[256];

  /* Line buffer output for stderr and stdout. */
#if defined (SunOS5)
  if (shell_initialized == 0)
#endif
    {
      setlinebuf (stderr);
      setlinebuf (stdout);
    }

  /* Sort the array of shell builtins so that the binary search in
     find_shell_builtin () works correctly. */
  initialize_shell_builtins ();

  /* Initialize the trap signal handlers before installing our own
     signal handlers.  traps.c:restore_original_signals () is responsible
     for restoring the original default signal handlers.  That function
     is called when we make a new child. */
  initialize_traps ();
  initialize_signals ();

  /* It's highly unlikely that this will change. */
  if (current_host_name == 0)
    {
      /* Initialize current_host_name. */
      if (gethostname (hostname, 255) < 0)
	current_host_name = "??host??";
      else
	current_host_name = savestring (hostname);
    }

  /* Initialize the stuff in current_user that comes from the password
     file.  We don't need to do this right away if the shell is not
     interactive. */
  if (interactive_shell)
    get_current_user_info ();

  /* Initialize our interface to the tilde expander. */
  tilde_initialize ();

  /* Initialize internal and environment variables.  Don't import shell
     functions from the environment if we are running in privileged or
     restricted mode or if the shell is running setuid. */
#if defined (RESTRICTED_SHELL)
  initialize_shell_variables (shell_environment, privileged_mode||restricted||running_setuid);
#else
  initialize_shell_variables (shell_environment, privileged_mode||running_setuid);
#endif

#if 0
  /* Initialize filename hash tables. */
  initialize_filename_hashing ();
#endif

  /* Initialize the data structures for storing and running jobs. */
  initialize_job_control (0);

  /* Initialize input streams to null. */
  initialize_bash_input ();

  /* Initialize the shell options.  Don't import the shell options
     from the environment variable $SHELLOPTS if we are running in
     privileged or restricted mode or if the shell is running setuid. */
#if defined (RESTRICTED_SHELL)
  initialize_shell_options (privileged_mode||restricted||running_setuid);
#else
  initialize_shell_options (privileged_mode||running_setuid);
#endif
}

/* Function called by main () when it appears that the shell has already
   had some initialization performed.  This is supposed to reset the world
   back to a pristine state, as if we had been exec'ed. */
static void
shell_reinitialize ()
{
  /* The default shell prompts. */
  primary_prompt = PPROMPT;
  secondary_prompt = SPROMPT;

  /* Things that get 1. */
  current_command_number = 1;

  /* We have decided that the ~/.bashrc file should not be executed
     for the invocation of each shell script.  If the variable $ENV
     (or $BASH_ENV) is set, its value is used as the name of a file
     to source. */
  no_rc = no_profile = 1;

  /* Things that get 0. */
  login_shell = make_login_shell = interactive = executing = 0;
  debugging = do_version = line_number = last_command_exit_value = 0;
  forced_interactive = interactive_shell = subshell_environment = 0;
  expand_aliases = 0;

#if defined (HISTORY)
  bash_history_reinit (0);
#endif /* HISTORY */

#if defined (RESTRICTED_SHELL)
  restricted = 0;
#endif /* RESTRICTED_SHELL */

  /* Ensure that the default startup file is used.  (Except that we don't
     execute this file for reinitialized shells). */
  bashrc_file = "~/.bashrc";

  /* Delete all variables and functions.  They will be reinitialized when
     the environment is parsed. */
  delete_all_variables (shell_variables);
  delete_all_variables (shell_functions);

#if 0
  /* Pretend the PATH variable has changed. */
  flush_hashed_filenames ();
#endif
}

static void
show_shell_usage (fp, extra)
     FILE *fp;
     int extra;
{
  int i;
  char *set_opts, *s, *t;

  if (extra)
    fprintf (fp, "GNU bash, version %s-(%s)\n", shell_version_string (), MACHTYPE);
  fprintf (fp, "Usage:\t%s [GNU long option] [option] ...\n\t%s [GNU long option] [option] script-file ...\n",
	     shell_name, shell_name);
  fputs ("GNU long options:\n", fp);
  for (i = 0; long_args[i].name; i++)
    fprintf (fp, "\t--%s\n", long_args[i].name);

  fputs ("Shell options:\n", fp);
  fputs ("\t-irsD or -c command\t\t(invocation only)\n", fp);

  for (i = 0, set_opts = 0; shell_builtins[i].name; i++)
    if (STREQ (shell_builtins[i].name, "set"))
      set_opts = savestring (shell_builtins[i].short_doc);
  if (set_opts)
    {
      s = strchr (set_opts, '[');
      if (s == 0)
	s = set_opts;
      while (*++s == '-')
	;
      t = strchr (s, ']');
      if (t)
	*t = '\0';
      fprintf (fp, "\t-%s or -o option\n", s);
      free (set_opts);
    }

  if (extra)
    {
      fprintf (fp, "Type `%s -c \"help set\"' for more information about shell options.\n", shell_name);
      fprintf (fp, "Type `%s -c help' for more information about shell builtin commands.\n", shell_name);
      fprintf (fp, "Use the `bashbug' command to report bugs.\n");
    }
}

/* The second and subsequent conditions must match those used to decide
   whether or not to call getpeername() in isnetconn(). */
#if defined (HAVE_SYS_SOCKET_H) && defined (HAVE_GETPEERNAME) && !defined (SVR4_2)
#  include <sys/socket.h>
#endif

/* Is FD a socket or network connection? */
static int
isnetconn (fd)
     int fd;
{
#if defined (HAVE_GETPEERNAME) && !defined (SVR4_2)
  int rv, l;
  struct sockaddr sa;

  l = sizeof(sa);
  rv = getpeername(fd, &sa, &l);
  /* Solaris 2.5 getpeername() returns EINVAL if the fd is not a socket. */
  return ((rv < 0 && (errno == ENOTSOCK || errno == EINVAL)) ? 0 : 1);
#else /* !HAVE_GETPEERNAME || SVR4_2 */
#  if defined (SVR4) || defined (SVR4_2)
  /* Sockets on SVR4 and SVR4.2 are character special (streams) devices. */
  struct stat sb;

  if (isatty (fd))
    return (0);
  if (fstat (fd, &sb) < 0)
    return (0);
#    if defined (S_ISFIFO)
  if (S_ISFIFO (sb.st_mode))
    return (0);
#    endif /* S_ISFIFO */
  return (S_ISCHR (sb.st_mode));
#  else /* !SVR4 && !SVR4_2 */
#    if defined (S_ISSOCK)
  struct stat sb;

  if (fstat (fd, &sb) < 0)
    return (0);
  return (S_ISSOCK (sb.st_mode));
#    else /* !S_ISSOCK */
  return (0);
#    endif /* !S_ISSOCK */
#  endif /* !SVR4 && !SVR4_2 */
#endif /* !HAVE_GETPEERNAME || SVR4_2 */
}
