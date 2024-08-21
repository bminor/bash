/* bashtypes.h -- Bash system types. */

/* Copyright (C) 1993-2009,2024 Free Software Foundation, Inc.

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

#if !defined (_BASHTYPES_H_)
#  define _BASHTYPES_H_

#if defined (CRAY)
#  define word __word
#endif

#include <sys/types.h>

#if defined (CRAY)
#  undef word
#endif

#if defined (HAVE_INTTYPES_H)
#  include <inttypes.h>
#endif

/* Fix PRIdMAX on systems where it's broken. */
#ifdef PRI_MACROS_BROKEN
#  undef PRIdMAX
#endif
#ifndef PRIdMAX
#  if HAVE_LONG_LONG
#    define PRIdMAX	"lld"
#  else
#    define PRIdMAX	"ld"
#  endif
#endif

#if HAVE_STDINT_H
#  include <stdint.h>
#endif

#endif /* _BASHTYPES_H_ */
