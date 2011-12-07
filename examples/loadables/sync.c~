/* sync - sync the disks by forcing pending filesystem writes to complete */

#include <config.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"

sync_builtin (list)
     WORD_LIST *list;
{
  sync();
  return (EXECUTION_SUCCESS);
}

char *sync_doc[] = {
	"force completion of pending disk writes",
	(char *)NULL
};

struct builtin sync_struct = {
	"sync",			/* builtin name */
	sync_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	sync_doc,		/* array of long documentation strings. */
	"sync",			/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
