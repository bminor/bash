/* variables.c -- Functions for hacking shell variables. */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"
#include "posixstat.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <ctype.h>
#include <pwd.h>
#include "bashansi.h"

#include "shell.h"
#include "flags.h"
#include "execute_cmd.h"
#include "mailcheck.h"
#include "input.h"

#include "builtins/common.h"
#include <tilde/tilde.h>

#if defined (HISTORY)
#  include "bashhist.h"
#endif /* HISTORY */

/* Variables used here and defined in other files. */
extern int posixly_correct;
extern int variable_context, line_number;
extern int interactive, interactive_shell, login_shell;
extern int subshell_environment, indirection_level;
extern int build_version, patch_level;
extern char *dist_version, *release_status;
extern char *shell_name;
extern char *primary_prompt, *secondary_prompt;
extern char *current_host_name;
extern Function *this_shell_builtin;
extern char *this_command_name;
extern time_t shell_start_time;

/* The list of shell variables that the user has created, or that came from
   the environment. */
HASH_TABLE *shell_variables = (HASH_TABLE *)NULL;

/* The list of shell functions that the user has created, or that came from
   the environment. */
HASH_TABLE *shell_functions = (HASH_TABLE *)NULL;

/* The current variable context.  This is really a count of how deep into
   executing functions we are. */
int variable_context = 0;

/* The array of shell assignments which are made only in the environment
   for a single command. */
char **temporary_env = (char **)NULL;

/* The array of shell assignments which are in the environment for the
   execution of a shell function. */
char **function_env = (char **)NULL;

/* The array of shell assignments which are made only in the environment
   for the execution of a shell builtin command which may cause more than
   one command to be executed (e.g., "source"). */
char **builtin_env = (char **)NULL;

/* Some funky variables which are known about specially.  Here is where
   "$*", "$1", and all the cruft is kept. */
char *dollar_vars[10];
WORD_LIST *rest_of_args = (WORD_LIST *)NULL;

/* The value of $$. */
int dollar_dollar_pid;

/* An array which is passed to commands as their environment.  It is
   manufactured from the overlap of the initial environment and the
   shell variables that are marked for export. */
char **export_env = (char **)NULL;

/* Non-zero means that we have to remake EXPORT_ENV. */
int array_needs_making = 1;

/* The number of times BASH has been executed.  This is set
   by initialize_variables (). */
int shell_level = 0;

static char *have_local_variables;
static int local_variable_stack_size;

/* Some forward declarations. */
static void uidset ();
static void initialize_dynamic_variables ();
static void make_vers_array ();
static void sbrand ();		/* set bash random number generator. */
static int qsort_var_comp ();

/* Make VAR be auto-exported.  VAR is a pointer to a SHELL_VAR. */
#define set_auto_export(var) \
  do { var->attributes |= att_exported; array_needs_making = 1; } while (0)

/* Initialize the shell variables from the current environment. */
void
initialize_shell_variables (env, no_functions)
     char **env;
     int no_functions;	/* If set, don't import functions from ENV. */
{
  char *name, *string, *temp_string;
  int c, char_index, string_index, string_length;
  SHELL_VAR *temp_var;

  if (!shell_variables)
    shell_variables = make_hash_table (0);

  if (!shell_functions)
    shell_functions = make_hash_table (0);

  for (string_index = 0; string = env[string_index++]; )
    {
      char_index = 0;

      string_length = strlen (string);
      name = xmalloc (1 + string_length);

      while ((c = *string++) && c != '=')
	name[char_index++] = c;

      name[char_index] = '\0';

      /* If exported function, define it now. */
      if (no_functions == 0 && STREQN ("() {", string, 4))
	{
	  temp_string = xmalloc (3 + string_length + strlen (name));
	  sprintf (temp_string, "%s %s", name, string);

	  parse_and_execute (temp_string, name, 0);

	  if (name[char_index - 1] == ')')
	    name[char_index - 2] = '\0';

	  if (temp_var = find_function (name))
	    {
	      temp_var->attributes |= (att_exported | att_imported);
	      array_needs_making = 1;
	    }
	  else
	    report_error ("error importing function definition for `%s'", name);
	}
#if defined (ARRAY_VARS)
#  if 0
      /* Array variables may not yet be exported. */
      else if (*string == '(' && string[1] == '[' && strchr (string, ')'))
	{
	  string_length = 1;
	  temp_string = extract_array_assignment_list (string, &string_length);
	  temp_var = assign_array_from_string (name, temp_string);
	  FREE (temp_string);
	  temp_var->attributes |= (att_exported | att_imported);
	  array_needs_making = 1;
	}
#  endif
#endif
      else
	{
	  temp_var = bind_variable (name, string);
	  temp_var->attributes |= (att_exported | att_imported);
	  array_needs_making = 1;
	}
      free (name);
    }

  /* If we got PWD from the environment, update our idea of the current
     working directory.  In any case, make sure that PWD exists before
     checking it.  It is possible for getcwd () to fail on shell startup,
     and in that case, PWD would be undefined. */
  temp_var = find_variable ("PWD");
  if (temp_var && imported_p (temp_var) &&
      (temp_string = value_cell (temp_var)) &&
      same_file (temp_string, ".", (struct stat *)NULL, (struct stat *)NULL))
    set_working_directory (temp_string);
  else
    {
      temp_string = get_working_directory ("shell-init");
      if (temp_string)
	{
	  bind_variable ("PWD", temp_string);
	  free (temp_string);
	}
    }

  /* Set up initial value of $_ */
  temp_var = bind_variable ("_", dollar_vars[0]);

  /* Remember this pid. */
  dollar_dollar_pid = (int)getpid ();

  /* Now make our own defaults in case the vars that we think are
     important are missing. */
  temp_var = set_if_not ("PATH", DEFAULT_PATH_VALUE);
  set_auto_export (temp_var);

  temp_var = set_if_not ("TERM", "dumb");
  set_auto_export (temp_var);

  /* set up the prompts. */
  if (interactive_shell)
    {
      set_if_not ("PS1", primary_prompt);
      set_if_not ("PS2", secondary_prompt);
    }
  set_if_not ("PS4", "+ ");

  /* Don't allow IFS to be imported from the environment. */
  temp_var = bind_variable ("IFS", " \t\n");

  /* Magic machine types.  Pretty convenient. */
  temp_var = bind_variable ("HOSTTYPE", HOSTTYPE);
  set_auto_export (temp_var);
  temp_var = bind_variable ("OSTYPE", OSTYPE);
  set_auto_export (temp_var);
  temp_var = bind_variable ("MACHTYPE", MACHTYPE);
  set_auto_export (temp_var);
  temp_var = bind_variable ("HOSTNAME", current_host_name);
  set_auto_export (temp_var);

  /* Default MAILCHECK for interactive shells.  Defer the creation of a
     default MAILPATH until the startup files are read, because MAIL
     names a mail file if MAILCHECK is not set, and we should provide a
     default only if neither is set. */
  if (interactive_shell)
    set_if_not ("MAILCHECK", "60");

  /* Do some things with shell level. */
  temp_var = set_if_not ("SHLVL", "0");
  set_auto_export (temp_var);
  adjust_shell_level (1);

  /* Make a variable $PPID, which holds the pid of the shell's parent.  */
  name = itos ((int) getppid ());
  temp_var = find_variable ("PPID");
  if (temp_var)
    temp_var->attributes &= ~(att_readonly | att_exported);
  temp_var = bind_variable ("PPID", name);
  temp_var->attributes |= (att_readonly | att_integer);
  free (name);

  /* Initialize the `getopts' stuff. */
  bind_variable ("OPTIND", "1");
  sv_optind ("OPTIND");
  bind_variable ("OPTERR", "1");
  sv_opterr ("OPTERR");

  /* Get the full pathname to THIS shell, and set the BASH variable
     to it. */
  if ((login_shell == 1) && (*shell_name != '/'))
    {
      /* If HOME doesn't exist, set it. */
      temp_var = set_if_not ("HOME", current_user.home_dir);
      temp_var->attributes |= att_exported;

      name = savestring (current_user.shell);
    }
  else if (*shell_name == '/')
    name = savestring (shell_name);
  else
    {
      char *tname;
      int s;

      tname = find_user_command (shell_name);

      if (tname == 0)
	{
	  /* Try the current directory.  If there is not an executable
	     there, just punt and use the login shell. */
	  s = file_status (shell_name);
	  if (s & FS_EXECABLE)
	    {
	      tname = make_absolute (shell_name, get_string_value ("PWD"));
	      if (*shell_name == '.')
		{
		  name = canonicalize_pathname (tname);
		  if (name == 0)
		    name = tname;
		  else
		    free (tname);
		}
	     else
		name = tname;
	    }
	  else
	    name = savestring (current_user.shell);
	}
      else
	{
	  name = full_pathname (tname);
	  free (tname);
	}
    }
  temp_var = bind_variable ("BASH", name);
  free (name);

  /* Make the exported environment variable SHELL be the user's login
     shell.  Note that the `tset' command looks at this variable
     to determine what style of commands to output; if it ends in "csh",
     then C-shell commands are output, else Bourne shell commands. */
  temp_var = set_if_not ("SHELL", current_user.shell);
  set_auto_export (temp_var);

  /* Make a variable called BASH_VERSION which contains the version info. */
  bind_variable ("BASH_VERSION", shell_version_string ());
#if defined (ARRAY_VARS)
  make_vers_array ();
#endif

  /* Find out if we're supposed to be in Posix.2 mode via an
     environment variable. */
  temp_var = find_variable ("POSIXLY_CORRECT");
  if (!temp_var)
    temp_var = find_variable ("POSIX_PEDANTIC");
  if (temp_var && imported_p (temp_var))
    sv_strict_posix (temp_var->name);

#if defined (HISTORY)
  /* Set history variables to defaults, and then do whatever we would
     do if the variable had just been set.  Do this only in the case
     that we are remembering commands on the history list. */
  if (remember_on_history)
    {
      name = bash_tilde_expand (posixly_correct ? "~/.sh_history" : "~/.bash_history");

      set_if_not ("HISTFILE", name);
      free (name);

      set_if_not ("HISTSIZE", "500");
      sv_histsize ("HISTSIZE");
    }
#endif /* HISTORY */

  /* Seed the random number generator. */
  sbrand (dollar_dollar_pid + (long)shell_start_time);

  /* Handle some "special" variables that we may have inherited from a
     parent shell. */

  temp_var = find_variable ("IGNOREEOF");
  if (!temp_var)
    temp_var = find_variable ("ignoreeof");
  if (temp_var && imported_p (temp_var))
    sv_ignoreeof (temp_var->name);

#if defined (HISTORY)
  if (interactive_shell && remember_on_history)
    {
      sv_history_control ("HISTCONTROL");
      sv_histignore ("HISTIGNORE");
    }
#endif /* HISTORY */

  /* Get the user's real and effective user ids. */
  uidset ();

  /* Initialize the dynamic variables, and seed their values. */
  initialize_dynamic_variables ();
}

void
adjust_shell_level (change)
     int change;
{
  char *new_level, *old_SHLVL;
  int old_level;

  old_SHLVL = get_string_value ("SHLVL");
  if (old_SHLVL)
    old_level = atoi (old_SHLVL);
  else
    old_level = 0;

  shell_level = old_level + change;
  if (shell_level < 0)
    shell_level = 0;
  new_level = itos (shell_level);
  bind_variable ("SHLVL", new_level);
  free (new_level);
}

static void
uidset ()
{
  char *buff;
  register SHELL_VAR *v;

  buff = itos (current_user.uid);
  v = find_variable ("UID");
  if (v)
    v->attributes &= ~att_readonly;

  v = bind_variable ("UID", buff);
  v->attributes |= (att_readonly | att_integer);

  if (current_user.euid != current_user.uid)
    {
      free (buff);
      buff = itos (current_user.euid);
    }

  v = find_variable ("EUID");
  if (v)
    v->attributes &= ~att_readonly;

  v = bind_variable ("EUID", buff);
  v->attributes |= (att_readonly | att_integer);
  free (buff);
}

#if defined (ARRAY_VARS)
static void
make_vers_array ()
{
  SHELL_VAR *vv;
  ARRAY *av;
  char *s, d[16];

  makunbound ("BASH_VERSINFO", shell_variables);

  vv = make_new_array_variable ("BASH_VERSINFO");
  av = array_cell (vv);
  strcpy (d, dist_version);
  s = strchr (d, '.');
  if (s)
    *s++ = '\0';
  array_add_element (av, 0, d);
  array_add_element (av, 1, s);
  s = itos (patch_level);
  array_add_element (av, 2, s);
  free (s);
  s = itos (build_version);
  array_add_element (av, 3, s);
  free (s);
  array_add_element (av, 4, release_status);
  array_add_element (av, 5, MACHTYPE);

  vv->attributes |= att_readonly;
}
#endif /* ARRAY_VARS */

/* Set the environment variables $LINES and $COLUMNS in response to
   a window size change. */
void
set_lines_and_columns (lines, cols)
     int lines, cols;
{
  char *val;

  val = itos (lines);
  bind_variable ("LINES", val);
  free (val);

  val = itos (cols);
  bind_variable ("COLUMNS", val);
  free (val);
}

/* Set NAME to VALUE if NAME has no value. */
SHELL_VAR *
set_if_not (name, value)
     char *name, *value;
{
  SHELL_VAR *v;

  v = find_variable (name);
  if (!v)
    v = bind_variable (name, value);
  return (v);
}

/* Map FUNCTION over the variables in VARIABLES.  Return an array of the
   variables that satisfy FUNCTION.  Satisfy means that FUNCTION returns
   a non-zero value for.  A NULL value for FUNCTION means to use all
   variables. */
SHELL_VAR **
map_over (function, var_hash_table)
     Function *function;
     HASH_TABLE* var_hash_table;
{
  register int i;
  register BUCKET_CONTENTS *tlist;
  SHELL_VAR *var, **list = (SHELL_VAR **)NULL;
  int list_index = 0, list_size = 0;

  for (i = 0; i < var_hash_table->nbuckets; i++)
    {
      tlist = get_hash_bucket (i, var_hash_table);

      while (tlist)
	{
	  var = (SHELL_VAR *)tlist->data;

	  if (!function || (*function) (var))
	    {
	      if (list_index + 1 >= list_size)
		list = (SHELL_VAR **)
		  xrealloc (list, (list_size += 20) * sizeof (SHELL_VAR *));

	      list[list_index++] = var;
	      list[list_index] = (SHELL_VAR *)NULL;
	    }
	  tlist = tlist->next;
	}
    }
  return (list);
}

void
sort_variables (array)
     SHELL_VAR **array;
{
  qsort (array, array_len ((char **)array), sizeof (SHELL_VAR *), qsort_var_comp);
}

static int
qsort_var_comp (var1, var2)
     SHELL_VAR **var1, **var2;
{
  int result;

  if ((result = (*var1)->name[0] - (*var2)->name[0]) == 0)
    result = strcmp ((*var1)->name, (*var2)->name);

  return (result);
}

/* Create a NULL terminated array of all the shell variables in TABLE. */
static SHELL_VAR **
all_vars (table)
     HASH_TABLE *table;
{
  SHELL_VAR **list;

  list = map_over ((Function *)NULL, table);
  if (list && posixly_correct)
    sort_variables (list);
  return (list);
}

/* Create a NULL terminated array of all the shell variables. */
SHELL_VAR **
all_shell_variables ()
{
  return (all_vars (shell_variables));
}

/* Create a NULL terminated array of all the shell functions. */
SHELL_VAR **
all_shell_functions ()
{
  return (all_vars (shell_functions));
}

/* Print VARS to stdout in such a way that they can be read back in. */
void
print_var_list (list)
     register SHELL_VAR **list;
{
  register int i;
  register SHELL_VAR *var;

  for (i = 0; list && (var = list[i]); i++)
    if (!invisible_p (var))
      print_assignment (var);
}

#if defined (NOTDEF)
/* Print LIST (a linked list of shell variables) to stdout
   by printing the names, without the values.  Used to support the
   `set +' command. */
void
print_vars_no_values (list)
     register SHELL_VAR **list;
{
  register int i;
  register SHELL_VAR *var;

  for (i = 0; list && (var = list[i]); i++)
    if (!invisible_p (var))
      printf ("%s\n", var->name);
}
#endif

/* Print the value of a single SHELL_VAR.  No newline is
   output, but the variable is printed in such a way that
   it can be read back in. */
void
print_assignment (var)
     SHELL_VAR *var;
{
  if (function_p (var) && var->value)
    {
      printf ("%s=", var->name);
      print_var_function (var);
      printf ("\n");
    }
#if defined (ARRAY_VARS)
  else if (array_p (var) && var->value)
    print_array_assignment (var, 0);
#endif /* ARRAY_VARS */
  else if (var->value)
    {
      printf ("%s=", var->name);
      print_var_value (var, 1);
      printf ("\n");
    }
}

/* Print the value cell of VAR, a shell variable.  Do not print
   the name, nor leading/trailing newline.  If QUOTE is non-zero,
   and the value contains shell metacharacters, quote the value
   in such a way that it can be read back in. */
void
print_var_value (var, quote)
     SHELL_VAR *var;
     int quote;
{
  char *t;

  if (var->value)
    {
      if (quote && contains_shell_metas (var->value))
	{
	  t = single_quote (var->value);
	  printf ("%s", t);
	  free (t);
	}
      else
	printf ("%s", var->value);
    }
}

/* Print the function cell of VAR, a shell variable.  Do not
   print the name, nor leading/trailing newline. */
void
print_var_function (var)
     SHELL_VAR *var;
{
  if (function_p (var) && var->value)
    printf ("%s", named_function_string ((char *)NULL, function_cell(var), 1));
}

#if defined (ARRAY_VARS)
void
print_array_assignment (var, quoted)
     SHELL_VAR *var;
     int quoted;
{
  char *vstr;

  if (quoted)
    vstr = quoted_array_assignment_string (array_cell (var));
  else
    vstr = array_to_assignment_string (array_cell (var));

  if (vstr == 0)
    printf ("%s=%s\n", var->name, quoted ? "'()'" : "()");
  else
    {
      printf ("%s=%s\n", var->name, vstr);
      free (vstr);
    }
}
#endif /* ARRAY_VARS */

/* **************************************************************** */
/*								    */
/*		 Dynamic Variable Extension		            */
/*								    */
/* **************************************************************** */

/* DYNAMIC VARIABLES

   These are variables whose values are generated anew each time they are
   referenced.  These are implemented using a pair of function pointers
   in the struct variable: assign_func, which is called from bind_variable,
   and dynamic_value, which is called from find_variable.

   assign_func is called from bind_variable, if bind_variable discovers
   that the variable being assigned to has such a function.  The function
   is called as
  	SHELL_VAR *temp = (*(entry->assign_func)) (entry, value)
   and the (SHELL_VAR *)temp is returned as the value of bind_variable.  It
   is usually ENTRY (self).

   dynamic_value is called from find_variable to return a `new' value for
   the specified dynamic varible.  If this function is NULL, the variable
   is treated as a `normal' shell variable.  If it is not, however, then
   this function is called like this:
  	tempvar = (*(var->dynamic_value)) (var);

   Sometimes `tempvar' will replace the value of `var'.  Other times, the
   shell will simply use the string value.  Pretty object-oriented, huh?

   Be warned, though: if you `unset' a special variable, it loses its
   special meaning, even if you subsequently set it.

   The special assignment code would probably have been better put in
   subst.c: do_assignment, in the same style as
   stupidly_hack_special_variables, but I wanted the changes as
   localized as possible.  */

/* The value of $SECONDS.  This is the number of seconds since shell
   invocation, or, the number of seconds since the last assignment + the
   value of the last assignment. */
static long seconds_value_assigned;

static SHELL_VAR *
assign_seconds (self, value)
     SHELL_VAR *self;
     char *value;
{
  seconds_value_assigned = string_to_long (value);
  shell_start_time = NOW;
  return (self);
}

static SHELL_VAR *
get_seconds (var)
     SHELL_VAR *var;
{
  time_t time_since_start;
  char *p;

  time_since_start = NOW - shell_start_time;
  p = itos((int) seconds_value_assigned + time_since_start);

  FREE (var->value);

  var->attributes |= att_integer;
  var->value = p;
  return (var);
}

/* The random number seed.  You can change this by setting RANDOM. */
static unsigned long rseed = 1;
static unsigned long last_random_value;

/* A linear congruential random number generator based on the ANSI
   C standard.  A more complicated one is overkill.  */

/* Returns a pseudo-random number between 0 and 32767. */
static int
brand ()
{
  rseed = rseed * 1103515245 + 12345;
  return ((unsigned int)(rseed / 65536) % 32768);
}

/* Set the random number generator seed to SEED. */
static void
sbrand (seed)
     int seed;
{
  rseed = seed;
}

static SHELL_VAR *
assign_random (self, value)
     SHELL_VAR *self;
     char *value;
{
  sbrand (atoi (value));
  return (self);
}

static SHELL_VAR *
get_random (var)
     SHELL_VAR *var;
{
  int rv;
  char *p;

  /* Reset for command and process substitution. */
  if (subshell_environment)
    sbrand ((int)(getpid() + NOW));

  do
    rv = brand ();
  while (rv == (int)last_random_value);

  last_random_value = rv;
  p = itos ((int)rv);

  FREE (var->value);

  var->attributes |= att_integer;
  var->value = p;
  return (var);
}

/* Function which returns the current line number. */
static SHELL_VAR *
get_lineno (var)
     SHELL_VAR *var;
{
  char *p;
  int ln;

  ln = executing_line_number ();
  p = itos (ln);
  FREE (var->value);
  var->value = p;
  return (var);
}

static SHELL_VAR *
assign_lineno (var, value)
     SHELL_VAR *var;
     char *value;
{
  line_number = atoi (value);
  return var;
}

#if defined (HISTORY)
static SHELL_VAR *
get_histcmd (var)
     SHELL_VAR *var;
{
  char *p;

  p = itos (history_number ());
  FREE (var->value);
  var->value = p;
  return (var);
}
#endif

#if defined (PUSHD_AND_POPD) && defined (ARRAY_VARS)
static SHELL_VAR *
get_dirstack (self)
     SHELL_VAR *self;
{
  ARRAY *a;
  WORD_LIST *l;

  l = get_directory_stack ();
  a = word_list_to_array (l);
  dispose_array (array_cell (self));
  self->value = (char *)a;
  return self;
}

static  SHELL_VAR *
assign_dirstack (self, ind, value)
     SHELL_VAR *self;
     int ind;
     char *value;
{
  set_dirstack_element (ind, 1, value);
  return self;
}
#endif /* PUSHD AND POPD && ARRAY_VARS */

static void
initialize_dynamic_variables ()
{
  SHELL_VAR *v;

  v = bind_variable ("SECONDS", (char *)NULL);
  v->dynamic_value = get_seconds;
  v->assign_func = assign_seconds;

  v = bind_variable ("RANDOM", (char *)NULL);
  v->dynamic_value = get_random;
  v->assign_func = assign_random;

  v = bind_variable ("LINENO", (char *)NULL);
  v->dynamic_value = get_lineno;
  v->assign_func = assign_lineno;

#if defined (HISTORY)
  v = bind_variable ("HISTCMD", (char *)NULL);
  v->dynamic_value = get_histcmd;
  v->assign_func = (DYNAMIC_FUNC *)NULL;
#endif

#if defined (PUSHD_AND_POPD) && defined (ARRAY_VARS)
  v = make_new_array_variable ("DIRSTACK");
  v->dynamic_value = get_dirstack;
  v->assign_func = assign_dirstack;
#endif /* PUSHD_AND_POPD && ARRAY_VARS */
}

/* How to get a pointer to the shell variable or function named NAME.
   HASHED_VARS is a pointer to the hash table containing the list
   of interest (either variables or functions). */
SHELL_VAR *
var_lookup (name, hashed_vars)
     char *name;
     HASH_TABLE *hashed_vars;
{
  BUCKET_CONTENTS *bucket;

  bucket = find_hash_item (name, hashed_vars);
  return (bucket ? (SHELL_VAR *)bucket->data : (SHELL_VAR *)NULL);
}

/* Look up the variable entry named NAME.  If SEARCH_TEMPENV is non-zero,
   then also search the temporarily built list of exported variables. */
SHELL_VAR *
find_variable_internal (name, search_tempenv)
     char *name;
     int search_tempenv;
{
  SHELL_VAR *var = (SHELL_VAR *)NULL;

  /* If explicitly requested, first look in the temporary environment for
     the variable.  This allows constructs such as "foo=x eval 'echo $foo'"
     to get the `exported' value of $foo.  This happens if we are executing
     a function or builtin, or if we are looking up a variable in a
     "subshell environment". */
  if ((search_tempenv || subshell_environment) &&
      (temporary_env || builtin_env || function_env))
    var = find_tempenv_variable (name);

  if (!var)
    var = var_lookup (name, shell_variables);

  if (!var)
    return ((SHELL_VAR *)NULL);

  return (var->dynamic_value ? (*(var->dynamic_value)) (var) : var);
}

/* Look up the variable entry named NAME.  Returns the entry or NULL. */
SHELL_VAR *
find_variable (name)
     char *name;
{
  return (find_variable_internal
	  (name, (variable_context || this_shell_builtin || builtin_env)));
}

/* Look up the function entry whose name matches STRING.
   Returns the entry or NULL. */
SHELL_VAR *
find_function (name)
     char *name;
{
  return (var_lookup (name, shell_functions));
}

/* Return the string value of a variable.  Return NULL if the variable
   doesn't exist, or only has a function as a value.  Don't cons a new
   string. */
char *
get_string_value (var_name)
     char *var_name;
{
  SHELL_VAR *var = find_variable (var_name);

  if (!var)
    return (char *)NULL;
#if defined (ARRAY_VARS)
  else if (array_p (var))
    return (array_reference (array_cell (var), 0));
#endif
  else
    return (var->value);
}

/* Create a local variable referenced by NAME. */
SHELL_VAR *
make_local_variable (name)
     char *name;
{
  SHELL_VAR *new_var, *old_var;
  BUCKET_CONTENTS *elt;

  /* local foo; local foo;  is a no-op. */
  old_var = find_variable (name);
  if (old_var && old_var->context == variable_context)
    return (old_var);

  elt = remove_hash_item (name, shell_variables);
  if (elt)
    {
      old_var = (SHELL_VAR *)elt->data;
      free (elt->key);
      free (elt);
    }
  else
    old_var = (SHELL_VAR *)NULL;

  /* If a variable does not already exist with this name, then
     just make a new one. */
  if (!old_var)
    {
      new_var = bind_variable (name, "");
    }
  else
    {
      new_var = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

      new_var->name = savestring (name);
      new_var->value = xmalloc (1);
      new_var->value[0] = '\0';

      new_var->dynamic_value = (DYNAMIC_FUNC *)NULL;
      new_var->assign_func = (DYNAMIC_FUNC *)NULL;

      new_var->attributes = exported_p (old_var) ? att_exported : 0;

      new_var->prev_context = old_var;
      elt = add_hash_item (savestring (name), shell_variables);
      elt->data = (char *)new_var;
    }

  new_var->context = variable_context;
  new_var->attributes |= att_local;

  /* XXX */
  if (variable_context >= local_variable_stack_size)
    {
      int old_size = local_variable_stack_size;
      RESIZE_MALLOCED_BUFFER (have_local_variables, variable_context, 1,
			      local_variable_stack_size, 8);
      bzero ((char *)have_local_variables + old_size,
	     local_variable_stack_size - old_size);
    }
  have_local_variables[variable_context] = 1;		/* XXX */

  return (new_var);
}

#if defined (ARRAY_VARS)
SHELL_VAR *
make_local_array_variable (name)
     char *name;
{
  SHELL_VAR *var;
  ARRAY *array;

  var = make_local_variable (name);
  array = new_array ();

  FREE (value_cell(var));
  var->value = (char *)array;
  var->attributes |= att_array;
  return var;
}
#endif /* ARRAY_VARS */

/* Create a new shell variable with name NAME and add it to the hash table
   of shell variables. */
static
SHELL_VAR *
make_new_variable (name)
     char *name;
{
  SHELL_VAR *entry;
  BUCKET_CONTENTS *elt;

  entry = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

  entry->attributes = 0;
  entry->name = savestring (name);
  entry->value = (char *)NULL;

  entry->dynamic_value = (DYNAMIC_FUNC *)NULL;
  entry->assign_func = (DYNAMIC_FUNC *)NULL;

  /* Always assume variables are to be made at toplevel!
     make_local_variable has the responsibilty of changing the
     variable context. */
  entry->context = 0;
  entry->prev_context = (SHELL_VAR *)NULL;

  elt = add_hash_item (savestring (name), shell_variables);
  elt->data = (char *)entry;

  return entry;
}

#if defined (ARRAY_VARS)
SHELL_VAR *
make_new_array_variable (name)
     char *name;
{
  SHELL_VAR *entry;
  ARRAY *array;

  entry = make_new_variable (name);
  array = new_array ();
  entry->value = (char *)array;
  entry->attributes |= att_array;
  return entry;
}
#endif

char *
make_variable_value (var, value)
     SHELL_VAR *var;
     char *value;
{
  char *retval;
  long lval;

  /* If this variable has had its type set to integer (via `declare -i'),
     then do expression evaluation on it and store the result.  The
     functions in expr.c (evalexp and bind_int_variable) are responsible
     for turning off the integer flag if they don't want further
     evaluation done. */
  if (integer_p (var))
    {
      lval = evalexp (value);
      retval = itos (lval);
    }
  else if (value)
    {
      if (*value)
	retval = savestring (value);
      else
	{
	  retval = xmalloc (1);
	  retval[0] = '\0';
	}
    }
  else
    retval = (char *)NULL;

  return retval;
}

/* Bind a variable NAME to VALUE.  This conses up the name
   and value strings. */
SHELL_VAR *
bind_variable (name, value)
     char *name, *value;
{
  char *newval;
  SHELL_VAR *entry;

  entry = var_lookup (name, shell_variables);

  if (entry == 0)
    {
      entry = make_new_variable (name);
      entry->value = make_variable_value (entry, value);
    }
#if defined (ARRAY_VARS)
  else if (entry->assign_func && array_p (entry) == 0)
#else
  else if (entry->assign_func)
#endif
    return ((*(entry->assign_func)) (entry, value));
  else
    {
      if (readonly_p (entry))
	{
	  report_error ("%s: readonly variable", name);
	  return (entry);
	}

      /* Variables which are bound are visible. */
      entry->attributes &= ~att_invisible;

      newval = make_variable_value (entry, value);

#if defined (ARRAY_VARS)
      /* XXX -- this bears looking at again -- XXX */
      /* If an existing array variable x is being assigned to with x=b or
	 `read x' or something of that nature, silently convert it to
	 x[0]=b or `read x[0]'. */
      if (array_p (entry))
        array_add_element (array_cell (entry), 0, newval);
      else
	{
	  FREE (entry->value);
	  entry->value = newval;
	}
#else
      FREE (entry->value);
      entry->value = newval;
#endif
    }

  if (mark_modified_vars)
    entry->attributes |= att_exported;

  if (exported_p (entry))
    array_needs_making = 1;

  return (entry);
}

#if defined (ARRAY_VARS)
/* Convert a shell variable to an array variable.  The original value is
   saved as array[0]. */
SHELL_VAR *
convert_var_to_array (var)
     SHELL_VAR *var;
{
  char *oldval;
  ARRAY *array;

  oldval = value_cell (var);
  array = new_array ();
  array_add_element (array, 0, oldval);
  FREE (value_cell (var));
  var->value = (char *)array;
  var->attributes |= att_array;
  var->attributes &= ~att_invisible;

  return var;
}

/* Perform an array assignment name[ind]=value.  If NAME already exists and
   is not an array, and IND is 0, perform name=value instead.  If NAME exists
   and is not an array, and IND is not 0, convert it into an array with the
   existing value as name[0].

   If NAME does not exist, just create an array variable, no matter what
   IND's value may be. */
SHELL_VAR *
bind_array_variable (name, ind, value)
     char *name;
     int ind;
     char *value;
{
  SHELL_VAR *entry;
  char *newval;

  entry = var_lookup (name, shell_variables);

  if (entry == (SHELL_VAR *) 0)
    entry = make_new_array_variable (name);
  else if (readonly_p (entry))
    {
      report_error ("%s: readonly variable", name);
      return (entry);
    }
  else if (array_p (entry) == 0)
    entry = convert_var_to_array (entry);

  /* ENTRY is an array variable, and ARRAY points to the value. */
  newval = make_variable_value (entry, value);
  if (entry->assign_func)
    (*entry->assign_func) (entry, ind, newval);
  else
    array_add_element (array_cell (entry), ind, newval);
  FREE (newval);

  return (entry);
}

SHELL_VAR *
assign_array_from_string (name, value)
     char *name, *value;
{
  SHELL_VAR *var;

  var = find_variable (name);
  if (var == 0)
    var = make_new_array_variable (name);
  else if (array_p (var) == 0)
    var = convert_var_to_array (var);

  return (assign_array_var_from_string (var, value));
}

SHELL_VAR *
assign_array_var_from_word_list (var, list)
     SHELL_VAR *var;
     WORD_LIST *list;
{
  register int i;
  register WORD_LIST *l;
  ARRAY *a;

  for (a = array_cell (var), l = list, i = 0; l; l = l->next, i++)
    if (var->assign_func)
      (*var->assign_func) (var, i, l->word->word);
    else
      array_add_element (a, i, l->word->word);
  return var;
}

SHELL_VAR *
assign_array_var_from_string (var, value)
     SHELL_VAR *var;
     char *value;
{
  ARRAY *a;
  WORD_LIST *list, *nlist;
  char *w, *val, *nval;
  int ni, len, ind, last_ind;

  a = array_cell (var);

  /* Expand the value string into a list of words, performing all the
     shell expansions including word splitting. */
  if (*value == '(')
    {
      ni = 1;
      val = extract_array_assignment_list (value, &ni);
      if (val == 0)
	return var;
      nlist = expand_string (val, 0);
      free (val);
    }
  else
    nlist = expand_string (value, 0);

  for (last_ind = 0, list = nlist; list; list = list->next)
    {
      w = list->word->word;

      /* We have a word of the form [ind]=value */
      if (w[0] == '[')
	{
	  len = skipsubscript (w, 0);

	  if (w[len] != ']' || w[len+1] != '=')
	    {
	      nval = make_variable_value (var, w);
	      if (var->assign_func)
		(*var->assign_func) (var, last_ind, nval);
	      else
		array_add_element (a, last_ind, nval);
	      FREE (nval);
	      last_ind++;
	      continue;
	    }

	  if (len == 1)
	    {
	      report_error ("%s: bad array subscript", w);
	      continue;
	    }

	  if (ALL_ELEMENT_SUB (w[1]) && len == 2)
	    {
	      report_error ("%s: cannot assign to non-numeric index", w);
	      continue;
	    }

	  ind = array_expand_index (w + 1, len);
	  if (ind < 0)
	    {
	      report_error ("%s: bad array subscript", w);
	      continue;
	    }
	  last_ind = ind;
	  val = w + len + 2;
	}
      else		/* No [ind]=value, just a stray `=' */
	{
	  ind = last_ind;
	  val = w;
	}

      if (integer_p (var))
        this_command_name = (char *)NULL;	/* no command name for errors */
      nval = make_variable_value (var, val);
      if (var->assign_func)
	(*var->assign_func) (var, ind, nval);
      else
	array_add_element (a, ind, nval);
      FREE (nval);
      last_ind++;
    }

  dispose_words (nlist);
  return (var);
}
#endif /* ARRAY_VARS */

/* Dispose of the information attached to VAR. */
void
dispose_variable (var)
     SHELL_VAR *var;
{
  if (!var)
    return;

  if (function_p (var))
    dispose_command (function_cell (var));
#if defined (ARRAY_VARS)
  else if (array_p (var))
    dispose_array (array_cell (var));
#endif
  else
    FREE (value_cell (var));

  free (var->name);

  if (exported_p (var))
    array_needs_making = 1;

  free (var);
}

#if defined (ARRAY_VARS)
/* This function is called with SUB pointing to just after the beginning
   `[' of an array subscript. */
int
unbind_array_element (var, sub)
     SHELL_VAR *var;
     char *sub;
{
  int len, ind;
  ARRAY_ELEMENT *ae;

  len = skipsubscript (sub, 0);
  if (sub[len] != ']' || len == 0)
    {
      builtin_error ("%s[%s: bad array subscript", var->name, sub);
      return -1;
    }
  sub[len] = '\0';

  if (ALL_ELEMENT_SUB (sub[0]) && sub[1] == 0)
    {
      makunbound (var->name, shell_variables);
      return (0);
    }
  ind = array_expand_index (sub, len+1);
  if (ind < 0)
    {
      builtin_error ("[%s: bad array subscript", sub);
      return -1;
    }
  ae = array_delete_element (array_cell (var), ind);
  if (ae)
    destroy_array_element (ae);
  return 0;
}
#endif

/* Unset the variable referenced by NAME. */
int
unbind_variable (name)
     char *name;
{
  SHELL_VAR *var = find_variable (name);

  if (!var)
    return (-1);

  /* This function should never be called with an array variable name. */
#if defined (ARRAY_VARS)
  if (array_p (var) == 0 && var->value)
#else
  if (var->value)
#endif
    {
      free (var->value);
      var->value = (char *)NULL;
    }

  makunbound (name, shell_variables);

  return (0);
}

/* Make the variable associated with NAME go away.  HASH_LIST is the
   hash table from which this variable should be deleted (either
   shell_variables or shell_functions).
   Returns non-zero if the variable couldn't be found. */
int
makunbound (name, hash_list)
     char *name;
     HASH_TABLE *hash_list;
{
  BUCKET_CONTENTS *elt;
  SHELL_VAR *old_var, *new_var;
  char *t;

  elt = remove_hash_item (name, hash_list);

  if (!elt)
    return (-1);

  old_var = (SHELL_VAR *)elt->data;
  new_var = old_var->prev_context;

  if (old_var && exported_p (old_var))
    array_needs_making++;

  /* If we're unsetting a local variable and we're still executing inside
     the function, just mark the variable as invisible.
     kill_all_local_variables will clean it up later.  This must be done
     so that if the variable is subsequently assigned a new value inside
     the function, the `local' attribute is still present.  We also need
     to add it back into the correct hash table. */
  if (old_var && local_p (old_var) && variable_context == old_var->context)
    {
      old_var->attributes |= att_invisible;
      elt = add_hash_item (savestring (old_var->name), hash_list);
      elt->data = (char *)old_var;
      stupidly_hack_special_variables (old_var->name);
      return (0);
    }

  if (new_var)
    {
      /* Has to be a variable, functions don't have previous contexts. */
      BUCKET_CONTENTS *new_elt;

      new_elt = add_hash_item (savestring (new_var->name), hash_list);
      new_elt->data = (char *)new_var;

      if (exported_p (new_var))
	set_auto_export (new_var);
    }

  /* Have to save a copy of name here, because it might refer to
     old_var->name.  If so, stupidly_hack_special_variables will
     reference freed memory. */
  t = savestring (name);

  free (elt->key);
  free (elt);

  dispose_variable (old_var);
  stupidly_hack_special_variables (t);
  free (t);
  return (0);
}

/* Remove the variable with NAME if it is a local variable in the
   current context. */
int
kill_local_variable (name)
     char *name;
{
  SHELL_VAR *temp = find_variable (name);

  if (temp && temp->context == variable_context)
    {
      makunbound (name, shell_variables);
      return (0);
    }
  return (-1);
}

/* Get rid of all of the variables in the current context. */
int
variable_in_context (var)
     SHELL_VAR *var;
{
  return (var && var->context == variable_context);
}

void
kill_all_local_variables ()
{
  register int i, pass;
  register SHELL_VAR *var, **list;
  HASH_TABLE *varlist;

  /* If HAVE_LOCAL_VARIABLES == 0, it means that we don't have any local
     variables at all.  If VARIABLE_CONTEXT >= LOCAL_VARIABLE_STACK_SIZE,
     it means that we have some local variables, but not in this variable
     context (level of function nesting).  Also, if
     HAVE_LOCAL_VARIABLES[VARIABLE_CONTEXT] == 0, we have no local variables
     at this context. */
  if (have_local_variables == 0 ||
      variable_context >= local_variable_stack_size ||
      have_local_variables[variable_context] == 0)
    return;

  for (pass = 0; pass < 2; pass++)
    {
      varlist = pass ? shell_functions : shell_variables;

      list = map_over (variable_in_context, varlist);

      if (list)
	{
	  for (i = 0; var = list[i]; i++)
	    {
	      var->attributes &= ~att_local;
	      makunbound (var->name, varlist);
	    }
	  free (list);
	}
    }

  have_local_variables[variable_context] = 0;		/* XXX */
}

static void
free_variable_hash_data (data)
     char *data;
{
  SHELL_VAR *var, *prev;

  var = (SHELL_VAR *)data;
  while (var)
    {
      prev = var->prev_context;
      dispose_variable (var);
      var = prev;
    }
}

/* Delete the entire contents of the hash table. */
void
delete_all_variables (hashed_vars)
     HASH_TABLE *hashed_vars;
{
  flush_hash_table (hashed_vars, free_variable_hash_data);
}

static SHELL_VAR *
new_shell_variable (name)
     char *name;
{
  SHELL_VAR *var;

  var = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

  bzero ((char *)var, sizeof (SHELL_VAR));
  var->name = savestring (name);
  return (var);
}

/* Do a function binding to a variable.  You pass the name and
   the command to bind to.  This conses the name and command. */
SHELL_VAR *
bind_function (name, value)
     char *name;
     COMMAND *value;
{
  SHELL_VAR *entry;

  entry = find_function (name);
  if (!entry)
    {
      BUCKET_CONTENTS *elt;

      elt = add_hash_item (savestring (name), shell_functions);

      elt->data = (char *)new_shell_variable (name);
      entry = (SHELL_VAR *)elt->data;
      entry->dynamic_value = entry->assign_func = (DYNAMIC_FUNC *)NULL;

      /* Functions are always made at the top level.  This allows a
	 function to define another function (like autoload). */
      entry->context = 0;
    }

  if (entry->value)
    dispose_command ((COMMAND *)entry->value);

  entry->value = value ? (char *)copy_command (value) : (char *)NULL;
  entry->attributes |= att_function;

  if (mark_modified_vars)
    entry->attributes |= att_exported;

  entry->attributes &= ~att_invisible;	/* Just to be sure */

  if (exported_p (entry))
    array_needs_making = 1;

  return (entry);
}

/* Copy VAR to a new data structure and return that structure. */
SHELL_VAR *
copy_variable (var)
     SHELL_VAR *var;
{
  SHELL_VAR *copy = (SHELL_VAR *)NULL;

  if (var)
    {
      copy = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

      copy->attributes = var->attributes;
      copy->name = savestring (var->name);

      if (function_p (var))
	copy->value = (char *)copy_command (function_cell (var));
#if defined (ARRAY_VARS)
      else if (array_p (var))
	copy->value = (char *)dup_array (array_cell (var));
#endif
      else if (value_cell (var))
	copy->value = savestring (value_cell (var));
      else
	copy->value = (char *)NULL;

      copy->dynamic_value = var->dynamic_value;
      copy->assign_func = var->assign_func;

      copy->context = var->context;

      /* Don't bother copying previous contexts along with this variable. */
      copy->prev_context = (SHELL_VAR *)NULL;
    }
  return (copy);
}

#define FIND_OR_MAKE_VARIABLE(name, entry) \
  do \
    { \
      entry = find_variable (name); \
      if (!entry) \
	{ \
	  entry = bind_variable (name, ""); \
	  if (!no_invisible_vars) entry->attributes |= att_invisible; \
	} \
    } \
  while (0)

/* Make the variable associated with NAME be readonly.
   If NAME does not exist yet, create it. */
void
set_var_read_only (name)
     char *name;
{
  SHELL_VAR *entry;

  FIND_OR_MAKE_VARIABLE (name, entry);
  entry->attributes |= att_readonly;
}

/* Make the function associated with NAME be readonly.
   If NAME does not exist, we just punt, like auto_export code below. */
void
set_func_read_only (name)
     char *name;
{
  SHELL_VAR *entry = find_function (name);

  if (entry)
    entry->attributes |= att_readonly;
}

/* Make the variable associated with NAME be auto-exported.
   If NAME does not exist yet, create it. */
void
set_var_auto_export (name)
     char *name;
{
  SHELL_VAR *entry;

  FIND_OR_MAKE_VARIABLE (name, entry);
  set_auto_export (entry);
}

/* Make the function associated with NAME be auto-exported. */
void
set_func_auto_export (name)
     char *name;
{
  SHELL_VAR *entry;

  entry = find_function (name);
  if (entry)
    set_auto_export (entry);
}

#if defined (ARRAY_VARS)
/* This function assumes s[i] == '['; returns with s[ret] == ']' if
   an array subscript is correctly parsed. */
int
skipsubscript (s, i)
     char *s;
     int i;
{
  int count, c;

  for (count = 1; count && (c = s[++i]); )
    {
      if (c == '[')
	count++;
      else if (c == ']')
	count--;
    }
  return i;
}
#endif /* ARRAY_VARS */

/* Returns non-zero if STRING is an assignment statement.  The returned value
   is the index of the `=' sign. */
int
assignment (string)
     char *string;
{
  register int c, newi, indx;

  c = string[indx = 0];

  if (legal_variable_starter (c) == 0)
    return (0);

  while (c = string[indx])
    {
      /* The following is safe.  Note that '=' at the start of a word
	 is not an assignment statement. */
      if (c == '=')
	return (indx);

#if defined (ARRAY_VARS)
      if (c == '[')
	{
	  newi = skipsubscript (string, indx);
	  if (string[newi++] != ']')
	    return (0);
	  return ((string[newi] == '=') ? newi : 0);
	}
#endif /* ARRAY_VARS */

      /* Variable names in assignment statements may contain only letters,
	 digits, and `_'. */
      if (legal_variable_char (c) == 0)
	return (0);

      indx++;
    }
  return (0);
}

static int
visible_var (var)
     SHELL_VAR *var;
{
  return (invisible_p (var) == 0);
}

static SHELL_VAR **
_visible_names (table)
     HASH_TABLE *table;
{
  SHELL_VAR **list;

  list = map_over (visible_var, table);

  if (list && posixly_correct)
    sort_variables (list);

  return (list);
}

SHELL_VAR **
all_visible_variables ()
{
  return (_visible_names (shell_variables));
}

SHELL_VAR **
all_visible_functions ()
{
  return (_visible_names (shell_functions));
}

/* Return non-zero if the variable VAR is visible and exported.  Array
   variables cannot be exported. */
static int
visible_and_exported (var)
     SHELL_VAR *var;
{
  return (invisible_p (var) == 0 && exported_p (var));
}

/* Make an array of assignment statements from the hash table
   HASHED_VARS which contains SHELL_VARs.  Only visible, exported
   variables are eligible. */
char **
make_var_array (hashed_vars)
     HASH_TABLE *hashed_vars;
{
  register int i, list_index;
  register SHELL_VAR *var;
  char **list, *value;
  SHELL_VAR **vars;

  list = (char **)NULL;
  vars = map_over (visible_and_exported, hashed_vars);

  if (!vars)
    return (char **)NULL;

  list = (char **)xmalloc ((1 + array_len ((char **)vars)) * sizeof (char *));

  for (i = 0, list_index = 0; var = vars[i]; i++)
    {
      if (function_p (var))
	value = named_function_string ((char *)NULL, function_cell (var), 0);
#if defined (ARRAY_VARS)
      else if (array_p (var))
#  if 0
	value = array_to_assignment_string (array_cell (var));
#  else
	continue;	/* XXX array vars cannot yet be exported */
#  endif
#endif
      else
	value = value_cell (var);

      if (value)
	{
	  int name_len, value_len;
	  char	*p;

	  name_len = strlen (var->name);
	  value_len = strlen (value);
	  p = list[list_index] = xmalloc (2 + name_len + value_len);
	  strcpy (p, var->name);
	  p[name_len] = '=';
	  strcpy (p + name_len + 1, value);
	  list_index++;
#if defined (ARRAY_VARS)
	  if (array_p (var))
	    free (value);
#endif
	}
    }

  free (vars);
  list[list_index] = (char *)NULL;
  return (list);
}

/* Add STRING to the array of foo=bar strings that we already
   have to add to the environment.  */
int
assign_in_env (string)
     char *string;
{
  int size, offset;
  char *name, *temp, *value;
  int nlen, vlen;
  WORD_LIST *list;
  SHELL_VAR *var;

  offset = assignment (string);
  name = savestring (string);
  value = (char *)NULL;

#define freetemp nlen
  if (name[offset] == '=')
    {
      name[offset] = 0;

      var = find_variable (name);
      if (var && readonly_p (var))
	{
	  report_error ("%s: readonly variable", name);
  	  return (0);
	}
      temp = name + offset + 1;
      freetemp = strchr (temp, '~') != 0;
      if (freetemp)
	temp = bash_tilde_expand (temp);

      list = expand_string_unsplit (temp, 0);
      value = string_list (list);

      if (list)
	dispose_words (list);

      if (freetemp)
	free (temp);
    }
#undef freetemp

  nlen = strlen (name);
  vlen = value ? strlen (value) : 0;
  temp = xmalloc (2 + nlen + vlen);
  strcpy (temp, name);
  temp[nlen] = '=';
  temp[nlen + 1] = '\0';
  if (value)
    {
      if (*value)
	strcpy (temp + nlen + 1, value);
      free (value);
    }
  free (name);

  if (temporary_env == 0)
    {
      temporary_env = (char **)xmalloc (sizeof (char *));
      temporary_env [0] = (char *)NULL;
    }

  size = array_len (temporary_env);
  temporary_env = (char **)
    xrealloc (temporary_env, (size + 2) * (sizeof (char *)));

  temporary_env[size] = temp;
  temporary_env[size + 1] = (char *)NULL;
  array_needs_making = 1;

  if (echo_command_at_execute)
    {
      /* The Korn shell prints the `+ ' in front of assignment statements,
	 so we do too. */
      fprintf (stderr, "%s%s\n", indirection_level_string (), temp);
      fflush (stderr);
    }

  return 1;
}

/* Search for NAME in ARRAY, an array of strings in the same format as the
   environment array (i.e, name=value).  If NAME is present, make a new
   variable and return it.  Otherwise, return NULL. */
static SHELL_VAR *
find_name_in_env_array (name, array)
     char *name;
     char **array;
{
  register int i, l;

  if (array == 0)
    return ((SHELL_VAR *)NULL);

  for (i = 0, l = strlen (name); array[i]; i++)
    {
      if (STREQN (array[i], name, l) && array[i][l] == '=')
	{
	  SHELL_VAR *temp;
	  char *w;

	  temp = new_shell_variable (name);
	  w = array[i] + l + 1;

	  temp->value = *w ? savestring (w) : (char *)NULL;

	  temp->attributes = att_exported;
	  temp->context = 0;
	  temp->prev_context = (SHELL_VAR *)NULL;

	  temp->dynamic_value = temp->assign_func = (DYNAMIC_FUNC *)NULL;

	  return (temp);
	}
    }
  return ((SHELL_VAR *)NULL);
}

/* Find a variable in the temporary environment that is named NAME.
   The temporary environment can be either the environment provided
   to a simple command, or the environment provided to a shell function.
   We only search the function environment if we are currently executing
   a shell function body (variable_context > 0).  Return a consed variable,
   or NULL if not found. */
SHELL_VAR *
find_tempenv_variable (name)
     char *name;
{
  SHELL_VAR *var = (SHELL_VAR *)NULL;

  if (temporary_env)
    var = find_name_in_env_array (name, temporary_env);

  /* We don't check this_shell_builtin because the command that needs the
     value from builtin_env may be a disk command run inside a script run
     with `.' and a temporary env. */
  if (!var && builtin_env)
    var = find_name_in_env_array (name, builtin_env);

  if (!var && variable_context && function_env)
    var = find_name_in_env_array (name, function_env);

  return (var);
}

/* Free the storage allocated to the string array pointed to by ARRAYP, and
   make that variable have a null pointer as a value. */
static void
dispose_temporary_vars (arrayp)
     char ***arrayp;
{
  if (!*arrayp)
    return;

  free_array (*arrayp);
  *arrayp = (char **)NULL;
  array_needs_making = 1;
}

/* Free the storage used in the variable array for temporary
   environment variables. */
void
dispose_used_env_vars ()
{
  dispose_temporary_vars (&temporary_env);
}

/* Free the storage used for temporary environment variables given to
   commands when executing inside of a function body. */
void
dispose_function_env ()
{
  dispose_temporary_vars (&function_env);
}

/* Free the storage used for temporary environment variables given to
   commands when executing a builtin command such as "source". */
void
dispose_builtin_env ()
{
  dispose_temporary_vars (&builtin_env);
}

/* Take all of the shell variables in ENV_ARRAY and make shell variables
   from them at the current variable context. */
static void
merge_env_array (env_array)
     char **env_array;
{
  register int i, l;
  SHELL_VAR *temp;
  char *w, *name;

  if (env_array == 0)
    return;

  for (i = 0; env_array[i]; i++)
    {
      l = assignment (env_array[i]);
      name = env_array[i];
      w = env_array[i] + l + 1;
      name[l] = '\0';
      temp = bind_variable (name, w);
      name[l] = '=';
    }
}

void
merge_temporary_env ()
{
  merge_env_array (temporary_env);
}

void
merge_builtin_env ()
{
  merge_env_array (builtin_env);
}

#define ISFUNC(s, o) ((s[o + 1] == '(')  && (s[o + 2] == ')'))

/* Add ASSIGN to ARRAY, or supercede a previous assignment in the
   array with the same left-hand side.  Return the new array. */
char **
add_or_supercede (assign, array)
     char *assign;
     register char **array;
{
  register int i;
  int equal_offset = assignment (assign);

  if (!equal_offset)
    return (array);

  /* If this is a function, then only supercede the function definition.
     We do this by including the `=(' in the comparison.  */
  if (assign[equal_offset + 1] == '(')
    equal_offset++;

  for (i = 0; array && array[i]; i++)
    {
      if (STREQN (assign, array[i], equal_offset + 1))
	{
	  free (array[i]);
	  array[i] = savestring (assign);
	  return (array);
	}
    }
  array = (char **)xrealloc (array, ((2 + i) * sizeof (char *)));
  array[i++] = savestring (assign);
  array[i] = (char *)NULL;
  return (array);
}

/* Make the environment array for the command about to be executed.  If the
   array needs making.  Otherwise, do nothing.  If a shell action could
   change the array that commands receive for their environment, then the
   code should `array_needs_making++'. */
void
maybe_make_export_env ()
{
  register int i;
  register char **temp_array;

  if (array_needs_making)
    {
      if (export_env)
	free_array (export_env);

      export_env = (char **)xmalloc (sizeof (char *));
      export_env[0] = (char *)NULL;

      temp_array = make_var_array (shell_variables);
      for (i = 0; temp_array && temp_array[i]; i++)
	export_env = add_or_supercede (temp_array[i], export_env);
      free_array (temp_array);

      temp_array = make_var_array (shell_functions);
      for (i = 0; temp_array && temp_array[i]; i++)
	export_env = add_or_supercede (temp_array[i], export_env);
      free_array (temp_array);

      if (function_env)
	for (i = 0; function_env[i]; i++)
	  export_env = add_or_supercede (function_env[i], export_env);

      if (temporary_env)
	for (i = 0; temporary_env[i]; i++)
	  export_env = add_or_supercede (temporary_env[i], export_env);

#if 0
      /* If we changed the array, then sort it alphabetically. */
      if (posixly_correct == 0 && (temporary_env || function_env))
	sort_char_array (export_env);
#endif

      array_needs_making = 0;
    }
}

/* We always put _ in the environment as the name of this command. */
void
put_command_name_into_env (command_name)
     char *command_name;
{
  char *dummy;

  dummy = xmalloc (4 + strlen (command_name));

  /* These three statements replace a call to sprintf */
  dummy[0] = '_';
  dummy[1] = '=';
  strcpy (dummy + 2, command_name);
  export_env = add_or_supercede (dummy, export_env);
  free (dummy);
}

void
put_gnu_argv_flags_into_env (pid, flags_string)
     int pid;
     char *flags_string;
{
  char *dummy, *pbuf;
  int l, fl;

  pbuf = itos (pid);
  l = strlen (pbuf);

  fl = strlen (flags_string);

  dummy = xmalloc (l + fl + 30);
  dummy[0] = '_';
  strcpy (dummy + 1, pbuf);
  strcpy (dummy + 1 + l, "_GNU_nonoption_argv_flags_");
  dummy[l + 27] = '=';
  strcpy (dummy + l + 28, flags_string);

  free (pbuf);

  export_env = add_or_supercede (dummy, export_env);
  free (dummy);
}

/* Return a string denoting what our indirection level is. */
static char indirection_string[100];

char *
indirection_level_string ()
{
  register int i, j;
  char *ps4;

  indirection_string[0] = '\0';
  ps4 = get_string_value ("PS4");

  if (ps4 == 0 || *ps4 == '\0')
    return (indirection_string);

  ps4 = decode_prompt_string (ps4);

  for (i = 0; *ps4 && i < indirection_level && i < 99; i++)
    indirection_string[i] = *ps4;

  for (j = 1; *ps4 && ps4[j] && i < 99; i++, j++)
    indirection_string[i] = ps4[j];

  indirection_string[i] = '\0';
  free (ps4);
  return (indirection_string);
}
