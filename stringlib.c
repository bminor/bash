/* stringlib.c - Miscellaneous string functions. */

/* Copyright (C) 1996
   Free Software Foundation, Inc.

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
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#include "config.h"

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdio.h>
#include <ctype.h>

#include "shell.h"
#include "pathexp.h"

#if defined (EXTENDED_GLOB)
#  include <glob/fnmatch.h>
#endif

#ifndef to_upper
#  define to_upper(c) (islower(c) ? toupper(c) : (c))
#  define to_lower(c) (isupper(c) ? tolower(c) : (c))
#endif

#define ISOCTAL(c)	((c) >= '0' && (c) <= '7')
#define OCTVALUE(c)	((c) - '0')

#ifndef isxdigit
#  define isxdigit(c)	(isdigit((c)) || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#endif

#define HEXVALUE(c) \
  ((c) >= 'a' && (c) <= 'f' ? (c)-'a'+10 : (c) >= 'A' && (c) <= 'F' ? (c)-'A'+10 : (c)-'0')

/* Convert STRING by expanding the escape sequences specified by the
   ANSI C standard.  If SAWC is non-null, recognize `\c' and use that
   as a string terminator.  If we see \c, set *SAWC to 1 before
   returning.  LEN is the length of STRING.  FOR_ECHO is a flag that
   means, if non-zero, that we're translating a string for `echo -e',
   and therefore should not treat a single quote as a character that
   may be escaped with a backslash. */
char *
ansicstr (string, len, for_echo, sawc, rlen)
     char *string;
     int len, for_echo, *sawc, *rlen;
{
  int c, temp;
  char *ret, *r, *s;

  if (string == 0 || *string == '\0')
    return ((char *)NULL);

  ret = xmalloc (len + 1);
  for (r = ret, s = string; s && *s; )
    {
      c = *s++;
      if (c != '\\' || *s == '\0')
        *r++ = c;
      else
	{
	  switch (c = *s++)
	    {
#if defined (__STDC__)
	    case 'a': c = '\a'; break;
	    case 'v': c = '\v'; break;
#else
	    case 'a': c = '\007'; break;
	    case 'v': c = (int) 0x0B; break;
#endif
	    case 'b': c = '\b'; break;
	    case 'e': case 'E':		/* ESC -- non-ANSI */
	      c = '\033'; break;
	    case 'f': c = '\f'; break;
	    case 'n': c = '\n'; break;
	    case 'r': c = '\r'; break;
	    case 't': c = '\t'; break;
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      for (temp = 2, c -= '0'; ISOCTAL (*s) && temp--; s++)
	        c = (c * 8) + OCTVALUE (*s);
	      break;
	    case 'x':			/* Hex digit -- non-ANSI */
	      for (temp = 3, c = 0; isxdigit (*s) && temp--; s++)
	        c = (c * 16) + HEXVALUE (*s);
	      /* \x followed by non-hex digits is passed through unchanged */
	      if (temp == 3)
		{
		  *r++ = '\\';
		  c = 'x';
		}
	      break;
	    case '\\':
	      break;
	    case '\'':
	      if (for_echo)
		*r++ = '\\';
	      break;
	    case 'c':
	      if (sawc)
		{
		  *sawc = 1;
		  *r = '\0';
		  if (rlen)
		    *rlen = r - ret;
		  return ret;
		}
	    default:  *r++ = '\\'; break;
	    }
	  *r++ = c;
	}
    }
  *r = '\0';
  if (rlen)
    *rlen = r - ret;
  return ret;
}

/* **************************************************************** */
/*								    */
/*		Functions to manage arrays of strings		    */
/*								    */
/* **************************************************************** */

#ifdef INCLUDE_UNUSED
/* Find NAME in ARRAY.  Return the index of NAME, or -1 if not present.
   ARRAY should be NULL terminated. */
int
find_name_in_array (name, array)
     char *name, **array;
{
  int i;

  for (i = 0; array[i]; i++)
    if (STREQ (name, array[i]))
      return (i);

  return (-1);
}
#endif

/* Allocate an array of strings with room for N members. */
char **
alloc_array (n)
     int n;
{
  return ((char **)xmalloc ((n) * sizeof (char *)));
}

/* Return the length of ARRAY, a NULL terminated array of char *. */
int
array_len (array)
     char **array;
{
  register int i;

  for (i = 0; array[i]; i++);
  return (i);
}

/* Free the contents of ARRAY, a NULL terminated array of char *. */
void
free_array_members (array)
     char **array;
{
  register int i;

  if (array == 0)
    return;

  for (i = 0; array[i]; i++)
    free (array[i]);
}

void
free_array (array)
     char **array;
{
  if (array == 0)
    return;

  free_array_members (array);
  free (array);
}

/* Allocate and return a new copy of ARRAY and its contents. */
char **
copy_array (array)
     char **array;
{
  register int i;
  int len;
  char **new_array;

  len = array_len (array);

  new_array = (char **)xmalloc ((len + 1) * sizeof (char *));
  for (i = 0; array[i]; i++)
    new_array[i] = savestring (array[i]);
  new_array[i] = (char *)NULL;

  return (new_array);
}

/* Comparison routine for use with qsort() on arrays of strings.  Uses
   strcoll(3) if available, otherwise it uses strcmp(3). */
int
qsort_string_compare (s1, s2)
     register char **s1, **s2;
{
#if defined (HAVE_STRCOLL)
   return (strcoll (*s1, *s2));
#else /* !HAVE_STRCOLL */
  int result;

  if ((result = **s1 - **s2) == 0)
    result = strcmp (*s1, *s2);

  return (result);
#endif /* !HAVE_STRCOLL */
}

/* Sort ARRAY, a null terminated array of pointers to strings. */
void
sort_char_array (array)
     char **array;
{
  qsort (array, array_len (array), sizeof (char *),
	 (Function *)qsort_string_compare);
}

/* Cons up a new array of words.  The words are taken from LIST,
   which is a WORD_LIST *.  If COPY is true, everything is malloc'ed,
   so you should free everything in this array when you are done.
   The array is NULL terminated.  If IP is non-null, it gets the
   number of words in the returned array.  STARTING_INDEX says where
   to start filling in the returned array; it can be used to reserve
   space at the beginning of the array. */
char **
word_list_to_argv (list, copy, starting_index, ip)
     WORD_LIST *list;
     int copy, starting_index, *ip;
{
  int count;
  char **array;

  count = list_length (list);
  array = (char **)xmalloc ((1 + count + starting_index) * sizeof (char *));

  for (count = 0; count < starting_index; count++)
    array[count] = (char *)NULL;
  for (count = starting_index; list; count++, list = list->next)
    array[count] = copy ? savestring (list->word->word) : list->word->word;
  array[count] = (char *)NULL;

  if (ip)
    *ip = count;
  return (array);
}

/* Convert an array of strings into the form used internally by the shell.
   COPY means to copy the values in ARRAY into the returned list rather
   than allocate new storage.  STARTING_INDEX says where in ARRAY to begin. */
WORD_LIST *
argv_to_word_list (array, copy, starting_index)
     char **array;
     int copy, starting_index;
{
  WORD_LIST *list;
  WORD_DESC *w;
  int i, count;

  if (array == 0 || array[0] == 0)
    return (WORD_LIST *)NULL;

  for (count = 0; array[count]; count++)
    ;

  for (i = starting_index, list = (WORD_LIST *)NULL; i < count; i++)
    {
      w = make_bare_word (copy ? "" : array[i]);
      if (copy)
	{
	  free (w->word);
	  w->word = array[i];
	}
      list = make_word_list (w, list);
    }
  return (REVERSE_LIST(list, WORD_LIST *));
}

/* Find STRING in ALIST, a list of string key/int value pairs.  If FLAGS
   is 1, STRING is treated as a pattern and matched using fnmatch. */
int
find_string_in_alist (string, alist, flags)
     char *string;
     STRING_INT_ALIST *alist;
     int flags;
{
  register int i;
  int r;

  for (i = r = 0; alist[i].word; i++)
    {
#if defined (EXTENDED_GLOB)
      if (flags)
        r = fnmatch (alist[i].word, string, FNM_EXTMATCH) != FNM_NOMATCH;
      else
#endif
        r = STREQ (string, alist[i].word);

      if (r)
        return (alist[i].token);
    }
  return -1;
}

/* **************************************************************** */
/*								    */
/*		    String Management Functions			    */
/*								    */
/* **************************************************************** */

/* Replace occurrences of PAT with REP in STRING.  If GLOBAL is non-zero,
   replace all occurrences, otherwise replace only the first.
   This returns a new string; the caller should free it. */
char *
strsub (string, pat, rep, global)
     char *string, *pat, *rep;
     int global;
{
  int patlen, replen, templen, tempsize, repl, i;
  char *temp, *r;

  patlen = strlen (pat);
  replen = strlen (rep);
  for (temp = (char *)NULL, i = templen = tempsize = 0, repl = 1; string[i]; )
    {
      if (repl && STREQN (string + i, pat, patlen))
        {
          RESIZE_MALLOCED_BUFFER (temp, templen, replen, tempsize, (replen * 2));

	  for (r = rep; *r; )
	    temp[templen++] = *r++;

	  i += patlen;
	  repl = global != 0;
        }
      else
	{
	  RESIZE_MALLOCED_BUFFER (temp, templen, 1, tempsize, 16);
	  temp[templen++] = string[i++];
	}
    }
  temp[templen] = 0;
  return (temp);
}

/* Replace all instances of C in STRING with TEXT.  TEXT may be empty or
   NULL.  If DO_GLOB is non-zero, we quote the replacement text for
   globbing.  Backslash may be used to quote C. */
char *
strcreplace (string, c, text, do_glob)
     char *string;
     int c;
     char *text;
     int do_glob;
{
  char *ret, *p, *r, *t;
  int len, rlen, ind, tlen;

  len = STRLEN (text);
  rlen = len + strlen (string) + 2;
  ret = xmalloc (rlen);

  for (p = string, r = ret; p && *p; )
    {
      if (*p == c)
	{
	  if (len)
	    {
	      ind = r - ret;
	      if (do_glob && (glob_pattern_p (text) || strchr (text, '\\')))
		{
		  t = quote_globbing_chars (text);
		  tlen = strlen (t);
		  RESIZE_MALLOCED_BUFFER (ret, ind, tlen, rlen, rlen);
		  r = ret + ind;	/* in case reallocated */
		  strcpy (r, t);
		  r += tlen;
		  free (t);
		}
	      else
		{
		  RESIZE_MALLOCED_BUFFER (ret, ind, len, rlen, rlen);
		  r = ret + ind;	/* in case reallocated */
		  strcpy (r, text);
		  r += len;
		}
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

#ifdef INCLUDE_UNUSED
/* Remove all leading whitespace from STRING.  This includes
   newlines.  STRING should be terminated with a zero. */
void
strip_leading (string)
     char *string;
{
  char *start = string;

  while (*string && (whitespace (*string) || *string == '\n'))
    string++;

  if (string != start)
    {
      int len = strlen (string);
      FASTCOPY (string, start, len);
      start[len] = '\0';
    }
}
#endif

/* Remove all trailing whitespace from STRING.  This includes
   newlines.  If NEWLINES_ONLY is non-zero, only trailing newlines
   are removed.  STRING should be terminated with a zero. */
void
strip_trailing (string, len, newlines_only)
     char *string;
     int len;
     int newlines_only;
{
  while (len >= 0)
    {
      if ((newlines_only && string[len] == '\n') ||
          (!newlines_only && whitespace (string[len])))
        len--;
      else
        break;
    }
  string[len + 1] = '\0';
}

/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case insensitive.  This is a
   case-insensitive strstr(3). */
char *
strindex (s1, s2)
     char *s1, *s2;
{
  register int i, l, len, c;

  c = to_upper (s2[0]);
  for (i = 0, len = strlen (s1), l = strlen (s2); (len - i) >= l; i++)
    if ((to_upper (s1[i]) == c) && (strncasecmp (s1 + i, s2, l) == 0))
      return (s1 + i);
  return ((char *)NULL);
}

/* A wrapper for bcopy that can be prototyped in general.h */
void
xbcopy (s, d, n)
     char *s, *d;
     int n;
{
  FASTCOPY (s, d, n);
}
