/* error.c -- Functions for handling errors. */
/* Copyright (C) 1993 Free Software Foundation, Inc.

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
#include <fcntl.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (PREFER_STDARG)
#  include <stdarg.h>
#else
#  if defined (PREFER_VARARGS)
#    include <varargs.h>
#  endif
#endif

#include <stdio.h>

#include <errno.h>
#if !defined (errno)
extern int errno;
#endif /* !errno */

#include "bashansi.h"
#include "flags.h"
#include "error.h"
#include "command.h"
#include "general.h"
#include "externs.h"
#include "input.h"

#if defined (HISTORY)
#  include "bashhist.h"
#endif

extern int interactive_shell, interactive;
extern char *dollar_vars[];
extern char *shell_name;
#if defined (JOB_CONTROL)
extern pid_t shell_pgrp;
extern int give_terminal_to ();
#endif /* JOB_CONTROL */

/* The current maintainer of the shell.  You change this in the
   Makefile. */
#if !defined (MAINTAINER)
#define MAINTAINER "bash-maintainers@prep.ai.mit.edu"
#endif

char *the_current_maintainer = MAINTAINER;

/* Return the name of the shell or the shell script for error reporting. */
char *
get_name_for_error ()
{
  char *name;

  name = (char *)NULL;
  if (interactive_shell == 0)
    name = dollar_vars[0];
  if (name == 0 && shell_name && *shell_name)
    name = base_pathname (shell_name);
  if (name == 0)
#if defined (PROGRAM)
    name = PROGRAM;
#else
    name = "bash";
#endif

  return (name);
}

/* Report an error having to do with FILENAME.  This does not use
   sys_error so the filename is not interpreted as a printf-style
   format string. */
void
file_error (filename)
     char *filename;
{
  report_error ("%s: %s", filename, strerror (errno));
}

#if !defined (USE_VARARGS)
void
programming_error (reason, arg1, arg2, arg3, arg4, arg5)
     char *reason;
{
  char *h;

#if defined (JOB_CONTROL)
  give_terminal_to (shell_pgrp);
#endif /* JOB_CONTROL */

  report_error (reason, arg1, arg2);

#if defined (HISTORY)
  if (remember_on_history)
    {
      h = last_history_line ();
      fprintf (stderr, "last command: %s\n", h ? h : "(null)");
    }
#endif

  fprintf (stderr, "Report this to %s\n", the_current_maintainer);
  fprintf (stderr, "Stopping myself...");
  fflush (stderr);

  abort ();
}

void
report_error (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  fprintf (stderr, "%s: ", get_name_for_error ());

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");
  if (exit_immediately_on_error)
    exit (1);
}

void
parser_error (lineno, format, arg1, arg2, arg3, arg4, arg5);
     int lineno;
     char *format;
     va_dcl
{
  char *ename, *iname;

  ename = get_name_for_error ();
  iname = bash_input.name ? bash_input.name : "stdin";

  if (interactive)
    fprintf (stderr, "%s: ", ename);
  else if (interactive_shell)
    fprintf (stderr, "%s: %s: line %d: ", ename, iname, lineno);
  else if (STREQ (ename, iname))
    fprintf (stderr, "%s: line %d: ", ename, lineno);
  else
    fprintf (stderr, "%s: %s: line %d: ", ename, iname, lineno);

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");

  if (exit_immediately_on_error)
    exit (2);
}

void
fatal_error (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  fprintf (stderr, "%s: ", get_name_for_error ());

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");

  exit (2);
}

void
internal_error (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  fprintf (stderr, "%s: ", get_name_for_error ());

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");
}

void
internal_warning (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  fprintf (stderr, "%s: warning: ", get_name_for_error ());

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");
}

void
sys_error (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  fprintf (stderr, "%s: ", get_name_for_error ());

  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, ": %s\n", strerror (errno));
}

#else /* We have VARARGS support, so use it. */

void
#if defined (PREFER_STDARG)
programming_error (const char *format, ...)
#else
programming_error (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;
  char *h;

#if defined (JOB_CONTROL)
  give_terminal_to (shell_pgrp);
#endif /* JOB_CONTROL */

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);

#if defined (HISTORY)
  if (remember_on_history)
    {
      h = last_history_line ();
      fprintf (stderr, "last command: %s\n", h ? h : "(null)");
    }
#endif

  fprintf (stderr, "Report this to %s\n", the_current_maintainer);
  fprintf (stderr, "Stopping myself...");
  fflush (stderr);

  abort ();
}

void
#if defined (PREFER_STDARG)
report_error (const char *format, ...)
#else
report_error (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  fprintf (stderr, "%s: ", get_name_for_error ());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
  if (exit_immediately_on_error)
    exit (1);
}

void
#if defined (PREFER_STDARG)
fatal_error (const char *format, ...)
#else
fatal_error (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  fprintf (stderr, "%s: ", get_name_for_error ());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
  exit (2);
}

void
#if defined (PREFER_STDARG)
internal_error (const char *format, ...)
#else
internal_error (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  fprintf (stderr, "%s: ", get_name_for_error ());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
}

void
#if defined (PREFER_STDARG)
internal_warning (const char *format, ...)
#else
internal_warning (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  fprintf (stderr, "%s: warning: ", get_name_for_error ());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
}

void
#if defined (PREFER_STDARG)
sys_error (const char *format, ...)
#else
sys_error (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  fprintf (stderr, "%s: ", get_name_for_error ());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, ": %s\n", strerror (errno));

  va_end (args);
}

/* An error from the parser takes the general form

	shell_name: input file name: line number: message

   The input file name and line number are omitted if the shell is
   currently interactive.  If the shell is not currently interactive,
   the input file name is inserted only if it is different from the
   shell name. */
void
#if defined (PREFER_STDARG)
parser_error (int lineno, const char *format, ...)
#else
parser_error (lineno, format, va_alist)
     int lineno;
     const char *format;
     va_dcl
#endif
{
  va_list args;
  char *ename, *iname;

  ename = get_name_for_error ();
  iname = bash_input.name ? bash_input.name : "stdin";

  if (interactive)
    fprintf (stderr, "%s: ", ename);
  else if (interactive_shell)
    fprintf (stderr, "%s: %s: line %d: ", ename, iname, lineno);
  else if (STREQ (ename, iname))
    fprintf (stderr, "%s: line %d: ", ename, lineno);
  else
    fprintf (stderr, "%s: %s: line %d: ", ename, iname, lineno);

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);

  if (exit_immediately_on_error)
    exit (2);
}

void
#if defined (PREFER_STDARG)
itrace (const char *format, ...)
#else
itrace (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

  fprintf(stderr, "TRACE: pid %d: ", (int)getpid());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);

  fflush(stderr);
}

#if 0
/* A trace function for silent debugging -- doesn't require a control
   terminal. */
void
#if defined (PREFER_STDARG)
trace (const char *format, ...)
#else
trace (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;
  static FILE *tracefp = (FILE *)NULL;

  if (tracefp == NULL)
    tracefp = fopen("/usr/tmp/bash-trace.log", "a+");

  if (tracefp == NULL)
    tracefp = stderr;
  else
    fcntl (fileno (tracefp), F_SETFD, 1);     /* close-on-exec */

  fprintf(tracefp, "TRACE: pid %d: ", getpid());

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (tracefp, format, args);
  fprintf (tracefp, "\n");

  va_end (args);

  fflush(tracefp);
}
#endif /* 0 */

#endif /* USE_VARARGS */

static char *cmd_error_table[] = {
	"unknown command error",	/* CMDERR_DEFAULT */
	"bad command type",		/* CMDERR_BADTYPE */
	"bad connector",		/* CMDERR_BADCONN */
	"bad jump",			/* CMDERR_BADJUMP */
	0
};

void
command_error (func, code, e, flags)
     const char *func;
     int code, e, flags;	/* flags currently unused */
{
  if (code > CMDERR_LAST)
    code = CMDERR_DEFAULT;

  programming_error ("%s: %s: %d", func, cmd_error_table[code], e);
}

char *
command_errstr (code)
     int code;
{
  if (code > CMDERR_LAST)
    code = CMDERR_DEFAULT;

  return (cmd_error_table[code]);
}
