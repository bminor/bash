/* rmdir - remove directory */

/* See Makefile for compilation details. */

#include "config.h"

#include <stdio.h>
#include <errno.h>
#include "builtins.h"
#include "shell.h"

#if !defined (errno)
extern int errno;
#endif

rmdir_builtin (list)
     WORD_LIST *list;
{
  int rval;
  WORD_LIST *l;

  if (no_options (list))
    return (EX_USAGE);

  for (rval = EXECUTION_SUCCESS, l = list; l; l = l->next)
    if (rmdir (l->word->word) < 0)
      {
	builtin_error ("%s: %s", l->word->word, strerror (errno));
	rval = EXECUTION_FAILURE;
      }

  return rval;
}

char *rmdir_doc[] = {
	"rmdir removes the directory entry specified by each argument,",
	"provided the directory is empty.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin rmdir_struct = {
	"rmdir",		/* builtin name */
	rmdir_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	rmdir_doc,		/* array of long documentation strings. */
	"rmdir directory ...",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
