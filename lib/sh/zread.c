/* Copyright (C) 1999 Free Software Foundation, Inc.

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

#include <config.h>

#include <sys/types.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <errno.h>

#if !defined (errno)
extern int errno;
#endif

#ifndef SEEK_CUR
#  define SEEK_CUR 1
#endif

/* Read LEN bytes from FD into BUF.  Retry the read on EINTR.  Any other
   error causes the loop to break. */
int
zread (fd, buf, len)
     int fd;
     char *buf;
     size_t len;
{
  int r;

  while ((r = read (fd, buf, len)) < 0 && errno == EINTR)
    ;
  return r;
}

/* Read LEN bytes from FD into BUF.  Retry the read on EINTR, up to three
   interrupts.  Any other error causes the loop to break. */

#ifdef NUM_INTR
#  undef NUM_INTR
#endif
#define NUM_INTR 3

int
zread1 (fd, buf, len)
     int fd;
     char *buf;
     size_t len;
{
  int r, nintr;

  for (nintr = 0; ; )
    {
      r = read (fd, buf, len);
      if (r >= 0)
        return r;
      if (r == -1 && errno == EINTR)
	{
	  if (++nintr > NUM_INTR)
	    return -1;
	  continue;
	}
      return r;
    }
}

/* Read one character from FD and return it in CP.  Return values are as
   in read(2).  This does some local buffering to avoid many one-character
   calls to read(2), like those the `read' builtin performs. */

static unsigned char lbuf[128];
static int lind, lused;

int
zreadc (fd, cp)
     int fd;
     char *cp;
{
  int r;

  if (lind == lused || lused == 0)
    {
      lused = zread (fd, lbuf, sizeof (lbuf));
      lind = 0;
      if (lused <= 0)
	return (lused);
    }
  if (cp)
    *cp = (char)lbuf[lind++];
  return 1;
}

void
zreset ()
{
  lind = lused = 0;
}

/* Sync the seek pointer for FD so that the kernel's idea of the last char
   read is the last char returned by zreadc. */
void
zsyncfd (fd)
     int fd;
{
  int off;

  off = lused - lind;
  if (off > 0)
    lseek (fd, -off, SEEK_CUR);
  lused = lind = 0;
}
