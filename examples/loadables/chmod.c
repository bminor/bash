/* chmod - change file mode bits */

/* See Makefile for compilation details. */

/*
   Copyright (C) 2024-2025 Free Software Foundation, Inc.

   This file is part of GNU Bash.
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

#include "bashtypes.h"
#include "posixstat.h"
#include <errno.h>
#include <stdio.h>
#include "bashansi.h"
#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

#if !defined (errno)
extern int errno;
#endif

#define ISOCTAL(c)	((c) >= '0' && (c) <= '7')

extern int parse_symbolic_mode (char *, mode_t);

#define STANDARD_BITS	(S_IRWXU | S_IRWXG | S_IRWXO)
#define ALLBITS (STANDARD_BITS | S_ISUID| S_ISGID | S_ISVTX)

int
chmod_builtin (WORD_LIST *list)
{
  int opt, nmode, lmode, rval;
  char *mode;
  struct stat st;
  WORD_LIST *l;

  reset_internal_getopt ();
  mode = (char *)NULL;
  while ((opt = internal_getopt(list, "fhvRHLP")) != -1)
    switch (opt)
      {
	CASE_HELPOPT;
	default:
	  return (EX_DISKFALLBACK);
      }
  list = loptend;

  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  mode = list->word->word;
  list = list->next;

  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  nmode = -1;
  if (ISOCTAL (*mode))	/* octal number */
    {
      nmode = read_octal (mode);
      if (nmode < 0)
	{
	  builtin_error ("invalid file mode: %s", mode);
	  return (EXECUTION_FAILURE);
	}
    }
  else 				/* test for valid symbolic mode */
    {
      /* initial bits are a=rwx; the mode argument modifies them */
      lmode = parse_symbolic_mode (mode, ALLBITS);
      if (lmode < 0)
	{
	  builtin_error ("invalid file mode: %s", mode);
	  return (EXECUTION_FAILURE);
	}
    }

  for (rval = EXECUTION_SUCCESS, l = list; l; l = l->next)
    {
      lmode = nmode;
      if (stat (l->word->word, &st) < 0)
	{
	  builtin_error ("`%s': cannot stat: %s", l->word->word, strerror (errno));
	  rval = EXECUTION_FAILURE;
	  continue;
	}

      if (lmode == -1)
	{
	  lmode = parse_symbolic_mode (mode, st.st_mode & ALLBITS);
	  if (lmode < 0)
	    {
	      builtin_error ("`%s': invalid file mode: %s", l->word->word, mode);
	      rval = EXECUTION_FAILURE;
	      continue;
	    }
	}

      if (chmod (l->word->word, lmode))
	{
	  builtin_error ("`%s': cannot change mode: %s", l->word->word, strerror (errno));
	  rval = EXECUTION_FAILURE;
	  continue;
	}
    }
  return rval;
}

char *chmod_doc[] = {
	"Change file mode bits.",
	"",
	"Change file mode bits.  Change the mode bits of files named as",
	"arguments, in the order specified, as specified by MODE."
	"The MODE argument may be an octal number or a symbolic mode like",
	"that described in chmod(1).  If a symbolic mode is used, the",
	"operations are interpreted relative to an initial mode of \"a=rwx\".",
	"",
	"The return value is 0 unless an error occurs.",
	(char *)NULL
};

struct builtin chmod_struct = {
	"chmod",
	chmod_builtin,
	BUILTIN_ENABLED,
	chmod_doc,
	"chmod [-R] mode file [file...]",
	0
};
