/* reallocarray.c - reallocate memory for an array given type size and
   number of elements */

/* Copyright (C) 2024 Free Software Foundation, Inc.

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

#include "bashansi.h"
#include <stdckdint.h>

#include <errno.h>
#if !defined (errno)
extern int errno;
#endif

void *
reallocarray (void *ptr, size_t nmemb, size_t size)
{
  size_t nbytes;

  if (ckd_mul (&nbytes, nmemb, size))
    {
      errno = ENOMEM;
      return NULL;
    }
	        
  return realloc (ptr, nbytes);
}
