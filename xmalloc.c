/* xmalloc.c -- safe versions of malloc and realloc */

/* Copyright (C) 1991-2016,2023-2024 Free Software Foundation, Inc.

   This file is part of GNU Bash, the GNU Bourne Again SHell.

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

#if defined (HAVE_CONFIG_H)
#include <config.h>
#endif

#include "bashtypes.h"
#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdckdint.h>

#include "error.h"

#include "bashintl.h"

#if !defined (PTR_T)
#  define PTR_T void *
#endif /* !PTR_T */

#if HAVE_SBRK && !HAVE_DECL_SBRK
extern char *sbrk();
#endif

#if HAVE_SBRK && defined (USING_BASH_MALLOC)
static PTR_T lbreak;
static int brkfound;
static size_t allocated;
#endif

/* **************************************************************** */
/*								    */
/*		   Memory Allocation and Deallocation.		    */
/*								    */
/* **************************************************************** */

#if HAVE_SBRK && defined (USING_BASH_MALLOC)
#define FINDBRK() \
do { \
  if (brkfound == 0) \
    { \
      lbreak = (PTR_T)sbrk (0); \
      brkfound++; \
    } \
} while (0)

static size_t
findbrk (void)
{
  FINDBRK();
  return (char *)sbrk (0) - (char *)lbreak;
}
#else
#define FINDBRK()
#endif

static void
allocerr (const char *func, size_t bytes)
{
#if HAVE_SBRK && defined (USING_BASH_MALLOC)
      allocated = findbrk ();
      fatal_error (_("%s: cannot allocate %lu bytes (%lu bytes allocated)"), func, (unsigned long)bytes, (unsigned long)allocated);
#else
      fatal_error (_("%s: cannot allocate %lu bytes"), func, (unsigned long)bytes);
#endif /* !HAVE_SBRK */
}

/* Return a pointer to free()able block of memory large enough
   to hold BYTES number of bytes.  If the memory cannot be allocated,
   print an error message and abort. */
PTR_T
xmalloc (size_t bytes)
{
  PTR_T temp;

#if defined (DEBUG)
  if (bytes == 0)
    internal_warning("xmalloc: size argument is 0");
#endif

  FINDBRK();
  temp = malloc (bytes);

  if (temp == 0)
    allocerr ("xmalloc", bytes);

  return (temp);
}

PTR_T
xrealloc (PTR_T pointer, size_t bytes)
{
  PTR_T temp;

#if defined (DEBUG)
  if (bytes == 0)
    internal_warning("xrealloc: size argument is 0");
#endif

  FINDBRK();
  temp = pointer ? realloc (pointer, bytes) : malloc (bytes);

  if (temp == 0)
    allocerr ("xrealloc", bytes);

  return (temp);
}

/* Reallocate the storage addressed by POINTER so that it is of
   size NMEMB*SIZE, preserving contents like realloc does.
   If POINTER is null, allocate new storage.  This is like
   glibc reallocarray except that it never returns a null pointer;
   if storage is exhausted it reports an error and exits. */
PTR_T
xreallocarray (PTR_T ptr, size_t nmemb, size_t size)
{
  size_t nbytes;

  if (ckd_mul (&nbytes, nmemb, size))
    allocerr ("xreallocarray", (size_t)-1);
	        
  return xrealloc (ptr, nbytes);
}

/* Use this as the function to call when adding unwind protects so we
   don't need to know what free() returns. */
void
xfree (PTR_T string)
{
  if (string)
    free (string);
}

#ifdef USING_BASH_MALLOC
#include <malloc/shmalloc.h>

static void
sh_allocerr (const char *func, size_t bytes, char *file, int line)
{
#if HAVE_SBRK
      allocated = findbrk ();
      fatal_error (_("%s: %s:%d: cannot allocate %lu bytes (%lu bytes allocated)"), func, file, line, (unsigned long)bytes, (unsigned long)allocated);
#else
      fatal_error (_("%s: %s:%d: cannot allocate %lu bytes"), func, file, line, (unsigned long)bytes);
#endif /* !HAVE_SBRK */
}

PTR_T
sh_xmalloc (size_t bytes, char *file, int line)
{
  PTR_T temp;

#if defined (DEBUG)
  if (bytes == 0)
    internal_warning("xmalloc: %s:%d: size argument is 0", file, line);
#endif

  FINDBRK();
  temp = sh_malloc (bytes, file, line);

  if (temp == 0)
    sh_allocerr ("xmalloc", bytes, file, line);

  return (temp);
}

PTR_T
sh_xrealloc (PTR_T pointer, size_t bytes, char *file, int line)
{
  PTR_T temp;

#if defined (DEBUG)
  if (bytes == 0)
    internal_warning("xrealloc: %s:%d: size argument is 0", file, line);
#endif

  FINDBRK();
  temp = pointer ? sh_realloc (pointer, bytes, file, line) : sh_malloc (bytes, file, line);

  if (temp == 0)
    sh_allocerr ("xrealloc", bytes, file, line);

  return (temp);
}

PTR_T
sh_xreallocarray (PTR_T ptr, size_t nmemb, size_t size, char *file, int line)
{
  size_t nbytes;

  if (ckd_mul (&nbytes, nmemb, size))
    sh_allocerr ("xreallocarray", (size_t)-1, file, line);
	        
  return sh_xrealloc (ptr, nbytes, file, line);
}

void
sh_xfree (PTR_T string, char *file, int line)
{
  if (string)
    sh_free (string, file, line);
}
#endif
