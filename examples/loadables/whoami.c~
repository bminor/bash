/*
 * whoami - print out username of current user
 */

#include <config.h>
#include <stdio.h>

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"

whoami_builtin (list)
     WORD_LIST *list;
{
  int opt;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "")) != -1)
    {
      switch (opt)
	{
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;
  if (list)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  if (current_user.user_name == 0)
    get_current_user_info ();
  printf ("%s\n", current_user.user_name);
  return (EXECUTION_SUCCESS);
}

char *whoami_doc[] = {
	"display name of current user",
	(char *)NULL
};

struct builtin whoami_struct = {
	"whoami",
	whoami_builtin,
	BUILTIN_ENABLED,
	whoami_doc,
	"whoami",
	0
};
