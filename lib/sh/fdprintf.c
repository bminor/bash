/* fdprintf -- printf to a file descriptor

   Copyright (C) 2008 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation,
   Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.  */
                                 
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdc.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (PREFER_STDARG)
#  include <stdarg.h>
#else
#  include <varargs.h>
#endif

#include <stdio.h>

int
#if defined (PREFER_STDARG)
fdprintf(int fd, const char *format, ...)
#else
fdprintf(fd, format, va_alist)
     int fd;
     const char *format;
     va_dcl
#endif
{
  FILE *fp;
  int fd2, rc, r2;
  va_list args;

  if ((fd2 = dup(fd)) < 0)
    return -1;
  fp = fdopen (dup (fd), "w");
  if (fp == 0)
    return -1;

  SH_VA_START (args, format);
  rc = vfprintf (fp, format, args);
  fflush (fp);
  va_end (args);

  r2 = fclose (fp);	/* check here */

  return rc;
}           
