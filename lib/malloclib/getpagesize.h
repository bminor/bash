/* Emulation of getpagesize() for systems that need it.
   Copyright (C) 1991 Free Software Foundation, Inc.

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#if !defined (USG)
extern size_t getpagesize __P ((void));
#  if !defined (HAVE_GETPAGESIZE)
#    define HAVE_GETPAGESIZE
#  endif /* !HAVE_GETPAGESIZE */
#endif /* !USG */

#if !defined (HAVE_GETPAGESIZE) && defined (HAVE_UNISTD_H)
#  include <unistd.h>
#  if defined (_SC_PAGESIZE)
#    define getpagesize() sysconf(_SC_PAGESIZE)
#  endif /* _SC_PAGESIZE */
#endif

#if !defined (HAVE_GETPAGESIZE)
#  include <sys/param.h>
#  if defined (PAGESIZE)
#     define getpagesize() PAGESIZE
#  else /* !PAGESIZE */
#    if defined (EXEC_PAGESIZE)
#      define getpagesize() EXEC_PAGESIZE
#    else /* !EXEC_PAGESIZE */
#      if defined (NBPG)
#        if !defined (CLSIZE)
#          define CLSIZE 1
#        endif /* !CLSIZE */
#        define getpagesize() (NBPG * CLSIZE)
#      else /* !NBPG */
#        if defined (NBPC)
#          define getpagesize() NBPC
#        endif /* NBPC */
#      endif /* !NBPG */
#    endif /* !EXEC_PAGESIZE */
#  endif /* !PAGESIZE */
#endif /* !getpagesize */

#if !defined (HAVE_GETPAGESIZE) && !defined (getpagesize)
#  define getpagesize() 4096  /* Just punt and use reasonable value */
#endif /* !HAVE_GETPAGESIZE && !getpagesize */
