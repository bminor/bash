/* Copyright (C) 2023-2024 Free Software Foundation, Inc.

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
#include <unistd.h>
#include <errno.h>

#if defined (HAVE_BRK) && !defined (HAVE_SBRK)

static void *initialbrk;
static void *curbrk;

static int
initbrk (void)
{
  if (initialbrk == 0)
    {
      void *b;

      b = brk (NULL);
      if (b == (void *)-1)
	return -1;
      initialbrk = curbrk = b;
    }
  return (0);
}

/* sbrk(3) implementation in terms of brk(2). Good enough for malloc to use. */
void *
sbrk (intptr_t incr)
{
  void *newbrk, *oldbrk;

  if (initialbrk == 0 && initbrk () == -1)
    {
      errno = ENOMEM;
      return (void *)-1;
    }

  if (incr == 0)
    return curbrk;

  /* bounds checking, overflow */
  if ((incr > 0 && (uintptr_t) curbrk + incr < (uintptr_t) curbrk) ||
      (incr < 0 && (uintptr_t) curbrk + incr > (uintptr_t) curbrk))
    {
      errno = ENOMEM;
      return (void *)-1;
    }

  newbrk = curbrk + incr;  
  if (newbrk < initialbrk)
    {
      errno = EINVAL;
      return (void *)-1;
    }

  if (brk (newbrk) == (void *)-1)
    return (void *)-1;		/* preserve errno */

  oldbrk = curbrk;
  curbrk = newbrk;

  return (oldbrk);
}
#endif /* HAVE_BRK && !HAVE_SBRK */
