/* tty - return terminal name */

/* See Makefile for compilation details. */

#include "config.h"

#include <stdio.h>
#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"

extern char *ttyname ();

tty_builtin (list)
     WORD_LIST *list;
{
  int opt, sflag;
  char *t;

  reset_internal_getopt ();
  sflag = 0;
  while ((opt = internal_getopt (list, "s")) != -1)
    {
      switch (opt)
	{
	case 's':
	  sflag = 1;
	  break;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }
  list = loptend;

  t = ttyname (0);
  if (sflag == 0)
    puts (t ? t : "not a tty");
  return (t ? EXECUTION_SUCCESS : EXECUTION_FAILURE);
}

char *tty_doc[] = {
	"tty writes the name of the terminal that is opened for standard",
	"input to standard output.  If the `-s' option is supplied, nothing",
	"is written; the exit status determines whether or not the standard",
	"input is connected to a tty.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures. */
struct builtin tty_struct = {
	"tty",			/* builtin name */
	tty_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	tty_doc,		/* array of long documentation strings. */
	"tty [-s]",		/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
