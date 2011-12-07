/*
 * sleep -- sleep for fractions of a second
 *
 * usage: sleep seconds[.fraction]
 */
#include "config.h"

#include "bashtypes.h"

#if defined (TIME_WITH_SYS_TIME)
#  include <sys/time.h>
#  include <time.h>
#else
#  if defined (HAVE_SYS_TIME_H)
#    include <sys/time.h>
#  else
#    include <time.h>
#  endif
#endif

#if defined (HAVE_UNISTD_H)
#include <unistd.h>
#endif

#include <stdio.h>
#include "chartypes.h"

#include "shell.h"
#include "builtins.h"
#include "common.h"

#define RETURN(x) \
	do { \
		if (sp) *sp = sec; \
		if (usp) *usp = usec; \
		return (x); \
	} while (0)

int
sleep_builtin (list)
WORD_LIST	*list;
{
	long	sec, usec;

	if (list == 0) {
		builtin_usage();
		return(EX_USAGE);
	}

	if (*list->word->word == '-' || list->next) {
		builtin_usage ();
		return (EX_USAGE);
	}

    	if (uconvert(list->word->word, &sec, &usec)) {
		fsleep(sec, usec);
		return(EXECUTION_SUCCESS);
    	}

	builtin_error("%s: bad sleep interval", list->word->word);
	return (EXECUTION_FAILURE);
}

static char *sleep_doc[] = {
	"Suspend execution for specified period.",
	""
	"sleep suspends execution for a minimum of SECONDS[.FRACTION] seconds.",
	(char *)NULL
};

struct builtin sleep_struct = {
	"sleep",
	sleep_builtin,
	BUILTIN_ENABLED,
	sleep_doc,
	"sleep seconds[.fraction]",
	0
};
