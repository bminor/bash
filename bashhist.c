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

#include "config.h"

#if defined (HISTORY)

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include "bashtypes.h"
#include <stdio.h>
#include <errno.h>
#include "bashansi.h"
#include "posixstat.h"
#include "filecntl.h"

#include "shell.h"
#include "flags.h"
#include "input.h"
#include "parser.h"	/* for the struct dstack stuff. */
#include "pathexp.h"	/* for the struct ignorevar stuff */
#include "builtins/common.h"

#include <readline/history.h>
#include <glob/fnmatch.h>

#if defined (READLINE)
#  include "bashline.h"
#endif

#if !defined (errno)
extern int errno;
#endif

extern int glob_pattern_p ();

static int histignore_item_func ();

static struct ignorevar histignore =
{
  "HISTIGNORE",
  (struct ign *)0,
  0,
  (char *)0,
  (Function *)histignore_item_func,
};

#define HIGN_EXPAND 0x01

/* Declarations of bash history variables. */
/* Non-zero means to remember lines typed to the shell on the history
   list.  This is different than the user-controlled behaviour; this
   becomes zero when we read lines from a file, for example. */
int remember_on_history = 1;

/* The number of lines that Bash has added to this history session. */
int history_lines_this_session;

/* The number of lines that Bash has read from the history file. */
int history_lines_in_file;

#if defined (BANG_HISTORY)
/* Non-zero means do no history expansion on this line, regardless
   of what history_expansion says. */
int history_expansion_inhibited;
#endif

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
int command_oriented_history = 1;

/* Non-zero means to store newlines in the history list when using
   command_oriented_history rather than trying to use semicolons. */
int literal_history;

/* Non-zero means to append the history to the history file at shell
   exit, even if the history has been stifled. */
int force_append_history;

/* A nit for picking at history saving.
   Value of 0 means save all lines parsed by the shell on the history.
   Value of 1 means save all lines that do not start with a space.
   Value of 2 means save all lines that do not match the last line saved. */
int history_control;

/* Set to 1 if the last command was added to the history list successfully
   as a separate history entry; set to 0 if the line was ignored or added
   to a previous entry as part of command-oriented-history processing. */
int hist_last_line_added;

#if defined (READLINE)
/* If non-zero, and readline is being used, the user is offered the
   chance to re-edit a failed history expansion. */
int history_reediting;

/* If non-zero, and readline is being used, don't directly execute a
   line with history substitution.  Reload it into the editing buffer
   instead and let the user further edit and confirm with a newline. */
int hist_verify;

#endif /* READLINE */

/* Variables declared in other files used here. */
extern int interactive;
extern int current_command_line_count;

extern struct dstack dstack;

extern char *extract_colon_unit ();
extern char *history_delimiting_chars ();
extern void maybe_add_history ();	/* forward declaration */
extern void bash_add_history ();	/* forward declaration */

static int history_should_ignore ();

/* Is the history expansion starting at string[i] one that should not
   be expanded? */
static int
bash_history_inhibit_expansion (string, i)
     char *string;
     int i;
{
  /* The shell uses ! as a pattern negation character in globbing [...]
     expressions, so let those pass without expansion. */
  if (i > 0 && (string[i - 1] == '[') && member (']', string + i + 1))
    return (1);
  /* The shell uses ! as the indirect expansion character, so let those
     expansions pass as well. */
  else if (i > 1 && string[i - 1] == '{' && string[i - 2] == '$' &&
	     member ('}', string + i + 1))
    return (1);
#if defined (EXTENDED_GLOB)
  else if (extended_glob && i > 1 && string[i+1] == '(' && member (')', string + i + 2))
    return (1);
#endif
  else
    return (0);
}

void
bash_initialize_history ()
{
  history_quotes_inhibit_expansion = 1;
  history_search_delimiter_chars = ";&()|<>";
  history_inhibit_expansion_function = bash_history_inhibit_expansion;
}

void
bash_history_reinit (interact)
     int interact;
{
#if defined (BANG_HISTORY)
  history_expansion = interact != 0;
  history_expansion_inhibited = 1;
#endif
  remember_on_history = interact != 0;
  history_inhibit_expansion_function = bash_history_inhibit_expansion;
}

void
bash_history_disable ()
{
  remember_on_history = 0;
#if defined (BANG_HISTORY)
  history_expansion_inhibited = 1;
#endif
}

void
bash_history_enable ()
{
  remember_on_history = 1;
#if defined (BANG_HISTORY)
  history_expansion_inhibited = 0;
#endif
  history_inhibit_expansion_function = bash_history_inhibit_expansion;
  sv_history_control ("HISTCONTROL");
  sv_histignore ("HISTIGNORE");
}

/* Load the history list from the history file. */
void
load_history ()
{
  char *hf;
  struct stat buf;

  /* Truncate history file for interactive shells which desire it.
     Note that the history file is automatically truncated to the
     size of HISTSIZE if the user does not explicitly set the size
     differently. */
  set_if_not ("HISTFILESIZE", get_string_value ("HISTSIZE"));
  sv_histsize ("HISTFILESIZE");

  /* Read the history in HISTFILE into the history list. */
  hf = get_string_value ("HISTFILE");

  if (hf && *hf && stat (hf, &buf) == 0)
    {
      read_history (hf);
      using_history ();
      history_lines_in_file = where_history ();
    }
}

#ifdef INCLUDE_UNUSED
/* Write the existing history out to the history file. */
void
save_history ()
{
  char *hf;
  struct stat buf;

  hf = get_string_value ("HISTFILE");
  if (hf && *hf && stat (hf, &buf) == 0)
    {
      /* Append only the lines that occurred this session to
	 the history file. */
      using_history ();

      if (history_lines_this_session < where_history () || force_append_history)
	append_history (history_lines_this_session, hf);
      else
	write_history (hf);

      sv_histsize ("HISTFILESIZE");
    }
}
#endif

int
maybe_append_history (filename)
     char *filename;
{
  int fd, result;
  struct stat buf;

  result = EXECUTION_SUCCESS;
  if (history_lines_this_session && (history_lines_this_session < where_history ()))
    {
      /* If the filename was supplied, then create it if necessary. */
      if (stat (filename, &buf) == -1 && errno == ENOENT)
	{
	  fd = open (filename, O_WRONLY|O_CREAT, 0600);
	  if (fd < 0)
	    {
	      builtin_error ("%s: cannot create: %s", filename, strerror (errno));
	      return (EXECUTION_FAILURE);
	    }
	  close (fd);
	}
      result = append_history (history_lines_this_session, filename);
      history_lines_in_file += history_lines_this_session;
      history_lines_this_session = 0;
    }
  return (result);
}

/* If this is an interactive shell, then append the lines executed
   this session to the history file. */
int
maybe_save_shell_history ()
{
  int result;
  char *hf;
  struct stat buf;

  result = 0;
  if (history_lines_this_session)
    {
      hf = get_string_value ("HISTFILE");

      if (hf && *hf)
	{
	  /* If the file doesn't exist, then create it. */
	  if (stat (hf, &buf) == -1)
	    {
	      int file;
	      file = open (hf, O_CREAT | O_TRUNC | O_WRONLY, 0600);
	      if (file != -1)
		close (file);
	    }

	  /* Now actually append the lines if the history hasn't been
	     stifled.  If the history has been stifled, rewrite the
	     history file. */
	  using_history ();
	  if (history_lines_this_session <= where_history () || force_append_history)
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

	  sv_histsize ("HISTFILESIZE");
	}
    }
  return (result);
}

#if defined (READLINE)
/* Tell readline () that we have some text for it to edit. */
static void
re_edit (text)
     char *text;
{
  if (bash_input.type == st_stdin)
    bash_re_edit (text);
}
#endif /* READLINE */

/* Return 1 if this line needs history expansion. */
static int
history_expansion_p (line)
     char *line;
{
  register char *s;

  for (s = line; *s; s++)
    if (*s == history_expansion_char || *s == history_subst_char)
      return 1;
  return 0;
}

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
  int expanded;

  return_value = line;
  expanded = 0;

#  if defined (BANG_HISTORY)
  /* History expand the line.  If this results in no errors, then
     add that line to the history if ADDIT is non-zero. */
  if (!history_expansion_inhibited && history_expansion && history_expansion_p (line))
    {
      expanded = history_expand (line, &history_value);

      if (expanded)
	{
	  if (print_changes)
	    {
	      if (expanded < 0)
		internal_error ("%s", history_value);
#if defined (READLINE)
	      else if (hist_verify == 0)
#else
	      else
#endif
		fprintf (stderr, "%s\n", history_value);
	    }

	  /* If there was an error, return NULL. */
	  if (expanded < 0 || expanded == 2)	/* 2 == print only */
	    {
	      free (history_value);

#    if defined (READLINE)
	      /* New hack.  We can allow the user to edit the
		 failed history expansion. */
	      if (history_reediting && expanded < 0)
		re_edit (line);
#    endif /* READLINE */
	      return ((char *)NULL);
	    }

#    if defined (READLINE)
	  if (hist_verify && expanded == 1)
	    {
	      re_edit (history_value);
	      return ((char *)NULL);
	    }
#    endif
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

#if 0
  if (expanded == 0)
    return_value = savestring (line);
#endif

  return (return_value);
}

/* Add LINE to the history list depending on the value of HISTORY_CONTROL. */
void
maybe_add_history (line)
     char *line;
{
  int should_add;
  HIST_ENTRY *temp;

  should_add = hist_last_line_added = 0;

  /* Don't use the value of history_control to affect the second
     and subsequent lines of a multi-line command (old code did
     this only when command_oriented_history is enabled). */
#if 0
  if (command_oriented_history && current_command_line_count > 1)
#else
  if (current_command_line_count > 1)
#endif
    {
      bash_add_history (line);
      return;
    }

  switch (history_control)
    {
    case 0:
      should_add = 1;
      break;
    case 1:
      if (*line != ' ')
	should_add = 1;
      break;
    case 3:
      if (*line == ' ')
	break;
      /* FALLTHROUGH if case == 3 (`ignoreboth') */
    case 2:
      using_history ();
      temp = previous_history ();

      if (temp == 0 || STREQ (temp->line, line) == 0)
	should_add = 1;

      using_history ();
      break;
    }

  if (should_add && history_should_ignore (line) == 0)
    bash_add_history (line);
}

/* Add a line to the history list.
   The variable COMMAND_ORIENTED_HISTORY controls the style of history
   remembering;  when non-zero, and LINE is not the first line of a
   complete parser construct, append LINE to the last history line instead
   of adding it as a new line. */
void
bash_add_history (line)
     char *line;
{
  int add_it, offset, curlen;
  HIST_ENTRY *current, *old;
  char *chars_to_add, *new_line;

  add_it = 1;
  if (command_oriented_history && current_command_line_count > 1)
    {
      chars_to_add = literal_history ? "\n" : history_delimiting_chars ();

      using_history ();
      current = previous_history ();

      if (current)
	{
	  /* If the previous line ended with an escaped newline (escaped
	     with backslash, but otherwise unquoted), then remove the quoted
	     newline, since that is what happens when the line is parsed. */
	  curlen = strlen (current->line);

	  if (dstack.delimiter_depth == 0 && current->line[curlen - 1] == '\\' &&
	      current->line[curlen - 2] != '\\')
	    {
	      current->line[curlen - 1] = '\0';
	      curlen--;
	      chars_to_add = "";
	    }

	  new_line = (char *) xmalloc (1
				       + curlen
				       + strlen (line)
				       + strlen (chars_to_add));
	  sprintf (new_line, "%s%s%s", current->line, chars_to_add, line);
	  offset = where_history ();
	  old = replace_history_entry (offset, new_line, current->data);
	  free (new_line);

	  if (old)
	    {
	      FREE (old->line);
	      free (old);
	    }
	  add_it = 0;
	}
    }

  if (add_it)
    {
      hist_last_line_added = 1;
      add_history (line);
      history_lines_this_session++;
    }
  using_history ();
}

int
history_number ()
{
  using_history ();
  return (get_string_value ("HISTSIZE") ? history_base + where_history () : 1);
}

static int
should_expand (s)
     char *s;
{
  char *p;

  for (p = s; p && *p; p++)
    {
      if (*p == '\\')
	p++;
      else if (*p == '&')
	return 1;
    }
  return 0;
}

static int
histignore_item_func (ign)
     struct ign *ign;
{
  if (should_expand (ign->val))
    ign->flags |= HIGN_EXPAND;
  return (0);
}

void
setup_history_ignore (varname)
     char *varname;
{
  setup_ignore_patterns (&histignore);
}

static HIST_ENTRY *
last_history_entry ()
{
  HIST_ENTRY *he;

  using_history ();
  he = previous_history ();
  using_history ();
  return he;
}

char *
last_history_line ()
{
  HIST_ENTRY *he;

  he = last_history_entry ();
  if (he == 0)
    return ((char *)NULL);
  return he->line;
}

static char *
expand_histignore_pattern (pat)
     char *pat;
{
  HIST_ENTRY *phe;
  char *ret, *p, *r, *t;
  int len, rlen, ind, tlen;

  phe = last_history_entry ();

  if (phe == (HIST_ENTRY *)0)
    return (savestring (pat));

  len = strlen (phe->line);
  rlen = len + strlen (pat) + 2;
  ret = xmalloc (rlen);

  for (p = pat, r = ret; p && *p; )
    {
      if (*p == '&')
	{
	  ind = r - ret;
	  if (glob_pattern_p (phe->line) || strchr (phe->line, '\\'))
	    {
	      t = quote_globbing_chars (phe->line);
	      tlen = strlen (t);
	      RESIZE_MALLOCED_BUFFER (ret, ind, tlen, rlen, rlen);
	      r = ret + ind;	/* in case reallocated */
	      strcpy (r, t);
	      r += tlen;
	      free (t);
	    }
	  else
	    {
	      tlen = strlen (phe->line);
	      RESIZE_MALLOCED_BUFFER (ret, ind, tlen, rlen, rlen);
	      r = ret + ind;	/* in case reallocated */
	      strcpy (r, phe->line);
	      r += len;
	    }
	  p++;
	  continue;
	}

      if (*p == '\\' && p[1] == '&')
	p++;

      *r++ = *p++;
    }
  *r = '\0';
  return ret;
}

/* Return 1 if we should not put LINE into the history according to the
   patterns in HISTIGNORE. */
static int
history_should_ignore (line)
     char *line;
{
  register int i, match;
  char *npat;

  if (histignore.num_ignores == 0)
    return 0;

  for (i = match = 0; i < histignore.num_ignores; i++)
    {
      if (histignore.ignores[i].flags & HIGN_EXPAND)
	npat = expand_histignore_pattern (histignore.ignores[i].val);
      else
	npat = histignore.ignores[i].val;

      match = fnmatch (npat, line, FNMATCH_EXTFLAG) != FNM_NOMATCH;

      if (histignore.ignores[i].flags & HIGN_EXPAND)
	free (npat);

      if (match)
	break;
    }

  return match;
}
#endif /* HISTORY */
