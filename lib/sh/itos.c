/* itos.c -- Convert integer to string. */

/* Copyright (C) 1998, Free Software Foundation, Inc.

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
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"
#include "shell.h"

/* Number of characters that can appear in a string representation
   of an integer.  32 is larger than the string rep of 2^^31 - 1. */
#define MAX_INT_LEN 32

/* Integer to string conversion.  This conses the string; the
   caller should free it. */
char *
itos (i)
     int i;
{
  char buf[MAX_INT_LEN], *p, *ret;
  int negative = 0;
  unsigned int ui;

  if (i < 0)
    {
      negative++;
      i = -i;
    }

  ui = (unsigned int) i;

  p = buf + MAX_INT_LEN - 2;
  p[1] = '\0';

  do
    *p-- = (ui % 10) + '0';
  while (ui /= 10);

  if (negative)
    *p-- = '-';

  ret = savestring (p + 1);
  return (ret);
}
