/* bashhist.c -- bash interface to the GNU history library. */

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

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include "bashansi.h"
#include "posixstat.h"
#include "filecntl.h"
#include "shell.h"
#include "flags.h"
#include <readline/history.h>

/* Declarations of bash history variables. */
/* Non-zero means to remember lines typed to the shell on the history
   list.  This is different than the user-controlled behaviour; this
   becomes zero when we read lines from a file, for example. */
int remember_on_history = 1;

/* The number of lines that Bash has added to this history session. */
int history_lines_this_session = 0;

/* The number of lines that Bash has read from the history file. */
int history_lines_in_file = 0;

/* Non-zero means do no history expansion on this line, regardless
   of what history_expansion says. */
int history_expansion_inhibited = 0;

/* By default, every line is saved in the history individually.  I.e.,
   if the user enters:
	bash$ for i in a b c
        > do
        > echo $i
        > done
   Each line will be individually saved in the history.
	bash$ history
	10  for i in a b c
        11  do
        12  echo $i
        13  done
        14  history
   If the variable command_oriented_history is set, multiple lines
   which form one command will be saved as one history entry.
	bash$ for i in a b c
        > do
        > echo $i
        > done
        bash$ history
	10  for i in a b c
    do
    echo $i
    done
        11  history
   The user can then recall the whole command all at once instead
   of just being able to recall one line at a time.
   */
int command_oriented_history = 0;

/* A nit for picking at history saving.
   Value of 0 means save all lines parsed by the shell on the history.
   Value of 1 means save all lines that do not start with a space.
   Value of 2 means save all lines that do not match the last line saved. */
int history_control = 0;

/* Variables declared in other files used here. */
extern int interactive;
extern int current_command_line_count;
extern int delimiter_depth;

extern char *history_delimiting_chars ();
extern void maybe_add_history ();	/* forward declaration */

static void bash_add_history ();

/* Load the history list from the history file. */
void
load_history ()
{
  char *hf;

  /* Truncate history file for interactive shells which desire it.
     Note that the history file is automatically truncated to the
     size of HISTSIZE if the user does not explicitly set the size
     differently. */
  set_if_not ("HISTFILESIZE", get_string_value ("HISTSIZE"));
  stupidly_hack_special_variables ("HISTFILESIZE");

  /* Read the history in HISTFILE into the history list. */
  hf = get_string_value ("HISTFILE");

  if (hf && *hf)
    {
      struct stat buf;

      if (stat (hf, &buf) == 0)
	{
	  read_history (hf);
	  using_history ();
	  history_lines_in_file = where_history ();
	}
    }
}

/* Write the existing history out to the history file. */
void
save_history ()
{
  char *hf = get_string_value ("HISTFILE");

  if (hf && *hf)
    {
      struct stat buf;

      if (stat (hf, &buf) == 0)
	{
	  /* Append only the lines that occurred this session to
	     the history file. */
	  using_history ();

	  if (history_lines_this_session < where_history ())
	    append_history (history_lines_this_session, hf);
	  else
	    write_history (hf);
	}
    }
}

/* If this is an interactive shell, then append the lines executed
   this session to the history file. */
int
maybe_save_shell_history ()
{
  int result = 0;

  if (history_lines_this_session)
    {
      char *hf = get_string_value ("HISTFILE");

      if (hf && *hf)
	{
	  struct stat buf;

	  /* If the file doesn't exist, then create it. */
	  if (stat (hf, &buf) == -1)
	    {
	      int file = open (hf, O_CREAT | O_TRUNC | O_WRONLY, 0666);
	      if (file != -1)
		close (file);
	    }

	  /* Now actually append the lines if the history hasn't been
	     stifled.  If the history has been stifled, rewrite the
	     history file. */
	  using_history ();
	  if (history_lines_this_session <= where_history ())
	    {
	      result = append_history (history_lines_this_session, hf);
	      history_lines_in_file += history_lines_this_session;
	    }
	  else
	    {
	      result = write_history (hf);
	      history_lines_in_file = history_lines_this_session;
	    }
	  history_lines_this_session = 0;
	}
    }
  return (result);
}

#if defined (HISTORY_REEDITING)
/* Tell readline () that we have some text for it to edit. */
static void
re_edit (text)
     char *text;
{
#if defined (READLINE)
  if (strcmp (bash_input.name, "readline stdin") == 0)
    bash_re_edit (text);
#endif /* READLINE */
}
#endif /* HISTORY_REEDITING */

/* Do pre-processing on LINE.  If PRINT_CHANGES is non-zero, then
   print the results of expanding the line if there were any changes.
   If there is an error, return NULL, otherwise the expanded line is
   returned.  If ADDIT is non-zero the line is added to the history
   list after history expansion.  ADDIT is just a suggestion;
   REMEMBER_ON_HISTORY can veto, and does.
   Right now this does history expansion. */
char *
pre_process_line (line, print_changes, addit)
     char *line;
     int print_changes, addit;
{
  char *history_value;
  char *return_value;
  int expanded = 0;

  return_value = line;

#  if defined (BANG_HISTORY)
  /* History expand the line.  If this results in no errors, then
     add that line to the history if ADDIT is non-zero. */
  if (!history_expansion_inhibited && history_expansion)
    {
      expanded = history_expand (line, &history_value);

      if (expanded)
	{
	  if (print_changes)
	    {
	      if (expanded < 0)
		internal_error (history_value);
	      else
		fprintf (stderr, "%s\n", history_value);
	    }

	  /* If there was an error, return NULL. */
	  if (expanded < 0 || expanded == 2)	/* 2 == print only */
	    {
	      free (history_value);

#    if defined (HISTORY_REEDITING)
	      /* New hack.  We can allow the user to edit the
		 failed history expansion. */
	      re_edit (line);
#    endif /* HISTORY_REEDITING */
	      return ((char *)NULL);
	    }
	}

      /* Let other expansions know that return_value can be free'ed,
	 and that a line has been added to the history list.  Note
	 that we only add lines that have something in them. */
      expanded = 1;
      return_value = history_value;
    }
#  endif /* BANG_HISTORY */

  if (addit && remember_on_history && *return_value)
    maybe_add_history (return_value);

  if (!expanded)
    return_value = savestring (line);

  return (return_value);
}

/* Add LINE to the history list depending on the value of HISTORY_CONTROL. */
void
maybe_add_history (line)
     char *line;
{
  int h;

  /* Don't use the value of history_control to affect the second
     and subsequent lines of a multi-line command when
     command_oriented_history is enabled. */
  if (command_oriented_history && current_command_line_count > 1)
    h = 0;
  else
    h = history_control;

  switch (h)
    {
    case 0:
      bash_add_history (line);
      break;
    case 1:
      if (*line != ' ')
        bash_add_history (line);
      break;
    case 3:
      if (*line == ' ')
        break;
      /* FALLTHROUGH if case == 3 (`ignoreboth') */
    case 2:
      {
	HIST_ENTRY *temp;

	using_history ();
	temp = previous_history ();

	if (!temp || (STREQ (temp->line, line) == 0))
	  bash_add_history (line);

	using_history ();
      }
      break;
    }
}

/* Add a line to the history list.
   The variable COMMAND_ORIENTED_HISTORY controls the style of history
   remembering;  when non-zero, and LINE is not the first line of a
   complete parser construct, append LINE to the last history line instead
   of adding it as a new line. */
static void
bash_add_history (line)
     char *line;
{
  int add_it = 1;

  if (command_oriented_history && current_command_line_count > 1)
    {
      register int offset;
      register HIST_ENTRY *current, *old;
      char *chars_to_add, *new_line;

      chars_to_add = history_delimiting_chars ();

      using_history ();

      current = previous_history ();

      if (current)
	{
	  /* If the previous line ended with an escaped newline (escaped
	     with backslash, but otherwise unquoted), then remove the quoted
	     newline, since that is what happens when the line is parsed. */
	  int curlen;

	  curlen = strlen (current->line);

	  if (!delimiter_depth && current->line[curlen - 1] == '\\' &&
	      current->line[curlen - 2] != '\\')
	    {
	      current->line[curlen - 1] = '\0';
	      curlen--;
	      chars_to_add = "";
	    }

	  offset = where_history ();
	  new_line = (char *) xmalloc (1
				       + curlen
				       + strlen (line)
				       + strlen (chars_to_add));
	  sprintf (new_line, "%s%s%s", current->line, chars_to_add, line);
	  old = replace_history_entry (offset, new_line, current->data);
	  free (new_line);

	  if (old)
	    {
	      /* Note that the old data is not freed, since it was simply
		 copied to the new history entry. */
	      if (old->line)
		free (old->line);

	      free (old);
	    }
	  add_it = 0;
	}
    }

  if (add_it)
    {
      add_history (line);
      history_lines_this_session++;
    }
  using_history ();
}

int
history_number ()
{
  using_history ();
  if (get_string_value ("HISTSIZE"))
    return (history_base + where_history ());
  else
    return (1);		/* default to command number 1 */
}
