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
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include "config.h"

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdio.h>
#include <ctype.h>

#include "shell.h"

#ifndef to_upper
#  define to_upper(c) (islower(c) ? toupper(c) : (c))
#  define to_lower(c) (isupper(c) ? tolower(c) : (c))
#endif

/* Convert STRING by expanding the escape sequences specified by the
   ANSI C standard.  If SAWC is non-null, recognize `\c' and use that
   as a string terminator.  If we see \c, set *SAWC to 1 before
   returning.  LEN is the length of STRING. */
char *
ansicstr (string, len, sawc, rlen)
     char *string;
     int len, *sawc, *rlen;
{
  int c;
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
	    case 'e': c = '\033'; break;	/* ESC -- non-ANSI */
	    case 'E': c = '\033'; break;	/* ESC -- non-ANSI */
	    case 'f': c = '\f'; break;
	    case 'n': c = '\n'; break;
	    case 'r': c = '\r'; break;
	    case 't': c = '\t'; break;
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      c -= '0';
	      if (*s >= '0' && *s <= '7')
	        c = c * 8 + (*s++ - '0');
	      if (*s >= '0' && *s <= '7')
	        c = c * 8 + (*s++ - '0');
	      break;
	    case '\\':
	    case '\'':
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
  int patlen, templen, tempsize, repl, i;
  char *temp, *r;

  patlen = strlen (pat);
  for (temp = (char *)NULL, i = templen = tempsize = 0, repl = 1; string[i]; )
    {
      if (repl && STREQN (string + i, pat, patlen))
        {
          RESIZE_MALLOCED_BUFFER (temp, templen, patlen, tempsize, (patlen * 2));

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
