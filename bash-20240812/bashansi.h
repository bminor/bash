/* bashansi.h -- Typically included information required by picky compilers. */

/* Copyright (C) 1993-2024 Free Software Foundation, Inc.

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

#if !defined (_BASHANSI_H_)
#define _BASHANSI_H_

#if defined (HAVE_STRING_H)
#  include <string.h>
#endif /* !HAVE_STRING_H */

#if defined (HAVE_STRINGS_H)
#  include <strings.h>
#endif /* !HAVE_STRINGS_H */

#if defined (HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  include "ansi_stdlib.h"
#endif /* !HAVE_STDLIB_H */

/* If bool is not a compiler builtin, prefer stdbool.h if we have it */
#if !defined (HAVE_C_BOOL)
#  if defined (HAVE_STDBOOL_H)
#    include <stdbool.h>
#  else
#    undef bool
typedef unsigned char bool;
#    define true 1
#    define false 0
#  endif
#endif

/* Include <stddef.h>, or define substitutes (config.h handles ptrdiff_t). */
#ifdef HAVE_STDDEF_H
#  include <stddef.h>
#endif
/* Substitutes for definitions in stddef.h */
#ifndef NULL
#  define NULL 0
#endif
#ifndef offsetof
#  define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

#endif /* !_BASHANSI_H_ */
