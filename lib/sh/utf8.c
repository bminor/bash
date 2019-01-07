/* utf8.c - UTF-8 character handling functions */

/* Copyright (C) 2018 Free Software Foundation, Inc.

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

#include <config.h>

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include "bashansi.h"
#include "shmbutil.h"

extern int locale_mb_cur_max;
extern int locale_utf8locale;

#if defined (HANDLE_MULTIBYTE)

char *
utf8_mbschr (s, c)
     const char *s;
     int c;
{
  return strchr (s, c);		/* for now */
}

int
utf8_mbscmp (s1, s2)
     const char *s1, *s2;
{
  /* Use the fact that the UTF-8 encoding preserves lexicographic order.  */
  return strcmp (s1, s2);
}

char *
utf8_mbsmbchar (str)
     const char *str;
{
  register char *s;

  for (s = (char *)str; *s; s++)
    if ((*s & 0xc0) == 0x80)
      return s;
  return (0);
}

int
utf8_mbsnlen(src, srclen, maxlen)
     const char *src;
     size_t srclen;
     int maxlen;
{
  register int sind, count;

  for (sind = count = 0; src[sind] && sind <= maxlen; sind++)
    {
      if ((src[sind] & 0xc0) != 0x80)
	count++;
    }
  return (count);
}

/* Adapted from GNU gnulib */
int
utf8_mblen (s, n)
     const char *s;
     size_t n;
{
  unsigned char c, c1;

  if (s == 0)
    return (0);	/* no shift states */
  if (n <= 0)
    return (-1);

  c = (unsigned char)*s;
  if (c < 0x80)
    return (c != 0);
  if (c >= 0xc2)
    {
      c1 = (unsigned char)s[1];
      if (c < 0xe0)
	{
	  if (n >= 2 && (s[1] ^ 0x80) < 0x40)
	    return 2;
	}
      else if (c < 0xf0)
	{
	  if (n >= 3
		&& (s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
		&& (c >= 0xe1 || c1 >= 0xa0)
		&& (c != 0xed || c1 < 0xa0))
	    return 3;
	}
      else if (c < 0xf8)
	{
	  if (n >= 4
		&& (s[1] ^ 0x80) < 0x40 && (s[2] ^ 0x80) < 0x40
	 	&& (s[3] ^ 0x80) < 0x40
		&& (c >= 0xf1 || c1 >= 0x90)
		&& (c < 0xf4 || (c == 0xf4 && c1 < 0x90)))
	    return 4;
	}
    }
  /* invalid or incomplete multibyte character */
  return -1;
}

/* We can optimize this if we know the locale is UTF-8, but needs to handle
   malformed byte sequences. */
size_t
utf8_mbstrlen(s)
     const char *s;
{
  size_t clen, nc;
  int mb_cur_max;

  nc = 0;
  mb_cur_max = MB_CUR_MAX;
  while (*s && (clen = (size_t)utf8_mblen(s, mb_cur_max)) != 0)
    {
      if (MB_INVALIDCH(clen))
	clen = 1;	/* assume single byte */

      s += clen;
      nc++;
    }
  return nc;
}

#endif
