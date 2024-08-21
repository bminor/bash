/* anonfile.c - open and close temporary files (anonymous and memory-backed if possible). */

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

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include <bashtypes.h>

#if defined (HAVE_MEMFD_CREATE) || defined (HAVE_SHM_OPEN) || defined (HAVE_SHM_MKSTEMP)
#  include <sys/mman.h>
#endif
#include <filecntl.h>

#include <errno.h>

#include <shell.h>
#include <bashansi.h>

static int anonunlink (const char *);

#if defined (HAVE_MEMFD_CREATE) && !defined (MFD_NOEXEC_SEAL)
#  define MFD_NOEXEC_SEAL 0
#endif

#if defined (HAVE_SHM_OPEN)
#ifndef O_NOFOLLOW
#  define O_NOFOLLOW 0
#endif

static int
anonshmunlink (const char *fn)
{
  return (shm_unlink (fn));
}

static int
anonshmopen (const char *name, int flags, char **fn)
{
  int fd;
  char *fname;

  fd = -1;
  if (fn)
    *fn = 0;

#if defined (HAVE_SHM_MKSTEMP)
  fname = savestring ("/shm-XXXXXXXXXX");
  fd = shm_mkstemp (fname);
  if (fd < 0)
    free (fname);
#endif

  if (fd < 0)
    {
      fname = sh_mktmpname (name, flags);
      fd = shm_open (fname, O_RDWR|O_CREAT|O_EXCL|O_NOFOLLOW, 0600);
    }

  if (fd < 0)
    {
      free (fname);
      return fd;
    }

  if (shm_unlink (fname) < 0)
    {
      int o;
      o = errno;
      free (fname);
      close (fd);
      errno = o;
      return -1;
    }

  if (fn)
    *fn = fname;
  else
    free (fname);

  return fd;  
}
#endif

int
anonopen (const char *name, int flags, char **fn)
{
  int fd, flag;
  char *fname;

#if defined (HAVE_MEMFD_CREATE)
  /* "Names do not affect the behavior of the file descriptor." */
  fd = memfd_create ("anonopen", MFD_NOEXEC_SEAL);
  if (fd >= 0)
    {
      if (fn)
	*fn = 0;
      return fd;
    }
  /* If memfd_create fails, we fall through to the unlinked-shm-or-regular-file
     implementation. */
#endif

  /* Heuristic */
  flag = (name && *name == '/') ? MT_TEMPLATE : MT_USETMPDIR;

#if defined (HAVE_SHM_OPEN)
  fd = anonshmopen (name, flag, fn);
  if (fd >= 0)
    return fd;		/* anonshmopen sets *FN appropriately */
#endif

  fd = sh_mktmpfd (name, flag|MT_USERANDOM|MT_READWRITE|MT_UNLINK, fn);
  return fd;
}

int
anonclose (int fd, const char *name)
{
  int r;

  r = close (fd);
  return r;
}

static int
anonunlink (const char *fn)
{
  int r;

  r = unlink (fn);
  return r;
}
