/* chartypes.h -- extend ctype.h */

/* Copyright (C) 2001 Free Software Foundation, Inc.

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

#ifndef _SH_CHARTYPES_H
#define _SH_CHARTYPES_H

#include <ctype.h>

/* Jim Meyering writes:

   "... Some ctype macros are valid only for character codes that
   isascii says are ASCII (SGI's IRIX-4.0.5 is one such system --when
   using /bin/cc or gcc but without giving an ansi option).  So, all
   ctype uses should be through macros like ISPRINT...  If
   STDC_HEADERS is defined, then autoconf has verified that the ctype
   macros don't need to be guarded with references to isascii. ...
   Defining IN_CTYPE_DOMAIN to 1 should let any compiler worth its salt
   eliminate the && through constant folding."
   Solaris defines some of these symbols so we must undefine them first.  */

#if defined STDC_HEADERS || (!defined isascii && !defined HAVE_ISASCII)
#  define IN_CTYPE_DOMAIN(c) 1
#else
#  define IN_CTYPE_DOMAIN(c) isascii(c)
#endif

#if !defined (isspace) && !defined (HAVE_ISSPACE)
#  define isspace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\f')
#endif

#if !defined (isprint) && !defined (HAVE_ISPRINT)
#  define isprint(c) (isalpha(c) || isdigit(c) || ispunct(c))
#endif

#if defined (isblank) || defined (HAVE_ISBLANK)
#  define ISBLANK(c) (IN_CTYPE_DOMAIN (c) && isblank (c))
#else
#  define ISBLANK(c) ((c) == ' ' || (c) == '\t')
#endif

#if defined (isgraph) || defined (HAVE_ISGRAPH)
#  define ISGRAPH(c) (IN_CTYPE_DOMAIN (c) && isgraph (c))
#else
#  define ISGRAPH(c) (IN_CTYPE_DOMAIN (c) && isprint (c) && !isspace (c))
#endif

#if !defined (isxdigit) && !defined (HAVE_ISXDIGIT)
#  define isxdigit(c)	(((c) >= '0' && (c) <= '9') || ((c) >= 'a' && (c) <= 'f') || ((c) >= 'A' && (c) <= 'F'))
#endif

#undef ISPRINT

#define ISPRINT(c) (IN_CTYPE_DOMAIN (c) && isprint (c))
#define ISDIGIT(c) (IN_CTYPE_DOMAIN (c) && isdigit (c))
#define ISALNUM(c) (IN_CTYPE_DOMAIN (c) && isalnum (c))
#define ISALPHA(c) (IN_CTYPE_DOMAIN (c) && isalpha (c))
#define ISCNTRL(c) (IN_CTYPE_DOMAIN (c) && iscntrl (c))
#define ISLOWER(c) (IN_CTYPE_DOMAIN (c) && islower (c))
#define ISPUNCT(c) (IN_CTYPE_DOMAIN (c) && ispunct (c))
#define ISSPACE(c) (IN_CTYPE_DOMAIN (c) && isspace (c))
#define ISUPPER(c) (IN_CTYPE_DOMAIN (c) && isupper (c))
#define ISXDIGIT(c) (IN_CTYPE_DOMAIN (c) && isxdigit (c))

#define ISLETTER(c)	(ISALPHA(c))

#define DIGIT(c)	((c) >= '0' && (c) <= '9')

#define HEXVALUE(c) \
  (((c) >= 'a' && (c) <= 'f') \
	? (c)-'a'+10 \
	: (c) >= 'A' && (c) <= 'F' ? (c)-'A'+10 : (c)-'0')
                  
#ifndef ISOCTAL
#  define ISOCTAL(c)	((c) >= '0' && (c) <= '7')
#endif
#define OCTVALUE(c)	((c) - '0')

#define TODIGIT(c)	((c) - '0')
#define TOCHAR(c)	((c) + '0')

#define TOLOWER(c)	(ISUPPER(c) ? tolower(c) : (c))
#define TOUPPER(c)	(ISLOWER(c) ? toupper(c) : (c))

#ifdef toascii
#  define TOASCII(c)	(toascii(c))
#else
#  define TOASCII(c)	((c) & 0x7F)
#endif

/* We remove any previous definition of `SIGN_EXTEND_CHAR',
   since ours (we hope) works properly with all combinations of
   machines, compilers, `char' and `unsigned char' argument types.
   (Per Bothner suggested the basic approach.)  */
#undef SIGN_EXTEND_CHAR
#if __STDC__
#  define SIGN_EXTEND_CHAR(c) ((signed char) (c))
#else  /* not __STDC__ */
   /* As in Harbison and Steele.  */
#  define SIGN_EXTEND_CHAR(c) ((((unsigned char) (c)) ^ 128) - 128)
#endif

#endif /* _SH_CHARTYPES_H */
