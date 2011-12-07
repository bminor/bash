/* unlink - remove a directory entry */

/* Should only be used to remove directories by a superuser prepared to let
   fsck clean up the file system. */

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"

#ifndef errno
extern int errno;
#endif

unlink_builtin (list)
     WORD_LIST *list;
{
  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (unlink (list->word->word) != 0)
    {
      builtin_error ("%s: cannot unlink: %s", list->word->word, strerror (errno));
      return (EXECUTION_FAILURE);
    }

  return (EXECUTION_SUCCESS);
}

char *unlink_doc[] = {
	"Remove a directory entry.",
	(char *)NULL
};

struct builtin unlink_struct = {
	"unlink",		/* builtin name */
	unlink_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	unlink_doc,		/* array of long documentation strings. */
	"unlink name",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
