/* stringvec.c - function for managing arrays of strings. */

/* Copyright (C) 2000 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include <stdio.h>
#include <ctype.h>

#include "shell.h"

#ifdef INCLUDE_UNUSED
/* Find NAME in ARRAY.  Return the index of NAME, or -1 if not present.
   ARRAY should be NULL terminated. */
int
find_name_in_array (name, array)
     char *name, **array;
{
  int i;

  for (i = 0; array[i]; i++)
    if (STREQ (name, array[i]))
      return (i);

  return (-1);
}
#endif

/* Allocate an array of strings with room for N members. */
char **
alloc_array (n)
     int n;
{
  return ((char **)xmalloc ((n) * sizeof (char *)));
}

/* Return the length of ARRAY, a NULL terminated array of char *. */
int
array_len (array)
     char **array;
{
  register int i;

  for (i = 0; array[i]; i++);
  return (i);
}

/* Free the contents of ARRAY, a NULL terminated array of char *. */
void
free_array_members (array)
     char **array;
{
  register int i;

  if (array == 0)
    return;

  for (i = 0; array[i]; i++)
    free (array[i]);
}

void
free_array (array)
     char **array;
{
  if (array == 0)
    return;

  free_array_members (array);
  free (array);
}

/* Allocate and return a new copy of ARRAY and its contents. */
char **
copy_array (array)
     char **array;
{
  register int i;
  int len;
  char **new_array;

  len = array_len (array);

  new_array = (char **)xmalloc ((len + 1) * sizeof (char *));
  for (i = 0; array[i]; i++)
    new_array[i] = savestring (array[i]);
  new_array[i] = (char *)NULL;

  return (new_array);
}

/* Comparison routine for use with qsort() on arrays of strings.  Uses
   strcoll(3) if available, otherwise it uses strcmp(3). */
int
qsort_string_compare (s1, s2)
     register char **s1, **s2;
{
#if defined (HAVE_STRCOLL)
   return (strcoll (*s1, *s2));
#else /* !HAVE_STRCOLL */
  int result;

  if ((result = **s1 - **s2) == 0)
    result = strcmp (*s1, *s2);

  return (result);
#endif /* !HAVE_STRCOLL */
}

/* Sort ARRAY, a null terminated array of pointers to strings. */
void
sort_char_array (array)
     char **array;
{
  qsort (array, array_len (array), sizeof (char *),
	 (Function *)qsort_string_compare);
}

