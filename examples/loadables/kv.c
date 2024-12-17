/* kv - process a series of lines containing key-value pairs and assign them
        to an associative array. */

/*
   Copyright (C) 2023 Free Software Foundation, Inc.

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

/* See Makefile for compilation details. */

#include <config.h>

#include <sys/types.h>
#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include "bashansi.h"
#include "posixstat.h"
#include <stdio.h>
#include <errno.h>

#include "loadables.h"

#ifndef errno
extern int errno;
#endif

#if defined (ARRAY_VARS)

#define KV_ARRAY_DEFAULT	"KV"

/* Split LINE into a key and value, with the delimiter between the key and
   value being a member of DSTRING. A sequence of one or more delimiters
   separates the key and value. Assign to associative array KV as if
   KV[key]=value without expansion. This does not treat single or double
   quotes specially, nor does it remove whitespace at the beginning or end
   of LINE. */
static int
kvsplit (SHELL_VAR *v, char *line, char *dstring)
{
  char *key, *value;
  size_t ind;

  key = line;
  value = 0;

  ind = (line && *line) ? strcspn (key, dstring) : 0;

  /* blank line or line starting with delimiter */
  if (ind == 0 || *key == 0)
    return 0;

  if (key[ind])
    {
      key[ind++] = '\0';
      value = key + ind;
      /* skip until non-delim; this allows things like key1 = value1 where delims = " =" */
      ind = strspn (value, dstring);
      value += ind;
    }
  else
    value = "";

  return (bind_assoc_variable (v, name_cell (v), savestring (key), value, 0) != 0);
}

int
kvfile (SHELL_VAR *v, int fd, char *delims, char *rs)
{
  ssize_t n;
  char *line;
  size_t llen;
  int unbuffered_read, nr;
  struct stat sb;

  nr = 0;
#ifndef __CYGWIN__
  /* We probably don't need to worry about setting this at all; we're not
     seeking back and forth yet. */
  if (*rs == '\n')
    unbuffered_read = (lseek (fd, 0L, SEEK_CUR) < 0) && (errno == ESPIPE);
  else
    unbuffered_read = (fstat (fd, &sb) != 0) || (S_ISREG (sb.st_mode) == 0);
#else
  unbuffered_read = 1;
#endif

  line = 0;
  llen = 0;

  zreset ();
  while ((n = zgetline (fd, &line, &llen, *rs, unbuffered_read)) != -1)
    {
      QUIT;
      if (line[n] == *rs)
	line[n] = '\0';		/* value doesn't include the record separator */
      nr += kvsplit (v, line, delims);
      free (line);
      line = 0;
      llen = 0;
    }

  QUIT;
  return nr;  
}
#endif

int
kv_builtin (WORD_LIST *list)
{
#if defined (ARRAY_VARS)
  int opt, rval, free_delims;
  char *array_name, *delims, *rs;
  SHELL_VAR *v;

  array_name = delims = rs = 0;
  rval = EXECUTION_SUCCESS;
  free_delims = 0;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "A:s:d:")) != -1)
    {
      switch (opt)
	{
	case 'A':
	  array_name = list_optarg;
	  break;
	case 's':
	  delims = list_optarg;
	  break;
	case 'd':
	  rs = list_optarg;
	  break;
	CASE_HELPOPT;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  if (list)
    {
      builtin_error ("too many arguments");
      builtin_usage ();
      return (EX_USAGE);
    }

  if (array_name == 0)
    array_name = KV_ARRAY_DEFAULT;

  if (valid_identifier (array_name) == 0)
    {
      sh_invalidid (array_name);
      return (EXECUTION_FAILURE);
    }

  if (delims == 0)
    {
      delims = getifs ();
      free_delims = 1;
    }
  if (rs == 0)
    rs = "\n";

  v = find_or_make_array_variable (array_name, 3);
  if (v == 0 || readonly_p (v) || noassign_p (v))
    {
      if (v && readonly_p (v))
	err_readonly (array_name);
      return (EXECUTION_FAILURE);
    }
  else if (assoc_p (v) == 0)
    {
      builtin_error ("%s: not an associative array", array_name);
      return (EXECUTION_FAILURE);
    }
  if (invisible_p (v))
    VUNSETATTR (v, att_invisible);
  assoc_flush (assoc_cell (v));

  rval = kvfile (v, 0, delims, rs);

  if (free_delims)
    free (delims);	/* getifs returns allocated memory */  
  return (rval > 0 ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
#else
  builtin_error ("arrays not available");
  return (EXECUTION_FAILURE);
#endif
}

/* Called when builtin is enabled and loaded from the shared object.  If this
   function returns 0, the load fails. */
int
kv_builtin_load (char *name)
{
  return (1);
}

/* Called when builtin is disabled. */
void
kv_builtin_unload (char *name)
{
}

char *kv_doc[] = {
	"Read key-value pairs into an associative array.",
	"",
	"Read delimiter-terminated records composed of a single key-value pair",
	"from the standard input and add the key and corresponding value",
	"to the associative array ARRAYNAME. The key and value are separated",
	"by a sequence of one or more characters in SEPARATORS. Records are",
	"terminated by the first character of RS, similar to the read and",
	"mapfile builtins.",
	"",
	"If SEPARATORS is not supplied, $IFS is used to separate the keys",
	"and values. If RS is not supplied, newlines terminate records.",
	"If ARRAYNAME is not supplied, \"KV\" is the default array name.",
	"",
	"Returns success if at least one key-value pair is stored in ARRAYNAME.",
	(char *)NULL
};

struct builtin kv_struct = {
	"kv",			/* builtin name */
	kv_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	kv_doc,		/* array of long documentation strings. */
	"kv [-A ARRAYNAME] [-s SEPARATORS] [-d RS]",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
