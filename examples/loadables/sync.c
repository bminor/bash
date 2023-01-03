/* sync - sync the disks by forcing pending filesystem writes to complete */

/*
   Copyright (C) 1999-2009,2022 Free Software Foundation, Inc.

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

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif
#include <errno.h>
#include <fcntl.h>

#include "builtins.h"
#include "shell.h"
#include "common.h"
#include "bashgetopt.h"

#ifndef errno
extern int errno;
#endif

int
sync_builtin (WORD_LIST *list)
{
  int fd, status;
  WORD_LIST *l;
  char *fn;

  if (no_options (list))
    return (EX_USAGE);
  list = loptend;

  if (list == 0)
    {  
      sync();
      return (EXECUTION_SUCCESS);
    }

  status = EXECUTION_SUCCESS;
  for (l = list; l; l = l->next)
    {
      fn = l->word->word;
      fd = open (fn, O_WRONLY);
      if (fd < 0)
	fd = open (fn, O_RDONLY);

      if (fd < 0)
	{
	  file_error (fn);
	  status = EXECUTION_FAILURE;
	  continue;
	}

      if (fsync (fd) < 0)
	{
	  builtin_error ("%s: cannot sync: %s", fn, strerror (errno));
	  status = EXECUTION_FAILURE;
	}
      close (fd);
    }

  return (status);
}

char *sync_doc[] = {
	"Sync disks or specified files.",
	"",
	"If one or more FILEs is supplied, force completion of pending writes",
	"to those files. Otherwise, force completion of any pending disk",
	"writes.",
	"",
	"Exit Status: zero unless any FILE could not be synced.",
	(char *)NULL
};

struct builtin sync_struct = {
	"sync",			/* builtin name */
	sync_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	sync_doc,		/* array of long documentation strings. */
	"sync [file ...]",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
