/* bashline.c -- Bash's interface to the readline library. */

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

#include "config.h"

#if defined (READLINE)

#include "bashtypes.h"
#include "posixstat.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include "bashansi.h"
#include "shell.h"
#include "builtins.h"
#include "bashhist.h"
#include "bashline.h"
#include "execute_cmd.h"
#include "findcmd.h"
#include "pathexp.h"
#include "builtins/common.h"
#include <readline/rlconf.h>
#include <readline/readline.h>
#include <readline/history.h>

#include <glob/glob.h>

#if defined (ALIAS)
#  include "alias.h"
#endif

#if defined (BRACE_COMPLETION)
extern void bash_brace_completion ();
#endif /* BRACE_COMPLETION */

/* Functions bound to keys in Readline for Bash users. */
static void shell_expand_line ();
static void display_shell_version (), operate_and_get_next ();
static void bash_ignore_filenames ();
static void cleanup_expansion_error (), set_up_new_line ();

#if defined (BANG_HISTORY)
static int history_expand_line ();
static int tcsh_magic_space ();
#endif /* BANG_HISTORY */
#ifdef ALIAS
static int alias_expand_line ();
#endif
#if defined (BANG_HISTORY) && defined (ALIAS)
static int history_and_alias_expand_line ();
#endif

/* Helper functions for Readline. */
static int bash_directory_completion_hook ();
static void filename_completion_ignore ();
static void bash_push_line ();

static char **attempt_shell_completion ();
static char *variable_completion_function ();
static char *hostname_completion_function ();
static char *command_word_completion_function ();
static char *command_subst_completion_function ();
static void dynamic_complete_history ();

static char *glob_complete_word ();
static void bash_glob_expand_word ();
static void bash_glob_list_expansions ();

static void snarf_hosts_from_file (), add_host_name ();

static char *bash_dequote_filename ();
static char *bash_quote_filename ();

#if defined (ALIAS)
static int posix_edit_macros ();
#endif

/* Variables used here but defined in other files. */
extern int posixly_correct, no_symbolic_links;
extern int rl_explicit_arg;
extern char *current_prompt_string, *ps1_prompt;
extern STRING_INT_ALIST word_token_alist[];
extern Function *rl_last_func;
extern int rl_filename_completion_desired;

/* Helper functions from subst.c */
extern int char_is_quoted ();
extern int unclosed_pair ();

/* SPECIFIC_COMPLETION_FUNCTIONS specifies that we have individual
   completion functions which indicate what type of completion should be
   done (at or before point) that can be bound to key sequences with
   the readline library. */
#define SPECIFIC_COMPLETION_FUNCTIONS

#if defined (SPECIFIC_COMPLETION_FUNCTIONS)
static void bash_specific_completion ();
static void bash_complete_filename (), bash_possible_filename_completions ();
static void bash_complete_filename_internal ();
static void bash_complete_username (), bash_possible_username_completions ();
static void bash_complete_username_internal ();
static void bash_complete_hostname (), bash_possible_hostname_completions ();
static void bash_complete_hostname_internal ();
static void bash_complete_variable (), bash_possible_variable_completions ();
static void bash_complete_variable_internal ();
static void bash_complete_command (), bash_possible_command_completions ();
static void bash_complete_command_internal ();
#endif /* SPECIFIC_COMPLETION_FUNCTIONS */

#if defined (VI_MODE)
static void vi_edit_and_execute_command ();
#endif

/* Non-zero once initalize_readline () has been called. */
int bash_readline_initialized = 0;

/* If non-zero, we do hostname completion, breaking words at `@' and
   trying to complete the stuff after the `@' from our own internal
   host list. */
int perform_hostname_completion = 1;

static char *bash_completer_word_break_characters = " \t\n\"'@><=;|&(:";
static char *bash_nohostname_word_break_characters = " \t\n\"'><=;|&(:";

static Function *old_rl_startup_hook = (Function *) NULL;

/* What kind of quoting is performed by bash_quote_filename:
	COMPLETE_DQUOTE = double-quoting the filename
	COMPLETE_SQUOTE = single_quoting the filename
	COMPLETE_BSQUOTE = backslash-quoting special chars in the filename
*/
#define COMPLETE_DQUOTE  1
#define COMPLETE_SQUOTE  2
#define COMPLETE_BSQUOTE 3
static int completion_quoting_style = COMPLETE_BSQUOTE;

/* Change the readline VI-mode keymaps into or out of Posix.2 compliance.
   Called when the shell is put into or out of `posix' mode. */
void
posix_readline_initialize (on_or_off)
     int on_or_off;
{
  if (on_or_off)
    rl_variable_bind ("comment-begin", "#");
#if defined (VI_MODE)
  rl_bind_key_in_map (CTRL('I'), on_or_off ? rl_insert : rl_complete, vi_insertion_keymap);
#endif
}

void
enable_hostname_completion (on_or_off)
     int on_or_off;
{
  if (on_or_off)
    {
      perform_hostname_completion = 1;
      rl_special_prefixes = "$@";
      rl_completer_word_break_characters = bash_completer_word_break_characters;
    }
  else
    {
      perform_hostname_completion = 0;
      rl_special_prefixes = "$";
      rl_completer_word_break_characters = bash_nohostname_word_break_characters;
    }
}

/* Called once from parse.y if we are going to use readline. */
void
initialize_readline ()
{
  if (bash_readline_initialized)
    return;

  rl_terminal_name = get_string_value ("TERM");
  rl_instream = stdin;
  rl_outstream = stderr;

  /* Allow conditional parsing of the ~/.inputrc file. */
  rl_readline_name = "Bash";

  /* Bind up our special shell functions. */
  rl_add_defun ("shell-expand-line", (Function *)shell_expand_line, -1);
  rl_bind_key_in_map (CTRL('E'), (Function *)shell_expand_line, emacs_meta_keymap);

  /* Bind up our special shell functions. */
#ifdef BANG_HISTORY
  rl_add_defun ("history-expand-line", (Function *)history_expand_line, -1);
  rl_bind_key_in_map ('^', (Function *)history_expand_line, emacs_meta_keymap);

  rl_add_defun ("magic-space", (Function *)tcsh_magic_space, -1);
#endif

#ifdef ALIAS
  rl_add_defun ("alias-expand-line", (Function *)alias_expand_line, -1);
  rl_add_defun ("history-and-alias-expand-line", (Function *)history_and_alias_expand_line, -1);
#endif

  /* Backwards compatibility. */
  rl_add_defun ("insert-last-argument", rl_yank_last_arg, -1);

  rl_add_defun
    ("operate-and-get-next", (Function *)operate_and_get_next, CTRL('O'));

  rl_add_defun
    ("display-shell-version", (Function *)display_shell_version, -1);
  rl_bind_key_in_map
    (CTRL ('V'), (Function *)display_shell_version, emacs_ctlx_keymap);

  /* In Bash, the user can switch editing modes with "set -o [vi emacs]",
     so it is not necessary to allow C-M-j for context switching.  Turn
     off this occasionally confusing behaviour. */
  rl_unbind_key_in_map (CTRL('J'), emacs_meta_keymap);
  rl_unbind_key_in_map (CTRL('M'), emacs_meta_keymap);
#if defined (VI_MODE)
  rl_unbind_key_in_map (CTRL('E'), vi_movement_keymap);
#endif

#if defined (BRACE_COMPLETION)
  rl_add_defun ("complete-into-braces", bash_brace_completion, -1);
  rl_bind_key_in_map ('{', bash_brace_completion, emacs_meta_keymap);
#endif /* BRACE_COMPLETION */

#if defined (SPECIFIC_COMPLETION_FUNCTIONS)
  rl_add_defun ("complete-filename", bash_complete_filename, -1);
  rl_bind_key_in_map ('/', bash_complete_filename, emacs_meta_keymap);
  rl_add_defun ("possible-filename-completions",
		bash_possible_filename_completions, -1);
  rl_bind_key_in_map ('/', bash_possible_filename_completions, emacs_ctlx_keymap);

  rl_add_defun ("complete-username", bash_complete_username, -1);
  rl_bind_key_in_map ('~', bash_complete_username, emacs_meta_keymap);
  rl_add_defun ("possible-username-completions",
		bash_possible_username_completions, -1);
  rl_bind_key_in_map ('~', bash_possible_username_completions, emacs_ctlx_keymap);

  rl_add_defun ("complete-hostname", bash_complete_hostname, -1);
  rl_bind_key_in_map ('@', bash_complete_hostname, emacs_meta_keymap);
  rl_add_defun ("possible-hostname-completions",
		bash_possible_hostname_completions, -1);
  rl_bind_key_in_map ('@', bash_possible_hostname_completions, emacs_ctlx_keymap);

  rl_add_defun ("complete-variable", bash_complete_variable, -1);
  rl_bind_key_in_map ('$', bash_complete_variable, emacs_meta_keymap);
  rl_add_defun ("possible-variable-completions",
		bash_possible_variable_completions, -1);
  rl_bind_key_in_map ('$', bash_possible_variable_completions, emacs_ctlx_keymap);

  rl_add_defun ("complete-command", bash_complete_command, -1);
  rl_bind_key_in_map ('!', bash_complete_command, emacs_meta_keymap);
  rl_add_defun ("possible-command-completions",
		bash_possible_command_completions, -1);
  rl_bind_key_in_map ('!', bash_possible_command_completions, emacs_ctlx_keymap);

  rl_add_defun ("glob-expand-word", bash_glob_expand_word, -1);
  rl_add_defun ("glob-list-expansions", bash_glob_list_expansions, -1);
  rl_bind_key_in_map ('*', bash_glob_expand_word, emacs_ctlx_keymap);
  rl_bind_key_in_map ('g', bash_glob_list_expansions, emacs_ctlx_keymap);

#endif /* SPECIFIC_COMPLETION_FUNCTIONS */

  rl_add_defun ("dynamic-complete-history", dynamic_complete_history, -1);
  rl_bind_key_in_map (TAB, dynamic_complete_history, emacs_meta_keymap);

  /* Tell the completer that we want a crack first. */
  rl_attempted_completion_function = (CPPFunction *)attempt_shell_completion;

  /* Tell the completer that we might want to follow symbolic links or
     do other expansion on directory names. */
  rl_directory_completion_hook = bash_directory_completion_hook;

  /* Tell the filename completer we want a chance to ignore some names. */
  rl_ignore_some_completions_function = (Function *)filename_completion_ignore;

#if defined (VI_MODE)
  rl_bind_key_in_map ('v', vi_edit_and_execute_command, vi_movement_keymap);
#  if defined (ALIAS)
  rl_bind_key_in_map ('@', posix_edit_macros, vi_movement_keymap);
#  endif
#endif

  rl_completer_quote_characters = "'\"";

  /* This sets rl_completer_word_break_characters and rl_special_prefixes
     to the appropriate values, depending on whether or not hostname
     completion is enabled. */
  enable_hostname_completion (perform_hostname_completion);

  /* characters that need to be quoted when appearing in filenames. */
  rl_filename_quote_characters = " \t\n\\\"'@<>=;|&()#$`?*[!:";
  rl_filename_quoting_function = bash_quote_filename;
  rl_filename_dequoting_function = bash_dequote_filename;
  rl_char_is_quoted_p = char_is_quoted;

  if (posixly_correct)
    posix_readline_initialize (1);

  bash_readline_initialized = 1;
}

/* On Sun systems at least, rl_attempted_completion_function can end up
   getting set to NULL, and rl_completion_entry_function set to do command
   word completion if Bash is interrupted while trying to complete a command
   word.  This just resets all the completion functions to the right thing.
   It's called from throw_to_top_level(). */
void
bashline_reinitialize ()
{
  tilde_initialize ();
  rl_attempted_completion_function = attempt_shell_completion;
  rl_completion_entry_function = (Function *)NULL;
  rl_directory_completion_hook = bash_directory_completion_hook;
  rl_ignore_some_completions_function = (Function *)filename_completion_ignore;
}

/* Contains the line to push into readline. */
static char *push_to_readline = (char *)NULL;

/* Push the contents of push_to_readline into the
   readline buffer. */
static void
bash_push_line ()
{
  if (push_to_readline)
    {
      rl_insert_text (push_to_readline);
      free (push_to_readline);
      push_to_readline = (char *)NULL;
      rl_startup_hook = old_rl_startup_hook;
    }
}

/* Call this to set the initial text for the next line to read
   from readline. */
int
bash_re_edit (line)
     char *line;
{
  FREE (push_to_readline);

  push_to_readline = savestring (line);
  old_rl_startup_hook = rl_startup_hook;
  rl_startup_hook = (Function *)bash_push_line;

  return (0);
}

static void
display_shell_version (count, c)
     int count, c;
{
  crlf ();
  show_shell_version (0);
  putc ('\r', rl_outstream);
  fflush (rl_outstream);
  rl_on_new_line ();
  rl_redisplay ();
}

/* **************************************************************** */
/*								    */
/*			     Readline Stuff			    */
/*								    */
/* **************************************************************** */

/* If the user requests hostname completion, then simply build a list
   of hosts, and complete from that forever more. */

/* The kept list of hostnames. */
static char **hostname_list = (char **)NULL;

/* The physical size of the above list. */
static int hostname_list_size;

/* The number of hostnames in the above list. */
static int hostname_list_length;

/* Whether or not HOSTNAME_LIST has been initialized. */
int hostname_list_initialized = 0;

/* Initialize the hostname completion table. */
static void
initialize_hostname_list ()
{
  char *temp;

  temp = get_string_value ("HOSTFILE");
  if (temp == 0)
    temp = get_string_value ("hostname_completion_file");
  if (temp == 0)
    temp = DEFAULT_HOSTS_FILE;

  snarf_hosts_from_file (temp);

  if (hostname_list)
    hostname_list_initialized++;
}

/* Add NAME to the list of hosts. */
static void
add_host_name (name)
     char *name;
{
  long size;

  if (hostname_list_length + 2 > hostname_list_size)
    {
      hostname_list_size = (hostname_list_size + 32) - (hostname_list_size % 32);
      size = hostname_list_size * sizeof (char *);
      hostname_list = (char **)xrealloc (hostname_list, size);
    }

  hostname_list[hostname_list_length++] = savestring (name);
  hostname_list[hostname_list_length] = (char *)NULL;
}

#define cr_whitespace(c) ((c) == '\r' || (c) == '\n' || whitespace(c))

static void
snarf_hosts_from_file (filename)
     char *filename;
{
  FILE *file;
  char *temp, buffer[256], name[256];
  register int i, start;

  file = fopen (filename, "r");
  if (file == 0)
    return;

  while (temp = fgets (buffer, 255, file))
    {
      /* Skip to first character. */
      for (i = 0; buffer[i] && cr_whitespace (buffer[i]); i++)
	;

      /* If comment or blank line, ignore. */
      if (buffer[i] == '\0' || buffer[i] == '#')
	continue;

      /* If `preprocessor' directive, do the include. */
      if (strncmp (buffer + i, "$include ", 9) == 0)
	{
	  char *incfile, *t;

	  /* Find start of filename. */
	  for (incfile = buffer + i + 9; *incfile && whitespace (*incfile); incfile++)
	    ;

	  /* Find end of filename. */
	  for (t = incfile; *t && cr_whitespace (*t) == 0; t++)
	    ;

	  *t = '\0';

	  snarf_hosts_from_file (incfile);
	  continue;
	}

      /* Skip internet address if present. */
      if (digit (buffer[i]))
	for (; buffer[i] && cr_whitespace (buffer[i]) == 0; i++);

      /* Gobble up names.  Each name is separated with whitespace. */
      while (buffer[i])
	{
	  for (; cr_whitespace (buffer[i]); i++)
	    ;
	  if (buffer[i] == '\0' || buffer[i] ==  '#')
	    break;

	  /* Isolate the current word. */
	  for (start = i; buffer[i] && cr_whitespace (buffer[i]) == 0; i++)
	    ;
	  if (i == start)
	    continue;
	  strncpy (name, buffer + start, i - start);
	  name[i - start] = '\0';
	  add_host_name (name);
	}
    }
  fclose (file);
}

/* Return a NULL terminated list of hostnames which begin with TEXT.
   Initialize the hostname list the first time if neccessary.
   The array is malloc ()'ed, but not the individual strings. */
static char **
hostnames_matching (text)
     char *text;
{
  register int i, len, nmatch, rsize;
  char **result;

  if (hostname_list_initialized == 0)
    initialize_hostname_list ();

  if (hostname_list_initialized == 0)
    return ((char **)NULL);

  /* Special case.  If TEXT consists of nothing, then the whole list is
     what is desired. */
  if (*text == '\0')
    {
      result = (char **)xmalloc ((1 + hostname_list_length) * sizeof (char *));
      for (i = 0; i < hostname_list_length; i++)
	result[i] = hostname_list[i];
      result[i] = (char *)NULL;
      return (result);
    }

  /* Scan until found, or failure. */
  len = strlen (text);
  result = (char **)NULL;
  for (i = nmatch = rsize = 0; i < hostname_list_length; i++)
    {
      if (STREQN (text, hostname_list[i], len) == 0)
        continue;

      /* OK, it matches.  Add it to the list. */
      if (nmatch >= rsize)
	{
	  rsize = (rsize + 16) - (rsize % 16);
	  result = (char **)xrealloc (result, rsize * sizeof (char *));
	}

      result[nmatch++] = hostname_list[i];
    }
  if (nmatch)
    result[nmatch] = (char *)NULL;
  return (result);
}

/* The equivalent of the Korn shell C-o operate-and-get-next-history-line
   editing command. */
static int saved_history_line_to_use = -1;

static void
set_saved_history ()
{
  if (saved_history_line_to_use >= 0)
    rl_get_previous_history (history_length - saved_history_line_to_use);
  saved_history_line_to_use = -1;
  rl_startup_hook = old_rl_startup_hook;
}

static void
operate_and_get_next (count, c)
     int count, c;
{
  int where;

  /* Accept the current line. */
  rl_newline ();

  /* Find the current line, and find the next line to use. */
  where = where_history ();

  if ((history_is_stifled () && (history_length >= max_input_history)) ||
      (where >= history_length - 1))
    saved_history_line_to_use = where;
  else
    saved_history_line_to_use = where + 1;

  old_rl_startup_hook = rl_startup_hook;
  rl_startup_hook = (Function *)set_saved_history;
}

#if defined (VI_MODE)
/* This vi mode command causes VI_EDIT_COMMAND to be run on the current
   command being entered (if no explicit argument is given), otherwise on
   a command from the history file. */

#define VI_EDIT_COMMAND "fc -e ${VISUAL:-${EDITOR:-vi}}"

static void
vi_edit_and_execute_command (count, c)
     int count, c;
{
  char *command;

  /* Accept the current line. */
  rl_newline ();

  if (rl_explicit_arg)
    {
      command = xmalloc (strlen (VI_EDIT_COMMAND) + 8);
      sprintf (command, "%s %d", VI_EDIT_COMMAND, count);
    }
  else
    {
      /* Take the command we were just editing, add it to the history file,
	 then call fc to operate on it.  We have to add a dummy command to
	 the end of the history because fc ignores the last command (assumes
	 it's supposed to deal with the command before the `fc'). */
      using_history ();
      bash_add_history (rl_line_buffer);
      bash_add_history ("");
      history_lines_this_session++;
      using_history ();
      command = savestring (VI_EDIT_COMMAND);
    }
  parse_and_execute (command, "v", SEVAL_NOHIST);
  rl_line_buffer[0] = '\0';	/* XXX */
}
#endif /* VI_MODE */

#if defined (ALIAS)
static int
posix_edit_macros (count, key)
     int count, key;
{
  int c;
  char alias_name[3], *alias_value, *macro;

  c = rl_read_key ();
  alias_name[0] = '_';
  alias_name[1] = c;
  alias_name[2] = '\0';

  alias_value = get_alias_value (alias_name);
  if (alias_value && *alias_value)
    {
      macro = savestring (alias_value);
      rl_push_macro_input (macro);
    }
  return 0;
}
#endif

/* **************************************************************** */
/*								    */
/*			How To Do Shell Completion		    */
/*								    */
/* **************************************************************** */

/* Do some completion on TEXT.  The indices of TEXT in RL_LINE_BUFFER are
   at START and END.  Return an array of matches, or NULL if none. */
static char **
attempt_shell_completion (text, start, end)
     char *text;
     int start, end;
{
  int in_command_position, ti;
  char **matches, *command_separator_chars;

  command_separator_chars = ";|&{(`";
  matches = (char **)NULL;
  rl_ignore_some_completions_function = (Function *)filename_completion_ignore;

  /* Determine if this could be a command word.  It is if it appears at
     the start of the line (ignoring preceding whitespace), or if it
     appears after a character that separates commands.  It cannot be a
     command word if we aren't at the top-level prompt. */
  ti = start - 1;

  while ((ti > -1) && (whitespace (rl_line_buffer[ti])))
    ti--;

  in_command_position = 0;
  if (ti < 0)
    {
      /* Only do command completion at the start of a line when we
         are prompting at the top level. */
      if (current_prompt_string == ps1_prompt)
	in_command_position++;
    }
  else if (member (rl_line_buffer[ti], command_separator_chars))
    {
      register int this_char, prev_char;

      in_command_position++;

      /* Handle the two character tokens `>&', `<&', and `>|'.
         We are not in a command position after one of these. */
      this_char = rl_line_buffer[ti];
      prev_char = rl_line_buffer[ti - 1];

      if ((this_char == '&' && (prev_char == '<' || prev_char == '>')) ||
	  (this_char == '|' && prev_char == '>'))
	in_command_position = 0;
      else if ((this_char == '{' && prev_char == '$') ||
	       (char_is_quoted (rl_line_buffer, ti)))
	in_command_position = 0;
    }
  else
    {
      /* This still could be in command position.  It is possible
	 that all of the previous words on the line are variable
	 assignments. */
    }

  /* Check that we haven't incorrectly flagged a closed command substitution
     as indicating we're in a command position. */
  if (in_command_position && ti >= 0 && rl_line_buffer[ti] == '`' &&
	*text != '`' && unclosed_pair (rl_line_buffer, 0, "`") == 0)
    in_command_position = 0;

  /* Special handling for command substitution.  If *TEXT is a backquote,
     it can be the start or end of an old-style command substitution, or
     unmatched.  If it's unmatched, both calls to unclosed_pair will
     succeed.  */
  if (*text == '`' && unclosed_pair (rl_line_buffer, start, "`") &&
	unclosed_pair (rl_line_buffer, end, "`"))
    matches = completion_matches (text, command_subst_completion_function);

  /* Variable name? */
  if (!matches && *text == '$')
    matches = completion_matches (text, variable_completion_function);

  /* If the word starts in `~', and there is no slash in the word, then
     try completing this word as a username. */
  if (!matches && *text == '~' && !strchr (text, '/'))
    matches = completion_matches (text, username_completion_function);

  /* Another one.  Why not?  If the word starts in '@', then look through
     the world of known hostnames for completion first. */
  if (!matches && perform_hostname_completion && *text == '@')
    matches = completion_matches (text, hostname_completion_function);

  /* And last, (but not least) if this word is in a command position, then
     complete over possible command names, including aliases, functions,
     and command names. */
  if (!matches && in_command_position)
    {
      matches = completion_matches (text, command_word_completion_function);
      /* If we are attempting command completion and nothing matches, we
	 do not want readline to perform filename completion for us.  We
	 still want to be able to complete partial pathnames, so set the
	 completion ignore function to something which will remove filenames
	 and leave directories in the match list. */
      if (!matches)
	rl_ignore_some_completions_function = (Function *)bash_ignore_filenames;
    }

  /* This could be a globbing pattern, so try to expand it using pathname
     expansion. */
  if (!matches && glob_pattern_p (text))
    {
      matches = completion_matches (text, glob_complete_word);
      /* A glob expression that matches more than one filename is problematic.
	 If we match more than one filename, punt. */
      if (matches && matches[1])
	{
	  free_array (matches);
	  matches = (char **)0;
	}
    }

  return (matches);
}

/* This is the function to call when the word to complete is in a position
   where a command word can be found.  It grovels $PATH, looking for commands
   that match.  It also scans aliases, function names, and the shell_builtin
   table. */
static char *
command_word_completion_function (hint_text, state)
     char *hint_text;
     int state;
{
  static char *hint = (char *)NULL;
  static char *path = (char *)NULL;
  static char *val = (char *)NULL;
  static char *filename_hint = (char *)NULL;
  static int path_index, hint_len, istate;
  static int mapping_over, local_index;
  static SHELL_VAR **varlist = (SHELL_VAR **)NULL;
#if defined (ALIAS)
  static alias_t **alias_list = (alias_t **)NULL;
#endif /* ALIAS */

  /* We have to map over the possibilities for command words.  If we have
     no state, then make one just for that purpose. */
  if (!state)
    {
      if (hint)
	free (hint);

      mapping_over = 0;
      val = (char *)NULL;

      /* If this is an absolute program name, do not check it against
	 aliases, reserved words, functions or builtins.  We must check
	 whether or not it is unique, and, if so, whether that filename
	 is executable. */
      if (absolute_program (hint_text))
	{
	  /* Perform tilde expansion on what's passed, so we don't end up
	     passing filenames with tildes directly to stat(). */
	  if (*hint_text == '~')
	    hint = bash_tilde_expand (hint_text);
	  else
	    hint = savestring (hint_text);
	  hint_len = strlen (hint);

	  if (filename_hint)
	    free (filename_hint);
	  filename_hint = savestring (hint);

	  mapping_over = 4;
	  istate = 0;
	  goto inner;
	}

      hint = savestring (hint_text);
      hint_len = strlen (hint);

      path = get_string_value ("PATH");
      path_index = 0;

      /* Initialize the variables for each type of command word. */
      local_index = 0;

      if (varlist)
	free (varlist);

      varlist = all_visible_functions ();

#if defined (ALIAS)
      if (alias_list)
	free (alias_list);

      alias_list = all_aliases ();
#endif /* ALIAS */
    }

  /* mapping_over says what we are currently hacking.  Note that every case
     in this list must fall through when there are no more possibilities. */

  switch (mapping_over)
    {
    case 0:			/* Aliases come first. */
#if defined (ALIAS)
      while (alias_list && alias_list[local_index])
	{
	  register char *alias;

	  alias = alias_list[local_index++]->name;

	  if (STREQN (alias, hint, hint_len))
	    return (savestring (alias));
	}
#endif /* ALIAS */
      local_index = 0;
      mapping_over++;

    case 1:			/* Then shell reserved words. */
      {
	while (word_token_alist[local_index].word)
	  {
	    register char *reserved_word;

	    reserved_word = word_token_alist[local_index++].word;

	    if (STREQN (reserved_word, hint, hint_len))
	      return (savestring (reserved_word));
	  }
	local_index = 0;
	mapping_over++;
      }

    case 2:			/* Then function names. */
      while (varlist && varlist[local_index])
	{
	  register char *varname;

	  varname = varlist[local_index++]->name;

	  if (STREQN (varname, hint, hint_len))
	    return (savestring (varname));
	}
      local_index = 0;
      mapping_over++;

    case 3:			/* Then shell builtins. */
      for (; local_index < num_shell_builtins; local_index++)
	{
	  /* Ignore it if it doesn't have a function pointer or if it
	     is not currently enabled. */
	  if (!shell_builtins[local_index].function ||
	      (shell_builtins[local_index].flags & BUILTIN_ENABLED) == 0)
	    continue;

	  if (STREQN (shell_builtins[local_index].name, hint, hint_len))
	    {
	      int i = local_index++;

	      return (savestring (shell_builtins[i].name));
	    }
	}
      local_index = 0;
      mapping_over++;
    }

  /* Repeatedly call filename_completion_function while we have
     members of PATH left.  Question:  should we stat each file?
     Answer: we call executable_file () on each file. */
 outer:

  istate = (val != (char *)NULL);

  if (!istate)
    {
      char *current_path;

      /* Get the next directory from the path.  If there is none, then we
	 are all done. */
      if (!path || !path[path_index] ||
	  (current_path = extract_colon_unit (path, &path_index)) == 0)
	return ((char *)NULL);

      if (*current_path == 0)
	{
	  free (current_path);
	  current_path = savestring (".");
	}

      if (*current_path == '~')
	{
	  char *t;

	  t = bash_tilde_expand (current_path);
	  free (current_path);
	  current_path = t;
	}

      if (filename_hint)
	free (filename_hint);

      filename_hint = xmalloc (2 + strlen (current_path) + hint_len);
      sprintf (filename_hint, "%s/%s", current_path, hint);

      free (current_path);
    }

 inner:
  val = filename_completion_function (filename_hint, istate);
  istate = 1;

  if (val == 0)
    {
      /* If the hint text is an absolute program, then don't bother
	 searching through PATH. */
      if (absolute_program (hint))
	return ((char *)NULL);

      goto outer;
    }
  else
    {
      int match, freetemp;
      char *temp;

      if (absolute_program (hint))
	{
	  match = strncmp (val, hint, hint_len) == 0;
	  /* If we performed tilde expansion, restore the original
	     filename. */
	  if (*hint_text == '~')
	    {
	      int l, tl, vl;
	      vl = strlen (val);
	      tl = strlen (hint_text);
	      l = vl - hint_len;	/* # of chars added */
	      temp = xmalloc (l + 2 + tl);
	      strcpy (temp, hint_text);
	      strcpy (temp + tl, val + vl - l);
	    }
	  else
	    temp = savestring (val);
	  freetemp = 1;
	}
      else
	{
	  temp = strrchr (val, '/');

	  if (temp)
	    {
	      temp++;
	      freetemp = match = strncmp (temp, hint, hint_len) == 0;
	      if (match)
		temp = savestring (temp);
	    }
	  else
	    freetemp = match = 0;
	}

      /* If we have found a match, and it is an executable file or a
	 directory name, return it. */
      if (match && (executable_file (val) || is_directory (val)))
	{
	  free (val);
	  val = "";		/* So it won't be NULL. */
	  return (temp);
	}
      else
	{
	  if (freetemp)
	    free (temp);
	  free (val);
	  goto inner;
	}
    }
}

/* Completion inside an unterminated command substitution. */
static char *
command_subst_completion_function (text, state)
     char *text;
     int state;
{
  static char **matches = (char **)NULL;
  static char *orig_start, *filename_text = (char *)NULL;
  static int cmd_index, start_len;
  char *value;

  if (state == 0)
    {
      if (filename_text)
	free (filename_text);
      orig_start = text;
      if (*text == '`')
        text++;
      else if (*text == '$' && text[1] == '(')	/* ) */
        text += 2;
      start_len = text - orig_start;
      filename_text = savestring (text);
      if (matches)
	free (matches);
      matches = completion_matches (filename_text, command_word_completion_function);
      cmd_index = 0;
    }

  if (!matches || !matches[cmd_index])
    {
      rl_filename_quoting_desired = 0;	/* disable quoting */
      return ((char *)NULL);
    }
  else
    {
      value = xmalloc (1 + start_len + strlen (matches[cmd_index]));

      if (start_len == 1)
        value[0] = *orig_start;
      else
        strncpy (value, orig_start, start_len);

      strcpy (value + start_len, matches[cmd_index]);

      cmd_index++;
      return (value);
    }
}

/* Okay, now we write the entry_function for variable completion. */
static char *
variable_completion_function (text, state)
     int state;
     char *text;
{
  register SHELL_VAR *var = (SHELL_VAR *)NULL;
  static SHELL_VAR **varlist = (SHELL_VAR **)NULL;
  static int varlist_index;
  static char *varname = (char *)NULL;
  static int namelen;
  static int first_char, first_char_loc;

  if (!state)
    {
      if (varname)
	free (varname);

      first_char_loc = 0;
      first_char = text[0];

      if (first_char == '$')
	first_char_loc++;

      if (text[first_char_loc] == '{')
        first_char_loc++;

      varname = savestring (text + first_char_loc);

      namelen = strlen (varname);
      if (varlist)
	free (varlist);
      varlist = all_visible_variables ();
      varlist_index = 0;
    }

  while (varlist && varlist[varlist_index])
    {
      var = varlist[varlist_index];

      /* Compare.  You can't do better than Zayre.  No text is also
	 a match.  */
      if (!*varname || (strncmp (varname, var->name, namelen) == 0))
	break;
      varlist_index++;
    }

  if (!varlist || !varlist[varlist_index])
    {
      return ((char *)NULL);
    }
  else
    {
      char *value = xmalloc (4 + strlen (var->name));

      if (first_char_loc)
	{
	  value[0] = first_char;
	  if (first_char_loc == 2)
	    value[1] = '{';
	}

      strcpy (&value[first_char_loc], var->name);
      if (first_char_loc == 2)
        strcat (value, "}");

      varlist_index++;
      return (value);
    }
}

/* How about a completion function for hostnames? */
static char *
hostname_completion_function (text, state)
     int state;
     char *text;
{
  static char **list = (char **)NULL;
  static int list_index = 0;
  static int first_char, first_char_loc;

  /* If we don't have any state, make some. */
  if (state == 0)
    {
      FREE (list);

      list = (char **)NULL;

      first_char_loc = 0;
      first_char = *text;

      if (first_char == '@')
	first_char_loc++;

      list = hostnames_matching (&text[first_char_loc]);
      list_index = 0;
    }

  if (list && list[list_index])
    {
      char *t;

      t = xmalloc (2 + strlen (list[list_index]));
      *t = first_char;
      strcpy (t + first_char_loc, list[list_index]);
      list_index++;
      return (t);
    }

  return ((char *)NULL);
}

/* Functions to perform history and alias expansions on the current line. */

#if defined (BANG_HISTORY)
/* Perform history expansion on the current line.  If no history expansion
   is done, pre_process_line() returns what it was passed, so we need to
   allocate a new line here. */
static char *
history_expand_line_internal (line)
     char *line;
{
  char *new_line;

  new_line = pre_process_line (line, 0, 0);
  return (new_line == line) ? savestring (line) : new_line;
}
#endif

/* There was an error in expansion.  Let the preprocessor print
   the error here. */
static void
cleanup_expansion_error ()
{
  char *to_free;

  fprintf (rl_outstream, "\r\n");
  to_free = pre_process_line (rl_line_buffer, 1, 0);
  if (to_free != rl_line_buffer)
    free (to_free);
  putc ('\r', rl_outstream);
  rl_forced_update_display ();
}

/* If NEW_LINE differs from what is in the readline line buffer, add an
   undo record to get from the readline line buffer contents to the new
   line and make NEW_LINE the current readline line. */
static void
maybe_make_readline_line (new_line)
     char *new_line;
{
  if (strcmp (new_line, rl_line_buffer) != 0)
    {
      rl_point = rl_end;

      rl_add_undo (UNDO_BEGIN, 0, 0, 0);
      rl_delete_text (0, rl_point);
      rl_point = rl_end = 0;
      rl_insert_text (new_line);
      rl_add_undo (UNDO_END, 0, 0, 0);
    }
}

/* Make NEW_LINE be the current readline line.  This frees NEW_LINE. */
static void
set_up_new_line (new_line)
     char *new_line;
{
  int old_point = rl_point;
  int at_end = rl_point == rl_end;

  /* If the line was history and alias expanded, then make that
     be one thing to undo. */
  maybe_make_readline_line (new_line);
  free (new_line);

  /* Place rl_point where we think it should go. */
  if (at_end)
    rl_point = rl_end;
  else if (old_point < rl_end)
    {
      rl_point = old_point;
      if (!whitespace (rl_line_buffer[rl_point]))
	rl_forward_word (1);
    }
}

#if defined (ALIAS)
/* Expand aliases in the current readline line. */
static int
alias_expand_line (ignore)
     int ignore;
{
  char *new_line;

  new_line = alias_expand (rl_line_buffer);

  if (new_line)
    {
      set_up_new_line (new_line);
      return (0);
    }
  else
    {
      cleanup_expansion_error ();
      return (1);
    }
}
#endif

#if defined (BANG_HISTORY)
/* History expand the line. */
static int
history_expand_line (ignore)
     int ignore;
{
  char *new_line;

  new_line = history_expand_line_internal (rl_line_buffer);

  if (new_line)
    {
      set_up_new_line (new_line);
      return (0);
    }
  else
    {
      cleanup_expansion_error ();
      return (1);
    }
}

/* Expand history substitutions in the current line and then insert a
   space wherever set_up_new_line decided to put rl_point. */
static int
tcsh_magic_space (ignore)
     int ignore;
{
  if (history_expand_line (ignore) == 0)
    {
      rl_insert (1, ' ');
      return (0);
    }
  else
    return (1);
}
#endif

/* History and alias expand the line. */
static int
history_and_alias_expand_line (ignore)
     int ignore;
{
  char *new_line;

  new_line = pre_process_line (rl_line_buffer, 0, 0);
  if (new_line == rl_line_buffer)
    new_line = savestring (new_line);

#if defined (ALIAS)
  if (new_line)
    {
      char *alias_line;

      alias_line = alias_expand (new_line);
      free (new_line);
      new_line = alias_line;
    }
#endif /* ALIAS */

  if (new_line)
    {
      set_up_new_line (new_line);
      return (0);
    }
  else
    {
      cleanup_expansion_error ();
      return (1);
    }
}

/* History and alias expand the line, then perform the shell word
   expansions by calling expand_string.  This can't use set_up_new_line()
   because we want the variable expansions as a separate undo'able
   set of operations. */
static void
shell_expand_line (ignore)
     int ignore;
{
  char *new_line;
  WORD_LIST *expanded_string;

  new_line = pre_process_line (rl_line_buffer, 0, 0);
  if (new_line == rl_line_buffer)
    new_line = savestring (new_line);

#if defined (ALIAS)
  if (new_line)
    {
      char *alias_line;

      alias_line = alias_expand (new_line);
      free (new_line);
      new_line = alias_line;
    }
#endif /* ALIAS */

  if (new_line)
    {
      int old_point = rl_point;
      int at_end = rl_point == rl_end;

      /* If the line was history and alias expanded, then make that
	 be one thing to undo. */
      maybe_make_readline_line (new_line);
      free (new_line);

      /* If there is variable expansion to perform, do that as a separate
	 operation to be undone. */
      new_line = savestring (rl_line_buffer);
      expanded_string = expand_string (new_line, 0);
      FREE (new_line);
      if (expanded_string == 0)
	{
	  new_line = xmalloc (1);
	  new_line[0] = '\0';
	}
      else
	{
	  new_line = string_list (expanded_string);
	  dispose_words (expanded_string);
	}

      maybe_make_readline_line (new_line);
      free (new_line);

      /* Place rl_point where we think it should go. */
      if (at_end)
	rl_point = rl_end;
      else if (old_point < rl_end)
	{
	  rl_point = old_point;
	  if (!whitespace (rl_line_buffer[rl_point]))
	    rl_forward_word (1);
	}
    }
  else
    cleanup_expansion_error ();
}

/* Define NO_FORCE_FIGNORE if you want to match filenames that would
   otherwise be ignored if they are the only possible matches. */
/* #define NO_FORCE_FIGNORE */

/* If FIGNORE is set, then don't match files with the given suffixes when
   completing filenames.  If only one of the possibilities has an acceptable
   suffix, delete the others, else just return and let the completer
   signal an error.  It is called by the completer when real
   completions are done on filenames by the completer's internal
   function, not for completion lists (M-?) and not on "other"
   completion types, such as hostnames or commands. */

static struct ignorevar fignore =
{
  "FIGNORE",
  (struct ign *)0,
  0,
  (char *)0,
  (Function *) 0,
};

static void
_ignore_completion_names (names, name_func)
     char **names;
     Function *name_func;
{
  char **newnames;
  int idx, nidx;
#ifdef NO_FORCE_FIGNORE
  char **oldnames;
  int oidx;
#endif

  /* If there is only one completion, see if it is acceptable.  If it is
     not, free it up.  In any case, short-circuit and return.  This is a
     special case because names[0] is not the prefix of the list of names
     if there is only one completion; it is the completion itself. */
  if (names[1] == (char *)0)
    {
#ifndef NO_FORCE_FIGNORE
      if ((*name_func) (names[0]) == 0)
        {
          free (names[0]);
          names[0] = (char *)NULL;
        }
#endif
      return;
    }

  /* Allocate space for array to hold list of pointers to matching
     filenames.  The pointers are copied back to NAMES when done. */
  for (nidx = 1; names[nidx]; nidx++)
    ;
  newnames = (char **)xmalloc ((nidx + 1) * (sizeof (char *)));
#ifdef NO_FORCE_FIGNORE
  oldnames = (char **)xmalloc ((nidx - 1) * (sizeof (char *)));
  oidx = 0;
#endif

  newnames[0] = names[0];
  for (idx = nidx = 1; names[idx]; idx++)
    {
      if ((*name_func) (names[idx]))
	newnames[nidx++] = names[idx];
      else
#ifndef NO_FORCE_FIGNORE
        free (names[idx]);
#else
	oldnames[oidx++] = names[idx];
#endif
    }

  newnames[nidx] = (char *)NULL;

  /* If none are acceptable then let the completer handle it. */
  if (nidx == 1)
    {
#ifndef NO_FORCE_FIGNORE
      free (names[0]);
      names[0] = (char *)NULL;
#else
      free (oldnames);
#endif
      free (newnames);
      return;
    }

#ifdef NO_FORCE_FIGNORE
  while (oidx)
    free (oldnames[--oidx]);
  free (oldnames);
#endif

  /* If only one is acceptable, copy it to names[0] and return. */
  if (nidx == 2)
    {
      free (names[0]);
      names[0] = newnames[1];
      names[1] = (char *)NULL;
      free (newnames);
      return;
    }

  /* Copy the acceptable names back to NAMES, set the new array end,
     and return. */
  for (nidx = 1; newnames[nidx]; nidx++)
    names[nidx] = newnames[nidx];
  names[nidx] = (char *)NULL;
  free (newnames);
}

static int
name_is_acceptable (name)
     char *name;
{
  struct ign *p;
  int nlen;

  for (nlen = strlen (name), p = fignore.ignores; p->val; p++)
    {
      if (nlen > p->len && p->len > 0 && STREQ (p->val, &name[nlen - p->len]))
	return (0);
    }

  return (1);
}

static void
filename_completion_ignore (names)
     char **names;
{
  setup_ignore_patterns (&fignore);

  if (fignore.num_ignores == 0)
    return;

  _ignore_completion_names (names, name_is_acceptable);
}

/* Return 1 if NAME is a directory. */
static int
test_for_directory (name)
     char *name;
{
  struct stat finfo;
  char *fn;

  fn = bash_tilde_expand (name);
  if (stat (fn, &finfo) != 0)
    {
      free (fn);
      return 0;
    }
  free (fn);
  return (S_ISDIR (finfo.st_mode));
}

/* Remove files from NAMES, leaving directories. */
static void
bash_ignore_filenames (names)
     char **names;
{
  _ignore_completion_names (names, test_for_directory);
}

/* Handle symbolic link references and other directory name
   expansions while hacking completion. */
static int
bash_directory_completion_hook (dirname)
     char **dirname;
{
  char *local_dirname, *t;
  int return_value = 0;
  WORD_LIST *wl;

  local_dirname = *dirname;
  if (strchr (local_dirname, '$') || strchr (local_dirname, '`'))
    {
      wl = expand_string (local_dirname, 0);
      if (wl)
	{
	  *dirname = string_list (wl);
	  /* Tell the completer to replace the directory name only if we
	     actually expanded something. */
	  return_value = STREQ (local_dirname, *dirname) == 0;
	  free (local_dirname);
	  dispose_words (wl);
	  local_dirname = *dirname;
	}
      else
	{
	  free (local_dirname);
	  *dirname = xmalloc (1);
	  **dirname = '\0';
	  return 1;
	}
    }

  if (!no_symbolic_links && (local_dirname[0] != '.' || local_dirname[1]))
    {
      char *temp1, *temp2;
      int len1, len2;

      t = get_working_directory ("symlink-hook");
      temp1 = make_absolute (local_dirname, t);
      free (t);
      temp2 = canonicalize_pathname (temp1);
      /* If we can't canonicalize, bail. */
      if (temp2 == 0)
	{
	  free (temp1);
	  return 1;
	}
      len1 = strlen (temp1);
      if (temp1[len1 - 1] == '/')
        {
	  len2 = strlen (temp2);
          temp2 = xrealloc (temp2, len2 + 2);
          temp2[len2] = '/';
          temp2[len2 + 1] = '\0';
        }
      free (local_dirname);
      *dirname = temp2;
      free (temp1);
    }
  return (return_value);
}

static char **history_completion_array = (char **)NULL;
static int harry_size;
static int harry_len;

static void
build_history_completion_array ()
{
  register int i, j;
  HIST_ENTRY **hlist;
  char **tokens;

  /* First, clear out the current dynamic history completion list. */
  if (harry_size)
    {
      for (i = 0; history_completion_array[i]; i++)
	free (history_completion_array[i]);

      free (history_completion_array);

      history_completion_array = (char **)NULL;
      harry_size = 0;
      harry_len = 0;
    }

  /* Next, grovel each line of history, making each shell-sized token
     a separate entry in the history_completion_array. */
  hlist = history_list ();

  if (hlist)
    {
      for (i = 0; hlist[i]; i++)
	{
	  /* Separate each token, and place into an array. */
	  tokens = history_tokenize (hlist[i]->line);

	  for (j = 0; tokens && tokens[j]; j++)
	    {
	      if (harry_len + 2 > harry_size)
		{
		  harry_size += 10;
		  history_completion_array = (char **) xrealloc
		    (history_completion_array, harry_size * sizeof (char *));
		}

	      history_completion_array[harry_len++] = tokens[j];
	      history_completion_array[harry_len] = (char *)NULL;
	    }
	  free (tokens);
	}

      /* Sort the complete list of tokens. */
      qsort (history_completion_array, harry_len, sizeof (char *), (Function *)qsort_string_compare);
    }
}

static char *
history_completion_generator (hint_text, state)
     char *hint_text;
     int state;
{
  static int local_index, len;
  static char *text;

  /* If this is the first call to the generator, then initialize the
     list of strings to complete over. */
  if (state == 0)
    {
      local_index = 0;
      build_history_completion_array ();
      text = hint_text;
      len = strlen (text);
    }

  while (history_completion_array && history_completion_array[local_index])
    {
      if (strncmp (text, history_completion_array[local_index++], len) == 0)
	return (savestring (history_completion_array[local_index - 1]));
    }
  return ((char *)NULL);
}

static void
dynamic_complete_history (count, key)
     int count, key;
{
  Function *orig_func;
  CPPFunction *orig_attempt_func;

  orig_func = rl_completion_entry_function;
  orig_attempt_func = rl_attempted_completion_function;
  rl_completion_entry_function = (Function *)history_completion_generator;
  rl_attempted_completion_function = (CPPFunction *)NULL;

  if (rl_last_func == (Function *)dynamic_complete_history)
    rl_complete_internal ('?');
  else
    rl_complete_internal (TAB);

  rl_completion_entry_function = orig_func;
  rl_attempted_completion_function = orig_attempt_func;
}

#if defined (SPECIFIC_COMPLETION_FUNCTIONS)
static void
bash_complete_username (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_username_internal (TAB);
}

static void
bash_possible_username_completions (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_username_internal ('?');
}

static void
bash_complete_username_internal (what_to_do)
     int what_to_do;
{
  bash_specific_completion
    (what_to_do, (Function *)username_completion_function);
}

static void
bash_complete_filename (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_filename_internal (TAB);
}

static void
bash_possible_filename_completions (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_filename_internal ('?');
}

static void
bash_complete_filename_internal (what_to_do)
     int what_to_do;
{
  Function  *orig_func, *orig_dir_func;
  CPPFunction *orig_attempt_func;
  char *orig_rl_completer_word_break_characters;

  orig_func = rl_completion_entry_function;
  orig_attempt_func = rl_attempted_completion_function;
  orig_dir_func = rl_directory_completion_hook;
  orig_rl_completer_word_break_characters = rl_completer_word_break_characters;
  rl_completion_entry_function = (Function *)filename_completion_function;
  rl_attempted_completion_function = (CPPFunction *)NULL;
  rl_directory_completion_hook = (Function *)NULL;
  rl_completer_word_break_characters = " \t\n\"\'";

  rl_complete_internal (what_to_do);

  rl_completion_entry_function = orig_func;
  rl_attempted_completion_function = orig_attempt_func;
  rl_directory_completion_hook = orig_dir_func;
  rl_completer_word_break_characters = orig_rl_completer_word_break_characters;
}

static void
bash_complete_hostname (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_hostname_internal (TAB);
}

static void
bash_possible_hostname_completions (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_hostname_internal ('?');
}

static void
bash_complete_variable (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_variable_internal (TAB);
}

static void
bash_possible_variable_completions (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_variable_internal ('?');
}

static void
bash_complete_command (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_command_internal (TAB);
}

static void
bash_possible_command_completions (ignore, ignore2)
     int ignore, ignore2;
{
  bash_complete_command_internal ('?');
}

static void
bash_complete_hostname_internal (what_to_do)
     int what_to_do;
{
  bash_specific_completion
    (what_to_do, (Function *)hostname_completion_function);
}

static void
bash_complete_variable_internal (what_to_do)
     int what_to_do;
{
  bash_specific_completion
    (what_to_do, (Function *)variable_completion_function);
}

static void
bash_complete_command_internal (what_to_do)
     int what_to_do;
{
  bash_specific_completion
    (what_to_do, (Function *)command_word_completion_function);
}

static char *
glob_complete_word (text, state)
     char *text;
     int state;
{
  static char **matches = (char **)NULL;
  static int ind;
  char *ret;

  if (state == 0)
    {
      rl_filename_completion_desired = 1;
      if (matches)
        free (matches);
      matches = shell_glob_filename (text);
      if (GLOB_FAILED (matches))
        matches = (char **)NULL;
      ind = 0;
    }

  ret = matches ? matches[ind] : (char *)NULL;
  ind++;
  return ret;
}

static void
bash_glob_completion_internal (what_to_do)
     int what_to_do;
{
  bash_specific_completion (what_to_do, (Function *)glob_complete_word);
}

static void
bash_glob_expand_word (count, key)
     int count, key;
{
  bash_glob_completion_internal ('*');
}

static void
bash_glob_list_expansions (count, key)
     int count, key;
{
  bash_glob_completion_internal ('?');
}

static void
bash_specific_completion (what_to_do, generator)
     int what_to_do;
     Function *generator;
{
  Function *orig_func;
  CPPFunction *orig_attempt_func;

  orig_func = rl_completion_entry_function;
  orig_attempt_func = rl_attempted_completion_function;
  rl_completion_entry_function = generator;
  rl_attempted_completion_function = (CPPFunction *)NULL;

  rl_complete_internal (what_to_do);

  rl_completion_entry_function = orig_func;
  rl_attempted_completion_function = orig_attempt_func;
}

#endif	/* SPECIFIC_COMPLETION_FUNCTIONS */

/* Filename quoting for completion. */
/* A function to strip quotes that are not protected by backquotes.  It
   allows single quotes to appear within double quotes, and vice versa.
   It should be smarter. */
static char *
bash_dequote_filename (text, quote_char)
     char *text;
{
  char *ret, *p, *r;
  int l, quoted;

  l = strlen (text);
  ret = xmalloc (l + 1);
  for (quoted = quote_char, p = text, r = ret; p && *p; p++)
    {
      /* Allow backslash-quoted characters to pass through unscathed. */
      if (*p == '\\')
	{
	  *r++ = *++p;
	  if (*p == '\0')
	    break;
	  continue;
	}
      /* Close quote. */
      if (quoted && *p == quoted)
        {
          quoted = 0;
          continue;
        }
      /* Open quote. */
      if (quoted == 0 && (*p == '\'' || *p == '"'))
        {
          quoted = *p;
          continue;
        }
      *r++ = *p;
    }
  *r = '\0';
  return ret;
}

/* Quote characters that the readline completion code would treat as
   word break characters with backslashes.  Pass backslash-quoted
   characters through without examination. */
static char *
quote_word_break_chars (text)
     char *text;
{
  char *ret, *r, *s;
  int l;

  l = strlen (text);
  ret = xmalloc ((2 * l) + 1);
  for (s = text, r = ret; *s; s++)
    {
      /* Pass backslash-quoted characters through, including the backslash. */
      if (*s == '\\')
	{
	  *r++ = '\\';
	  *r++ = *++s;
	  if (*s == '\0')
	    break;
	  continue;
	}
      /* OK, we have an unquoted character.  Check its presence in
	 rl_completer_word_break_characters. */
      if (strchr (rl_completer_word_break_characters, *s))
        *r++ = '\\';
      *r++ = *s;
    }
  *r = '\0';
  return ret;
}

/* Quote a filename using double quotes, single quotes, or backslashes
   depending on the value of completion_quoting_style.  If we're
   completing using backslashes, we need to quote some additional
   characters (those that readline treats as word breaks), so we call
   quote_word_break_chars on the result. */
static char *
bash_quote_filename (s, rtype, qcp)
     char *s;
     int rtype;
     char *qcp;
{
  char *rtext, *mtext, *ret;
  int rlen, cs;

  rtext = (char *)NULL;

  /* If RTYPE == MULT_MATCH, it means that there is
     more than one match.  In this case, we do not add
     the closing quote or attempt to perform tilde
     expansion.  If RTYPE == SINGLE_MATCH, we try
     to perform tilde expansion, because single and double
     quotes inhibit tilde expansion by the shell. */

  mtext = s;
  if (mtext[0] == '~' && rtype == SINGLE_MATCH)
    mtext = bash_tilde_expand (s);

  cs = completion_quoting_style;
  /* Might need to modify the default completion style based on *qcp,
     since it's set to any user-provided opening quote. */
  if (*qcp == '"')
    cs = COMPLETE_DQUOTE;
  else if (*qcp == '\'')
    cs = COMPLETE_SQUOTE;
#if defined (BANG_HISTORY)
  else if (*qcp == '\0' && history_expansion && cs == COMPLETE_DQUOTE &&
	   history_expansion_inhibited == 0 && strchr (mtext, '!'))
    cs = COMPLETE_BSQUOTE;

  if (*qcp == '"' && history_expansion && cs == COMPLETE_DQUOTE &&
        history_expansion_inhibited == 0 && strchr (mtext, '!'))
    {
      cs = COMPLETE_BSQUOTE;
      *qcp = '\0';
    }
#endif

  switch (cs)
    {
    case COMPLETE_DQUOTE:
      rtext = double_quote (mtext);
      break;
    case COMPLETE_SQUOTE:
      rtext = single_quote (mtext);
      break;
    case COMPLETE_BSQUOTE:
      rtext = backslash_quote (mtext);
      break;
    }

  if (mtext != s)
    free (mtext);

  /* We may need to quote additional characters: those that readline treats
     as word breaks that are not quoted by backslash_quote. */
  if (rtext && cs == COMPLETE_BSQUOTE)
    {
      mtext = quote_word_break_chars (rtext);
      free (rtext);
      rtext = mtext;
    }

  /* Leave the opening quote intact.  The readline completion code takes
     care of avoiding doubled opening quotes. */
  rlen = strlen (rtext);
  ret = xmalloc (rlen + 1);
  strcpy (ret, rtext);

  /* If there are multiple matches, cut off the closing quote. */
  if (rtype == MULT_MATCH && cs != COMPLETE_BSQUOTE)
    ret[rlen - 1] = '\0';
  free (rtext);
  return ret;
}

#endif /* READLINE */
