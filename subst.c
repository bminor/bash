/* subst.c -- The part of the shell that does parameter, command, and
   globbing substitutions. */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

#include "bashtypes.h"
#include <stdio.h>
#include <pwd.h>
#include <signal.h>
#include <errno.h>
/* Not all systems declare ERRNO in errno.h... and some systems #define it! */
#if !defined (errno)
extern int errno;
#endif /* !errno */

#include "bashansi.h"
#include "posixstat.h"

#include "shell.h"
#include "flags.h"
#include "jobs.h"
#include "execute_cmd.h"
#include "filecntl.h"

#if defined (READLINE)
#  include <readline/readline.h>
#else
#  include <tilde/tilde.h>
#endif

#if defined (HISTORY)
#  include "bashhist.h"
#  include <readline/history.h>
#endif

#include <glob/fnmatch.h>
#include "builtins/getopt.h"

/* The size that strings change by. */
#define DEFAULT_ARRAY_SIZE 512

/* How to quote and determine the quoted state of the character C. */
static char *make_quoted_char ();
#define QUOTED_CHAR(c)  ((c) == CTLESC)

/* Process ID of the last command executed within command substitution. */
pid_t last_command_subst_pid = NO_PID;

/* Extern functions and variables from different files. */
extern int last_command_exit_value, interactive, interactive_shell;
extern int subshell_environment;
extern int dollar_dollar_pid, no_brace_expansion;
extern int posixly_correct;
extern int eof_encountered, eof_encountered_limit, ignoreeof;
extern char *this_command_name;
extern jmp_buf top_level;
#if defined (READLINE)
extern int no_line_editing;
extern int hostname_list_initialized;
#endif

#if !defined (USE_POSIX_GLOB_LIBRARY)
extern int glob_dot_filenames, noglob_dot_filenames;
extern char *glob_error_return;
#endif

static WORD_LIST expand_word_error, expand_word_fatal;
static char expand_param_error, expand_param_fatal;

static WORD_LIST *expand_string_internal ();
static WORD_LIST *expand_word_internal (), *expand_words_internal ();
static WORD_LIST *expand_string_leave_quoted ();
static WORD_LIST *word_list_split ();
static char *quote_string ();
static int unquoted_substring (), unquoted_member ();
static int unquoted_glob_pattern_p ();
static void quote_list (), dequote_list ();
static int do_assignment_internal ();
static char *string_extract_verbatim (), *string_extract ();
static char *string_extract_double_quoted (), *string_extract_single_quoted ();
static char *extract_delimited_string ();
static char *extract_dollar_brace_string ();

/* **************************************************************** */
/*								    */
/*			Utility Functions			    */
/*								    */
/* **************************************************************** */

/* Cons a new string from STRING starting at START and ending at END,
   not including END. */
char *
substring (string, start, end)
     char *string;
     int start, end;
{
  register int len = end - start;
  register char *result = xmalloc (len + 1);

  strncpy (result, string + start, len);
  result[len] = '\0';
  return (result);
}

/* Conventions:

     A string with s[0] == CTLNUL && s[1] == 0 is a quoted null string.
     The parser passes CTLNUL as CTLESC CTLNUL. */

/* The parser passes us CTLESC as CTLESC CTLESC and CTLNUL as CTLESC CTLNUL.
   This is necessary to make unquoted CTLESC and CTLNUL characters in the
   data stream pass through properly.
   Here we remove doubled CTLESC characters inside quoted strings before
   quoting the entire string, so we do not double the number of CTLESC
   characters. */
static char *
remove_quoted_escapes (string)
     char *string;
{
  register char *s;

  for (s = string; s && *s; s++)
    {
      if (*s == CTLESC && (s[1] == CTLESC || s[1] == CTLNUL))
	strcpy (s, s + 1);	/* XXX - should be memmove */
    }
  return (string);
}

/* Quote escape characters in string s, but no other characters.  This is
   used to protect CTLESC and CTLNUL in variable values from the rest of
   the word expansion process after the variable is expanded. */
static char *
quote_escapes (string)
     char *string;
{
  register char *s, *t;
  char *result;

  result = xmalloc ((strlen (string) * 2) + 1);
  for (s = string, t = result; s && *s; )
    {
      if (*s == CTLESC || *s == CTLNUL)
	*t++ = CTLESC;
      *t++ = *s++;
    }
  *t = '\0';
  return (result);
}      

/* Just like string_extract, but doesn't hack backslashes or any of
   that other stuff.  Obeys quoting.  Used to do splitting on $IFS. */
static char *
string_extract_verbatim (string, sindex, charlist)
     char *string, *charlist;
     int *sindex;
{
  register int i = *sindex;
  int c;
  char *temp;

  if (charlist[0] == '\'' && !charlist[1])
    {
      temp = string_extract_single_quoted (string, sindex);
      i = *sindex - 1;
      *sindex = i;
      return (temp);
    }

  for (i = *sindex; (c = string[i]); i++)
    {
      if (c == CTLESC)
	{
	  i++;
	  continue;
	}

      if (MEMBER (c, charlist))
	break;
    }

  temp = xmalloc (1 + (i - *sindex));
  strncpy (temp, string + (*sindex), i - (*sindex));
  temp[i - (*sindex)] = '\0';
  *sindex = i;

  return (temp);
}

/* Extract a substring from STRING, starting at SINDEX and ending with
   one of the characters in CHARLIST.  Don't make the ending character
   part of the string.  Leave SINDEX pointing at the ending character.
   Understand about backslashes in the string. */
static char *
string_extract (string, sindex, charlist)
     char *string, *charlist;
     int *sindex;
{
  register int c, i = *sindex;
  char *temp;

  while (c = string[i])
    {
      if (c == '\\')
	if (string[i + 1])
	  i++;
	else
	  break;
      else
	if (MEMBER (c, charlist))
	  break;
      i++;
    }
  temp = xmalloc (1 + (i - *sindex));
  strncpy (temp, string + (*sindex), i - (*sindex));
  temp[i - (*sindex)] = '\0';
  *sindex = i;
  return (temp);
}

/* Remove backslashes which are quoting backquotes from STRING.  Modifies
   STRING, and returns a pointer to it. */
char *
de_backslash (string)
     char *string;
{
  register int i, l = strlen (string);

  for (i = 0; i < l; i++)
    if (string[i] == '\\' && (string[i + 1] == '`' || string[i + 1] == '\\' ||
			      string[i + 1] == '$'))
      strcpy (string + i, string + i + 1);	/* XXX - should be memmove */
  return (string);
}

#if 0
/* Replace instances of \! in a string with !. */
void
unquote_bang (string)
     char *string;
{
  register int i, j;
  register char *temp;

  temp = xmalloc (1 + strlen (string));

  for (i = 0, j = 0; (temp[j] = string[i]); i++, j++)
    {
      if (string[i] == '\\' && string[i + 1] == '!')
	{
	  temp[j] = '!';
	  i++;
	}
    }
  strcpy (string, temp);
  free (temp);
}
#endif

/* Extract the $( construct in STRING, and return a new string.
   Start extracting at (SINDEX) as if we had just seen "$(".
   Make (SINDEX) get the position just after the matching ")". */
char *
extract_command_subst (string, sindex)
     char *string;
     int *sindex;
{
  return (extract_delimited_string (string, sindex, "$(", "(", ")"));
}

/* Extract the $[ construct in STRING, and return a new string.
   Start extracting at (SINDEX) as if we had just seen "$[".
   Make (SINDEX) get the position just after the matching "]". */
char *
extract_arithmetic_subst (string, sindex)
     char *string;
     int *sindex;
{
  return (extract_delimited_string (string, sindex, "$[", "[", "]"));
}

#if defined (PROCESS_SUBSTITUTION)
/* Extract the <( or >( construct in STRING, and return a new string.
   Start extracting at (SINDEX) as if we had just seen "<(".
   Make (SINDEX) get the position just after the matching ")". */
char *
extract_process_subst (string, starter, sindex)
     char *string;
     char *starter;
     int *sindex;
{
  return (extract_delimited_string (string, sindex, starter, "(", ")"));
}
#endif /* PROCESS_SUBSTITUTION */

/* Extract and create a new string from the contents of STRING, a
   character string delimited with OPENER and CLOSER.  SINDEX is
   the address of an int describing the current offset in STRING;
   it should point to just after the first OPENER found.  On exit,
   SINDEX gets the position just after the matching CLOSER.  If
   OPENER is more than a single character, ALT_OPENER, if non-null,
   contains a character string that can also match CLOSER and thus
   needs to be skipped. */
static char *
extract_delimited_string (string, sindex, opener, alt_opener, closer)
     char *string;
     int *sindex;
     char *opener, *alt_opener, *closer;
{
  register int i, c, l;
  int pass_character, nesting_level;
  int delimiter, delimited_nesting_level;
  int len_closer, len_opener, len_alt_opener;
  char *result;

  len_opener = STRLEN (opener);
  len_alt_opener = STRLEN (alt_opener);
  len_closer = STRLEN (closer);

  pass_character = delimiter = delimited_nesting_level = 0;

  nesting_level = 1;

  for (i = *sindex; c = string[i]; i++)
    {
      if (pass_character)
	{
	  pass_character = 0;
	  continue;
	}

      if (c == CTLESC)
	{
	  pass_character++;
	  continue;
	}

      if (c == '\\')
	{
	  if ((delimiter == '"') &&
	      (member (string[i + 1], slashify_in_quotes)))
	    {
	      pass_character++;
	      continue;
	    }
	}

      if (!delimiter || delimiter == '"')
	{
	  if (STREQN (string + i, opener, len_opener))
	    {
	      if (!delimiter)
		nesting_level++;
	      else
		delimited_nesting_level++;

	      i += len_opener - 1;
	      continue;
	    }

	  if (len_alt_opener && STREQN (string + i, alt_opener, len_alt_opener))
	    {
	      if (!delimiter)
		nesting_level++;
	      else
		delimited_nesting_level++;

	      i += len_alt_opener - 1;
	      continue;
	    }

	  if (STREQN (string + i, closer, len_closer))
	    {
	      i += len_closer - 1;

	      if (delimiter && delimited_nesting_level)
		delimited_nesting_level--;

	      if (!delimiter)
		{
		  nesting_level--;
		  if (nesting_level == 0)
		    break;
		}
	    }
	}

      if (delimiter)
	{
	  if (c == delimiter || delimiter == '\\')
	    delimiter = 0;
	  continue;
	}
      else
	{
	  if (c == '"' || c == '\'' || c == '\\')
	    delimiter = c;
	}
    }

  l = i - *sindex;
  result = xmalloc (1 + l);
  strncpy (result, string + *sindex, l);
  result[l] = '\0';
  *sindex = i;

  if (!c && (delimiter || nesting_level))
    {
      report_error ("bad substitution: no `%s' in %s", closer, string);
      free (result);
      longjmp (top_level, DISCARD);
    }
  return (result);
}

/* Extract a parameter expansion expression within ${ and } from STRING.
   Obey the Posix.2 rules for finding the ending `}': count braces while
   skipping over enclosed quoted strings and command substitutions.
   SINDEX is the address of an int describing the current offset in STRING;
   it should point to just after the first `{' found.  On exit, SINDEX
   gets the position just after the matching `}'. */
/* XXX -- this is very similar to extract_delimited_string -- XXX */
static char *
extract_dollar_brace_string (string, sindex)
     char *string;
     int *sindex;
{
  register int i, c, l;
  int pass_character, nesting_level;
  int delimiter, delimited_nesting_level;
  char *result;

  pass_character = delimiter = delimited_nesting_level = 0;

  nesting_level = 1;

  for (i = *sindex; c = string[i]; i++)
    {
      if (pass_character)
	{
	  pass_character = 0;
	  continue;
	}

      if (c == CTLESC)
	{
	  pass_character++;
	  continue;
	}

      /* Backslashes quote the next character. */
      if (c == '\\')
	{
	  if ((delimiter == '"') &&
	      (member (string[i + 1], slashify_in_quotes)))
	    {
	      pass_character++;
	      continue;
	    }
	}

      if (!delimiter || delimiter == '"')
	{
	  if (string[i] == '$' && string[i+1] == '{')
	    {
	      if (!delimiter)
		nesting_level++;
	      else
		delimited_nesting_level++;

	      i++;
	      continue;
	    }

	  /* Pass the contents of old-style command substitutions through
	     verbatim. */
	  if (string[i] == '`')
	    {
	      int si;
	      char *t;

	      si = i + 1;
	      t = string_extract (string, &si, "`");
	      i = si;
	      free (t);
	      continue;
	    }

	  /* Pass the contents of new-style command substitutions through
	     verbatim. */
	  if (string[i] == '$' && string[i+1] == '(')
	    {
	      int si;
	      char *t;

	      si = i + 2;
	      t = extract_delimited_string (string, &si, "$(", "(", ")");
	      i = si;
	      free (t);
	      continue;
	    }

	  if (string[i] == '{')
	    {
	      if (!delimiter)
		nesting_level++;
	      else
		delimited_nesting_level++;

	      continue;
	    }

	  if (string[i] == '}')
	    {
	      if (delimiter && delimited_nesting_level)
		delimited_nesting_level--;

	      if (!delimiter)
		{
		  nesting_level--;
		  if (nesting_level == 0)
		    break;
		}
	    }
	}

      if (delimiter)
	{
	  if (c == delimiter || delimiter == '\\')
	    delimiter = 0;
	  continue;
	}
      else
	{
	  if (c == '"' || c == '\'' || c == '\\')
	    delimiter = c;
	}
    }

  l = i - *sindex;
  result = xmalloc (1 + l);
  strncpy (result, string + *sindex, l);
  result[l] = '\0';
  *sindex = i;

  if (!c && (delimiter || nesting_level))
    {
      report_error ("bad substitution: no ending `}' in %s", string);
      free (result);
      longjmp (top_level, DISCARD);
    }
  return (result);
}

/* Extract the contents of STRING as if it is enclosed in double quotes.
   SINDEX, when passed in, is the offset of the character immediately
   following the opening double quote; on exit, SINDEX is left pointing after
   the closing double quote. */
static char *
string_extract_double_quoted (string, sindex)
     char *string;
     int *sindex;
{
  register int c, j, i;
  char *temp;			/* The new string we return. */
  int pass_next, backquote;	/* State variables for the machine. */

  pass_next = backquote = 0;
  temp = xmalloc (1 + strlen (string) - *sindex);

  for (j = 0, i = *sindex; c = string[i]; i++)
    {
      /* Process a character that was quoted by a backslash. */
      if (pass_next)
	{
	  /* Posix.2 sez:

	     ``The backslash shall retain its special meaning as an escape
	     character only when followed by one of the characters:
	     	$	`	"	\	<newline>''.

	     We handle the double quotes here.  expand_word_internal handles
	     the rest. */
	  if (c != '"')
	    temp[j++] = '\\';
	  temp[j++] = c;
	  pass_next = 0;
	  continue;
	}

      /* A backslash protects the next character.  The code just above
	 handles preserving the backslash in front of any character but
	 a double quote. */
      if (c == '\\')
	{
	  pass_next++;
	  continue;
	}

      /* Inside backquotes, ``the portion of the quoted string from the
	 initial backquote and the characters up to the next backquote
	 that is not preceded by a backslash, having escape characters
	 removed, defines that command''. */
      if (backquote)
	{
	  if (c == '`')
	    backquote = 0;
	  temp[j++] = c;
	  continue;
	}

      if (c == '`')
	{
	  temp[j++] = c;
	  backquote++;
	  continue;
	}

      /* Pass everything between `$(' and the matching `)' or a quoted
	 ${ ... } pair through according to the Posix.2 specification. */
      if (c == '$' && ((string[i + 1] == '(') || (string[i + 1] == '{')))
	{
	  register int t;
	  int si;
	  char *ret;

	  si = i + 2;
	  if (string[i + 1] == '(')
	    ret = extract_delimited_string (string, &si, "$(", "(", ")");
	  else
	    ret = extract_dollar_brace_string (string, &si);

	  temp[j++] = '$';
	  temp[j++] = string[i + 1];

	  for (t = 0; ret[t]; t++)
	    temp[j++] = ret[t];

	  i = si;
	  temp[j++] = string[i];
	  free (ret);
	  continue;
	}

      /* An unescaped double quote serves to terminate the string. */
      if (c == '"')
	break;

      /* Add the character to the quoted string we're accumulating. */
      temp[j++] = c;
    }
  temp[j] = '\0';

  /* Point to after the closing quote. */
  if (c)
    i++;
  *sindex = i;

  return (temp);
}

/* Extract the contents of STRING as if it is enclosed in single quotes.
   SINDEX, when passed in, is the offset of the character immediately
   following the opening single quote; on exit, SINDEX is left pointing after
   the closing single quote. */
static char *
string_extract_single_quoted (string, sindex)
     char *string;
     int *sindex;
{
  register int i = *sindex;
  char *temp;

  while (string[i] && string[i] != '\'')
    i++;

  temp = xmalloc (1 + i - *sindex);
  strncpy (temp, string + *sindex, i - *sindex);
  temp[i - *sindex] = '\0';

  if (string[i])
    i++;
  *sindex = i;

  return (temp);
}

/* Return 1 if the portion of STRING ending at EINDEX is quoted (there is
   an unclosed quoted string), or if the character at EINDEX is quoted
   by a backslash. */
int
char_is_quoted (string, eindex)
     char *string;
     int eindex;
{
  int i, pass_next, quoted;
  char *temp;

  for (i = pass_next = quoted = 0; i <= eindex; i++)
    {
      if (pass_next)
	{
	  pass_next = 0;
	  if (i >= eindex)	/* XXX was if (i >= eindex - 1) */
	    return 1;
	  continue;
	}
      else if (string[i] == '\'')
        {
          i++;
          temp = string_extract_single_quoted (string, &i);
          free (temp);
          if (i > eindex)
            return 1;
          i--;
        }
      else if (string[i] == '"')
        {
          i++;
          temp = string_extract_double_quoted (string, &i);
          free (temp);
          if (i > eindex)
            return 1;
          i--;
        }
      else if (string[i] == '\\')
        {
          pass_next = 1;
          continue;
        }
    }
  return (0);
}

#if defined (READLINE)
int
unclosed_pair (string, eindex, openstr)
     char *string;
     int eindex;
     char *openstr;
{
  int i, pass_next, openc, c, olen;
  char *temp, *s;

  olen = strlen (openstr);
  for (i = pass_next = openc = 0; i <= eindex; i++)
    {
      if (pass_next)
	{
	  pass_next = 0;
	  if (i >= eindex)	/* XXX was if (i >= eindex - 1) */
	    return 0;
	  continue;
	}
      else if (STREQN (string + i, openstr, olen))
	{
	  openc = 1 - openc;
	  i += olen - 1;
	}
      else if (string[i] == '\'')
	{
	  i++;
	  temp = string_extract_single_quoted (string, &i);
	  free (temp);
	  if (i > eindex)
	    return 0;
	}
      else if (string[i] == '"')
	{
	  i++;
	  temp = string_extract_double_quoted (string, &i);
	  free (temp);
	  if (i > eindex)
	    return 0;
	}
      else if (string[i] == '\\')
	{
	  pass_next = 1;
	  continue;
	}
    }
  return (openc);
}
#endif /* READLINE */

/* Extract the name of the variable to bind to from the assignment string. */
char *
assignment_name (string)
     char *string;
{
  int offset = assignment (string);
  char *temp;

  if (!offset)
    return (char *)NULL;
  temp = xmalloc (offset + 1);
  strncpy (temp, string, offset);
  temp[offset] = '\0';
  return (temp);
}

/* Return a single string of all the words in LIST.  SEP is the separator
   to put between individual elements of LIST in the output string. */
static char *
string_list_internal (list, sep)
     WORD_LIST *list;
     char *sep;
{
  register WORD_LIST *t;
  char *result, *r;
  int word_len, sep_len, result_size;

  if (!list)
    return ((char *)NULL);

  /* This is nearly always called with either sep[0] == 0 or sep[1] == 0. */
  sep_len = STRLEN (sep);
  result_size = 0;

  for (t = list; t; t = t->next)
    {
      if (t != list)
	result_size += sep_len;
      result_size += strlen (t->word->word);
    }

  r = result = xmalloc (result_size + 1);

  for (t = list; t; t = t->next)
    {
      if (t != list && sep_len)
	{
	  FASTCOPY (sep, r, sep_len);
	  r += sep_len;
	}

      word_len = strlen (t->word->word);
      FASTCOPY (t->word->word, r, word_len);
      r += word_len;
    }

  *r = '\0';    
  return (result);
}

/* Return a single string of all the words present in LIST, separating
   each word with a space. */
char *
string_list (list)
     WORD_LIST *list;
{
  return (string_list_internal (list, " "));
}

/* Return a single string of all the words present in LIST, obeying the
   quoting rules for "$*", to wit: (P1003.2, draft 11, 3.5.2) "If the
   expansion [of $*] appears within a double quoted string, it expands
   to a single field with the value of each parameter separated by the
   first character of the IFS variable, or by a <space> if IFS is unset." */
char *
string_list_dollar_star (list)
     WORD_LIST *list;
{
  char *ifs = get_string_value ("IFS");
  char sep[2];

  if (!ifs)
    sep[0] = ' ';
  else if (!*ifs)
    sep[0] = '\0';
  else
    sep[0] = *ifs;

  sep[1] = '\0';

  return (string_list_internal (list, sep));
}

/* Return the list of words present in STRING.  Separate the string into
   words at any of the characters found in SEPARATORS.  If QUOTED is
   non-zero then word in the list will have its quoted flag set, otherwise
   the quoted flag is left as make_word () deemed fit.

   This obeys the P1003.2 word splitting semantics.  If `separators' is
   exactly <space><tab><newline>, then the splitting algorithm is that of
   the Bourne shell, which treats any sequence of characters from `separators'
   as a delimiter.  If IFS is unset, which results in `separators' being set
   to "", no splitting occurs.  If separators has some other value, the
   following rules are applied (`IFS white space' means zero or more
   occurrences of <space>, <tab>, or <newline>, as long as those characters
   are in `separators'):

	1) IFS white space is ignored at the start and the end of the
	   string.
	2) Each occurrence of a character in `separators' that is not
	   IFS white space, along with any adjacent occurrences of
	   IFS white space delimits a field.
	3) Any nonzero-length sequence of IFS white space delimits a field.
   */

/* BEWARE!  list_string strips null arguments.  Don't call it twice and
   expect to have "" preserved! */

/* Is the first character of STRING a quoted NULL character? */
#define QUOTED_NULL(string) ((string)[0] == CTLNUL && (string)[1] == '\0')

/* Perform quoted null character removal on STRING.  We don't allow any
   quoted null characters in the middle or at the ends of strings because
   of how expand_word_internal works.  remove_quoted_nulls () simply
   turns STRING into an empty string iff it only consists of a quoted null. */
/*
#define remove_quoted_nulls(string) \
  do { if (QUOTED_NULL (string)) string[0] ='\0'; } while (0)
*/
static void
remove_quoted_nulls (string)
     char *string;
{
  char *nstr, *s, *p;

  nstr = savestring (string);
  nstr[0] = '\0';
  for (p = nstr, s = string; *s; s++)
    {
      if (*s == CTLESC)
	{
	  *p++ = *s++;  /* CTLESC */
	  if (*s == 0)
	    break;
	  *p++ = *s;    /* quoted char */
	  continue;
	}
      if (*s == CTLNUL)
	continue;
      *p++ = *s;
    }
  *p = '\0';
  strcpy (string, nstr);
  free (nstr);
}

/* Perform quoted null character removal on each element of LIST.
   This modifies LIST. */
void
word_list_remove_quoted_nulls (list)
     WORD_LIST *list;
{
  register WORD_LIST *t;

  t = list;

  while (t)
    {
      remove_quoted_nulls (t->word->word);
      t = t->next;
    }
}

/* This performs word splitting and quoted null character removal on
   STRING. */

#define issep(c)	(member ((c), separators))

WORD_LIST *
list_string (string, separators, quoted)
     register char *string, *separators;
     int quoted;
{
  WORD_LIST *result = (WORD_LIST *)NULL;
  char *current_word = (char *)NULL, *s;
  int sindex = 0;
  int sh_style_split;

  if (!string || !*string)
    return ((WORD_LIST *)NULL);

  sh_style_split =
    separators && *separators && (STREQ (separators, " \t\n"));

  /* Remove sequences of whitespace at the beginning of STRING, as
     long as those characters appear in IFS.  Do not do this if
     STRING is quoted or if there are no separator characters. */
  if (!quoted || !separators || !*separators)
    {
      for (s = string; *s && spctabnl (*s) && issep (*s); s++);

      if (!*s)
	return ((WORD_LIST *)NULL);

      string = s;
    }

  /* OK, now STRING points to a word that does not begin with white space.
     The splitting algorithm is:
     	extract a word, stopping at a separator
     	skip sequences of spc, tab, or nl as long as they are separators
     This obeys the field splitting rules in Posix.2. */

  while (string[sindex])
    {
      current_word = string_extract_verbatim (string, &sindex, separators);
      if (!current_word)
	break;

      /* If we have a quoted empty string, add a quoted null argument.  We
	 want to preserve the quoted null character iff this is a quoted
	 empty string; otherwise the quoted null characters are removed
	 below. */
      if (QUOTED_NULL (current_word))
	{
	  WORD_DESC *t = make_word (" ");
	  t->quoted++;
	  free (t->word);
	  t->word = make_quoted_char ('\0');
	  result = make_word_list (t, result);
	}
      else if (strlen (current_word))
	{
	  /* If we have something, then add it regardless.  However,
	     perform quoted null character removal on the current word. */
	  remove_quoted_nulls (current_word);
	  result = make_word_list (make_word (current_word), result);
	  if (quoted)
	    result->word->quoted = 1;
	}

      /* If we're not doing sequences of separators in the traditional
	 Bourne shell style, then add a quoted null argument. */

      else if (!sh_style_split && !spctabnl (string[sindex]))
	{
	  result = make_word_list (make_word (""), result);
	  result->word->quoted = 1;
	}

      free (current_word);

      /* Move past the current separator character. */
      if (string[sindex])
	sindex++;

      /* Now skip sequences of space, tab, or newline characters if they are
	 in the list of separators. */
      while (string[sindex] && spctabnl (string[sindex]) && issep (string[sindex]))
	sindex++;

    }
  return (REVERSE_LIST (result, WORD_LIST *));
}

/* Parse a single word from STRING, using SEPARATORS to separate fields.
   ENDPTR is set to the first character after the word.  This is used by
   the `read' builtin.
   XXX - this function is very similar to list_string; they should be
	 combined - XXX */
char *
get_word_from_string (stringp, separators, endptr)
     char **stringp, *separators, **endptr;
{
  register char *s;
  char *current_word;
  int sindex, sh_style_split;

  if (!stringp || !*stringp || !**stringp)
    return ((char *)NULL);
    
  s = *stringp;

  sh_style_split =
    separators && *separators && (STREQ (separators, " \t\n"));

  /* Remove sequences of whitespace at the beginning of STRING, as
     long as those characters appear in IFS. */
  if (sh_style_split || !separators || !*separators)
    {
      for (; *s && spctabnl (*s) && issep (*s); s++);

      /* If the string is nothing but whitespace, update it and return. */
      if (!*s)
	{
	  *stringp = s;
	  if (endptr)
	    *endptr = s;
	  return ((char *)NULL);
	}
    }

  /* OK, S points to a word that does not begin with white space.
     Now extract a word, stopping at a separator, save a pointer to
     the first character after the word, then skip sequences of spc,
     tab, or nl as long as they are separators.
     
     This obeys the field splitting rules in Posix.2. */
  sindex = 0;
  current_word = string_extract_verbatim (s, &sindex, separators);

  /* Set ENDPTR to the first character after the end of the word. */
  if (endptr)
    *endptr = s + sindex;

  /* Move past the current separator character. */
  if (s[sindex])
    sindex++;

  /* Now skip sequences of space, tab, or newline characters if they are
     in the list of separators. */
  while (s[sindex] && spctabnl (s[sindex]) && issep (s[sindex]))
    sindex++;

  /* Update STRING to point to the next field. */
  *stringp = s + sindex;
  return (current_word);
}

/* Remove IFS white space at the end of STRING.  Start at the end
   of the string and walk backwards until the beginning of the string
   or we find a character that's not IFS white space and not CTLESC.
   Only let CTLESC escape a white space character if SAW_ESCAPE is
   non-zero.  */
char *
strip_trailing_ifs_whitespace (string, separators, saw_escape)
     char *string, *separators;
     int saw_escape;
{
  char *s;
  
  s = string + STRLEN (string) - 1;
  while (s > string && ((spctabnl (*s) && issep (*s)) ||
			(saw_escape && *s == CTLESC && spctabnl (s[1]))))
    s--;
  *++s = '\0';
  return string;
}

#if defined (PROCESS_SUBSTITUTION)
#define EXP_CHAR(s) (s == '$' || s == '`' || s == '<' || s == '>' || s == CTLESC)
#else
#define EXP_CHAR(s) (s == '$' || s == '`' || s == CTLESC)
#endif

/* If there are any characters in STRING that require full expansion,
   then call FUNC to expand STRING; otherwise just perform quote
   removal if necessary.  This returns a new string. */
static char *
maybe_expand_string (string, quoted, func)
     char *string;
     int quoted;
     WORD_LIST *(*func)();
{
  WORD_LIST *list;
  int i, saw_quote;
  char *ret;
     
  for (i = saw_quote = 0; string[i]; i++)
    {
      if (EXP_CHAR (string[i]))
	break;
      else if (string[i] == '\'' || string[i] == '\\' || string[i] == '"')
	saw_quote = 1;
    }

  if (string[i])
    {	  
      list = (*func) (string, quoted);
      if (list)
	{
	  ret = string_list (list);
	  dispose_words (list);
	}
      else
	ret = (char *)NULL;
    }
  else if (saw_quote && !quoted)
    ret = string_quote_removal (string, quoted);
  else
    ret = savestring (string);
  return ret;
}

/* Given STRING, an assignment string, get the value of the right side
   of the `=', and bind it to the left side.  If EXPAND is true, then
   perform parameter expansion, command substitution, and arithmetic
   expansion on the right-hand side.  Perform tilde expansion in any
   case.  Do not perform word splitting on the result of expansion. */
static int
do_assignment_internal (string, expand)
     char *string;
     int expand;
{
  int offset = assignment (string);
  char *name = savestring (string);
  char *value = (char *)NULL;
  SHELL_VAR *entry = (SHELL_VAR *)NULL;

  if (name[offset] == '=')
    {
      char *temp;

      name[offset] = 0;
      temp = name + offset + 1;

      if (expand && temp[0])
	{
	  if (strchr (temp, '~') && unquoted_member ('~', temp))
	    temp = tilde_expand (temp);
	  else
	    temp = savestring (temp);

	  value = maybe_expand_string (temp, 0, expand_string_unsplit);
	  free (temp);
	}
      else
	value = savestring (temp);
    }

  if (value == 0)
    value = savestring ("");

  entry = bind_variable (name, value);

  if (echo_command_at_execute)
    fprintf (stderr, "%s%s=%s\n", indirection_level_string (), name, value);

  stupidly_hack_special_variables (name);

  if (entry)
    entry->attributes &= ~att_invisible;

  FREE (value);
  free (name);

  /* Return 1 if the assignment seems to have been performed correctly. */
  return (entry ? ((entry->attributes & att_readonly) == 0) : 0);
}

/* Perform the assignment statement in STRING, and expand the
   right side by doing command and parameter expansion. */
do_assignment (string)
     char *string;
{
  return do_assignment_internal (string, 1);
}

/* Given STRING, an assignment string, get the value of the right side
   of the `=', and bind it to the left side.  Do not do command and
   parameter substitution on the right hand side. */
do_assignment_no_expand (string)
     char *string;
{
  return do_assignment_internal (string, 0);
}

/* Most of the substitutions must be done in parallel.  In order
   to avoid using tons of unclear goto's, I have some functions
   for manipulating malloc'ed strings.  They all take INDX, a
   pointer to an integer which is the offset into the string
   where manipulation is taking place.  They also take SIZE, a
   pointer to an integer which is the current length of the
   character array for this string. */

/* Append SOURCE to TARGET at INDEX.  SIZE is the current amount
   of space allocated to TARGET.  SOURCE can be NULL, in which
   case nothing happens.  Gets rid of SOURCE by free ()ing it.
   Returns TARGET in case the location has changed. */
inline char *
sub_append_string (source, target, indx, size)
     char *source, *target;
     int *indx, *size;
{
  if (source)
    {
      int srclen, n;

      srclen = strlen (source);
      if (srclen >= (int)(*size - *indx))
	{
	  n = srclen + *indx;
	  n = (n + DEFAULT_ARRAY_SIZE) - (n % DEFAULT_ARRAY_SIZE);
	  target = xrealloc (target, (*size = n));
	}

      FASTCOPY (source, target + *indx, srclen);
      *indx += srclen;
      target[*indx] = '\0';

      free (source);
    }
  return (target);
}

/* Append the textual representation of NUMBER to TARGET.
   INDX and SIZE are as in SUB_APPEND_STRING. */
char *
sub_append_number (number, target, indx, size)
     int number, *indx, *size;
     char *target;
{
  char *temp;

  temp = itos (number);
  return (sub_append_string (temp, target, indx, size));
}

/* Return the word list that corresponds to `$*'. */
WORD_LIST *
list_rest_of_args ()
{
  register WORD_LIST *list = (WORD_LIST *)NULL;
  register WORD_LIST *args = rest_of_args;
  int i;

  /* Break out of the loop as soon as one of the dollar variables is null. */
  for (i = 1; i < 10 && dollar_vars[i]; i++)
    list = make_word_list (make_word (dollar_vars[i]), list);

  while (args)
    {
      list = make_word_list (make_word (args->word->word), list);
      args = args->next;
    }
  return (REVERSE_LIST (list, WORD_LIST *));
}

/* Make a single large string out of the dollar digit variables,
   and the rest_of_args.  If DOLLAR_STAR is 1, then obey the special
   case of "$*" with respect to IFS. */
char *
string_rest_of_args (dollar_star)
     int dollar_star;
{
  register WORD_LIST *list = list_rest_of_args ();
  char *string;

  string = dollar_star ? string_list_dollar_star (list) : string_list (list);
  dispose_words (list);
  return (string);
}

/***************************************************
 *						   *
 *	   Functions to Expand a String		   *
 *						   *
 ***************************************************/
/* Call expand_word_internal to expand W and handle error returns.
   A convenience function for functions that don't want to handle
   any errors or free any memory before aborting. */
static WORD_LIST *
call_expand_word_internal (w, q, c, e)
     WORD_DESC *w;
     int q, *c, *e;
{
  WORD_LIST *result;

  result = expand_word_internal (w, q, c, e);
  if (result == &expand_word_error)
    longjmp (top_level, DISCARD);
  else if (result == &expand_word_fatal)
    longjmp (top_level, FORCE_EOF);
  else
    return (result);
}

/* Perform parameter expansion, command substitution, and arithmetic
   expansion on STRING, as if it were a word.  Leave the result quoted. */
static WORD_LIST *
expand_string_internal (string, quoted)
     char *string;
     int quoted;
{
  WORD_DESC td;
  WORD_LIST *tresult;

  if (!string || !*string)
    return ((WORD_LIST *)NULL);

  bzero (&td, sizeof (td));
  td.word = string;
  tresult = call_expand_word_internal (&td, quoted, (int *)NULL, (int *)NULL);
  return (tresult);
}

/* Expand STRING by performing parameter expansion, command substitution,
   and arithmetic expansion.  Dequote the resulting WORD_LIST before
   returning it, but do not perform word splitting.  The call to
   remove_quoted_nulls () is in here because word splitting normally
   takes care of quote removal. */
WORD_LIST *
expand_string_unsplit (string, quoted)
     char *string;
     int quoted;
{
  WORD_LIST *value;

  if (!string || !*string)
    return ((WORD_LIST *)NULL);

  value = expand_string_internal (string, quoted);
  if (value)
    {
      if (value->word)
	remove_quoted_nulls (value->word->word);
      dequote_list (value);
    }
  return (value);
}

/* This does not perform word splitting or dequote the WORD_LIST
   it returns. */
static WORD_LIST *
expand_string_for_rhs (string, quoted, dollar_at_p, has_dollar_at)
     char *string;
     int quoted, *dollar_at_p, *has_dollar_at;
{
  WORD_DESC td;
  WORD_LIST *tresult;

  if (string == 0 || *string == '\0')
    return (WORD_LIST *)NULL;
     
  bzero (&td, sizeof (td));
  td.word = string; 
  tresult = call_expand_word_internal (&td, quoted, dollar_at_p, has_dollar_at);
  return (tresult);
}

/* Expand STRING just as if you were expanding a word, but do not dequote
   the resultant WORD_LIST.  This is called only from within this file,
   and is used to correctly preserve quoted characters when expanding
   things like ${1+"$@"}.  This does parameter expansion, command
   subsitution, arithmetic expansion, and word splitting. */
static WORD_LIST *
expand_string_leave_quoted (string, quoted)
     char *string;
     int quoted;
{
  WORD_LIST *tlist;
  WORD_LIST *tresult;

  if (!string || !*string)
    return ((WORD_LIST *)NULL);

  tlist = expand_string_internal (string, quoted);

  if (tlist)
    {
      tresult = word_list_split (tlist);
      dispose_words (tlist);
      return (tresult);
    }
  return ((WORD_LIST *)NULL);
}

/* Expand STRING just as if you were expanding a word.  This also returns
   a list of words.  Note that filename globbing is *NOT* done for word
   or string expansion, just when the shell is expanding a command.  This
   does parameter expansion, command substitution, arithmetic expansion,
   and word splitting.  Dequote the resultant WORD_LIST before returning. */
WORD_LIST *
expand_string (string, quoted)
     char *string;
     int quoted;
{
  WORD_LIST *result;

  if (!string || !*string)
    return ((WORD_LIST *)NULL);

  result = expand_string_leave_quoted (string, quoted);

  if (result)
    dequote_list (result);
  return (result);
}

/***************************************************
 *						   *
 *	Functions to handle quoting chars	   *
 *						   *
 ***************************************************/

/* I'm going to have to rewrite expansion because filename globbing is
   beginning to make the entire arrangement ugly.  I'll do this soon. */
static void
dequote_list (list)
     register WORD_LIST *list;
{
  register char *s;

  while (list)
    {
      s = dequote_string (list->word->word);
      free (list->word->word);
      list->word->word = s;
      list = list->next;
    }
}

static char *
make_quoted_char (c)
     int c;
{
  char *temp;

  temp = xmalloc (3);
  if (c == 0)
    {
      temp[0] = CTLNUL;
      temp[1] = '\0';
    }
  else
    {
      temp[0] = CTLESC;
      temp[1] = c;
      temp[2] = '\0';
    }
  return (temp);
}

/* Quote STRING.  Return a new string. */
static char *
quote_string (string)
     char *string;
{
  char *result;

  if (!*string)
    {
      result = xmalloc (2);
      result[0] = CTLNUL;
      result[1] = '\0';
    }
  else
    {
      register char *t;

      result = xmalloc ((strlen (string) * 2) + 1);

      for (t = result; string && *string; )
	{
	  *t++ = CTLESC;
	  *t++ = *string++;
	}
      *t = '\0';
    }
  return (result);
}

/* De-quoted quoted characters in STRING. */
char *
dequote_string (string)
     char *string;
{
  register char *t;
  char *result;

  result = xmalloc (strlen (string) + 1);

  if (QUOTED_NULL (string))
    {
      result[0] = '\0';
      return (result);
    }

  /* If no character in the string can be quoted, don't bother examining
     each character.  Just return a copy of the string passed to us. */
  if (strchr (string, CTLESC) == NULL)		/* XXX */
    {						/* XXX */
      strcpy (result, string);			/* XXX */
      return (result);				/* XXX */
    }

  for (t = result; string && *string; string++)
    {
      if (*string == CTLESC)
	{
	  string++;

	  if (!*string)
	    break;
	}

      *t++ = *string;
    }

  *t = '\0';
  return (result);
}

/* Quote the entire WORD_LIST list. */
static void
quote_list (list)
     WORD_LIST *list;
{
  register WORD_LIST *w;

  for (w = list; w; w = w->next)
    {
      char *t = w->word->word;
      w->word->word = quote_string (t);
      free (t);
      w->word->quoted = 1;
    }
}

/* **************************************************************** */
/*								    */
/*		    Functions for Removing Patterns		    */
/*								    */
/* **************************************************************** */

/* Remove the portion of PARAM matched by PATTERN according to OP, where OP
   can have one of 4 values:
	RP_LONG_LEFT	remove longest matching portion at start of PARAM
	RP_SHORT_LEFT	remove shortest matching portion at start of PARAM
	RP_LONG_RIGHT	remove longest matching portion at end of PARAM
	RP_SHORT_RIGHT	remove shortest matching portion at end of PARAM
*/

#define RP_LONG_LEFT	1
#define RP_SHORT_LEFT	2
#define RP_LONG_RIGHT	3
#define RP_SHORT_RIGHT	4

static char *
remove_pattern (param, pattern, op)
     char *param, *pattern;
     int op;
{
  register int len = param ? strlen (param) : 0;
  register char *end = param + len;
  register char *p, *ret, c;

  if (pattern == NULL || *pattern == '\0')	/* minor optimization */
    return (savestring (param));

  if (param == NULL || *param == '\0')
    return (param);

  switch (op)
    {
      case RP_LONG_LEFT:	/* remove longest match at start */
	for (p = end; p >= param; p--)
	  {
	    c = *p; *p = '\0';
	    if (fnmatch (pattern, param, 0) != FNM_NOMATCH)
	      {
		*p = c;
		return (savestring (p));
	      }
	    *p = c;
	  }
	break;

      case RP_SHORT_LEFT:	/* remove shortest match at start */
	for (p = param; p <= end; p++)
	  {
	    c = *p; *p = '\0';
	    if (fnmatch (pattern, param, 0) != FNM_NOMATCH)
	      {
		*p = c;
		return (savestring (p));
	      }
	    *p = c;
	  }
	break;

      case RP_LONG_RIGHT:	/* remove longest match at end */
	for (p = param; p <= end; p++)
	  {
	    if (fnmatch (pattern, p, 0) != FNM_NOMATCH)
	      {
		c = *p;
		*p = '\0';
		ret = savestring (param);
		*p = c;
		return (ret);
	      }
	  }
	break;

      case RP_SHORT_RIGHT:	/* remove shortest match at end */
	for (p = end; p >= param; p--)
	  {
	    if (fnmatch (pattern, p, 0) != FNM_NOMATCH)
	      {
		c = *p;
		*p = '\0';
		ret = savestring (param);
		*p = c;
		return (ret);
	      }
	  }
	break;
    }
  return (savestring (param));	/* no match, return original string */
}

/*******************************************
 *					   *
 *	Functions to expand WORD_DESCs	   *
 *					   *
 *******************************************/

/* Expand WORD, performing word splitting on the result.  This does
   parameter expansion, command substitution, arithmetic expansion,
   word splitting, and quote removal. */

WORD_LIST *
expand_word (word, quoted)
     WORD_DESC *word;
     int quoted;
{
  WORD_LIST *result, *tresult;

  tresult = call_expand_word_internal (word, quoted, (int *)NULL, (int *)NULL);
  result = word_list_split (tresult);
  dispose_words (tresult);
  if (result)
    dequote_list (result);
  return (result);
}

/* Expand WORD, but do not perform word splitting on the result.  This
   does parameter expansion, command substitution, arithmetic expansion,
   and quote removal. */
WORD_LIST *
expand_word_no_split (word, quoted)
     WORD_DESC *word;
     int quoted;
{
  WORD_LIST *result;

  result = call_expand_word_internal (word, quoted, (int *)NULL, (int *)NULL);
  if (result)
    dequote_list (result);
  return (result);
}

/* Perform shell expansions on WORD, but do not perform word splitting or
   quote removal on the result. */
WORD_LIST *
expand_word_leave_quoted (word, quoted)
     WORD_DESC *word;
     int quoted;
{
  WORD_LIST *result;

  result = call_expand_word_internal (word, quoted, (int *)NULL, (int *)NULL);
  return (result);
}

/* Return the value of a positional parameter.  This handles values > 10. */
char *
get_dollar_var_value (ind)
     int ind;
{
  char *temp;

  if (ind < 10)
    {
      if (dollar_vars[ind])
	temp = savestring (dollar_vars[ind]);
      else
	temp = (char *)NULL;
    }
  else	/* We want something like ${11} */
    {
      WORD_LIST *p = rest_of_args;

      ind -= 10;
      while (p && ind--)
	p = p->next;
      if (p)
	temp = savestring (p->word->word);
      else
	temp = (char *)NULL;
    }
  return (temp);
}

#if defined (PROCESS_SUBSTITUTION)

/* **************************************************************** */
/*								  */
/*		    Hacking Process Substitution		    */
/*								  */
/* **************************************************************** */

extern struct fd_bitmap *current_fds_to_close;
extern char *mktemp ();

#if !defined (HAVE_DEV_FD)
/* Named pipes must be removed explicitly with `unlink'.  This keeps a list
   of FIFOs the shell has open.  unlink_fifo_list will walk the list and
   unlink all of them. add_fifo_list adds the name of an open FIFO to the
   list.  NFIFO is a count of the number of FIFOs in the list. */
#define FIFO_INCR 20

static char **fifo_list = (char **)NULL;
static int nfifo = 0;
static int fifo_list_size = 0;

static void
add_fifo_list (pathname)
     char *pathname;
{
  if (nfifo >= fifo_list_size - 1)
    {
      fifo_list_size += FIFO_INCR;
      fifo_list = (char **)xrealloc (fifo_list,
				     fifo_list_size * sizeof (char *));
    }

  fifo_list[nfifo++] = savestring (pathname);
}

void
unlink_fifo_list ()
{
  if (!nfifo)
    return;

  while (nfifo--)
    {
      unlink (fifo_list[nfifo]);
      free (fifo_list[nfifo]);
      fifo_list[nfifo] = (char *)NULL;
    }
  nfifo = 0;
}

static char *
make_named_pipe ()
{
  char *tname;

  tname = mktemp (savestring ("/tmp/sh-np-XXXXXX"));
  if (mkfifo (tname, 0600) < 0)
    {
      free (tname);
      return ((char *)NULL);
    }

  add_fifo_list (tname);
  return (tname);
}

#if !defined (_POSIX_VERSION)
int
mkfifo (path, mode)
     char *path;
     int mode;
{
#if defined (S_IFIFO)
  return (mknod (path, (mode | S_IFIFO), 0));
#else /* !S_IFIFO */
  return (-1);
#endif /* !S_IFIFO */
}
#endif /* !_POSIX_VERSION */

#else /* HAVE_DEV_FD */

/* DEV_FD_LIST is a bitmap of file descriptors attached to pipes the shell
   has open to children.  NFDS is a count of the number of bits currently
   set in DEV_FD_LIST.  TOTFDS is a count of the highest possible number
   of open files. */
static char *dev_fd_list = (char *)NULL;
static int nfds = 0;
static int totfds;	/* The highest possible number of open files. */

static void
add_fifo_list (fd)
     int fd;
{
  if (!dev_fd_list || fd >= totfds)
    {
      int ofds;

      ofds = totfds;
      totfds = getdtablesize ();
      if (totfds < 0 || totfds > 256)
	totfds = 256;
      if (fd > totfds)
	totfds = fd + 2;

      dev_fd_list = xrealloc (dev_fd_list, totfds);
      bzero (dev_fd_list + ofds, totfds - ofds);
    }

  dev_fd_list[fd] = 1;
  nfds++;
}

void
unlink_fifo_list ()
{
  register int i;

  if (!nfds)
    return;

  for (i = 0; nfds && i < totfds; i++)
    if (dev_fd_list[i])
      {
	close (i);
	dev_fd_list[i] = 0;
	nfds--;
      }

  nfds = 0;
}

#if defined (NOTDEF)
print_dev_fd_list ()
{
  register int i;

  fprintf (stderr, "pid %d: dev_fd_list:", getpid ());
  fflush (stderr);

  for (i = 0; i < totfds; i++)
    {
      if (dev_fd_list[i])
	fprintf (stderr, " %d", i);
    }
  fprintf (stderr, "\n");
}
#endif /* NOTDEF */

static char *
make_dev_fd_filename (fd)
     int fd;
{
  char *ret;

  ret = xmalloc (16 * sizeof (char));
  sprintf (ret, "/dev/fd/%d", fd);
  add_fifo_list (fd);
  return (ret);
}

#endif /* HAVE_DEV_FD */

/* Return a filename that will open a connection to the process defined by
   executing STRING.  HAVE_DEV_FD, if defined, means open a pipe and return
   a filename in /dev/fd corresponding to a descriptor that is one of the
   ends of the pipe.  If not defined, we use named pipes on systems that have
   them.  Systems without /dev/fd and named pipes are out of luck.

   OPEN_FOR_READ_IN_CHILD, if 1, means open the named pipe for reading or
   use the read end of the pipe and dup that file descriptor to fd 0 in
   the child.  If OPEN_FOR_READ_IN_CHILD is 0, we open the named pipe for
   writing or use the write end of the pipe in the child, and dup that
   file descriptor to fd 1 in the child.  The parent does the opposite. */

static char *
process_substitute (string, open_for_read_in_child)
     char *string;
     int open_for_read_in_child;
{
  char *pathname;
  int fd, result;
  pid_t old_pid, pid;
#if defined (HAVE_DEV_FD)
  int parent_pipe_fd, child_pipe_fd;
  int fildes[2];
#endif /* HAVE_DEV_FD */
#if defined (JOB_CONTROL)
  pid_t old_pipeline_pgrp;
#endif  

  if (!string || !*string)
    return ((char *)NULL);

#if !defined (HAVE_DEV_FD)
  pathname = make_named_pipe ();
#else /* HAVE_DEV_FD */
  if (pipe (fildes) < 0)
    {
      internal_error ("can't make pipes for process substitution: %s",
	strerror (errno));
      return ((char *)NULL);
    }
  /* If OPEN_FOR_READ_IN_CHILD == 1, we want to use the write end of
     the pipe in the parent, otherwise the read end. */
  parent_pipe_fd = fildes[open_for_read_in_child];
  child_pipe_fd = fildes[1 - open_for_read_in_child];
  pathname = make_dev_fd_filename (parent_pipe_fd);
#endif /* HAVE_DEV_FD */

  if (!pathname)
    {
      internal_error ("cannot make pipe for process subsitution: %s",
	strerror (errno));
      return ((char *)NULL);
    }

  old_pid = last_made_pid;

#if defined (JOB_CONTROL)
  old_pipeline_pgrp = pipeline_pgrp;
  pipeline_pgrp = shell_pgrp;
  cleanup_the_pipeline ();
  pid = make_child ((char *)NULL, 1);
  if (pid == 0)
    {
      /* Cancel traps, in trap.c. */
      restore_original_signals ();
      setup_async_signals ();
      subshell_environment++;
    }
  set_sigchld_handler ();
  stop_making_children ();
  pipeline_pgrp = old_pipeline_pgrp;
#else /* !JOB_CONTROL */
  pid = make_child ((char *)NULL, 1);
  if (pid == 0)
    {
      /* Cancel traps, in trap.c. */
      restore_original_signals ();
      setup_async_signals ();
      subshell_environment++;
    }
#endif /* !JOB_CONTROL */

  if (pid < 0)
    {
      internal_error ("cannot make a child for process substitution: %s",
	strerror (errno));
      free (pathname);
#if defined (HAVE_DEV_FD)
      close (parent_pipe_fd);
      close (child_pipe_fd);
#endif /* HAVE_DEV_FD */
      return ((char *)NULL);
    }

  if (pid > 0)
    {
      last_made_pid = old_pid;

#if defined (JOB_CONTROL) && defined (PGRP_PIPE)
      close_pgrp_pipe ();
#endif /* JOB_CONTROL && PGRP_PIPE */

#if defined (HAVE_DEV_FD)
      close (child_pipe_fd);
#endif /* HAVE_DEV_FD */

      return (pathname);
    }

  set_sigint_handler ();

#if defined (JOB_CONTROL)
  set_job_control (0);
#endif /* JOB_CONTROL */

#if !defined (HAVE_DEV_FD)
  /* Open the named pipe in the child. */
  fd = open (pathname, open_for_read_in_child ? O_RDONLY : O_WRONLY);
  if (fd < 0)
    {
      internal_error ("cannot open named pipe %s for %s: %s", pathname,
	open_for_read_in_child ? "reading" : "writing", strerror (errno));
      exit (127);
    }
#else /* HAVE_DEV_FD */
  fd = child_pipe_fd;
#endif /* HAVE_DEV_FD */

  if (dup2 (fd, open_for_read_in_child ? 0 : 1) < 0)
    {
      internal_error ("cannot duplicate named pipe %s as fd %d: %s",
	pathname, open_for_read_in_child ? 0 : 1, strerror (errno));
      exit (127);
    }

  close (fd);

  /* Need to close any files that this process has open to pipes inherited
     from its parent. */
  if (current_fds_to_close)
    {
      close_fd_bitmap (current_fds_to_close);
      current_fds_to_close = (struct fd_bitmap *)NULL;
    }

#if defined (HAVE_DEV_FD)
  /* Make sure we close the parent's end of the pipe and clear the slot
     in the fd list so it is not closed later, if reallocated by, for
     instance, pipe(2). */
  close (parent_pipe_fd);
  dev_fd_list[parent_pipe_fd] = 0;
#endif /* HAVE_DEV_FD */

  result = parse_and_execute (string, "process substitution", 0);

#if !defined (HAVE_DEV_FD)
  /* Make sure we close the named pipe in the child before we exit. */
  close (open_for_read_in_child ? 0 : 1);
#endif /* !HAVE_DEV_FD */

  exit (result);
  /*NOTREACHED*/
}
#endif /* PROCESS_SUBSTITUTION */

/* Perform command substitution on STRING.  This returns a string,
   possibly quoted. */
static char *
command_substitute (string, quoted)
     char *string;
     int quoted;
{
  pid_t pid, old_pid;
  int fildes[2];
  char *istring = (char *)NULL;
  int istring_index, istring_size, c = 1;
  int result;

  istring_index = istring_size = 0;

  /* Don't fork () if there is no need to.  In the case of no command to
     run, just return NULL. */
  if (!string || !*string || (string[0] == '\n' && !string[1]))
    return ((char *)NULL);

  /* Pipe the output of executing STRING into the current shell. */
  if (pipe (fildes) < 0)
    {
      internal_error ("Can't make pipes for command substitution!");
      goto error_exit;
    }

  old_pid = last_made_pid;
#if defined (JOB_CONTROL)
  {
    pid_t old_pipeline_pgrp = pipeline_pgrp;

    pipeline_pgrp = shell_pgrp;
    cleanup_the_pipeline ();
    pid = make_child ((char *)NULL, 0);
    if (pid == 0)
      /* Reset the signal handlers in the child, but don't free the
	 trap strings. */
      reset_signal_handlers ();
    set_sigchld_handler ();
    stop_making_children ();
    pipeline_pgrp = old_pipeline_pgrp;
  }
#else /* !JOB_CONTROL */
  pid = make_child ((char *)NULL, 0);

  if (pid == 0)
    /* Reset the signal handlers in the child, but don't free the
       trap strings. */
    reset_signal_handlers ();
#endif /* !JOB_CONTROL */

  if (pid < 0)
    {
      internal_error ("Can't make a child for command substitution: %s",
	strerror (errno));
    error_exit:

      FREE (istring);
      close (fildes[0]);
      close (fildes[1]);
      return ((char *)NULL);
    }

  if (pid == 0)
    {
      set_sigint_handler ();	/* XXX */
#if defined (JOB_CONTROL)
      set_job_control (0);
#endif
      if (dup2 (fildes[1], 1) < 0)
	{
	  internal_error
	    ("command_substitute: cannot duplicate pipe as fd 1: %s",
	     strerror (errno));
	  exit (EXECUTION_FAILURE);
	}

      /* If standard output is closed in the parent shell
	 (such as after `exec >&-'), file descriptor 1 will be
	 the lowest available file descriptor, and end up in
	 fildes[0].  This can happen for stdin and stderr as well,
	 but stdout is more important -- it will cause no output
	 to be generated from this command. */
      if ((fildes[1] != fileno (stdin)) &&
	  (fildes[1] != fileno (stdout)) &&
	  (fildes[1] != fileno (stderr)))
	close (fildes[1]);

      if ((fildes[0] != fileno (stdin)) &&
	  (fildes[0] != fileno (stdout)) &&
	  (fildes[0] != fileno (stderr)))
	close (fildes[0]);

      /* The currently executing shell is not interactive. */
      interactive = 0;

      /* Command substitution does not inherit the -e flag. */
      exit_immediately_on_error = 0;

      remove_quoted_escapes (string);

      /* Give command substitution a place to jump back to on failure,
	 so we don't go back up to main (). */
      result = setjmp (top_level);

      if (result == EXITPROG)
	exit (last_command_exit_value);
      else if (result)
	exit (EXECUTION_FAILURE);
      else
	exit (parse_and_execute (string, "command substitution", -1));
    }
  else
    {
      FILE *istream;

      istream = fdopen (fildes[0], "r");

#if defined (JOB_CONTROL) && defined (PGRP_PIPE)
      close_pgrp_pipe ();
#endif /* JOB_CONTROL && PGRP_PIPE */

      close (fildes[1]);

      if (!istream)
	{
	  internal_error ("Can't reopen pipe to command substitution (fd %d): %s",
			fildes[0], strerror (errno));
	  goto error_exit;
	}

      /* Read the output of the command through the pipe. */
      while (1)
	{
#if defined (NO_READ_RESTART_ON_SIGNAL)
	  c = getc_with_restart (istream);
#else
	  c = getc (istream);
#endif /* !NO_READ_RESTART_ON_SIGNAL */

	  if (c == EOF)
	    break;

	  /* Add the character to ISTRING. */
	  if (istring_index + 2 >= istring_size)
	    {
	      while (istring_index + 2 >= istring_size)
		istring_size += DEFAULT_ARRAY_SIZE;
	      istring = xrealloc (istring, istring_size);
	    }

	  if (quoted || c == CTLESC || c == CTLNUL)
	    istring[istring_index++] = CTLESC;

	  istring[istring_index++] = c;
	  istring[istring_index] = '\0';
	}

      fclose (istream);
      close (fildes[0]);

      last_command_exit_value = wait_for (pid);
      last_command_subst_pid = pid;
      last_made_pid = old_pid;

#if defined (JOB_CONTROL)
      /* If last_command_exit_value > 128, then the substituted command
	 was terminated by a signal.  If that signal was SIGINT, then send
	 SIGINT to ourselves.  This will break out of loops, for instance. */
      if (last_command_exit_value == (128 + SIGINT))
	kill (getpid (), SIGINT);

      /* wait_for gives the terminal back to shell_pgrp.  If some other
	 process group should have it, give it away to that group here. */
      if (interactive && pipeline_pgrp != (pid_t)0)
	give_terminal_to (pipeline_pgrp);
#endif /* JOB_CONTROL */

      /* If we read no output, just return now and save ourselves some
	 trouble. */
      if (istring_index == 0)
	goto error_exit;

      /* Strip trailing newlines from the output of the command. */
      if (quoted)
	{
	  while (istring_index > 0)
	    {
	      if (istring[istring_index - 1] == '\n')
		{
		  --istring_index;

		  /* If the newline was quoted, remove the quoting char. */
		  if (istring[istring_index - 1] == CTLESC)
		    --istring_index;
		}
	      else
		break;
	    }
	  istring[istring_index] = '\0';
	}
      else
	strip_trailing (istring, 1);

      return (istring);
    }
}

/********************************************************
 *							*
 *	Utility functions for parameter expansion	*
 *							*
 ********************************************************/

/* Handle removing a pattern from a string as a result of ${name%[%]value}
   or ${name#[#]value}. */
static char *
parameter_brace_remove_pattern (value, temp, c)
     char *value, *temp;
     int c;
{
  int pattern_specifier;
  WORD_LIST *l;
  char *pattern, *t, *tword;

  if (c == '#')
    {
      if (*value == '#')
	{
	  value++;
	  pattern_specifier = RP_LONG_LEFT;
	}
      else
	pattern_specifier = RP_SHORT_LEFT;
    }
  else	/* c == '%' */
    {
      if (*value == '%')
	{
	  value++;
	  pattern_specifier = RP_LONG_RIGHT;
	}
      else
	pattern_specifier = RP_SHORT_RIGHT;
    }

  /* Posix.2 says that the WORD should be run through tilde expansion,
     parameter expansion, command substitution and arithmetic expansion.
     This leaves the result quoted, so quote_string_for_globbing () has
     to be called to fix it up for fnmatch (). */
  if (strchr (value, '~'))
    tword = tilde_expand (value);
  else
    tword = savestring (value);

  /* expand_string_internal () leaves WORD quoted and does not perform
     word splitting. */
  l = expand_string_internal (tword, 0);
  free (tword);
  pattern = string_list (l);
  dispose_words (l);

  if (pattern)
    {
      tword = quote_string_for_globbing (pattern, 1);
      free (pattern);
      pattern = tword;
    }

  t = remove_pattern (temp, pattern, pattern_specifier);

  FREE (pattern);
  return (t);
}

static int
valid_brace_expansion_word (name, var_is_special)
     char *name;
     int var_is_special;
{
  if (digit (*name) && all_digits (name))
    return 1;
  else if (var_is_special)
    return 1;
  else if (legal_identifier (name))
    return 1;
  else
    return 0;
}
/* Parameter expand NAME, and return a new string which is the expansion,
   or NULL if there was no expansion.
   VAR_IS_SPECIAL is non-zero if NAME is one of the special variables in
   the shell, e.g., "@", "$", "*", etc.  QUOTED, if non-zero, means that
   NAME was found inside of a double-quoted expression. */
static char *
parameter_brace_expand_word (name, var_is_special, quoted)
     char *name;
     int var_is_special, quoted;
{
  char *temp = (char *)NULL;

  /* Handle multiple digit arguments, as in ${11}. */
  if (digit (*name))
    {
      int arg_index = atoi (name);

      temp = get_dollar_var_value (arg_index);
    }
  else if (var_is_special)      /* ${@} */
    {
      char *tt;
      WORD_LIST *l;

      tt = xmalloc (2 + strlen (name));
      tt[0] = '$'; tt[1] = '\0';
      strcpy (tt + 1, name);
      l = expand_string_leave_quoted (tt, quoted);
      free (tt);
      temp = string_list (l);
      dispose_words (l);
    }
  else
    {
      SHELL_VAR *var = find_variable (name);

      if (var && !invisible_p (var) && (temp = value_cell (var)))
	temp = quoted && temp && *temp ? quote_string (temp)
				       : quote_escapes (temp);
    }
  return (temp);
}

/* Expand the right side of a parameter expansion of the form ${NAMEcVALUE},
   depending on the value of C, the separating character.  C can be one of
   "-", "+", or "=". */
static char *
parameter_brace_expand_rhs (name, value, c, quoted)
     char *name, *value;
     int c, quoted;
{
  WORD_LIST *l;
  char *t, *t1, *temp;
  int i, lquote, hasdol;

  if (value[0] == '~' ||
      (strchr (value, '~') && unquoted_substring ("=~", value)))
    temp = tilde_expand (value);
  else
    temp = savestring (value);

  /* This is a hack.  A better fix is coming later. */
  lquote = 0;
  if (*temp == '"' && temp[strlen (temp) - 1] == '"')
    {
      i = 1;
      t = string_extract_double_quoted (temp, &i);	/* XXX */
      free (temp);
      temp = t;
      lquote = 1;	/* XXX */
    }
  hasdol = 0;
  /* XXX was quoted not lquote */
  l = *temp ? expand_string_for_rhs (temp, quoted||lquote, &hasdol, (int *)NULL)
	    : (WORD_LIST *)NULL;
  free (temp);
  /* expand_string_for_rhs does not dequote the word list it returns, but
     there are a few cases in which we need to add quotes. */
  if (lquote && quoted == 0 && hasdol == 0 && l && l->word->quoted == 0)
    quote_list (l);

  if (l)
    {
      temp = string_list (l);
      dispose_words (l);
    }
  else if (lquote)
    {
      temp = xmalloc (2);
      temp[0] = CTLNUL;
      temp[1] = '\0';
    }
  else
    temp = (char *)NULL;

  if (c == '-' || c == '+')
    return (temp);

  /* c == '=' */
  if (temp)
    t = savestring (temp);
  else
    t = savestring ("");
  t1 = dequote_string (t);
  free (t);
  bind_variable (name, t1);
  free (t1);
  return (temp);
}

/* Deal with the right hand side of a ${name:?value} expansion in the case
   that NAME is null or not set.  If VALUE is non-null it is expanded and
   used as the error message to print, otherwise a standard message is
   printed. */
static void
parameter_brace_expand_error (name, value)
     char *name, *value;
{
  if (value && *value)
    {
      WORD_LIST *l = expand_string (value, 0);
      char *temp1 = string_list (l);
      report_error ("%s: %s", name, temp1 ? temp1 : value);
      FREE (temp1);
      dispose_words (l);
    }
  else
    report_error ("%s: parameter null or not set", name);

  /* Free the data we have allocated during this expansion, since we
     are about to longjmp out. */
  free (name);
  FREE (value);
}

/* Return 1 if NAME is something for which parameter_brace_expand_length is
   OK to do. */
static int
valid_length_expression (name)
     char *name;
{
  return (!name[1] ||						/* ${#} */
	  ((name[1] == '@' || name[1] == '*') && !name[2]) ||	/* ${#@}, ${#*} */
	  (digit (name[1]) && all_digits (name + 1)) ||		/* ${#11} */
	  legal_identifier (name + 1));				/* ${#PS1} */
}

/* Handle the parameter brace expansion that requires us to return the
   length of a parameter. */
static int
parameter_brace_expand_length (name)
     char *name;
{
  char *t;
  int number = 0;

  if (name[1] == '\0')			/* ${#} */
    {
      WORD_LIST *l = list_rest_of_args ();
      number = list_length (l);
      dispose_words (l);
    }
  else if (name[1] != '*' && name[1] != '@')
    {
      number = 0;

      if (digit (name[1]))		/* ${#1} */
	{
	  if (t = get_dollar_var_value (atoi (name + 1)))
	    {
	      number = strlen (t);
	      free (t);
	    }
	}
      else				/* ${#PS1} */
	{
	  WORD_LIST *list;
	  char *newname;

	  newname = savestring (name);
	  newname[0] = '$';
	  list = expand_string (newname, 0);
	  t = string_list (list);
	  free (newname);
	  dispose_words (list);

	  if (t)
	    number = strlen (t);

	  FREE (t);
	}
    }
  else					/* ${#@} and ${#*} */
    {
#if !defined (KSH_INCOMPATIBLE)
      WORD_LIST *l = list_rest_of_args ();
      number = l ? list_length (l) : 0;
      dispose_words (l);
#else
      if (t = string_rest_of_args (1))
	{
	  number = strlen (t);
	  free (t);
	}
#endif /* KSH_INCOMPATIBLE */
    }
  return (number);
}

/* Make a word list which is the parameter and variable expansion,
   command substitution, arithmetic substitution, and quote removed
   expansion of WORD.  Return a pointer to a WORD_LIST which is the
   result of the expansion.  If WORD contains a null word, the word
   list returned is also null.

   QUOTED, when non-zero specifies that the text of WORD is treated
   as if it were surrounded by double quotes.
   CONTAINS_DOLLAR_AT and EXPANDED_SOMETHING are return values; when non-null
   they point to an integer value which receives information about expansion.
   CONTAINS_DOLLAR_AT gets non-zero if WORD contained "$@", else zero.
   EXPANDED_SOMETHING get non-zero if WORD contained any parameter expansions,
   else zero.

   This only does word splitting in the case of $@ expansion.  In that
   case, we split on ' '. */

/* Values for the local variable quoted_state. */
#define UNQUOTED	 0
#define PARTIALLY_QUOTED 1
#define WHOLLY_QUOTED    2

static WORD_LIST *
expand_word_internal (word, quoted, contains_dollar_at, expanded_something)
     WORD_DESC *word;
     int quoted;
     int *contains_dollar_at;
     int *expanded_something;
{
  /* The thing that we finally output. */
  WORD_LIST *result = (WORD_LIST *)NULL;

  /* The intermediate string that we build while expanding. */
  char *istring = xmalloc (DEFAULT_ARRAY_SIZE);

  /* The current size of the above object. */
  int istring_size = DEFAULT_ARRAY_SIZE;

  /* Index into ISTRING. */
  int istring_index = 0;

  /* Temporary string storage. */
  char *temp = (char *)NULL;

  /* The text of WORD. */
  register char *string = word->word;

  /* The index into STRING. */
  int sindex = 0;

  /* This gets 1 if we see a $@ while quoted. */
  int quoted_dollar_at = 0;

  /* One of UNQUOTED, PARTIALLY_QUOTED, or WHOLLY_QUOTED, depending on
     whether WORD contains no quoting characters, a partially quoted
     string (e.g., "xx"ab), or is fully quoted (e.g., "xxab"). */
  int quoted_state = UNQUOTED;

  register int c;		/* Current character. */
  int number;			/* Temporary number value. */
  int t_index;			/* For calls to string_extract_xxx. */
  char *command_subst_result;	/* For calls to command_substitute (). */

  istring[0] = '\0';

  if (!string) goto final_exit;

  if (contains_dollar_at)
    *contains_dollar_at = 0;

  /* Begin the expansion. */

  for (;;)
    {
      c = string[sindex];

      /* Case on toplevel character. */
      switch (c)
	{
	case '\0':
	  goto finished_with_string;

	case CTLESC:
	  temp = xmalloc (3);
	  temp[0] = CTLESC;
	  temp[1] = c = string[++sindex];
	  temp[2] = '\0';

	  if (string[sindex])
	    sindex++;

	  goto add_string;

#if defined (PROCESS_SUBSTITUTION)
	  /* Process substitution. */
	case '<':
	case '>':
	  {
	    char *temp1;
	    int old_index;

	    if (string[++sindex] != '(' || quoted || posixly_correct)
	      {
		sindex--;
		goto add_character;
	      }
	    else
	      old_index = ++sindex; /* skip past both '<' and '(' */

	    temp1 = extract_process_subst
	      (string, (c == '<') ? "<(" : ">(", &old_index);
	    sindex = old_index;

	    /* If the process substitution specification is `<()', we want to
	       open the pipe for writing in the child and produce output; if
	       it is `>()', we want to open the pipe for reading in the child
	       and consume input. */
	    temp = process_substitute (temp1, (c == '>'));

	    FREE (temp1);

	    goto dollar_add_string;
	  }
#endif /* PROCESS_SUBSTITUTION */

	/* See about breaking this into a separate function:
	    char *
	    param_expand (string, sindex, quoted, expanded_something,
			  contains_dollar_at, quoted_dollar_at)
	    char *string;
	    int *sindex, quoted, *expanded_something, *contains_dollar_at;
	    int *quoted_dollar_at;
	*/
	case '$':

	  if (expanded_something)
	    *expanded_something = 1;

	  c = string[++sindex];

	  /* Do simple cases first. Switch on what follows '$'. */
	  switch (c)
	    {
	      /* $0 .. $9? */
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	    case '8':
	    case '9':
	      temp = dollar_vars[digit_value (c)];
	      if (unbound_vars_is_error && temp == (char *)NULL)
		{
		  report_error ("$%c: unbound variable", c);
		  free (string);
		  free (istring);
		  last_command_exit_value = 1;
		  return (&expand_word_error);
		}
	      if (temp)
		temp = savestring (temp);
	      goto dollar_add_string;

	      /* $$ -- pid of the invoking shell. */
	    case '$':
	      number = dollar_dollar_pid;

	    add_number:
	      temp = itos (number);
	    dollar_add_string:
	      if (string[sindex]) sindex++;

	      /* Add TEMP to ISTRING. */
	    add_string:
	      istring = sub_append_string
		(temp, istring, &istring_index, &istring_size);
	      temp = (char *)NULL;
	      break;

	      /* $# -- number of positional parameters. */
	    case '#':
	      {
		WORD_LIST *list = list_rest_of_args ();
		number = list_length (list);
		dispose_words (list);
		goto add_number;
	      }

	      /* $? -- return value of the last synchronous command. */
	    case '?':
	      number = last_command_exit_value;
	      goto add_number;

	      /* $- -- flags supplied to the shell on invocation or
		 by `set'. */
	    case '-':
	      temp = which_set_flags ();
	      goto dollar_add_string;

	      /* $! -- Pid of the last asynchronous command. */
	    case '!':
	      number = (int)last_asynchronous_pid;

	      /* If no asynchronous pids have been created, echo nothing. */
	      if (number == (int)NO_PID)
		{
		  if (string[sindex])
		    sindex++;
		  if (expanded_something)
		    *expanded_something = 0;
		  break;
		}
	      goto add_number;

	      /* The only difference between this and $@ is when the
		 arg is quoted. */
	    case '*':		/* `$*' */
	      temp = string_rest_of_args (quoted);

	      if (quoted && temp && *temp == '\0' /* && istring_index > 0 */)
		{
		  free (temp);
		  temp = (char *)NULL;
		}

	      /* In the case of a quoted string, quote the entire arg-list.
		 "$1 $2 $3". */
	      if (quoted && temp)
		{
		  char *james_brown = temp;
		  temp = quote_string (temp);
		  free (james_brown);
		}
	      goto dollar_add_string;

	      /* When we have "$@" what we want is "$1" "$2" "$3" ... This
		 means that we have to turn quoting off after we split into
		 the individually quoted arguments so that the final split
		 on the first character of $IFS is still done.  */
	    case '@':		/* `$@' */
	      {
		WORD_LIST *tlist = list_rest_of_args ();
		if (quoted && tlist)
		  quote_list (tlist);

		/* We want to flag the fact that we saw this.  We can't turn
		   off quoting entirely, because other characters in the
		   string might need it (consider "\"$@\""), but we need some
		   way to signal that the final split on the first character
		   of $IFS should be done, even though QUOTED is 1. */
		if (quoted)
		  quoted_dollar_at = 1;
		if (contains_dollar_at)
		  *contains_dollar_at = 1;
		temp = string_list (tlist);
		dispose_words (tlist);		
		goto dollar_add_string;
	      }

	      /* ${[#]name[[:]#[#]%[%]-=?+[word]]} */
	    case '{':
	      {
		int check_nullness = 0;
		int var_is_set = 0;
		int var_is_null = 0;
		int var_is_special = 0;
		char *name, *value;

		t_index = ++sindex;
		name = string_extract (string, &t_index, "#%:-=?+}");
		value = (char *)NULL;

		/* If the name really consists of a special variable, then
		   make sure that we have the entire name. */
		if (sindex == t_index &&
		    (string[sindex] == '-' ||
		     string[sindex] == '?' ||
		     string[sindex] == '#'))
		  {
		    char *tt;
		    t_index++;
		    free (name);
		    tt = string_extract (string, &t_index, "#%:-=?+}");
		    name = xmalloc (2 + (strlen (tt)));
		    *name = string[sindex];
		    strcpy (name + 1, tt);
		    free (tt);
		  }
		sindex = t_index;

		/* Find out what character ended the variable name.  Then
		   do the appropriate thing. */
		if (c = string[sindex])
		  sindex++;

		if (c == ':')
		  {
		    check_nullness++;
		    if (c = string[sindex])
		      sindex++;
		  }

		/* Determine the value of this variable. */
		if ((digit (*name) && all_digits (name)) ||
		    (strlen (name) == 1 && member (*name, "#-?$!@*")))
		  var_is_special++;

		/* Check for special expansion things. */
		if (*name == '#')
		  {
		    /* Handle ${#-} and ${#?}.  They return the lengths of
		       $- and $?, respectively. */
		    if (string[sindex] == '}' &&
			!name[1] &&
			!check_nullness &&
			(c == '-' || c == '?'))
		      {
			char *s;

			free (name);

			if (c == '-')
			  s = which_set_flags ();
			else
			  s = itos (last_command_exit_value);

			number = STRLEN (s);
			FREE (s);
			goto add_number;
		      }

		    /* Don't allow things like ${#:-foo} to go by; they are
		       errors.  If we are not pointing at the character just
		       after the closing brace, then we haven't gotten all of
		       the name.  Since it begins with a special character,
		       this is a bad substitution.  Explicitly check for ${#:},
		       which the rules do not catch. */
		    if (string[sindex - 1] != '}' || member (c, "?-=+") ||
			(string[sindex - 1] == '}' && !name[1] && c == '}' &&
			 check_nullness))
		      {
			free (name);
			name = string;
			goto bad_substitution;
		      }

		    /* Check NAME for validity before trying to go on. */
		    if (!valid_length_expression (name))
		      {
			free (name);
			name = string;
			goto bad_substitution;
		      }

		    number = parameter_brace_expand_length (name);
		    free (name);
		    /* We are pointing one character after the brace which
		       closes this expression.  Since the code at add_number
		       increments SINDEX, we back up a single character. */
		    sindex--;
		    goto add_number;
		  }

		/* ${@} is identical to $@. */
		if (name[0] == '@' && name[1] == '\0')
		  {
		    if (quoted)
		      quoted_dollar_at = 1;

		    if (contains_dollar_at)
		      *contains_dollar_at = 1;
		  }

		/* Make sure that NAME is valid before trying to go on. */
		if (!valid_brace_expansion_word (name, var_is_special))
		  {
		    free (name);
		    name = string;
		    goto bad_substitution;
		  }

		temp =
		  parameter_brace_expand_word (name, var_is_special, quoted);

		if (temp)
		  var_is_set++;

		if (!var_is_set || !temp || !*temp)
		  var_is_null++;

		if (!check_nullness)
		  var_is_null = 0;

		/* Get the rest of the stuff inside the braces. */
		if (c && c != '}')
		  {
		    /* Extract the contents of the ${ ... } expansion
		       according to the Posix.2 rules.  It's much less of
		       a hack that the former extract_delimited_string ()
		       scheme. */
		    value = extract_dollar_brace_string (string, &sindex);

		    if (string[sindex] == '}')
		      sindex++;
		    else
		      {
			free (name);
			name = string;
			goto bad_substitution;
		      }
		  }
		else
		  value = (char *)NULL;

		/* Do the right thing based on which character ended the
		   variable name. */
		switch (c)
		  {
		  default:
		    free (name);
		    name = string;
		    /* FALL THROUGH */

		  case '\0':
		  bad_substitution:
		    report_error ("%s: bad substitution", name ? name : "??");
		    FREE (value);
		    free (temp);
		    free (name);
		    free (istring);
		    return &expand_word_error;

		  case '}':
		    if (!var_is_set && unbound_vars_is_error)
		      {
			report_error ("%s: unbound variable", name);
			FREE (value);
			free (temp);
			free (name);
			free (string);
			last_command_exit_value = 1;
			free (istring);
			return &expand_word_error;
		      }
		    break;

		  case '#':	/* ${param#[#]pattern} */
		  case '%':	/* ${param%[%]pattern} */
		    {
		      char *t;
		      if (!value || !*value || !temp || !*temp)
			break;
		      if (quoted)
			{
			  t = dequote_string (temp);
			  free (temp);
			  temp = t;
			}
		      t = parameter_brace_remove_pattern (value, temp, c);
		      free (temp);
		      free (value);
		      temp = t;
		    }
		    break;

		  case '-':
		  case '=':
		  case '?':
		  case '+':
		    if (var_is_set && !var_is_null)
		      {
			/* We don't want the value of the named variable for
			   anything, just the value of the right hand side. */
			if (c == '+')
			  {
			    FREE (temp);
			    if (value)
			      {
				temp = parameter_brace_expand_rhs
				  (name, value, c, quoted);
				/* XXX - this is a hack.  A better fix is
					 coming later. */
				if ((value[0] == '$' && value[1] == '@') ||
				    (value[0] == '"' && value[1] == '$' && value[2] == '@'))
				  {
				    if (quoted)
				      quoted_dollar_at++;
				    if (contains_dollar_at)
				      *contains_dollar_at = 1;
				  }
				free (value);
			      }
			    else
			      temp = (char *)NULL;
			  }
			else
			  {
			    FREE (value);
			  }
			/* Otherwise do nothing; just use the value in TEMP. */
		      }
		    else	/* VAR not set or VAR is NULL. */
		      {
			FREE (temp);
			temp = (char *)NULL;
			if (c == '=' && var_is_special)
			  {
			    report_error
			      ("$%s: cannot assign in this way", name);
			    free (name);
			    free (value);
			    free (string);
			    free (istring);
			    return &expand_word_error;
			  }
			else if (c == '?')
			  {
			    free (string);
			    free (istring);
			    parameter_brace_expand_error (name, value);
			    if (!interactive)
			      return &expand_word_fatal;
			    else
			      return &expand_word_error;
			  }
			else if (c != '+')
			  temp = parameter_brace_expand_rhs
			    (name, value, c, quoted);
			free (value);
		      }
		    break;
		  }		/* end case on closing character. */
		free (name);
		goto add_string;
	      }			/* end case '{' */
	      /* break; */

	      /* Do command or arithmetic substitution. */
	    case '(':
	      /* We have to extract the contents of this paren substitution. */
	      {
		int old_index = ++sindex;

		temp = extract_command_subst (string, &old_index);
		sindex = old_index;

		/* For Posix.2-style `$(( ))' arithmetic substitution,
		   extract the expression and pass it to the evaluator. */
		if (temp && *temp == '(')
		  {
		    char *t = temp + 1;
		    int last = strlen (t) - 1;

		    if (t[last] != ')')
		      {
			report_error ("%s: bad arithmetic substitution", temp);
			free (temp);
			free (string);
			free (istring);
			return &expand_word_error;
		      }

		    /* Cut off ending `)' */
		    t[last] = '\0';

		    /* Expand variables found inside the expression. */
		    {
		      WORD_LIST *l;

		      l = expand_string (t, 1);
		      t = string_list (l);
		      dispose_words (l);
		    }

		    /* No error messages. */
		    this_command_name = (char *)NULL;

		    number = evalexp (t);
		    free (temp);
		    free (t);

		    goto add_number;
		  }

		goto handle_command_substitution;
	      }

	      /* Do straight arithmetic substitution. */
	    case '[':
	      /* We have to extract the contents of this
		 arithmetic substitution. */
	      {
		char *t;
		int old_index = ++sindex;
		WORD_LIST *l;

		temp = extract_arithmetic_subst (string, &old_index);
		sindex = old_index;

		/* Do initial variable expansion. */
		l = expand_string (temp, 1);
		t = string_list (l);
		dispose_words (l);

		/* No error messages. */
		this_command_name = (char *)NULL;
		number = evalexp (t);
		free (t);
		free (temp);

		goto add_number;
	      }

	    default:
	      {
		/* Find the variable in VARIABLE_LIST. */
		int old_index;
		char *name;
		SHELL_VAR *var;

		temp = (char *)NULL;

		for (old_index = sindex;
		     (c = string[sindex]) &&
		     (isletter (c) || digit (c) || c == '_');
		     sindex++);
		name = substring (string, old_index, sindex);

		/* If this isn't a variable name, then just output the `$'. */
		if (!name || !*name)
		  {
		    FREE (name);
		    temp = savestring ("$");
		    if (expanded_something)
		      *expanded_something = 0;
		    goto add_string;
		  }

		/* If the variable exists, return its value cell. */
		var = find_variable (name);

		if (var && !invisible_p (var) && value_cell (var))
		  {
		    temp = value_cell (var);
		    temp = quoted && temp && *temp ? quote_string (temp)
						   : quote_escapes (temp);
		    free (name);
		    goto add_string;
		  }
		else
		  temp = (char *)NULL;

		if (unbound_vars_is_error)
		  report_error ("%s: unbound variable", name);
		else
		  {
		    free (name);
		    goto add_string;
		  }

		free (name);
		free (string);
		last_command_exit_value = 1;
		free (istring);
		return &expand_word_error;
	      }
	    }
	  break;		/* End case '$': */

	case '`':		/* Backquoted command substitution. */
	  {
	    sindex++;

	    if (expanded_something)
	      *expanded_something = 1;

	    temp = string_extract (string, &sindex, "`");
	    de_backslash (temp);

	  handle_command_substitution:
	    command_subst_result = command_substitute (temp, quoted);

	    FREE (temp);

	    temp = command_subst_result;

	    if (string[sindex])
	      sindex++;

	    goto add_string;
	  }

	case '\\':
	  if (string[sindex + 1] == '\n')
	    {
	      sindex += 2;
	      continue;
	    }
	  else
	    {
	      char *slashify_chars = "";

	      c = string[++sindex];

	      if (quoted == Q_HERE_DOCUMENT)
		slashify_chars = slashify_in_here_document;
	      else if (quoted == Q_DOUBLE_QUOTES)
		slashify_chars = slashify_in_quotes;

	      if (quoted && !member (c, slashify_chars))
		{
		  temp = xmalloc (3);
		  temp[0] = '\\'; temp[1] = c; temp[2] = '\0';
		  if (c)
		    sindex++;
		  goto add_string;
		}
	      else
		{
		  /* This character is quoted, so add it in quoted mode. */
		  temp = make_quoted_char (c);
		  if (c)
		    sindex++;
		  goto add_string;
		}
	    }

	case '"':
	  if (quoted)
	    goto add_character;
	  sindex++;
	  {
	    WORD_LIST *tresult = (WORD_LIST *)NULL;

	    t_index = sindex;
	    temp = string_extract_double_quoted (string, &sindex);

	    /* If the quotes surrounded the entire string, then the
	       whole word was quoted. */
	    if (t_index == 1 && !string[sindex])
	      quoted_state = WHOLLY_QUOTED;
	    else
	      quoted_state = PARTIALLY_QUOTED;

	    if (temp && *temp)
	      {
		int dollar_at_flag;
		int quoting_flags = Q_DOUBLE_QUOTES;
		WORD_DESC *temp_word = make_word (temp);

		free (temp);

		tresult = expand_word_internal
		  (temp_word, quoting_flags, &dollar_at_flag, (int *)NULL);

		if (tresult == &expand_word_error || tresult == &expand_word_fatal)
		  {
		    free (istring);
		    free (string);
		    /* expand_word_internal has already freed temp_word->word
		       for us because of the way it prints error messages. */
		    temp_word->word = (char *)NULL;
		    dispose_word (temp_word);
		    return tresult;
		  }

		dispose_word (temp_word);

		/* "$@" (a double-quoted dollar-at) expands into nothing,
		   not even a NULL word, when there are no positional
		   parameters. */
		if (!tresult && dollar_at_flag)
		  {
		    quoted_dollar_at++;
		    break;
		  }

		/* If we get "$@", we know we have expanded something, so we
		   need to remember it for the final split on $IFS.  This is
		   a special case; it's the only case where a quoted string
		   can expand into more than one word.  It's going to come back
		   from the above call to expand_word_internal as a list with
		   a single word, in which all characters are quoted and
		   separated by blanks.  What we want to do is to turn it back
		   into a list for the next piece of code. */
		dequote_list (tresult);

		if (dollar_at_flag)
		  {
		    quoted_dollar_at++;
		    if (expanded_something)
		      *expanded_something = 1;
		  }
	      }
	    else
	      {
		/* What we have is "".  This is a minor optimization. */
		free (temp);
		tresult = (WORD_LIST *)NULL;
	      }

	    /* The code above *might* return a list (consider the case of "$@",
	       where it returns "$1", "$2", etc.).  We can't throw away the
	       rest of the list, and we have to make sure each word gets added
	       as quoted.  We test on tresult->next:  if it is non-NULL, we
	       quote the whole list, save it to a string with string_list, and
	       add that string. We don't need to quote the results of this
	       (and it would be wrong, since that would quote the separators
	       as well), so we go directly to add_string. */
	    if (tresult)
	      {
		if (tresult->next)
		  {
		    quote_list (tresult);
		    temp = string_list (tresult);
		    dispose_words (tresult);
		    goto add_string;
		  }
		else
		  {
		    temp = savestring (tresult->word->word);
		    dispose_words (tresult);
		  }
	      }
	    else
	      temp = (char *)NULL;

	    /* We do not want to add quoted nulls to strings that are only
	       partially quoted; we can throw them away. */
	    if (!temp && (quoted_state == PARTIALLY_QUOTED))
	      continue;

	  add_quoted_string:

	    if (temp)
	      {
		char *t = temp;
		temp = quote_string (temp);
		free (t);
	      }
	    else
	      {
		/* Add NULL arg. */
		temp = xmalloc (2);
		temp[0] = CTLNUL;
		temp[1] = '\0';
	      }
	    goto add_string;
	  }
	  /* break; */

	case '\'':
	  {
	    if (!quoted)
	      {
		sindex++;

		t_index = sindex;
		temp = string_extract_single_quoted (string, &sindex);

		/* If the entire STRING was surrounded by single quotes,
		   then the string is wholly quoted. */
		if (t_index == 1 && !string[sindex])
		  quoted_state = WHOLLY_QUOTED;
		else
		  quoted_state = PARTIALLY_QUOTED;

		/* If all we had was '', it is a null expansion. */
		if (!*temp)
		  {
		    free (temp);
		    temp = (char *)NULL;
		  }
		else
		  remove_quoted_escapes (temp);

		/* We do not want to add quoted nulls to strings that are only
		   partially quoted; such nulls are discarded. */
		if (!temp && (quoted_state == PARTIALLY_QUOTED))
		  continue;

		goto add_quoted_string;
	      }
	    else
	      goto add_character;

	    break;
	  }

	default:

	  /* This is the fix for " $@ " */
	  if (quoted)
	    {
	      temp = make_quoted_char (c);
	      if (string[sindex])
		sindex++;
	      goto add_string;
	    }

	add_character:
	  if (istring_index + 1 >= istring_size)
	    {
	      while (istring_index + 1 >= istring_size)
		istring_size += DEFAULT_ARRAY_SIZE;
	      istring = xrealloc (istring, istring_size);
	    }
	  istring[istring_index++] = c;
	  istring[istring_index] = '\0';

	  /* Next character. */
	  sindex++;
	}
    }

finished_with_string:
final_exit:
  /* OK, we're ready to return.  If we have a quoted string, and
     quoted_dollar_at is not set, we do no splitting at all; otherwise
     we split on ' '.  The routines that call this will handle what to
     do if nothing has been expanded. */
  if (istring)
    {
      WORD_LIST *temp_list;

      /* Partially and wholly quoted strings which expand to the empty
	 string are retained as an empty arguments.  Unquoted strings
	 which expand to the empty string are discarded.  The single
	 exception is the case of expanding "$@" when there are no
	 positional parameters.  In that case, we discard the expansion. */

      /* Because of how the code that handles "" and '' in partially
	 quoted strings works, we need to make ISTRING into a QUOTED_NULL
	 if we saw quoting characters, but the expansion was empty.
	 "" and '' are tossed away before we get to this point when
	 processing partially quoted strings.  This makes "" and $xxx""
	 equivalent when xxx is unset. */
      if (!*istring && quoted_state == PARTIALLY_QUOTED)
	{
	  if (istring_size < 2)
	    istring = xrealloc (istring, istring_size += 2);
	  istring[0] = CTLNUL;
	  istring[1] = '\0';
	}

      /* If we expand to nothing and there were no single or double quotes
	 in the word, we throw it away.  Otherwise, we return a NULL word.
	 The single exception is for $@ surrounded by double quotes when
	 there are no positional parameters.  In that case, we also throw
	 the word away. */
      if (!*istring)
	{
	  if (quoted_state == UNQUOTED ||
	      (quoted_dollar_at && quoted_state == WHOLLY_QUOTED))
	    temp_list = (WORD_LIST *)NULL;
	  else
	    {
	      temp_list = make_word_list
		(make_word (istring), (WORD_LIST *)NULL);
	      temp_list->word->quoted = quoted;
	    }
	}
      else if (word->assignment)
	{
	  temp_list = make_word_list (make_word (istring), (WORD_LIST *)NULL);
	  temp_list->word->quoted = quoted;
	  temp_list->word->assignment = assignment (temp_list->word->word);
	}
      else
	{
	  char *ifs_chars = (char *)NULL;

	  if (quoted_dollar_at)
	    {
	      SHELL_VAR *ifs = find_variable ("IFS");	  
	      if (ifs)
		ifs_chars = value_cell (ifs);
	      else
		ifs_chars = " \t\n";
	    }

	  /* According to Posix.2, "$@" expands to a single word if
	     IFS="" and the positional parameters are not empty. */
	  if (quoted_dollar_at && ifs_chars && *ifs_chars)
	    {
	      temp_list = list_string (istring, " ", 1);
#if 0
	      /* This turns quoted null strings back into CTLNULs */
	      dequote_list (temp_list);
	      quote_list (temp_list);
#endif
	    }
	  else
	    {
	      WORD_DESC *tword;
	      tword = make_word (istring);
	      temp_list = make_word_list (tword, (WORD_LIST *)NULL);
	      tword->quoted = quoted || (quoted_state == WHOLLY_QUOTED);
	      tword->assignment = word->assignment;
	    }
	}

      free (istring);
      result = (WORD_LIST *)
	list_append (REVERSE_LIST (result, WORD_LIST *), temp_list);
    }
  else
    result = (WORD_LIST *)NULL;

  return (result);
}

/* **************************************************************** */
/*								    */
/*		   Functions for Quote Removal			    */
/*								    */
/* **************************************************************** */

/* Perform quote removal on STRING.  If QUOTED > 0, assume we are obeying the
   backslash quoting rules for within double quotes. */
char *
string_quote_removal (string, quoted)
     char *string;
     int quoted;
{
  char *r, *result_string, *temp, *temp1;
  int sindex, tindex, c, dquote;

  /* The result can be no longer than the original string. */
  r = result_string = xmalloc (strlen (string) + 1);

  for (sindex = dquote = 0; c = string[sindex];)
    {
      switch (c)
	{
	case '\\':
	  c = string[++sindex];
	  if ((quoted || dquote) && !member (c, slashify_in_quotes))
	    *r++ = '\\';

	default:
	  *r++ = c;
	  sindex++;
	  break;

	case '\'':
	  if (quoted || dquote)
	    {
	      *r++ = c;
	      sindex++;
	    }
	  else
	    {
	      tindex = ++sindex;
	      temp = string_extract_single_quoted (string, &tindex);
	      sindex = tindex;

	      if (temp)
	        {
		  strcpy (r, temp);
		  r += strlen (r);
		  free (temp);
		}
	    }
	  break;

	case '"':
	  dquote = 1 - dquote;
	  sindex++;
	  break;
	}
    }
    *r = '\0';
    return (result_string);
}

/* Perform quote removal on word WORD.  This allocates and returns a new
   WORD_DESC *. */
WORD_DESC *
word_quote_removal (word, quoted)
     WORD_DESC *word;
     int quoted;
{
  WORD_DESC *w;
  char *t;

  t = string_quote_removal (word->word, quoted);
  w = make_word (t);
  return (w);
}

/* Perform quote removal on all words in LIST.  If QUOTED is non-zero,
   the members of the list are treated as if they are surrounded by
   double quotes.  Return a new list, or NULL if LIST is NULL. */
WORD_LIST *
word_list_quote_removal (list, quoted)
     WORD_LIST *list;
     int quoted;
{
  WORD_LIST *result = (WORD_LIST *)NULL, *t, *tresult;

  t = list;
  while (t)
    {
      tresult = (WORD_LIST *)xmalloc (sizeof (WORD_LIST));
      tresult->word = word_quote_removal (t->word, quoted);
      tresult->next = (WORD_LIST *)NULL;
      result = (WORD_LIST *) list_append (result, tresult);
      t = t->next;
    }
  return (result);
}

/* Return 1 if CHARACTER appears in an unquoted portion of
   STRING.  Return 0 otherwise. */
static int
unquoted_member (character, string)
     int character;
     char *string;
{
  int sindex, tindex, c;
  char *temp;

  sindex = 0;

  while (c = string[sindex])
    {
      if (c == character)
	return (1);

      switch (c)
	{
	  case '\\':
	    sindex++;
	    if (string[sindex])
	      sindex++;
	    break;

	  case '"':
	  case '\'':

	    tindex = ++sindex;
	    if (c == '"')
	      temp = string_extract_double_quoted (string, &tindex);
	    else
	      temp = string_extract_single_quoted (string, &tindex);
	    sindex = tindex;

	    FREE (temp);
	    break;

	  default:
	    sindex++;
	    break;
	}
    }
  return (0);
}

/* Return 1 if SUBSTR appears in an unquoted portion of STRING. */
static int
unquoted_substring (substr, string)
     char *substr, *string;
{
  int sindex, tindex, c, sublen;
  char *temp;

  if (!substr || !*substr)
    return (0);

  sublen = strlen (substr);
  sindex = 0;

  while (c = string[sindex])
    {
      if (STREQN (string + sindex, substr, sublen))
	return (1);

      switch (c)
	{
	  case '\\':
	    sindex++;

	    if (string[sindex])
	      sindex++;
	    break;

	  case '"':
	  case '\'':

	    tindex = ++sindex;

	    if (c == '"')
	      temp = string_extract_double_quoted (string, &tindex);
	    else
	      temp = string_extract_single_quoted (string, &tindex);
	    sindex = tindex;

	    FREE (temp);

	    break;

	  default:
	    sindex++;
	    break;
	}
    }
  return (0);
}

/*******************************************
 *					   *
 *    Functions to perform word splitting  *
 *					   *
 *******************************************/

/* This splits a single word into a WORD LIST on $IFS, but only if the word
   is not quoted.  list_string () performs quote removal for us, even if we
   don't do any splitting. */
WORD_LIST *
word_split (w)
     WORD_DESC *w;
{
  WORD_LIST *result;

  if (w)
    {
      SHELL_VAR *ifs = find_variable ("IFS");
      char *ifs_chars;

      /* If IFS is unset, it defaults to " \t\n". */
      if (ifs)
	ifs_chars = value_cell (ifs);
      else
	ifs_chars = " \t\n";

      if (w->quoted || !ifs_chars)
	ifs_chars = "";

#ifdef NOT_YET_MAYBE_LATER
      if (!*ifs)
	{
	  /* No splitting done if word quoted or ifs set to "". */
	  WORD_DESC *wtemp;
	  wtemp = make_word (w->word);
	  wtemp->quoted = w->quoted;
	  result = make_word_list (wtemp);
	}
      else
#endif
      result = list_string (w->word, ifs_chars, w->quoted);
    }
  else
    result = (WORD_LIST *)NULL;
  return (result);
}

/* Perform word splitting on LIST and return the RESULT.  It is possible
   to return (WORD_LIST *)NULL. */
static WORD_LIST *
word_list_split (list)
     WORD_LIST *list;
{
  WORD_LIST *result = (WORD_LIST *)NULL, *t, *tresult;

  t = list;
  while (t)
    {
      tresult = word_split (t->word);
      result = (WORD_LIST *) list_append (result, tresult);
      t = t->next;
    }
  return (result);
}

/**************************************************
 * 						  *
 *	Functions to expand an entire WORD_LIST	  *
 *						  *
 **************************************************/

static WORD_LIST *varlist = (WORD_LIST *)NULL;

/* Separate out any initial variable assignments from TLIST.  If set -k has
   been executed, remove all assignment statements from TLIST.  Initial
   variable assignments and other environment assignments are placed
   on VARLIST. */
static WORD_LIST *
separate_out_assignments (tlist)
     WORD_LIST *tlist;
{
  register WORD_LIST *vp, *lp;

  if (!tlist)
    return ((WORD_LIST *)NULL);

  varlist = (WORD_LIST *)NULL;
  vp = lp = tlist;

  /* Separate out variable assignments at the start of the command.
     Loop invariant: vp->next == lp
     Loop postcondition:
     	lp = list of words left after assignment statements skipped
     	tlist = original list of words
  */
  while (lp && lp->word->assignment)
    {
      vp = lp;
      lp = lp->next;
    }

  /* If lp != tlist, we have some initial assignment statements. */
  /* We make VARLIST point to the list of assignment words and
     TLIST point to the remaining words.  */
  if (lp != tlist)
    {
      varlist = tlist;
      /* ASSERT(vp->next == lp); */
      vp->next = (WORD_LIST *)NULL;	/* terminate variable list */
      tlist = lp;			/* remainder of word list */
    }

  /* vp == end of variable list */
  /* tlist == remainder of original word list without variable assignments */
  if (!tlist)
    /* All the words in tlist were assignment statements */
    return ((WORD_LIST *)NULL);

  /* ASSERT(tlist != NULL); */
  /* ASSERT(tlist->word->assignment == 0); */

  /* If the -k option is in effect, we need to go through the remaining
     words, separate out the assignment words, and place them on VARLIST. */
  if (place_keywords_in_env)
    {
      WORD_LIST *tp;	/* tp == running pointer into tlist */

      tp = tlist;
      lp = tlist->next;

      /* Loop Invariant: tp->next == lp */
      /* Loop postcondition: tlist == word list without assignment statements */
      while (lp)
	{
	  if (lp->word->assignment)
	    {
	      /* Found an assignment statement, add this word to end of
		 varlist (vp). */
	      if (!varlist)
		varlist = vp = lp;
	      else
		{
		  vp->next = lp;
		  vp = lp;
		}

	      /* Remove the word pointed to by LP from TLIST. */
	      tp->next = lp->next;
	      /* ASSERT(vp == lp); */
	      lp->next = (WORD_LIST *)NULL;
	      lp = tp->next;
	    }
	  else
	    {
	      tp = lp;
	      lp = lp->next;
	    }
	}
    }
  return (tlist);
}

/* Take the list of words in LIST and do the various substitutions.  Return
   a new list of words which is the expanded list, and without things like
   variable assignments. */

WORD_LIST *
expand_words (list)
     WORD_LIST *list;
{
  return (expand_words_internal (list, 1));
}

/* Same as expand_words (), but doesn't hack variable or environment
   variables. */
WORD_LIST *
expand_words_no_vars (list)
     WORD_LIST *list;
{
  return (expand_words_internal (list, 0));
}

/* Non-zero means to allow unmatched globbed filenames to expand to
   a null file. */
static int allow_null_glob_expansion = 0;

/* The workhorse for expand_words () and expand_words_no_var ().
   First arg is LIST, a WORD_LIST of words.
   Second arg DO_VARS is non-zero if you want to do environment and
   variable assignments, else zero.

   This does all of the substitutions: brace expansion, tilde expansion,
   parameter expansion, command substitution, arithmetic expansion,
   process substitution, word splitting, and pathname expansion.
   Words with the `quoted' or `assignment' bits set, or for which no
   expansion is done, do not undergo word splitting.  Words with the
   `assignment' but set do not undergo pathname expansion. */
static WORD_LIST *
expand_words_internal (list, do_vars)
     WORD_LIST *list;
     int do_vars;
{
  register WORD_LIST *tlist, *new_list = (WORD_LIST *)NULL;
  WORD_LIST *orig_list;

  if (!list)
    return ((WORD_LIST *)NULL);

  tlist = copy_word_list (list);

  if (do_vars)
    {
      tlist = separate_out_assignments (tlist);
      if (!tlist)
	{
	  if (varlist)
	    {
	      /* All the words were variable assignments, so they are placed
		 into the shell's environment. */
	      register WORD_LIST *lp;
	      for (lp = varlist; lp; lp = lp->next)
		do_assignment (lp->word->word);
	      dispose_words (varlist);
	      varlist = (WORD_LIST *)NULL;
	    }
	  return ((WORD_LIST *)NULL);
	}
    }

  /* Begin expanding the words that remain.  The expansions take place on
     things that aren't really variable assignments. */

#if defined (BRACE_EXPANSION)
  /* Do brace expansion on this word if there are any brace characters
     in the string. */
  if (!no_brace_expansion)
    {
      register char **expansions;
      WORD_LIST *braces = (WORD_LIST *)NULL, *disposables = (WORD_LIST *)NULL;
      int eindex;

      while (tlist)
	{
	  WORD_LIST *next;

	  next = tlist->next;

	  /* Only do brace expansion if the word has a brace character.  If
	     not, just add the word list element to BRACES and continue.  In
	     the common case, at least when running shell scripts, this will
	     degenerate to a bunch of calls to `strchr', and then what is
	     basically a reversal of TLIST into BRACES, which is corrected
	     by a call to reverse_list () on BRACES when the end of TLIST
	     is reached. */
	  if (strchr (tlist->word->word, '{'))
	    {
	      expansions = brace_expand (tlist->word->word);

	      for (eindex = 0; expansions[eindex]; eindex++)
		{
		  braces = make_word_list (make_word (expansions[eindex]),
		  			   braces);
		  free (expansions[eindex]);
		}
	      free (expansions);

	      /* Add TLIST to the list of words to be freed after brace
		 expansion has been performed. */
	      tlist->next = disposables;
	      disposables = tlist;
	    }
	  else
	    {
	      tlist->next = braces;
	      braces = tlist;
	    }

	  tlist = next;
	}

      dispose_words (disposables);
      tlist = REVERSE_LIST (braces, WORD_LIST *);
    }
#endif /* BRACE_EXPANSION */

  orig_list = tlist;

  /* We do tilde expansion all the time.  This is what 1003.2 says. */
  while (tlist)
    {
      register char *current_word;
      WORD_LIST *expanded, *t, *reversed, *next;
      int expanded_something = 0;

      current_word = tlist->word->word;

      next = tlist->next;

      /* Posix.2 section 3.6.1 says that tildes following `=' in words
	 which are not assignment statements are not expanded.  We do
	 this only if POSIXLY_CORRECT is enabled. */
      if (current_word[0] == '~' ||
	  (!posixly_correct && strchr (current_word, '~') &&
	   unquoted_substring ("=~", current_word)))
	{
	  char *tt;

	  tt = tlist->word->word;
	  tlist->word->word = tilde_expand (tt);
	  free (tt);
	}

      expanded = expand_word_internal
	(tlist->word, 0, (int *)NULL, &expanded_something);

      if (expanded == &expand_word_error || expanded == &expand_word_fatal)
	{
	  /* By convention, each time this error is returned,
	     tlist->word->word has already been freed. */
	  tlist->word->word = (char *)NULL;
	  
	  /* Dispose our copy of the original list. */
	  dispose_words (orig_list);
	  /* Dispose the  new list we're building. */
	  dispose_words (new_list);

	  if (expanded == &expand_word_error)
	    longjmp (top_level, DISCARD);
	  else
	    longjmp (top_level, FORCE_EOF);
	}

      /* Don't split assignment words, even when they do not precede a
	 command name. */
      if (expanded_something && tlist->word->assignment == 0)
	{
	  t = word_list_split (expanded);
	  dispose_words (expanded);
	}
      else
	{
	  /* If no parameter expansion, command substitution, process
	     substitution, or arithmetic substitution took place, then
	     do not do word splitting.  We still have to remove quoted
	     null characters from the result. */
	  word_list_remove_quoted_nulls (expanded);
	  t = expanded;
	}

      /* In the most common cases, t will be a list containing only one
	 element, so the call to reverse_list would be wasted. */
      reversed = REVERSE_LIST (t, WORD_LIST *);
      new_list = (WORD_LIST *)list_append (reversed, new_list);

      tlist = next;
    }

  new_list = REVERSE_LIST (new_list, WORD_LIST *);

  dispose_words (orig_list);

#if defined (USE_POSIX_GLOB_LIBRARY)
#  define GLOB_FAILED(glist)	!(glist)
#else /* !USE_POSIX_GLOB_LIBRARY */
#  define GLOB_FAILED(glist)	(glist) == (char **)&glob_error_return
#endif /* !USE_POSIX_GLOB_LIBRARY */

  /* Okay, we're almost done.  Now let's just do some filename
     globbing. */
  if (new_list)
    {
      char **temp_list = (char **)NULL;
      register int list_index;
      WORD_LIST *glob_list, *disposables;

      orig_list = disposables = (WORD_LIST *)NULL;
      tlist = new_list;

      /* orig_list == output list, despite the name. */
      if (!disallow_filename_globbing)
	{
	  while (tlist)
	    {
	      /* For each word, either globbing is attempted or the word is
		 added to orig_list.  If globbing succeeds, the results are
		 added to orig_list and the word (tlist) is added to the list
		 of disposable words.  If globbing fails and failed glob
		 expansions are left unchanged (the shell default), the
		 original word is added to orig_list.  If globbing fails and
		 failed glob expansions are removed, the original word is
		 added to the list of disposable words.  orig_list ends up
		 in reverse order and requires a call to reverse_list to
		 be set right.  After all words are examined, the disposable
		 words are freed. */
	      WORD_LIST *next;

	      next = tlist->next;

	      /* If the word isn't quoted and there is an unquoted pattern
		 matching character in the word, then glob it. */
	      if (!tlist->word->quoted && !tlist->word->assignment &&
		  unquoted_glob_pattern_p (tlist->word->word))
		{
		  temp_list = shell_glob_filename (tlist->word->word);

		  /* Handle error cases.
		     I don't think we should report errors like "No such file
		     or directory".  However, I would like to report errors
		     like "Read failed". */

		  if (GLOB_FAILED (temp_list))
		    {
		      temp_list = (char **) xmalloc (sizeof (char *));
		      temp_list[0] = (char *)NULL;
		    }

		  /* Dequote the current word in case we have to use it. */
		  if (!temp_list[0])
		    {
		      register char *t = dequote_string (tlist->word->word);
		      free (tlist->word->word);
		      tlist->word->word = t;
		    }

		  /* Make the array into a word list. */
		  glob_list = (WORD_LIST *)NULL;
		  for (list_index = 0; temp_list[list_index]; list_index++)
		    glob_list = make_word_list
		      (make_word (temp_list[list_index]), glob_list);

		  if (glob_list)
		    {
		      orig_list = (WORD_LIST *)list_append
			(glob_list, orig_list);
		      tlist->next = disposables;
		      disposables = tlist;
		    }
		  else
		    if (!allow_null_glob_expansion)
		      {
			/* Failed glob expressions are left unchanged. */
			tlist->next = orig_list;
			orig_list = tlist;
		      }
		    else
		      {
			/* Failed glob expressions are removed. */
			tlist->next = disposables;
			disposables = tlist;
		      }
		}
	      else
		{
		  /* Dequote the string. */
		  register char *t = dequote_string (tlist->word->word);
		  free (tlist->word->word);
		  tlist->word->word = t;
		  tlist->next = orig_list;
		  orig_list = tlist;
		}

	      free_array (temp_list);
	      temp_list = (char **)NULL;

	      tlist = next;
	    }

	  if (disposables)
	    dispose_words (disposables);

	  new_list = REVERSE_LIST (orig_list, WORD_LIST *);
	}
      else
	{
	  /* Dequote the words, because we're not performing globbing. */
	  register WORD_LIST *wl = new_list;
	  register char *wp;
	  while (wl)
	    {
	      wp = dequote_string (wl->word->word);
	      free (wl->word->word);
	      wl->word->word = wp;
	      wl = wl->next;
	    }
	}
    }

  if (do_vars)
    {
      register WORD_LIST *lp;
      Function *assign_func;

      /* If the remainder of the words expand to nothing, Posix.2 requires
	 that the variable and environment assignments affect the shell's
	 environment. */
      assign_func = new_list ? assign_in_env : do_assignment;

      for (lp = varlist; lp; lp = lp->next)
	(*assign_func) (lp->word->word);

      dispose_words (varlist);
      varlist = (WORD_LIST *)NULL;
    }

  return (new_list);
}

/* Return nonzero if S has any unquoted special globbing chars in it.  */
static int
unquoted_glob_pattern_p (string)
     register char *string;
{
  register int c;
  int open = 0;

  while (c = *string++)
    {
      switch (c)
	{
	case '?':
	case '*':
	  return (1);

	case '[':
	  open++;
	  continue;

	case ']':
	  if (open)
	    return (1);
	  continue;

	case CTLESC:
	case '\\':
	  if (*string++ == '\0')
	    return (0);
	}
    }
  return (0);
}

/* PATHNAME can contain characters prefixed by CTLESC; this indicates
   that the character is to be quoted.  We quote it here in the style
   that the glob library recognizes.  If CONVERT_QUOTED_NULLS is non-zero,
   we change quoted null strings (pathname[0] == CTLNUL) into empty
   strings (pathname[0] == 0).  If this is called after quote removal
   is performed, CONVERT_QUOTED_NULLS should be 0; if called when quote
   removal has not been done (for example, before attempting to match a
   pattern while executing a case statement), CONVERT_QUOTED_NULLS should
   be 1. */
char *
quote_string_for_globbing (pathname, convert_quoted_nulls)
     char *pathname;
     int convert_quoted_nulls;
{
  char *temp;
  register int i;

  temp = savestring (pathname);

  if (convert_quoted_nulls && QUOTED_NULL (pathname))
    {
      temp[0] = '\0';
      return temp;
    }

  for (i = 0; temp[i]; i++)
    {
      if (temp[i] == CTLESC)
	temp[i++] = '\\';
    }

  return (temp);
}

/* Call the glob library to do globbing on PATHNAME. */
char **
shell_glob_filename (pathname)
     char *pathname;
{
#if defined (USE_POSIX_GLOB_LIBRARY)
  extern int glob_dot_filenames;
  register int i;
  char *temp, **return_value;
  glob_t filenames;
  int glob_flags;

  temp = quote_string_for_globbing (pathname, 0);

  filenames.gl_offs = 0;

  glob_flags = glob_dot_filenames ? GLOB_PERIOD : 0;
  glob_flags |= (GLOB_ERR | GLOB_DOOFFS);

  i = glob (temp, glob_flags, (Function *)NULL, &filenames);

  free (temp);

  if (i == GLOB_NOSPACE || i == GLOB_ABEND)
    return ((char **)NULL);

  if (i == GLOB_NOMATCH)
    filenames.gl_pathv[0] = (char *)NULL;

  return (filenames.gl_pathv);

#else /* !USE_POSIX_GLOB_LIBRARY */

  char *temp, **results;

  noglob_dot_filenames = !glob_dot_filenames;

  temp = quote_string_for_globbing (pathname, 0);

  results = glob_filename (temp);
  free (temp);

  if (results && !(GLOB_FAILED(results)))
    sort_char_array (results);

  return (results);
#endif /* !USE_POSIX_GLOB_LIBRARY */
}

/*************************************************
 *						 *
 *	Functions to manage special variables	 *
 *						 *
 *************************************************/

/* An alist of name.function for each special variable.  Most of the
   functions don't do much, and in fact, this would be faster with a
   switch statement, but by the end of this file, I am sick of switch
   statements. */

/* The functions that get called. */
void
  sv_path (), sv_mail (), sv_uids (), sv_ignoreeof (),
  sv_glob_dot_filenames (), sv_nolinks (),
  sv_noclobber (), sv_allow_null_glob_expansion (), sv_strict_posix ();

#if defined (READLINE)
void sv_terminal (), sv_hostname_completion_file ();
#endif

#if defined (HISTORY)
void sv_histsize (), sv_histfilesize (),
     sv_history_control (), sv_command_oriented_history ();
#  if defined (BANG_HISTORY)
void sv_histchars ();
#  endif
#endif /* HISTORY */

#if defined (GETOPTS_BUILTIN)
void sv_optind (), sv_opterr ();
#endif /* GETOPTS_BUILTIN */

#if defined (JOB_CONTROL)
void sv_notify ();
#endif

#define SET_INT_VAR(name, intvar)  intvar = find_variable (name) != 0

struct name_and_function {
  char *name;
  VFunction *function;
} special_vars[] = {
  { "PATH", sv_path },
  { "MAIL", sv_mail },
  { "MAILPATH", sv_mail },
  { "MAILCHECK", sv_mail },

  { "POSIXLY_CORRECT", sv_strict_posix },
  { "POSIX_PEDANTIC", sv_strict_posix },
  /* Variables which only do something special when READLINE is defined. */
#if defined (READLINE)
  { "TERM", sv_terminal },
  { "TERMCAP", sv_terminal },
  { "TERMINFO", sv_terminal },
  { "hostname_completion_file", sv_hostname_completion_file },
  { "HOSTFILE", sv_hostname_completion_file },
#endif /* READLINE */

  /* Variables which only do something special when HISTORY is defined. */
#if defined (HISTORY)
  { "HISTSIZE", sv_histsize },
  { "HISTFILESIZE", sv_histfilesize },
  { "command_oriented_history", sv_command_oriented_history },
#  if defined (BANG_HISTORY)
  { "histchars", sv_histchars },
#  endif
  { "history_control", sv_history_control },
  { "HISTCONTROL", sv_history_control },
#endif /* HISTORY */

  { "EUID", sv_uids},
  { "UID", sv_uids},
  { "IGNOREEOF", sv_ignoreeof },
  { "ignoreeof", sv_ignoreeof },

#if defined (GETOPTS_BUILTIN)
  { "OPTIND", sv_optind },
  { "OPTERR", sv_opterr },
#endif /* GETOPTS_BUILTIN */

#if defined (JOB_CONTROL)
  { "notify", sv_notify },
#endif  /* JOB_CONTROL */

  { "glob_dot_filenames", sv_glob_dot_filenames },
  { "allow_null_glob_expansion", sv_allow_null_glob_expansion },
  { "noclobber", sv_noclobber },
  { "nolinks", sv_nolinks },
  { (char *)0x00, (VFunction *)0x00 }
};

/* The variable in NAME has just had its state changed.  Check to see if it
   is one of the special ones where something special happens. */
void
stupidly_hack_special_variables (name)
     char *name;
{
  int i = 0;

  while (special_vars[i].name)
    {
      if (STREQ (special_vars[i].name, name))
	{
	  (*(special_vars[i].function)) (name);
	  return;
	}
      i++;
    }
}

/* Set/unset noclobber. */
void
sv_noclobber (name)
     char *name;
{
  SET_INT_VAR (name, noclobber);
}

/* What to do just after the PATH variable has changed. */
void
sv_path (name)
     char *name;
{
  /* hash -r */
  WORD_LIST *args;

  args = make_word_list (make_word ("-r"), NULL);
  hash_builtin (args);
  dispose_words (args);
}

/* What to do just after one of the MAILxxxx variables has changed.  NAME
   is the name of the variable.  This is called with NAME set to one of
   MAIL, MAILCHECK, or MAILPATH.  */
void
sv_mail (name)
     char *name;
{
  /* If the time interval for checking the files has changed, then
     reset the mail timer.  Otherwise, one of the pathname vars
     to the users mailbox has changed, so rebuild the array of
     filenames. */
  if (name[4] == 'C')  /* if (strcmp (name, "MAILCHECK") == 0) */
    reset_mail_timer ();
  else
    {
      free_mail_files ();
      remember_mail_dates ();
    }
}

#if defined (READLINE)
/* What to do just after one of the TERMxxx variables has changed.
   If we are an interactive shell, then try to reset the terminal
   information in readline. */
void
sv_terminal (name)
     char *name;
{
  if (interactive_shell && !no_line_editing)
    rl_reset_terminal (get_string_value ("TERM"));
}

void
sv_hostname_completion_file (name)
     char *name;
{
  hostname_list_initialized = 0;
}
#endif /* READLINE */

#if defined (HISTORY)
/* What to do after the HISTSIZE variable changes.
   If there is a value for this variable (and it is numeric), then stifle
   the history.  Otherwise, if there is NO value for this variable,
   unstifle the history. */
void
sv_histsize (name)
     char *name;
{
  char *temp = get_string_value (name);

  if (temp && *temp)
    {
      int num;
      if (sscanf (temp, "%d", &num) == 1)
	{
	  stifle_history (num);
	  if (history_lines_this_session > where_history ())
	    history_lines_this_session = where_history ();
	}
    }
  else
    unstifle_history ();
}

/* What to do if the HISTFILESIZE variable changes. */
void
sv_histfilesize (name)
     char *name;
{
  char *temp = get_string_value (name);

  if (temp && *temp)
    {
      int num;
      if (sscanf (temp, "%d", &num) == 1)
	{
	  history_truncate_file (get_string_value ("HISTFILE"), num);
	  if (num <= history_lines_in_file)
	    history_lines_in_file = num;
	}
    }
}

/* What to do after the HISTORY_CONTROL variable changes. */
void
sv_history_control (name)
     char *name;
{
  char *temp = get_string_value (name);

  history_control = 0;

  if (temp && *temp)
    {
      if (strcmp (temp, "ignorespace") == 0)
	history_control = 1;
      else if (strcmp (temp, "ignoredups") == 0)
	history_control = 2;
      else if (strcmp (temp, "ignoreboth") == 0)
	history_control = 3;
    }
}

/* What to do after the COMMAND_ORIENTED_HISTORY variable changes. */
void
sv_command_oriented_history (name)
     char *name;
{
  SET_INT_VAR (name, command_oriented_history);
}

#  if defined (BANG_HISTORY)
/* Setting/unsetting of the history expansion character. */

void
sv_histchars (name)
     char *name;
{
  char *temp = get_string_value (name);

  if (temp)
    {
      history_expansion_char = *temp;
      if (temp[0] && temp[1])
	{
	  history_subst_char = temp[1];
	  if (temp[2])
	      history_comment_char = temp[2];
	}
    }
  else
    {
      history_expansion_char = '!';
      history_subst_char = '^';
      history_comment_char = '#';
    }
}
#  endif /* BANG_HISTORY */
#endif /* HISTORY */

void
sv_allow_null_glob_expansion (name)
     char *name;
{
  SET_INT_VAR (name, allow_null_glob_expansion);
}

/* If the variable exists, then the value of it can be the number
   of times we actually ignore the EOF.  The default is small,
   (smaller than csh, anyway). */
void
sv_ignoreeof (name)
     char *name;
{
  SHELL_VAR *tmp_var;
  char *temp;
  int new_limit;

  eof_encountered = 0;

  tmp_var = find_variable (name);
  ignoreeof = tmp_var != 0;
  temp = tmp_var ? value_cell (tmp_var) : (char *)NULL;
  if (temp)
    {
      if (sscanf (temp, "%d", &new_limit) == 1)
	eof_encountered_limit = new_limit;
      else
	eof_encountered_limit = 10; /* csh uses 26. */
    }
}

/* Control whether * matches .files in globbing.  Yechh. */
int glob_dot_filenames = 0;

void
sv_glob_dot_filenames (name)
     char *name;
{
  SET_INT_VAR (name, glob_dot_filenames);
}

#if defined (JOB_CONTROL)
/* Job notification feature desired? */
void
sv_notify (name)
     char *name;
{
  SET_INT_VAR (name, asynchronous_notification);
}
#endif  /* JOB_CONTROL */

/* If the variable `nolinks' exists, it specifies that symbolic links are
   not to be followed in `cd' commands. */
void
sv_nolinks (name)
     char *name;
{
  SET_INT_VAR (name, no_symbolic_links);
}

/* Don't let users hack the user id variables. */
void
sv_uids (name)
     char *name;
{
  char *buff;
  register SHELL_VAR *v;

  buff = itos (current_user.uid);
  v = find_variable ("UID");
  if (v)
    v->attributes &= ~att_readonly;

  v = bind_variable ("UID", buff);
  v->attributes |= (att_readonly | att_integer);
  free (buff);

  buff = itos (current_user.euid);
  v = find_variable ("EUID");
  if (v)
    v->attributes &= ~att_readonly;

  v = bind_variable ("EUID", buff);
  v->attributes |= (att_readonly | att_integer);
  free (buff);
}

#if defined (GETOPTS_BUILTIN)
void
sv_optind (name)
     char *name;
{
  char *tt = get_string_value ("OPTIND");
  int s = 0;

  if (tt && *tt)
    {
      s = atoi (tt);

      /* According to POSIX, setting OPTIND=1 resets the internal state
	 of getopt (). */
      if (s < 0 || s == 1)
	s = 0;
    }
  getopts_reset (s);
}

void
sv_opterr (name)
     char *name;
{
  char *tt = get_string_value ("OPTERR");
  int s = 1;

  if (tt && *tt)
    s = atoi (tt);
  sh_opterr = s;
}
#endif /* GETOPTS_BUILTIN */

void
sv_strict_posix (name)
     char *name;
{
  SET_INT_VAR (name, posixly_correct);
  if (posixly_correct)
    interactive_comments = 1;
#if defined (READLINE)
  posix_readline_initialize (posixly_correct);
#endif /* READLINE */
}
