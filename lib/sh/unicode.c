/* unicode.c - functions to convert unicode characters */

/* Copyright (C) 2010 Free Software Foundation, Inc.

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

#if defined (HANDLE_MULTIBYTE)

#include <stdc.h>
#include <wchar.h>
#include <bashansi.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <limits.h>

#if HAVE_ICONV
#  include <iconv.h>
#endif

#include <xmalloc.h>

#ifndef USHORT_MAX
#  ifdef USHRT_MAX
#    define USHORT_MAX USHRT_MAX
#  else
#    define USHORT_MAX ((unsigned short) ~(unsigned short)0)
#  endif
#endif

#if !defined (STREQ)
#  define STREQ(a, b) ((a)[0] == (b)[0] && strcmp ((a), (b)) == 0)
#endif /* !STREQ */

#if defined (HAVE_LOCALE_CHARSET)
extern const char *locale_charset __P((void));
#else
extern char *get_locale_var __P((char *));
#endif

static int u32init = 0;
static int utf8locale = 0;
#if defined (HAVE_ICONV)
static iconv_t localconv;
#endif

#ifndef HAVE_LOCALE_CHARSET
static char *
stub_charset ()
{
  char *locale, *s, *t;

  locale = get_locale_var ("LC_CTYPE");
  if (locale == 0 || *locale == 0)
    return "ASCII";
  s = strrchr (locale, '.');
  if (s)
    {
      t = strchr (s, '@');
      if (t)
	*t = 0;
      return ++s;
    }
  else if (STREQ (locale, "UTF-8"))
    return "UTF-8";
  else
    return "ASCII";
}
#endif

/* u32toascii ? */
int
u32tochar (wc, s)
     wchar_t wc;
     char *s;
{
  unsigned long x;
  int l;

  x = wc;
  l = (x <= UCHAR_MAX) ? 1 : ((x <= USHORT_MAX) ? 2 : 4);

  if (x <= UCHAR_MAX)
    s[0] = x & 0xFF;
  else if (x <= USHORT_MAX)	/* assume unsigned short = 16 bits */
    {
      s[0] = (x >> 8) & 0xFF;
      s[1] = x & 0xFF;
    }
  else
    {
      s[0] = (x >> 24) & 0xFF;
      s[1] = (x >> 16) & 0xFF;
      s[2] = (x >> 8) & 0xFF;
      s[3] = x & 0xFF;
    }
  s[l] = '\0';
  return l;  
}

int
u32toutf8 (wc, s)
     wchar_t wc;
     char *s;
{
  int l;

  l = (wc < 0x0080) ? 1 : ((wc < 0x0800) ? 2 : 3);

  if (wc < 0x0080)
    s[0] = (unsigned char)wc;
  else if (wc < 0x0800)
    {
      s[0] = (wc >> 6) | 0xc0;
      s[1] = (wc & 0x3f) | 0x80;
    }
  else
    {
      s[0] = (wc >> 12) | 0xe0;
      s[1] = ((wc >> 6) & 0x3f) | 0x80;
      s[2] = (wc & 0x3f) | 0x80;
    }
  s[l] = '\0';
  return l;
}

/* convert a single unicode-32 character into a multibyte string and put the
   result in S, which must be large enough (at least MB_LEN_MAX bytes) */
int
u32cconv (c, s)
     unsigned long c;
     char *s;
{
  wchar_t wc;
  int n;
#if HAVE_ICONV
  const char *charset;
  char obuf[25], *optr;
  size_t obytesleft;
  const char *iptr;
  size_t sn;
#endif

  wc = c;

#if __STDC_ISO_10646__
  if (sizeof (wchar_t) == 4)
    {
      n = wctomb (s, wc);
      return n;
    }
#endif

#if HAVE_NL_LANGINFO
  codeset = nl_langinfo (CODESET);
  if (STREQ (codeset, "UTF-8"))
    {
      n = u32toutf8 (wc, s);
      return n;
    }
#endif

#if HAVE_ICONV
  /* this is mostly from coreutils-8.5/lib/unicodeio.c */
  if (u32init == 0)
    {
#  if HAVE_LOCALE_CHARSET
      charset = locale_charset ();	/* XXX - fix later */
#  else
      charset = stub_charset ();
#  endif
      if (STREQ (charset, "UTF-8"))
	utf8locale = 1;
      else
	{
	  localconv = iconv_open (charset, "UTF-8");
	  if (localconv == (iconv_t)-1)
	    localconv = iconv_open (charset, "ASCII");
	}
      u32init = 1;
    }

  if (utf8locale)
    {
      n = u32toutf8 (wc, s);
      return n;
    }

  if (localconv == (iconv_t)-1)
    {
      n = u32tochar (wc, s);
      return n;
    }

  n = u32toutf8 (wc, s);

  optr = obuf;
  obytesleft = sizeof (obuf);
  iptr = s;
  sn = n;

  iconv (localconv, NULL, NULL, NULL, NULL);

  if (iconv (localconv, (ICONV_CONST char **)&iptr, &sn, &optr, &obytesleft) == (size_t)-1)
    return n;	/* You get utf-8 if iconv fails */

  *optr = '\0';

  /* number of chars to be copied is optr - obuf if we want to do bounds
     checking */
  strcpy (s, obuf);
  return (optr - obuf);
#endif

  n = u32tochar (wc, s);	/* fallback */
  return n;
}

#endif /* HANDLE_MULTIBYTE */
