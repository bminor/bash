/* imalloc.h -- internal malloc definitions shared by source files. */

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

/* Must be included *after* config.h */

#ifndef _IMALLOC_H_
#define _IMALLOC_H

#ifdef MALLOC_DEBUG
#define MALLOC_STATS
#define MALLOC_TRACE
#define MALLOC_REGISTER
#endif

/* Generic pointer type. */
#ifndef PTR_T
#  if defined (__STDC__)
#    define PTR_T void *
#  else
#    define PTR_T char *
#  endif
#endif

#if !defined (NULL)
#  define NULL 0
#endif

#if !defined (__STRING)
#  if defined (HAVE_STRINGIZE)
#    define __STRING(x) #x
#  else
#    define __STRING(x) "x"
#  endif /* !HAVE_STRINGIZE */
#endif /* !__STRING */

#if __GNUC__ > 1
#  define FASTCOPY(s, d, n)  __builtin_memcpy (d, s, n)
#else /* !__GNUC__ */
#  if !defined (HAVE_BCOPY)
#    if !defined (HAVE_MEMMOVE)
#      define FASTCOPY(s, d, n)  memcpy (d, s, n)
#    else
#      define FASTCOPY(s, d, n)  memmove (d, s, n)
#    endif /* !HAVE_MEMMOVE */
#  else /* HAVE_BCOPY */
#    define FASTCOPY(s, d, n)  bcopy (s, d, n)
#  endif /* HAVE_BCOPY */
#endif /* !__GNUC__ */

#endif
