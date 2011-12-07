/* necho - echo without options or argument interpretation */

/* Sample builtin to be dynamically loaded with enable -f and replace an
   existing builtin. */

#include <stdio.h>
#include "builtins.h"
#include "shell.h"

necho_builtin (list)
WORD_LIST *list;
{
	print_word_list (list, " ");
	printf("\n");
	fflush (stdout);
	return (EXECUTION_SUCCESS);
}

char *necho_doc[] = {
	"Print the arguments to the standard ouput separated",
	"by space characters and terminated with a newline.",
	(char *)NULL
};
	
struct builtin necho_struct = {
	"echo",
	necho_builtin,
	BUILTIN_ENABLED,
	necho_doc,
	"echo [args]",
	0
};
	
