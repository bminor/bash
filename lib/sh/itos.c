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
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <bashansi.h>
#include <chartypes.h>
#include "shell.h"

char *
inttostr (i, buf, len)
     long i;
     char *buf;
     size_t len;
{
  return (fmtulong (i, 10, buf, len, 0));
}

/* Integer to string conversion.  This conses the string; the
   caller should free it. */
char *
itos (i)
     long i;
{
  char *p, lbuf[INT_STRLEN_BOUND(long) + 1];

  p = fmtulong (i, 10, lbuf, sizeof(lbuf), 0);
  return (savestring (p));
}

char *
uinttostr (i, buf, len)
     unsigned long i;
     char *buf;
     size_t len;
{
  return (fmtulong (i, 10, buf, len, FL_UNSIGNED));
}

/* Integer to string conversion.  This conses the string; the
   caller should free it. */
char *
uitos (i)
     unsigned long i;
{
  char *p, lbuf[INT_STRLEN_BOUND(long) + 1];

  p = fmtulong (i, 10, lbuf, sizeof(lbuf), FL_UNSIGNED);
  return (savestring (p));
}
