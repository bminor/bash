/* strtrans.c - Translate and untranslate strings with ANSI-C escape
		sequences. */

/* Copyright (C) 2000
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

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <bashansi.h>
#include <stdio.h>
#include <chartypes.h>

#include "shell.h"

#ifdef ESC
#undef ESC
#endif
#define ESC '\033'	/* ASCII */

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

  ret = (char *)xmalloc (len + 1);
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
	      c = ESC; break;
	    case 'f': c = '\f'; break;
	    case 'n': c = '\n'; break;
	    case 'r': c = '\r'; break;
	    case 't': c = '\t'; break;
	    case '0': case '1': case '2': case '3':
	    case '4': case '5': case '6': case '7':
	      for (temp = 2, c -= '0'; ISOCTAL (*s) && temp--; s++)
		c = (c * 8) + OCTVALUE (*s);
	      c &= 0xFF;
	      break;
	    case 'x':			/* Hex digit -- non-ANSI */
	      for (temp = 2, c = 0; ISXDIGIT ((unsigned char)*s) && temp--; s++)
		c = (c * 16) + HEXVALUE (*s);
	      /* \x followed by non-hex digits is passed through unchanged */
	      if (temp == 2)
		{
		  *r++ = '\\';
		  c = 'x';
		}
	      c &= 0xFF;
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

/* Take a string STR, possibly containing non-printing characters, and turn it
   into a $'...' ANSI-C style quoted string.  Returns a new string. */
char *
ansic_quote (str, flags, rlen)
     char *str;
     int flags, *rlen;
{
  char *r, *ret, *s, obuf[8];
  int l, rsize, t;
  unsigned char c;

  if (str == 0 || *str == 0)
    return ((char *)0);

  l = strlen (str);
  rsize = 2 * l + 4;
  r = ret = (char *)xmalloc (rsize);

  *r++ = '$';
  *r++ = '\'';

  for (s = str, l = 0; *s; s++)
    {
      c = *s;
      l = 1;		/* 1 == add backslash; 0 == no backslash */
      switch (c)
	{
	case ESC: c = 'E'; break;
#ifdef __STDC__
	case '\a': c = 'a'; break;
	case '\v': c = 'v'; break;
#else
	case '\007': c = 'a'; break;
	case 0x0b: c = 'v'; break;
#endif

	case '\b': c = 'b'; break;
	case '\f': c = 'f'; break;
	case '\n': c = 'n'; break;
	case '\r': c = 'r'; break;
	case '\t': c = 't'; break;
	case '\\':
	case '\'':
	  break;
	default:
	  if (ISPRINT (c) == 0)
	    {
	      sprintf (obuf, "\\%.3o", c);
	      t = r - ret;
	      RESIZE_MALLOCED_BUFFER (ret, t, 5, rsize, 16);
	      r = ret + t;	/* in case reallocated */
	      for (t = 0; t < 4; t++)
		*r++ = obuf[t];
	      continue;
	    }
	  l = 0;
	  break;
	}
      if (l)
	*r++ = '\\';
      *r++ = c;
    }

  *r++ = '\'';
  *r = '\0';
  if (rlen)
    *rlen = r - ret;
  return ret;
}

/* return 1 if we need to quote with $'...' because of non-printing chars. */
ansic_shouldquote (string)
     const char *string;
{
  const char *s;
  unsigned char c;

  if (string == 0)
    return 0;

  for (s = string; c = *s; s++)
    if (ISPRINT (c) == 0)
      return 1;

  return 0;
}
