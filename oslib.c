/* oslib.c - functions present only in some unix versions. */

/* Copyright (C) 1995 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"
#include <sys/param.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "posixstat.h"
#include "filecntl.h"
#include "bashansi.h"

#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#include "shell.h"
#include "maxpath.h"

#if !defined (errno)
extern int errno;
#endif /* !errno */

/* A standard error message to use when getcwd() returns NULL. */
char *bash_getcwd_errstr = "getcwd: cannot access parent directories";

#if !defined (HAVE_SYSCONF) || !defined (_SC_CLK_TCK)
#  if !defined (CLK_TCK)
#    if defined (HZ)
#      define CLK_TCK HZ
#    else
#      define CLK_TCK 60
#    endif
#  endif /* !CLK_TCK */
#endif /* !HAVE_SYSCONF && !_SC_CLK_TCK */

long
get_clk_tck ()
{
  static long retval = 0;

  if (retval != 0)
    return (retval);

#if defined (HAVE_SYSCONF) && defined (_SC_CLK_TCK)
  retval = sysconf (_SC_CLK_TCK);
#else /* !SYSCONF || !_SC_CLK_TCK */
  retval = CLK_TCK;
#endif /* !SYSCONF || !_SC_CLK_TCK */

  return (retval);
}

/* Make the functions strchr and strrchr if they do not exist. */
#if !defined (HAVE_STRCHR)
char *
strchr (string, c)
     char *string;
     int c;
{
  register char *s;

  for (s = string; s && *s; s++)
    if (*s == c)
      return (s);

  return ((char *) NULL);
}

char *
strrchr (string, c)
     char *string;
     int c;
{
  register char *s, *t;

  for (s = string, t = (char *)NULL; s && *s; s++)
    if (*s == c)
      t = s;
  return (t);
}
#endif /* !HAVE_STRCHR */

#if !defined (HAVE_STRCASECMP)

#if !defined (to_lower)
#  define to_lower(c) (islower(c) ? (c) : tolower(c))
#endif /* to_lower */

/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int
strncasecmp (string1, string2, count)
     char *string1, *string2;
     int count;
{
  register char *s1, *s2;
  register int r;

  if (count > 0)
    {
      s1 = string1;
      s2 = string2;
      do
	{
	  if ((r = to_lower (*s1) - to_lower (*s2)) != 0)
	    return r;
	  if (*s1++ == '\0')
	    break;
	  s2++;
	}
      while (--count != 0);
    }
  return (0);
}

/* strcmp (), but caseless. */
int
strcasecmp (string1, string2)
     char *string1, *string2;
{
  register char *s1, *s2;
  register int r;

  s1 = string1;
  s2 = string2;

  while ((r = to_lower (*s1) - to_lower (*s2)) == 0)
    {
      if (*s1++ == '\0')
        return 0;
      s2++;
    }
  return (r);
}
#endif /* !HAVE_STRCASECMP */

/* Return a string corresponding to the error number E.  From
   the ANSI C spec. */
#if defined (strerror)
#  undef strerror
#endif

#if !defined (HAVE_STRERROR)
char *
strerror (e)
     int e;
{
  static char emsg[40];
#if defined (HAVE_SYS_ERRLIST)
  extern int sys_nerr;
  extern char *sys_errlist[];

  if (e > 0 && e < sys_nerr)
    return (sys_errlist[e]);
  else
#endif /* HAVE_SYS_ERRLIST */
    {
      sprintf (emsg, "Unknown error %d", e);
      return (&emsg[0]);
    }
}
#endif /* HAVE_STRERROR */

#if !defined (HAVE_DUP2) || defined (DUP2_BROKEN)
/* Replacement for dup2 (), for those systems which either don't have it,
   or supply one with broken behaviour. */
int
dup2 (fd1, fd2)
     int fd1, fd2;
{
  int saved_errno, r;

  /* If FD1 is not a valid file descriptor, then return immediately with
     an error. */
  if (fcntl (fd1, F_GETFL, 0) == -1)
    return (-1);

  if (fd2 < 0 || fd2 >= getdtablesize ())
    {
      errno = EBADF;
      return (-1);
    }

  if (fd1 == fd2)
    return (0);

  saved_errno = errno;

  (void) close (fd2);
  r = fcntl (fd1, F_DUPFD, fd2);

  if (r >= 0)
    errno = saved_errno;
  else
    if (errno == EINVAL)
      errno = EBADF;

  /* Force the new file descriptor to remain open across exec () calls. */
  SET_OPEN_ON_EXEC (fd2);
  return (r);
}
#endif /* !HAVE_DUP2 */

/*
 * Return the total number of available file descriptors.
 *
 * On some systems, like 4.2BSD and its descendents, there is a system call
 * that returns the size of the descriptor table: getdtablesize().  There are
 * lots of ways to emulate this on non-BSD systems.
 *
 * On System V.3, this can be obtained via a call to ulimit:
 *	return (ulimit(4, 0L));
 *
 * On other System V systems, NOFILE is defined in /usr/include/sys/param.h
 * (this is what we assume below), so we can simply use it:
 *	return (NOFILE);
 *
 * On POSIX systems, there are specific functions for retrieving various
 * configuration parameters:
 *	return (sysconf(_SC_OPEN_MAX));
 *
 */

#if !defined (HAVE_GETDTABLESIZE)
int
getdtablesize ()
{
#  if defined (_POSIX_VERSION) && defined (HAVE_SYSCONF) && defined (_SC_OPEN_MAX)
  return (sysconf(_SC_OPEN_MAX));	/* Posix systems use sysconf */
#  else /* ! (_POSIX_VERSION && HAVE_SYSCONF && _SC_OPEN_MAX) */
#    if defined (ULIMIT_MAXFDS)
  return (ulimit (4, 0L));	/* System V.3 systems use ulimit(4, 0L) */
#    else /* !ULIMIT_MAXFDS */
#      if defined (NOFILE)	/* Other systems use NOFILE */
  return (NOFILE);
#      else /* !NOFILE */
  return (20);			/* XXX - traditional value is 20 */
#      endif /* !NOFILE */
#    endif /* !ULIMIT_MAXFDS */
#  endif /* ! (_POSIX_VERSION && _SC_OPEN_MAX) */
}
#endif /* !HAVE_GETDTABLESIZE */

#if !defined (HAVE_BCOPY)
void
bcopy (s,d,n)
     char *d, *s;
     int n;
{
  FASTCOPY (s, d, n);
}
#endif /* !HAVE_BCOPY */

#if !defined (HAVE_BZERO)
void
bzero (s, n)
     char *s;
     int n;
{
  register int i;
  register char *r;

  for (i = 0, r = s; i < n; i++)
    *r++ = '\0';
}
#endif

#if !defined (HAVE_GETHOSTNAME)
#  if defined (HAVE_UNAME)
#    include <sys/utsname.h>
int
gethostname (name, namelen)
     char *name;
     int namelen;
{
  int i;
  struct utsname ut;

  --namelen;

  uname (&ut);
  i = strlen (ut.nodename) + 1;
  strncpy (name, ut.nodename, i < namelen ? i : namelen);
  name[namelen] = '\0';
  return (0);
}
#  else /* !HAVE_UNAME */
int
gethostname (name, namelen)
     int name, namelen;
{
  strncpy (name, "unknown", namelen);
  name[namelen] = '\0';
  return 0;
}
#  endif /* !HAVE_UNAME */
#endif /* !HAVE_GETHOSTNAME */

#if !defined (HAVE_KILLPG)
int
killpg (pgrp, sig)
     pid_t pgrp;
     int sig;
{
  return (kill (-pgrp, sig));
}
#endif /* !HAVE_KILLPG */


/* We supply our own version of getenv () because we want library
   routines to get the changed values of exported variables. */

/* The NeXT C library has getenv () defined and used in the same file.
   This screws our scheme.  However, Bash will run on the NeXT using
   the C library getenv (), since right now the only environment variable
   that we care about is HOME, and that is already defined.  */
#if defined (CAN_REDEFINE_GETENV)
static char *last_tempenv_value = (char *)NULL;
extern char **environ;

char *
getenv (name)
#if defined (__linux__) || defined (__bsdi__) || defined (convex)
     const char *name;
#else
     char const *name;
#endif /* !__linux__ && !__bsdi__ && !convex */
{
  SHELL_VAR *var;

  var = find_tempenv_variable ((char *)name);
  if (var)
    {
      FREE (last_tempenv_value);

      last_tempenv_value = savestring (value_cell (var));
      dispose_variable (var);
      return (last_tempenv_value);
    }
  else if (shell_variables)
    {
      var = find_variable ((char *)name);
      if (var && exported_p (var))
	return (value_cell (var));
    }
  else
    {
      register int i, len;

      /* In some cases, s5r3 invokes getenv() before main(); BSD systems
	 using gprof also exhibit this behavior.  This means that
	 shell_variables will be 0 when this is invoked.  We look up the
	 variable in the real environment in that case. */

      for (i = 0, len = strlen (name); environ[i]; i++)
	{
	  if ((STREQN (environ[i], name, len)) && (environ[i][len] == '='))
	    return (environ[i] + len + 1);
	}
    }

  return ((char *)NULL);
}

/* Some versions of Unix use _getenv instead. */
char *
_getenv (name)
#if defined (__linux__) || defined (__bsdi__) || defined (convex)
     const char *name;
#else
     char const *name;
#endif /* !__linux__ && !__bsdi__ && !convex */
{
  return (getenv (name));
}

#endif /* CAN_REDEFINE_GETENV */

#if !defined (HAVE_MKFIFO) && defined (PROCESS_SUBSTITUTION)
int
mkfifo (path, mode)
     char *path;
     int mode;
{
#if defined (S_IFIFO)
  return (mknod (path, (mode | S_IFIFO), 0));
#else /* !S_IFIFO */
  return (-1);
#endif /* !S_IFIFO */
}
#endif

#if !defined (HAVE_SETLINEBUF)
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
