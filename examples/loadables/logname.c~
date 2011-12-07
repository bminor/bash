/* logname - print login name of current user */

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdio.h>
#include <errno.h>

#include "builtins.h"
#include "shell.h"

#if !defined (errno)
extern int errno;
#endif

logname_builtin (list)
     WORD_LIST *list;
{
  char *np;

  if (no_options (list))
    return (EX_USAGE);

  np = getlogin ();
  if (np == 0)
    {
      builtin_error ("cannot find username: %s", strerror (errno));
      return (EXECUTION_FAILURE);
    }
  printf ("%s\n", np);
  return (EXECUTION_SUCCESS);
}

char *logname_doc[] = {
	"write the current user's login name to the standard output",
	"and exit.  logname ignores the LOGNAME and USER variables.",
	"logname ignores any non-option arguments.",
	(char *)NULL
};
	
struct builtin logname_struct = {
	"logname",
	logname_builtin,
	BUILTIN_ENABLED,
	logname_doc,
	"logname",
	0
};
	
