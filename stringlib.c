/* stringlib.c - Miscellaneous string functions. */

/* Copyright (C) 1996-2009,2022-2024 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "config.h"

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdio.h>
#include "chartypes.h"

#include "shell.h"
#include "pathexp.h"

#include <glob/glob.h>

#if defined (EXTENDED_GLOB)
#  include <glob/strmatch.h>
#endif

/* **************************************************************** */
/*								    */
/*		Functions to manage arrays of strings		    */
/*								    */
/* **************************************************************** */

/* Find STRING in ALIST, a list of string key/int value pairs.  If FLAGS
   is 1, STRING is treated as a pattern and matched using strmatch. */
int
find_string_in_alist (char *string, STRING_INT_ALIST *alist, int flags)
{
  register int i;
  int r;

  for (i = r = 0; alist[i].word; i++)
    {
#if defined (EXTENDED_GLOB)
      if (flags)
	r = strmatch (alist[i].word, string, FNM_EXTMATCH) != FNM_NOMATCH;
      else
#endif
	r = STREQ (string, alist[i].word);

      if (r)
	return (alist[i].token);
    }
  return -1;
}

/* Find TOKEN in ALIST, a list of string/int value pairs.  Return the
   corresponding string.  Allocates memory for the returned
   string.  FLAGS == 0 means to allocate new memory, the existing
   behavior. If FLAGS&1, we just return the token and expect the caller
   to allocate new memory and save it, if necessary. */
char *
find_token_in_alist (int token, STRING_INT_ALIST *alist, int flags)
{
  register int i;

  for (i = 0; alist[i].word; i++)
    {
      if (alist[i].token == token)
        return (flags ? alist[i].word : savestring (alist[i].word));
    }
  return (NULL);
}

int
find_index_in_alist (char *string, STRING_INT_ALIST *alist, int flags)
{
  register int i;
  int r;

  for (i = r = 0; alist[i].word; i++)
    {
#if defined (EXTENDED_GLOB)
      if (flags)
	r = strmatch (alist[i].word, string, FNM_EXTMATCH) != FNM_NOMATCH;
      else
#endif
	r = STREQ (string, alist[i].word);

      if (r)
	return (i);
    }

  return -1;
}

/* **************************************************************** */
/*								    */
/*		    String Management Functions			    */
/*								    */
/* **************************************************************** */

/* Cons a new string from STRING starting at START and ending at END,
   not including END. */
char *
substring (const char *string, size_t start, size_t end)
{
  size_t len;
  char *result;

  len = end - start;
  result = (char *)xmalloc (len + 1);
  memcpy (result, string + start, len);
  result[len] = '\0';
  return (result);
}

/* Replace occurrences of PAT with REP in STRING.  If GLOBAL is non-zero,
   replace all occurrences, otherwise replace only the first.
   This returns a new string; the caller should free it. */
char *
strsub (const char *string, const char *pat, const char *rep, int global)
{
  size_t patlen, replen, templen, tempsize, i;
  int repl;
  char *temp, *r;;

  patlen = strlen (pat);
  replen = strlen (rep);
  for (temp = NULL, i = templen = tempsize = 0, repl = 1; string[i]; )
    {
      if (repl && STREQN (string + i, pat, patlen))
	{
	  if (replen)
	    RESIZE_MALLOCED_BUFFER (temp, templen, replen, tempsize, (replen * 2));

#if 0
	  for (r = (char *)rep; *r; )	/* can rep == "" */
	    temp[templen++] = *r++;
#else
	  memcpy (temp + templen, rep, replen);
	  templen += replen;
#endif

	  i += patlen ? patlen : 1;	/* avoid infinite recursion */
	  repl = global != 0;
	}
      else
	{
	  RESIZE_MALLOCED_BUFFER (temp, templen, 1, tempsize, 16);
	  temp[templen++] = string[i++];
	}
    }
  if (temp)
    temp[templen] = 0;
  else
    temp = savestring (string);
  return (temp);
}

/* Replace all instances of C in STRING with TEXT.  TEXT may be empty or
   NULL.  If (FLAGS & 1) is non-zero, we quote the replacement text for
   globbing.  Backslash may be used to quote C. If (FLAGS & 2) we allow
   backslash to escape backslash as well. */
char *
strcreplace (const char *string, int c, const char *text, int flags)
{
  char *ret, *r, *t;
  const char *p;
  size_t len, rlen, ind, tlen;
  int do_glob, escape_backslash;

  do_glob = flags & 1;
  escape_backslash = flags & 2;

  len = STRLEN (text);
  rlen = len + strlen (string) + 2;
  ret = (char *)xmalloc (rlen);

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

      if (*p == '\\' && p[1] == c)
	p++;
      else if (escape_backslash && *p == '\\' && p[1] == '\\')
	p++;

      ind = r - ret;
      RESIZE_MALLOCED_BUFFER (ret, ind, 2, rlen, rlen);
      r = ret + ind;			/* in case reallocated */
      *r++ = *p++;
    }
  *r = '\0';

  return ret;
}

#ifdef INCLUDE_UNUSED
/* Remove all leading whitespace from STRING.  This includes
   newlines.  STRING should be terminated with a zero. */
void
strip_leading (char *string)
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
strip_trailing (char *string, int len, int newlines_only)
{
  while (len >= 0)
    {
      if ((newlines_only && string[len] == '\n') ||
	  (!newlines_only && whitespace (string[len])))
	{
	  len--;
#ifdef __MSYS__
	  if (newlines_only && string[len + 1] == '\n' && string[len] == '\r')
	    len--;
#endif
	}
      else
	break;
    }
  string[len + 1] = '\0';
}

int
str_firstdiff (const char *s, const char *t)
{
  int n;

  if (s == 0 || t == 0 || *s == '\0' || *t == '\0')
    return 0;
  for (n = 0; s[n] && t[n] && s[n] == t[n]; n++)
    ;
  return n;
}

/* This returns the index in OLD of a common suffix of OLD and NEW */
int
str_lastsame (const char *old, const char *new)
{
  const char *o, *n;

  if (old == 0 || *old == '\0' || new == 0 || *new == '\0')
    return 0;	/* XXX */

  o = old + STRLEN (old) - 1;
  n = new + STRLEN (new) - 1;

  while (o > old && n > new && (*o == *n))
    o--, n--;

  return (o - old);
}

/* A wrapper for bcopy that can be prototyped in general.h */
void
xbcopy (const void *s, void *d, size_t n)
{
  FASTCOPY (s, d, n);
}
