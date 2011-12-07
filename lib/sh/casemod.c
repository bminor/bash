/* casemod.c -- functions to change case of strings */

/* Copyright (C) 2008 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   The GNU General Public License is often shipped with GNU software, and
   is generally kept in a file called COPYING or LICENSE.  If you do not
   have a copy of the license, write to the Free Software Foundation,
   59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif /* HAVE_UNISTD_H */

#include <stdc.h>

#include <bashansi.h>
#include <bashintl.h>
#include <bashtypes.h>

#include <stdio.h>
#include <ctype.h>
#include <xmalloc.h>

#include <shmbutil.h>
#include <chartypes.h>

#include <glob/strmatch.h>

#define _to_wupper(wc)	(iswlower (wc) ? towupper (wc) : (wc))
#define _to_wlower(wc)	(iswupper (wc) ? towlower (wc) : (wc))

#if !defined (HANDLE_MULTIBYTE)
#  define cval(s, i)	((s)[(i)])
#  define iswalnum(c)	(isalnum(c))
#  define TOGGLE(x)	(ISUPPER (x) ? tolower (x) : (TOUPPER (x)))
#else
#  define TOGGLE(x)	(iswupper (x) ? towlower (x) : (_to_wupper(x)))
#endif

/* These must agree with the defines in externs.h */
#define CASE_NOOP	0x0
#define CASE_LOWER	0x01
#define CASE_UPPER	0x02
#define CASE_CAPITALIZE	0x04
#define CASE_UNCAP	0x08
#define CASE_TOGGLE	0x10
#define CASE_TOGGLEALL	0x20

extern char *substring __P((char *, int, int));

#if defined (HANDLE_MULTIBYTE)
static wchar_t
cval (s, i)
     char *s;
     int i;
{
  size_t tmp;
  wchar_t wc;
  int l;
  mbstate_t mps;  

  if (MB_CUR_MAX == 1)
    return ((wchar_t)s[i]);
  l = strlen (s);
  if (i >= (l - 1))
    return ((wchar_t)s[i]);
  memset (&mps, 0, sizeof (mbstate_t));
  tmp = mbrtowc (&wc, s + i, l - i, &mps);
  if (MB_INVALIDCH (tmp) || MB_NULLWCH (tmp))
    return ((wchar_t)s[i]);
  return wc;  
}
#endif

/* Modify the case of characters in STRING matching PAT based on the value of
   FLAGS.  If PAT is null, modify the case of each character */
char *
sh_modcase (string, pat, flags)
     const char *string;
     char *pat;
     int flags;
{
  int start, next, end;
  int inword, c, nc, nop, match;
  char *ret, *s;
  wchar_t wc;
#if defined (HANDLE_MULTIBYTE)
  wchar_t nwc;
  char mb[MB_LEN_MAX+1];
  int mlen;
  mbstate_t state;
#endif

#if defined (HANDLE_MULTIBYTE)
  memset (&state, 0, sizeof (mbstate_t));
#endif

  start = 0;
  end = strlen (string);

  ret = (char *)xmalloc (end + 1);
  strcpy (ret, string);

  inword = 0;
  while (start < end)
    {
      wc = cval (ret, start);

      if (iswalnum (wc) == 0)
	{
	  inword = 0;
	  ADVANCE_CHAR (ret, end, start);
	  continue;
	}

      if (pat)
	{
	  next = start;
	  ADVANCE_CHAR (ret, end, next);
	  s = substring (ret, start, next);
	  match = strmatch (pat, s, FNM_EXTMATCH) != FNM_NOMATCH;
	  free (s);
	  if (match == 0)
            {
              start = next;
              inword = 1;
              continue;
            }
	}

      if (flags == CASE_CAPITALIZE)
	{
	  nop = inword ? CASE_LOWER : CASE_UPPER;
	  inword = 1;
	}
      else if (flags == CASE_UNCAP)
	{
	  nop = inword ? CASE_UPPER : CASE_LOWER;
	  inword = 1;
	}
      else if (flags == CASE_TOGGLE)
	{
	  nop = inword ? CASE_NOOP : CASE_TOGGLE;
	  inword = 1;
	}
      else
	nop = flags;

      if (MB_CUR_MAX == 1 || isascii (wc))
	{
	  switch (nop)
	  {
	  case CASE_NOOP:  nc = wc; break;
	  case CASE_UPPER:  nc = TOUPPER (wc); break;
	  case CASE_LOWER:  nc = TOLOWER (wc); break;
	  case CASE_TOGGLEALL:
	  case CASE_TOGGLE: nc = TOGGLE (wc); break;
	  }
	  ret[start] = nc;
	}
#if defined (HANDLE_MULTIBYTE)
      else
	{
	  mbrtowc (&wc, string + start, end - start, &state);
	  switch (nop)
	  {
	  case CASE_NOOP:  nwc = wc; break;
	  case CASE_UPPER:  nwc = TOUPPER (wc); break;
	  case CASE_LOWER:  nwc = TOLOWER (wc); break;
	  case CASE_TOGGLEALL:
	  case CASE_TOGGLE: nwc = TOGGLE (wc); break;
	  }
	  if  (nwc != wc)	/*  just skip unchanged characters */
	    {
	      mlen = wcrtomb (mb, nwc, &state);
	      if (mlen > 0)
		mb[mlen] = '\0';
	      /* Assume the same width */
	      strncpy (ret + start, mb, mlen);
	    }
	}
#endif

      /*  This assumes that the upper and lower case versions are the same width. */
      ADVANCE_CHAR (ret, end, start);
    }

  return ret;
}
