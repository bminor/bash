/* setlinebuf.c - line-buffer a stdio stream. */

/* Copyright (C) 1997 Free Software Foundation, Inc.

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
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA */

#include <config.h>

#if !defined (HAVE_SETLINEBUF)

#include <stdio.h>

/* Cause STREAM to buffer lines as opposed to characters or blocks. */
int
setlinebuf (stream)
     FILE *stream;
{
#if defined (_IOLBF)
#  if defined (SETVBUF_REVERSED)
  setvbuf (stream, _IOLBF, (char *)NULL, BUFSIZ);
#  else /* !SETVBUF_REVERSED */
  setvbuf (stream, (char *)NULL, _IOLBF, BUFSIZ);
#  endif /* !SETVBUF_REVERSED */
#endif /* _IOLBF */
  return (0);
}
#endif /* !HAVE_SETLINEBUF */
