/* error.c -- Functions for handling errors. */

/* Copyright (C) 1993-2022 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"
#include <fcntl.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdarg.h>
#include <stdio.h>

#include <errno.h>
#if !defined (errno)
extern int errno;
#endif /* !errno */

#include "bashansi.h"
#include "bashintl.h"

#include "shell.h"
#include "execute_cmd.h"
#include "flags.h"
#include "input.h"

#if defined (HISTORY)
#  include "bashhist.h"
#endif

extern int executing_line_number (void);

#if defined (JOB_CONTROL)
extern pid_t shell_pgrp;
extern int give_terminal_to (pid_t, int);
#endif /* JOB_CONTROL */

#if defined (ARRAY_VARS)
extern const char * const bash_badsub_errmsg;
#endif

static void error_prolog (int);

/* The current maintainer of the shell.  You change this in the
   Makefile. */
#if !defined (MAINTAINER)
#define MAINTAINER "bash-maintainers@gnu.org"
#endif

const char * const the_current_maintainer = MAINTAINER;

int gnu_error_format = 0;

static void
error_prolog (int print_lineno)
{
  char *ename;
  int line;

  ename = get_name_for_error ();
  line = (print_lineno && interactive_shell == 0) ? executing_line_number () : -1;

  if (line > 0)
    fprintf (stderr, "%s:%s%d: ", ename, gnu_error_format ? "" : _(" line "), line);
  else
    fprintf (stderr, "%s: ", ename);
}

/* Return the name of the shell or the shell script for error reporting. */
char *
get_name_for_error (void)
{
  char *name;
#if defined (ARRAY_VARS)
  SHELL_VAR *bash_source_v;
  ARRAY *bash_source_a;
#endif

  name = (char *)NULL;
  if (interactive_shell == 0)
    {
#if defined (ARRAY_VARS)
      bash_source_v = find_variable ("BASH_SOURCE");
      if (bash_source_v && array_p (bash_source_v) &&
	  (bash_source_a = array_cell (bash_source_v)))
	name = array_reference (bash_source_a, 0);
      if (name == 0 || *name == '\0')	/* XXX - was just name == 0 */
#endif
	name = dollar_vars[0];
    }
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
file_error (const char *filename)
{
  report_error ("%s: %s", filename, strerror (errno));
}

void
programming_error (const char *format, ...)
{
  va_list args;
  char *h;

#if defined (JOB_CONTROL)
  give_terminal_to (shell_pgrp, 0);
#endif /* JOB_CONTROL */

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
  va_end (args);

#if defined (HISTORY)
  if (remember_on_history)
    {
      h = last_history_line ();
      fprintf (stderr, _("last command: %s\n"), h ? h : "(null)");
    }
#endif

#if 0
  fprintf (stderr, "Report this to %s\n", the_current_maintainer);
#endif

  fprintf (stderr, _("Aborting..."));
  fflush (stderr);

  abort ();
}

/* Print an error message and, if `set -e' has been executed, exit the
   shell.  Used in this file by file_error and programming_error.  Used
   outside this file mostly to report substitution and expansion errors,
   and for bad invocation options. */
void
report_error (const char *format, ...)
{
  va_list args;

  error_prolog (1);

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
  if (exit_immediately_on_error)
    {
      if (last_command_exit_value == 0)
	last_command_exit_value = EXECUTION_FAILURE;
      exit_shell (last_command_exit_value);
    }
}

void
fatal_error (const char *format, ...)
{
  va_list args;

  error_prolog (0);

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
  sh_exit (2);
}

void
internal_error (const char *format, ...)
{
  va_list args;

  error_prolog (1);

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
}

void
internal_warning (const char *format, ...)
{
  va_list args;

  error_prolog (1);
  fprintf (stderr, _("warning: "));

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
}

void
internal_inform (const char *format, ...)
{
  va_list args;

  error_prolog (1);
  /* TRANSLATORS: this is a prefix for informational messages. */
  fprintf (stderr, _("INFORM: "));

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
}

void
internal_debug (const char *format, ...)
{
#ifdef DEBUG
  va_list args;

  error_prolog (1);
  fprintf (stderr, _("DEBUG warning: "));

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);
#else
  return;
#endif
}

void
sys_error (const char *format, ...)
{
  int e;
  va_list args;

  e = errno;
  error_prolog (0);

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, ": %s\n", strerror (e));

  va_end (args);
}

/* An error from the parser takes the general form

	shell_name: input file name: line number: message

   The input file name and line number are omitted if the shell is
   currently interactive.  If the shell is not currently interactive,
   the input file name is inserted only if it is different from the
   shell name. */
void
parser_error (int lineno, const char *format, ...)
{
  va_list args;
  char *ename, *iname;

  ename = get_name_for_error ();
  iname = yy_input_name ();

  if (interactive)
    fprintf (stderr, "%s: ", ename);
  else if (interactive_shell)
    fprintf (stderr, "%s: %s:%s%d: ", ename, iname, gnu_error_format ? "" : _(" line "), lineno);
  else if (STREQ (ename, iname))
    fprintf (stderr, "%s:%s%d: ", ename, gnu_error_format ? "" : _(" line "), lineno);
  else
    fprintf (stderr, "%s: %s:%s%d: ", ename, iname, gnu_error_format ? "" : _(" line "), lineno);

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);

  if (exit_immediately_on_error)
    exit_shell (last_command_exit_value = 2);
}

#ifdef DEBUG
/* This assumes ASCII and is suitable only for debugging */
char *
strescape (const char *str)
{
  char *r, *result;
  unsigned char *s;

  r = result = (char *)xmalloc (strlen (str) * 2 + 1);

  for (s = (unsigned char *)str; s && *s; s++)
    {
      if (*s < ' ')
	{
	  *r++ = '^';
	  *r++ = *s+64;
	}
      else if (*s == 127)
	{
	  *r++ = '^';
	  *r++ = '?';
	}
     else
	*r++ = *s;
    }

  *r = '\0';
  return result;
}

void
itrace (const char *format, ...)
{
  va_list args;

  fprintf(stderr, "TRACE: pid %ld: ", (long)getpid());

  va_start (args, format);

  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");

  va_end (args);

  fflush(stderr);
}

/* A trace function for silent debugging -- doesn't require a control
   terminal. */
void
trace (const char *format, ...)
{
  va_list args;
  static FILE *tracefp = (FILE *)NULL;

  if (tracefp == NULL)
    tracefp = fopen("/tmp/bash-trace.log", "a+");

  if (tracefp == NULL)
    tracefp = stderr;
  else
    fcntl (fileno (tracefp), F_SETFD, 1);     /* close-on-exec */

  fprintf(tracefp, "TRACE: pid %ld: ", (long)getpid());

  va_start (args, format);

  vfprintf (tracefp, format, args);
  fprintf (tracefp, "\n");

  va_end (args);

  fflush(tracefp);
}

#endif /* DEBUG */

/* **************************************************************** */
/*								    */
/*  		    Common error reporting			    */
/*								    */
/* **************************************************************** */


static const char * const cmd_error_table[] = {
	N_("unknown command error"),	/* CMDERR_DEFAULT */
	N_("bad command type"),		/* CMDERR_BADTYPE */
	N_("bad connector"),		/* CMDERR_BADCONN */
	N_("bad jump"),			/* CMDERR_BADJUMP */
	0
};

void
command_error (const char *func, int code, int e, int flags)
{
  if (code > CMDERR_LAST)
    code = CMDERR_DEFAULT;

  programming_error ("%s: %s: %d", func, _(cmd_error_table[code]), e);
}

char *
command_errstr (int code)
{
  if (code > CMDERR_LAST)
    code = CMDERR_DEFAULT;

  return (_(cmd_error_table[code]));
}

#ifdef ARRAY_VARS
void
err_badarraysub (const char *s)
{
  report_error ("%s: %s", s, _(bash_badsub_errmsg));
}
#endif

void
err_unboundvar (const char *s)
{
  report_error (_("%s: unbound variable"), s);
}

void
err_readonly (const char *s)
{
  report_error (_("%s: readonly variable"), s);
}

void
err_invalidid (const char *s)
{
  internal_error (_("`%s': not a valid identifier"), s);
}
