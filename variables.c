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

#include <stdio.h>
#include "bashtypes.h"
#include "posixstat.h"
#include <ctype.h>
#include <pwd.h>

#include "bashansi.h"
#include "shell.h"
#include "hash.h"
#include "flags.h"
#include "execute_cmd.h"

#include "builtins/common.h"
#include <tilde/tilde.h>

/* Variables used here and defined in other files. */
extern int posixly_correct;
extern int variable_context, line_number;
extern int interactive, interactive_shell, login_shell, shell_level;
extern int subshell_environment;
extern int build_version;
extern char *dist_version;
extern char *shell_name;
extern char *primary_prompt, *secondary_prompt;
extern Function *this_shell_builtin;
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

/* The list of variables that may not be unset in this shell. */
char **non_unsettable_vars = (char **)NULL;

static char *have_local_variables;		/* XXX */
static int local_variable_stack_size = 0;	/* XXX */

/* Some forward declarations. */
static void initialize_dynamic_variables ();
static void sbrand ();		/* set bash random number generator. */
static int qsort_var_comp ();

/* Make VAR be auto-exported.  VAR is a pointer to a SHELL_VAR. */
#define set_auto_export(var) \
  do { var->attributes |= att_exported; array_needs_making = 1; } while (0)

#if defined (HISTORY)
#  include "bashhist.h"
#endif /* HISTORY */

/* Initialize the shell variables from the current environment. */
void
initialize_shell_variables (env)
     char **env;
{
  char *name, *string, *current_dir;
  int c, char_index;
  int string_index = 0;
  SHELL_VAR *temp_var;

  if (!shell_variables)
    shell_variables = make_hash_table (0);

  if (!shell_functions)
    shell_functions = make_hash_table (0);

  while (string = env[string_index++])
    {
      int string_length;

      char_index = 0;

      string_length = strlen (string);
      name = xmalloc (1 + string_length);

      while ((c = *string++) && c != '=')
	name[char_index++] = c;

      name[char_index] = '\0';

      /* If exported function, define it now. */
      if (!privileged_mode && STREQN ("() {", string, 4))
	{
	  SHELL_VAR *f;
	  char *eval_string;

	  eval_string = xmalloc (3 + string_length + strlen (name));
	  sprintf (eval_string, "%s %s", name, string);

	  parse_and_execute (eval_string, name, 0);

	  if (name[char_index - 1] == ')')
	    name[char_index - 2] = '\0';

	  if (f = find_function (name))
	    {
	      f->attributes |= (att_exported | att_imported);
	      array_needs_making = 1;
	    }
	  else
	    report_error ("error importing function definition for `%s'", name);
	}
      else
	{
	  SHELL_VAR *v;

	  v = bind_variable (name, string);
	  v->attributes |= (att_exported | att_imported);
	  array_needs_making = 1;
	}
      free (name);
    }

  /* If we got PWD from the environment, update our idea of the current
     working directory.  In any case, make sure that PWD exists before
     checking it.  It is possible for getwd () to fail on shell startup,
     and in that case, PWD would be undefined. */
  temp_var = find_variable ("PWD");
  if (temp_var && imported_p (temp_var) &&
      (current_dir = value_cell (temp_var)) &&
      same_file (current_dir, ".", (struct stat *)NULL, (struct stat *)NULL))
    set_working_directory (current_dir);
  else
    {
      current_dir = get_working_directory ("shell-init");
      if (current_dir)
        {
	  bind_variable ("PWD", current_dir);
	  free (current_dir);
        }
    }

  /* Remember this pid. */
  dollar_dollar_pid = (int)getpid ();

  /* Now make our own defaults in case the vars that we think are
     important are missing. */
  temp_var = set_if_not ("PATH", DEFAULT_PATH_VALUE);
  set_auto_export (temp_var);

  temp_var = set_if_not ("TERM", "dumb");
  set_auto_export (temp_var);

  if (interactive_shell)
    {
      set_if_not ("PS1", primary_prompt);
      set_if_not ("PS2", secondary_prompt);
    }
  set_if_not ("PS4", "+ ");

#if defined (INSECURITY)
  set_if_not ("IFS", " \t\n");
#else
  bind_variable ("IFS", " \t\n");
#endif /* INSECURITY */

  /* Magic machine types.  Pretty convenient. */
  temp_var = set_if_not ("HOSTTYPE", HOSTTYPE);
  set_auto_export (temp_var);
  temp_var = set_if_not ("OSTYPE", OSTYPE);
  set_auto_export (temp_var);

  /* Default MAILCHECK for interactive shells. */
  if (interactive_shell)
    set_if_not ("MAILCHECK", "60");

  /* Do some things with shell level. */
  temp_var = set_if_not ("SHLVL", "0");
  set_auto_export (temp_var);
  adjust_shell_level (1);

  /* Make a variable $PPID, which holds the pid of the shell's parent.  */
  {
    char *ppid;
    SHELL_VAR *v;

    ppid = itos ((int) getppid ());
    v = find_variable ("PPID");

    if (v)
      v->attributes &= ~(att_readonly | att_exported);

    v = bind_variable ("PPID", ppid);
    v->attributes |= (att_readonly | att_integer);

    non_unsettable ("PPID");
    free (ppid);
  }

#if defined (GETOPTS_BUILTIN)
  /* Initialize the `getopts' stuff. */
  bind_variable ("OPTIND", "1");
  bind_variable ("OPTERR", "1");
#endif /* GETOPTS_BUILTIN */

  /* Get the full pathname to THIS shell, and set the BASH variable
     to it. */
  {
    char *tname;

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

    /* Make the exported environment variable SHELL be the user's login
       shell.  Note that the `tset' command looks at this variable
       to determine what style of commands to output; if it ends in "csh",
       then C-shell commands are output, else Bourne shell commands. */
    temp_var = set_if_not ("SHELL", current_user.shell);
    set_auto_export (temp_var);

    /* Make a variable called BASH, which is the name of THIS shell. */
    temp_var = bind_variable ("BASH", name);

    free (name);
  }

  /* Make a variable called BASH_VERSION which contains the version info. */
  bind_variable ("BASH_VERSION", shell_version_string ());

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
      if (posixly_correct)
	name = tilde_expand ("~/.sh_history");
      else
	name = tilde_expand ("~/.bash_history");

      set_if_not ("HISTFILE", name);
      free (name);

      set_if_not ("HISTSIZE", "500");
      sv_histsize ("HISTSIZE");
    }
#endif /* HISTORY */

  /* Seed the random number generator. */
  sbrand (dollar_dollar_pid);

  /* Handle some "special" variables that we may have inherited from a
     parent shell. */

  noclobber = find_variable ("noclobber") != (SHELL_VAR *)NULL;

  temp_var = find_variable ("IGNOREEOF");
  if (!temp_var)
    temp_var = find_variable ("ignoreeof");
  if (temp_var && imported_p (temp_var))
    sv_ignoreeof (temp_var->name);

#if defined (HISTORY)
  if (interactive_shell && remember_on_history)
    {
      sv_command_oriented_history ("command_oriented_history");
      if (find_variable ("history_control"))
	sv_history_control ("history_control");	/* gone in next release */
      else
	sv_history_control ("HISTCONTROL");
    }
#endif /* HISTORY */

  /* Initialize the dynamic variables, and seed their values. */
  initialize_dynamic_variables ();

  non_unsettable ("PATH");
  non_unsettable ("IFS");

  if (interactive_shell)
    {
      non_unsettable ("PS1");
      non_unsettable ("PS2");
    }

  /* Get the users real user id, and save that in a readonly variable.
     To make the variable *really* readonly, we have added it to a special
     list of vars. */

  sv_uids ();
  set_var_read_only ("UID");
  set_var_read_only ("EUID");

  non_unsettable ("EUID");
  non_unsettable ("UID");
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

/* Add NAME to the list of variables that cannot be unset
   if it isn't already there. */
void
non_unsettable (name)
     char *name;
{
  register int i;

  if (!non_unsettable_vars)
    {
      non_unsettable_vars = (char **)xmalloc (1 * sizeof (char *));
      non_unsettable_vars[0] = (char *)NULL;
    }

  for (i = 0; non_unsettable_vars[i]; i++)
    if (STREQ (non_unsettable_vars[i], name))
      return;

  non_unsettable_vars = (char **)
    xrealloc (non_unsettable_vars, (2 + i) * sizeof (char *));
  non_unsettable_vars[i] = savestring (name);
  non_unsettable_vars[i + 1] = (char *)NULL;
}

/* Set NAME to VALUE if NAME has no value. */
SHELL_VAR *
set_if_not (name, value)
     char *name, *value;
{
  SHELL_VAR *v = find_variable (name);

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
  if (list)
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
  else if (var->value)
    {
      printf ("%s=", var->name);
      print_var_value (var);
      printf ("\n");
    }
}

/* Print the value cell of VAR, a shell variable.  Do not print
   the name, nor leading/trailing newline. */
void
print_var_value (var)
     SHELL_VAR *var;
{
  if (var->value)
    printf ("%s", var->value);
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

/* **************************************************************** */
/*                                                                  */
/*                 Dynamic Variable Extension                       */
/*                                                                  */
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
static long seconds_value_assigned = (long)0;

static SHELL_VAR *
assign_seconds (self, value)
     SHELL_VAR *self;
     char *value;
{
  seconds_value_assigned = atol (value);
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
  int s = atoi (value);

  sbrand (s);
  return (self);
}

static SHELL_VAR *
get_random (var)
     SHELL_VAR *var;
{
  int rv;
  char *p;

  rv = brand ();
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

  p = itos (line_number);
  FREE (var->value);
  var->value = p;
  return (var);
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
  v->assign_func = (DYNAMIC_FUNC *)NULL;

#if defined (HISTORY)
  v = bind_variable ("HISTCMD", (char *)NULL);
  v->dynamic_value = get_histcmd;
  v->assign_func = (DYNAMIC_FUNC *)NULL;
#endif
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

  if (bucket)
    return ((SHELL_VAR *)bucket->data);
  else
    return ((SHELL_VAR *)NULL);
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

  if (var->dynamic_value)
    return ((*(var->dynamic_value)) (var));
  else
    return (var);
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
      new_var->value = savestring ("");

      new_var->dynamic_value = (DYNAMIC_FUNC *)NULL;
      new_var->assign_func = (DYNAMIC_FUNC *)NULL;

      new_var->attributes = 0;

      if (exported_p (old_var))
	new_var->attributes |= att_exported;

      new_var->prev_context = old_var;
      elt = add_hash_item (savestring (name), shell_variables);
      elt->data = (char *)new_var;
    }

  new_var->context = variable_context;

  /* XXX */
  if (local_variable_stack_size <= variable_context)
    {
      int old_size = local_variable_stack_size;
      while (local_variable_stack_size <= variable_context)
        local_variable_stack_size += 8;
      have_local_variables =
        xrealloc (have_local_variables, local_variable_stack_size);
      bzero ((char *)have_local_variables + old_size,
	     local_variable_stack_size - old_size);
    }
  have_local_variables[variable_context] = 1;		/* XXX */

  return (new_var);
}

/* Bind a variable NAME to VALUE.  This conses up the name
   and value strings. */
SHELL_VAR *
bind_variable (name, value)
     char *name, *value;
{
  SHELL_VAR *entry = var_lookup (name, shell_variables);
  BUCKET_CONTENTS *elt;

  if (!entry)
    {
      /* Make a new entry for this variable.  Then do the binding. */
      entry = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));

      entry->attributes = 0;
      entry->name = savestring (name);

      if (value)
	{
	  if (*value)
	    entry->value = savestring (value);
	  else
	    {
	      entry->value = xmalloc (1);
	      entry->value[0] = '\0';
	    }
	}
      else
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
    }
  else if (entry->assign_func)
    return ((*(entry->assign_func)) (entry, value));
  else
    {
      if (readonly_p (entry))
	{
	  report_error ("%s: read-only variable", name);
	  return (entry);
	}

      /* Variables which are bound are visible. */
      entry->attributes &= ~att_invisible;

      /* If this variable has had its type set to integer (via `declare -i'),
	 then do expression evaluation on it and store the result.  The
	 functions in expr.c (evalexp and bind_int_variable) are responsible
	 for turning off the integer flag if they don't want further
	 evaluation done. */
      if (integer_p (entry))
	{
	  long val;

	  val = evalexp (value);
	  /* We cannot free () entry->value before this; what if the string
	     we are working is `even=even+2'?  We need the original value
	     around while we are doing the evaluation to handle any possible
	     recursion. */
	  FREE (entry->value);
	  entry->value = itos (val);
	}
      else
	{
	  FREE (entry->value);

	  if (value)
	    {
	      if (*value)
		entry->value = savestring (value);
	      else
		{
		  entry->value = xmalloc (1);
		  entry->value[0] = '\0';
		}
	    }
	  else
	    entry->value = (char *)NULL;
	}
    }

  if (mark_modified_vars)
    entry->attributes |= att_exported;

  if (exported_p (entry))
    array_needs_making = 1;

  return (entry);
}

/* Dispose of the information attached to VAR. */
void
dispose_variable (var)
     SHELL_VAR *var;
{
  if (!var)
    return;

  if (function_p (var))
    dispose_command ((COMMAND *)var->value);
  else if (var->value)
    free (var->value);

  free (var->name);

  if (exported_p (var))
    array_needs_making = 1;

  free (var);
}

/* Unset the variable referenced by NAME. */
unbind_variable (name)
     char *name;
{
  SHELL_VAR *var = find_variable (name);

  if (!var)
    return (-1);

  if (var->value)
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

  if (new_var)
    {
      /* Has to be a variable, functions don't have previous contexts. */
      BUCKET_CONTENTS *new_elt;

      new_elt = add_hash_item (savestring (new_var->name), hash_list);
      new_elt->data = (char *)new_var;

      if (exported_p (new_var))
	set_var_auto_export (new_var->name);
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
kill_local_variable (name)
     char *name;
{
  SHELL_VAR *temp = find_variable (name);

  if (temp && (temp->context == variable_context))
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

  /* XXX */
  if (!have_local_variables || have_local_variables[variable_context] == 0)
    return;

  for (pass = 0; pass < 2; pass++)
    {
      varlist = pass ? shell_functions : shell_variables;

      list = map_over (variable_in_context, varlist);

      if (list)
	{
	  for (i = 0; var = list[i]; i++)
	    makunbound (var->name, varlist);

	  free (list);
	}
    }

  have_local_variables[variable_context] = 0;		/* XXX */
}

/* Delete the entire contents of the hash table. */
void
delete_all_variables (hashed_vars)
     HASH_TABLE *hashed_vars;
{
  register int i;
  register BUCKET_CONTENTS *bucket;

  for (i = 0; i < hashed_vars->nbuckets; i++)
    {
      bucket = hashed_vars->bucket_array[i];

      while (bucket)
	{
	  BUCKET_CONTENTS *temp = bucket;
	  SHELL_VAR *var, *prev;

	  bucket = bucket->next;

	  var = (SHELL_VAR *)temp->data;

	  while (var)
	    {
	      prev = var->prev_context;
	      dispose_variable (var);

	      var = prev;
	    }

	  free (temp->key);
	  free (temp);
	}
      hashed_vars->bucket_array[i] = (BUCKET_CONTENTS *)NULL;
    }
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
  SHELL_VAR *entry = find_function (name);

  if (!entry)
    {
      BUCKET_CONTENTS *elt;

      elt = add_hash_item (savestring (name), shell_functions);

      elt->data = (char *)new_shell_variable (name);
      entry = (SHELL_VAR *)elt->data;
      entry->dynamic_value = (DYNAMIC_FUNC *)NULL;
      entry->assign_func = (DYNAMIC_FUNC *)NULL;

      /* Functions are always made at the top level.  This allows a
	 function to define another function (like autoload). */
      entry->context = 0;
    }

  if (entry->value)
    dispose_command ((COMMAND *)entry->value);

  if (value)	/* I don't think this can happen anymore */
    entry->value = (char *)copy_command (value);
  else
    entry->value = (char *)NULL;

  entry->attributes |= att_function;

  if (mark_modified_vars)
    entry->attributes |= att_exported;

  entry->attributes &= ~att_invisible;	/* Just to be sure */

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
	copy->value = (char *)copy_command ((COMMAND *)var->value);
      else if (var->value)
	copy->value = savestring (var->value);
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

/* Make the variable associated with NAME be read-only.
   If NAME does not exist yet, create it. */
void
set_var_read_only (name)
     char *name;
{
  SHELL_VAR *entry = find_variable (name);

  if (!entry)
    {
      entry = bind_variable (name, "");
      if (!no_invisible_vars)
	entry->attributes |= att_invisible;
    }
  entry->attributes |= att_readonly;
}

/* Make the function associated with NAME be read-only.
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
  SHELL_VAR *entry = find_variable (name);

  if (!entry)
    {
      entry = bind_variable (name, "");
      if (!no_invisible_vars)
	entry->attributes |= att_invisible;
    }

  set_auto_export (entry);
}

/* Make the function associated with NAME be auto-exported. */
void
set_func_auto_export (name)
     char *name;
{
  SHELL_VAR *entry = find_function (name);

  if (entry)
    {
      entry->attributes |= att_exported;
      array_needs_making = 1;
    }
}

/* Returns non-zero if STRING is an assignment statement.  The returned value
   is the index of the `=' sign. */
assignment (string)
     char *string;
{
  register int c, indx = 0;

  c = string[indx];

  if (!isletter (c) && c != '_')
    return (0);

  while (c = string[indx])
    {
      /* The following is safe.  Note that '=' at the start of a word
	 is not an assignment statement. */
      if (c == '=')
	return (indx);

      if (!isletter (c) && !digit (c) && c != '_')
	return (0);

      indx++;
    }
  return (0);
}

static int
visible_var (var)
     SHELL_VAR *var;
{
  return (!invisible_p (var));
}

SHELL_VAR **
all_visible_variables ()
{
  SHELL_VAR **list;

  list = map_over (visible_var, shell_variables);

  if (list)
    sort_variables (list);

  return (list);
}

SHELL_VAR **
all_visible_functions ()
{
  SHELL_VAR **list;

  list = map_over (visible_var, shell_functions);

  if (list)
    sort_variables (list);

  return (list);
}

/* Return non-zero if the variable VAR is visible and exported. */
static int
visible_and_exported (var)
     SHELL_VAR *var;
{
  return (!invisible_p (var) && exported_p (var));
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
  char **list = (char **)NULL;
  SHELL_VAR **vars;

  vars = map_over (visible_and_exported, hashed_vars);

  if (!vars)
    return (char **)NULL;

  list = (char **)xmalloc ((1 + array_len ((char **)vars)) * sizeof (char *));

  for (i = 0, list_index = 0; var = vars[i]; i++)
    {
      char *value;

      if (function_p (var))
	value = named_function_string
	  ((char *)NULL, (COMMAND *)function_cell (var), 0);
      else
	value = value_cell (var);

      if (value)
	{
	  int name_len = strlen (var->name);
	  int value_len = strlen (value);
	  char	*p;

	  p = list[list_index] = xmalloc (2 + name_len + value_len);
	  strcpy (p, var->name);
	  p[name_len] = '=';
	  strcpy (p + name_len + 1, value);
	  list_index++;
	}
    }

  free (vars);
  list[list_index] = (char *)NULL;
  return (list);
}

/* Add STRING to the array of foo=bar strings that we already
   have to add to the environment.  */
assign_in_env (string)
     char *string;
{
  int size;

  int offset = assignment (string);
  char *name = savestring (string);
  char *temp, *value = (char *)NULL;
  int nlen, vlen;

  if (name[offset] == '=')
    {
      WORD_LIST *list;

      name[offset] = 0;
      temp = name + offset + 1;
      temp = tilde_expand (temp);

      list = expand_string_unsplit (temp, 0);
      value = string_list (list);

      if (list)
	dispose_words (list);

      free (temp);
    }

  if (!value)
    value = savestring ("");

  nlen = strlen (name);
  vlen = strlen (value);
  temp = xmalloc (2 + nlen + vlen);
  strcpy (temp, name);
  temp[nlen] = '=';
  strcpy (temp + nlen + 1, value);
  free (name);
  free (value);

  if (!temporary_env)
    {
      temporary_env = (char **)xmalloc (sizeof (char *));
      temporary_env [0] = (char *)NULL;
    }

  size = array_len (temporary_env);
  temporary_env = (char **)
    xrealloc (temporary_env, (size + 2) * (sizeof (char *)));

  temporary_env[size] = (temp);
  temporary_env[size + 1] = (char *)NULL;
  array_needs_making = 1;

  if (echo_command_at_execute)
    {
      /* The K*rn shell prints the `+ ' in front of assignment statements,
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
  register int i, l = strlen (name);

  if (!array)
    return ((SHELL_VAR *)NULL);

  for (i = 0; array[i]; i++)
    {
      if (STREQN (array[i], name, l) && array[i][l] == '=')
	{
	  SHELL_VAR *temp;

	  temp = new_shell_variable (name);

	  if (array[i][l + 1])
	    temp->value = savestring (&array[i][l + 1]);
	  else
	    temp->value = (char *) NULL;

	  temp->attributes = att_exported;
	  temp->context = 0;
	  temp->prev_context = (SHELL_VAR *)NULL;

	  temp->dynamic_value = (DYNAMIC_FUNC *)NULL;
	  temp->assign_func = (DYNAMIC_FUNC *)NULL;

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

/* Sort ARRAY, a null terminated array of pointers to strings. */
void
sort_char_array (array)
     char **array;
{
  qsort (array, array_len (array), sizeof (char *),
	 (Function *)qsort_string_compare);
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

#ifdef SHADOWED_ENV
      export_env =
	(char **)xmalloc ((1 + array_len (shell_environment)) * sizeof (char *));

      for (i = 0; shell_environment[i]; i++)
	export_env[i] = savestring (shell_environment[i]);
      export_env[i] = (char *)NULL;

#else /* !SHADOWED_ENV */

      export_env = (char **)xmalloc (sizeof (char *));
      export_env[0] = (char *)NULL;

#endif /* SHADOWED_ENV */

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

      /* If we changed the array, then sort it alphabetically. */
      if (temporary_env || function_env)
	sort_char_array (export_env);

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

/* We supply our own version of getenv () because we want library
   routines to get the changed values of exported variables. */

/* The NeXT C library has getenv () defined and used in the same file.
   This screws our scheme.  However, Bash will run on the NeXT using
   the C library getenv (), since right now the only environment variable
   that we care about is HOME, and that is already defined.  */
#if !defined (NeXT) && !defined (HPOSF1)
static char *last_tempenv_value = (char *)NULL;
extern char **environ;

char *
getenv (name)
#if defined (Linux) || defined (__bsdi__) || defined (convex)
     const char *name;
#else
     char const *name;
#endif /* !Linux */
{
  SHELL_VAR *var = find_tempenv_variable ((char *)name);

  if (var)
    {
      FREE (last_tempenv_value);

      last_tempenv_value = savestring (value_cell (var));
      dispose_variable (var);
      return (last_tempenv_value);
    }
  else if (shell_variables)
    {
      var = find_variable ((char *)name);
      if (var && exported_p (var))
	return (value_cell (var));
    }
  else
    {
      register int i, len = strlen (name);

      /* In some cases, s5r3 invokes getenv() before main(); BSD systems
         using gprof also exhibit this behavior.  This means that
         shell_variables will be 0 when this is invoked.  We look up the
	 variable in the real environment in that case. */

      for (i = 0; environ[i]; i++)
	{
	  if ((STREQN (environ[i], name, len)) && (environ[i][len] == '='))
	    return (environ[i] + len + 1);
	}
    }

  return ((char *)NULL);
}
#endif /* !NeXT && !HPOSF1 */
