/* flags.c -- Everything about flags except the `set' command.  That
   is in builtins.c */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

/* Flags hacking. */

#include "shell.h"
#include "flags.h"

/* **************************************************************** */
/*								    */
/*			The Standard Sh Flags.			    */
/*								    */
/* **************************************************************** */

/* Non-zero means automatically mark variables which are modified or created
   as auto export variables. */
int mark_modified_vars = 0;

/* Non-zero causes asynchronous job notification.  Otherwise, job state
   notification only takes place just before a primary prompt is printed. */
int asynchronous_notification = 0;

/* Non-zero means exit immediately if a command exits with a non-zero
   exit status. */
int exit_immediately_on_error = 0;

/* Non-zero means disable filename globbing. */
int disallow_filename_globbing = 0;

/* Non-zero means to locate and remember function commands as functions are
   defined.  Function commands are normally located when the function is
   executed. */
int locate_commands_in_functions = 0;

/* Non-zero means that all keyword arguments are placed into the environment
   for a command, not just those that appear on the line before the command
   name. */
int place_keywords_in_env = 0;

/* Non-zero means read commands, but don't execute tham.  This is useful
   for debugging shell scripts that should do something hairy and possibly
   desctructive. */
int read_but_dont_execute = 0;

/* Non-zero means end of file is after one command. */
int just_one_command = 0;

/* Non-zero means don't overwrite existing files while doing redirections. */
int noclobber = 0;

/* Non-zero means trying to get the value of $i where $i is undefined
   causes an error, instead of a null substitution. */
int unbound_vars_is_error = 0;

/* Non-zero means type out input lines after you read them. */
int echo_input_at_read = 0;

/* Non-zero means type out the command definition after reading, but
   before executing. */
int echo_command_at_execute = 0;

/* Non-zero means turn on the job control features. */
int jobs_m_flag = 0;

/* Non-zero means this shell is interactive, even if running under a
   pipe. */
int forced_interactive = 0;

/* By default, follow the symbolic links as if they were real directories
   while hacking the `cd' command.  This means that `cd ..' moves up in
   the string of symbolic links that make up the current directory, instead
   of the absolute directory.  The shell variable `nolinks' also controls
   this flag. */
int no_symbolic_links = 0;

/* **************************************************************** */
/*								    */
/*		     Non-Standard Flags Follow Here.		    */
/*								    */
/* **************************************************************** */


/* Non-zero means do lexical scoping in the body of a FOR command. */
int lexical_scoping = 0;

/* Non-zero means no such thing as invisible variables. */
int no_invisible_vars = 0;

/* Non-zero means don't look up or remember command names in a hash table, */
int hashing_disabled = 0;

#if defined (BANG_HISTORY)
/* Non-zero means that we are doing history expansion.  The default.
   This means !22 gets the 22nd line of history. */
int history_expansion = 1;
#endif /* BANG_HISTORY */

/* Non-zero means that we allow comments to appear in interactive commands. */
#if defined (INTERACTIVE_COMMENTS)
int interactive_comments = 1;
#else
int interactive_comments = 0;
#endif /* INTERACTIVE_COMMENTS */

#if defined (RESTRICTED_SHELL)
/* Non-zero means that this shell is `restricted'.  A restricted shell
   disallows: changing directories, command or path names containing `/',
   unsetting or resetting the values of $PATH and $SHELL, and any type of
   output redirection. */
int restricted = 0;
#endif /* RESTRICTED_SHELL */

/* Non-zero means that this shell is running in `privileged' mode.  This
   mode is entered on startup if the real and effective uids or gids
   differ. */
int privileged_mode = 0;

/* **************************************************************** */
/*								    */
/*			The Flags ALIST.			    */
/*								    */
/* **************************************************************** */

struct flags_alist shell_flags[] = {
  /* Standard sh flags. */
  { 'a', &mark_modified_vars },
#if defined (JOB_CONTROL)
  { 'b', &asynchronous_notification },
#endif /* JOB_CONTROL */
  { 'e', &exit_immediately_on_error },
  { 'f', &disallow_filename_globbing },
  { 'h', &locate_commands_in_functions }, /* Oh, yeah, good mnemonic. */
  { 'i', &forced_interactive },
  { 'k', &place_keywords_in_env },
#if defined (JOB_CONTROL)
  { 'm', &jobs_m_flag },
#endif /* JOB_CONTROL */
  { 'n', &read_but_dont_execute },
  { 'p', &privileged_mode },
#if defined (RESTRICTED_SHELL)
  { 'r', &restricted },
#endif /* RESTRICTED_SHELL */
  { 't', &just_one_command },
  { 'u', &unbound_vars_is_error },
  { 'v', &echo_input_at_read },
  { 'x', &echo_command_at_execute },
  { 'C', &noclobber },

  /* New flags that control non-standard things. */
  { 'l', &lexical_scoping },
  { 'I', &no_invisible_vars },

  /* I want `h', but locate_commands_in_functions has it.  Great. */
  { 'd', &hashing_disabled },

  { 'P', &no_symbolic_links },

#if defined (BANG_HISTORY)
  /* Once again, we don't have the right mnemonic. */
  { 'H', &history_expansion },
#endif /* BANG_HISTORY */

  {0, (int *)NULL}
};

#define NUM_SHELL_FLAGS (sizeof (shell_flags) / sizeof (struct flags_alist))

int *
find_flag (name)
     int name;
{
  int i = 0;
  while (shell_flags[i].name)
    {
      if (shell_flags[i].name == name)
	return (shell_flags[i].value);
      i++;
    }
  return (FLAG_UNKNOWN);
}

/* Change the state of a flag, and return it's original value, or return
   FLAG_ERROR if there is no flag called NAME.  ON_OR_OFF should be one
   of FLAG_ON or FLAG_OFF. */
int
change_flag (flag, on_or_off)
  int flag;
  int on_or_off;
{
  int *value = find_flag (flag);
  int old_value;

#if defined (RESTRICTED_SHELL)
  /* Don't allow "set +r" in a shell which is `restricted'. */
  if (restricted && flag == 'r' && on_or_off == FLAG_OFF)
    return (FLAG_ERROR);
#endif /* RESTRICTED_SHELL */

  if (value == (int *)FLAG_UNKNOWN)
    return (FLAG_ERROR);
  else
    old_value = *value;

  if (on_or_off == FLAG_ON)
    *value = 1;
  else
    {
      if (on_or_off == FLAG_OFF)
	*value = 0;
      else
	return (FLAG_ERROR);
    }

  /* Special cases for a few flags. */
  switch (flag)
    {
#if defined (JOB_CONTROL)
    case 'm':
      set_job_control (on_or_off == '-');
      break;
#endif /* JOB_CONTROL */

    case 'p':
      if (on_or_off == '+')
	{
	  setuid (current_user.uid);
	  setgid (current_user.gid);
	  current_user.euid = current_user.uid;
	  current_user.egid = current_user.gid;
	}
      break;
    }
    
  return (old_value);
}

/* Return a string which is the names of all the currently
   set shell flags. */
char *
which_set_flags ()
{
  char *temp = (char *)xmalloc (1 + NUM_SHELL_FLAGS);

  int i, string_index = 0;

  for (i = 0; shell_flags[i].name; i++)
    if (*(shell_flags[i].value))
      temp[string_index++] = shell_flags[i].name;

  temp[string_index] = '\0';
  return (temp);
}
