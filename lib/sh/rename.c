/*
 * rename - rename a file
 */

/* Copyright (C) 1999 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#include <config.h>

#if !defined (HAVE_RENAME)

#include <bashtypes.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdc.h>

int
rename (from, to)
     const char *from, *to;
{
  unlink (to);
  if (link (from, to) < 0)
    return (-1);
  unlink (from);
  return (0);
}
#endif /* !HAVE_RENAME */
