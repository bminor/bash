/*
 * tmpfile.c - functions to create and safely open temp files for the shell.
 */

/* Copyright (C) 2000 Free Software Foundation, Inc.

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

#include <bashtypes.h>
#include <posixstat.h>
#include <filecntl.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include <shell.h>

#ifndef errno
extern int errno;
#endif

#define BASEOPENFLAGS	(O_CREAT | O_TRUNC | O_EXCL)

#define DEFAULT_TMPDIR		"."	/* bogus default, should be changed */
#define DEFAULT_NAMEROOT	"shtmp"

extern int dollar_dollar_pid;

static char *sys_tmpdir = (char *)NULL;
static int ntmpfiles;
static int tmpnamelen = -1;
static unsigned long filenum = 1L;

static char *
get_sys_tmpdir ()
{
  struct stat sb;

  if (sys_tmpdir)
    return sys_tmpdir;

  sys_tmpdir = "/tmp";
  if (stat (sys_tmpdir, &sb) == 0)
    return sys_tmpdir;

  sys_tmpdir = "/var/tmp";
  if (stat (sys_tmpdir, &sb) == 0)
    return sys_tmpdir;

  sys_tmpdir = "/usr/tmp";
  if (stat (sys_tmpdir, &sb) == 0)
    return sys_tmpdir;

#ifdef P_tmpdir
  sys_tmpdir = P_tmpdir;
#else
  sys_tmpdir = DEFAULT_TMPDIR;
#endif

  return sys_tmpdir;
}

static char *
get_tmpdir (flags)
     int flags;
{
  char *tdir;

  tdir = (flags & MT_USETMPDIR) ? get_string_value ("TMPDIR") : (char *)NULL;
  if (tdir == 0)
    tdir = get_sys_tmpdir ();

#if defined (HAVE_PATHCONF) && defined (_PC_NAME_MAX)
  if (tmpnamelen == -1)
    tmpnamelen = pathconf (tdir, _PC_NAME_MAX);
#else
  tmpnamelen = 0;
#endif

  return tdir;
}

char *
sh_mktmpname (nameroot, flags)
     char *nameroot;
     int flags;
{
  char *filename, *tdir;
  struct stat sb;
  int r, tdlen;

  filename = xmalloc (PATH_MAX + 1);
  tdir = get_tmpdir (flags);
  tdlen = strlen (tdir);

  if (nameroot == 0)
    nameroot = DEFAULT_NAMEROOT;

  while (1)
    {
      filenum *= (int)time ((time_t *)0) * dollar_dollar_pid *
		((flags & MT_USERANDOM) ? get_random_number () : ntmpfiles++);
      sprintf (filename, "%s/%s-%lu", tdir, nameroot, filenum);
      if (tmpnamelen > 0 && tmpnamelen < 32)
	filename[tdlen + 1 + tmpnamelen] = '\0';
#ifdef HAVE_LSTAT
      r = lstat (filename, &sb);
#else
      r = stat (filename, &sb);
#endif
      if (r < 0 && errno == ENOENT)
	break;
    }

  return filename;
}

int
sh_mktmpfd (nameroot, flags, namep)
     char *nameroot;
     int flags;
     char **namep;
{
  char *filename, *tdir;
  int fd, tdlen;

  filename = xmalloc (PATH_MAX + 1);
  tdir = get_tmpdir (flags);
  tdlen = strlen (tdir);

  if (nameroot == 0)
    nameroot = DEFAULT_NAMEROOT;

  do
    {
      filenum *= (int)time ((time_t *)0) * dollar_dollar_pid *
		((flags & MT_USERANDOM) ? get_random_number () : ntmpfiles++);
      sprintf (filename, "%s/%s-%lu", tdir, nameroot, filenum);
      if (tmpnamelen > 0 && tmpnamelen < 32)
	filename[tdlen + 1 + tmpnamelen] = '\0';
      fd = open (filename, BASEOPENFLAGS | ((flags & MT_READWRITE) ? O_RDWR : O_WRONLY), 0600);
    }
  while (fd < 0 && errno == EEXIST);

  if (namep)
    *namep = filename;
  else
    free (filename);

  return fd;
}

FILE *
sh_mktmpfp (nameroot, flags, namep)
     char *nameroot;
     int flags;
     char **namep;
{
  int fd;

  fd = sh_mktmpfd (nameroot, flags, namep);
  return ((fd >= 0) ? (fdopen (fd, (flags & MT_READWRITE) ? "w+" : "w")) : (FILE *)NULL);
}
