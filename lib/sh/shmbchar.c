/* Copyright (C) 2001, 2006, 2009, 2010, 2012, 2015 Free Software Foundation, Inc.

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.  */


#include <config.h>

#if defined (HANDLE_MULTIBYTE)
#include <stdlib.h>
#include <limits.h>

#include <errno.h>

#include <shmbutil.h>
#include <shmbchar.h>

#ifndef errno
extern int errno;
#endif

#if IS_BASIC_ASCII

/* Bit table of characters in the ISO C "basic character set".  */
const unsigned int is_basic_table [UCHAR_MAX / 32 + 1] =
{
  0x00001a00,           /* '\t' '\v' '\f' */
  0xffffffef,           /* ' '...'#' '%'...'?' */
  0xfffffffe,           /* 'A'...'Z' '[' '\\' ']' '^' '_' */
  0x7ffffffe            /* 'a'...'z' '{' '|' '}' '~' */
  /* The remaining bits are 0.  */
};

#endif /* IS_BASIC_ASCII */

extern int locale_utf8locale;

/* We can optimize this if we know the locale is UTF-8, but needs to handle
   malformed byte sequences. */
static inline size_t
utf8_mbstrlen(s)
     const char *s;
{
  size_t num = 0;
  register unsigned char c;

  while ((c = *s++))
    /* bytes 0xc0 through 0xff are first byte of multi-byte sequence */
    if ((c & 0xc0) != 0x80)	/* skip continuation bytes */
      ++num;
  return (num);
}

/* Adapted from GNU libutf8 */
static inline int
utf8_mblen (s, n)
     const char *s;
     int n;
{
  unsigned char c;

  if (s == 0)
    return 0;
  else if (n == 0)
    return -1;

  c = (unsigned char) *s;
  if (c < 0x80)
    return (c != 0);
  else if (c < 0xc0)
    goto return_error;
  else
    {
      const char *start = s;
      size_t count;
      int check_unsafe;

      if (c < 0xe0)
        {
          count = 1;
          if (c < 0xc2)
	    goto return_error;
	  check_unsafe = 0;
        }
      else if (c < 0xf0)
	{
	  count = 2;
	  check_unsafe = (c == 0xe0);
	}
#if SIZEOF_WCHAR_T == 4
      else if (c < 0xf8)
	{
	  count = 3;
	  check_unsafe = (c == 0xe0);
	}
      else if (c < 0xfc)
	{
	  count = 4;
	  check_unsafe = (c == 0xf8);
	}
      else if (c < 0xfe)
	{
	  count = 5;
	  check_unsafe = (c == 0xfc);
	}
#endif
      else
	goto return_error;
      if (n <= count)
	return -1;
      s++;
      c = (unsigned char) *s++ ^ 0x80;
      if (c >= 0x40)
	goto return_error;
      if (--count > 0)
	{
	  if (check_unsafe && ((c >> (6 - count)) == 0))
	    goto return_error;
	  do
	    {
	      c = (unsigned char) *s++ ^ 0x80;
	      if (c >= 0x40)
		goto return_error;
	    }
	  while (--count > 0);
	}
      return s - start;
    }
return_error:
  errno = EILSEQ;
  return -1;
}
  
/* Count the number of characters in S, counting multi-byte characters as a
   single character. */
size_t
mbstrlen (s)
     const char *s;
{
  size_t clen, nc;
  mbstate_t mbs = { 0 }, mbsbak = { 0 };
  int f, mb_cur_max;

  nc = 0;
  mb_cur_max = MB_CUR_MAX;
  while (*s && (clen = (f = is_basic (*s)) ? 1 : mbrlen(s, mb_cur_max, &mbs)) != 0)
    {
      if (MB_INVALIDCH(clen))
	{
	  clen = 1;	/* assume single byte */
	  mbs = mbsbak;
	}

      if (f == 0)
	mbsbak = mbs;

      s += clen;
      nc++;
    }
  return nc;
}

static inline char *
utf8_mbsmbchar (str)
     const char *str;
{
  register char *s;

  for (s = (char *)str; *s; s++)
    if ((*s & 0xc0) == 0x80)
      return s;
  return (0);
}

/* Return pointer to first multibyte char in S, or NULL if none. */
/* XXX - if we know that the locale is UTF-8, we can just check whether or
   not any byte has the eighth bit turned on */
char *
mbsmbchar (s)
     const char *s;
{
  char *t;
  size_t clen;
  mbstate_t mbs = { 0 };
  int mb_cur_max;

  if (locale_utf8locale)
    return (utf8_mbsmbchar (s));	/* XXX */

  mb_cur_max = MB_CUR_MAX;
  for (t = (char *)s; *t; t++)
    {
      if (is_basic (*t))
	continue;

      if (locale_utf8locale)		/* not used if above code active */
	clen = utf8_mblen (t, mb_cur_max);
      else
	clen = mbrlen (t, mb_cur_max, &mbs);

      if (clen == 0)
        return 0;
      if (MB_INVALIDCH(clen))
	continue;

      if (clen > 1)
	return t;
    }
  return 0;
}

static inline int
utf_mbsnlen(src, srclen, maxlen)
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

int
sh_mbsnlen(src, srclen, maxlen)
     const char *src;
     size_t srclen;
     int maxlen;
{
  int count;
  int sind;
  DECLARE_MBSTATE;

  for (sind = count = 0; src[sind]; )
    {
      count++;		/* number of multibyte characters */
      ADVANCE_CHAR (src, srclen, sind);
      if (sind > maxlen)
        break;
    }

  return count;
}
#endif
