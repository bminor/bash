/* getenv.c - get environment variable value from the shell's variable
	      list. */

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

#if defined (CAN_REDEFINE_GETENV)

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <bashansi.h>
#include <shell.h>

extern char **environ;

/* We supply our own version of getenv () because we want library
   routines to get the changed values of exported variables. */

/* The NeXT C library has getenv () defined and used in the same file.
   This screws our scheme.  However, Bash will run on the NeXT using
   the C library getenv (), since right now the only environment variable
   that we care about is HOME, and that is already defined.  */
static char *last_tempenv_value = (char *)NULL;

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
