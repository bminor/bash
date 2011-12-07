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

#define RETURN(x) \
	do { \
		if (sp) *sp = sec; \
		if (usp) *usp = usec; \
		return (x); \
	} while (0)

#if defined (HAVE_TIMEVAL) && defined (HAVE_SELECT)
static int
fsleep(sec, usec)
long	sec, usec;
{
	struct timeval tv;

	tv.tv_sec = sec;
	tv.tv_usec = usec;

	return select(0, (fd_set *)0, (fd_set *)0, (fd_set *)0, &tv);
}
#else /* !HAVE_TIMEVAL || !HAVE_SELECT */
static int
fsleep(sec, usec)
long	sec, usec;
{
	if (usec >= 500000)	/* round */
		sec++;
	return (sleep(sec));
}
#endif /* !HAVE_TIMEVAL || !HAVE_SELECT */

/*
 * An incredibly simplistic floating point converter.
 */
static int multiplier[7] = { 1, 100000, 10000, 1000, 100, 10, 1 };

static int
convert(s, sp, usp)
char	*s;
long	*sp, *usp;
{
	int n;
	long sec, usec;
	char	*p;

	sec = usec = 0;

#define DECIMAL	'.'

	for (p = s; p && *p; p++) {
		if (*p == DECIMAL)		/* decimal point */
			break;
		if (DIGIT(*p) == 0)
			RETURN(0);
		sec = (sec * 10) + (*p - '0');
	}

	if (*p == 0)
		RETURN(1);

	if (*p == DECIMAL)
		p++;

	/* Look for up to six digits past a decimal point. */
	for (n = 0; n < 6 && p[n]; n++) {
		if (DIGIT(p[n]) == 0)
			RETURN(0);
		usec = (usec * 10) + (p[n] - '0');
	}

	/* Now convert to millionths */
	usec *= multiplier[n];

	if (n == 6 && p[6] >= '5' && p[6] <= '9')
		usec++;			/* round up 1 */

	RETURN(1);
}

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

    	if (convert(list->word->word, &sec, &usec)) {
		fsleep(sec, usec);
		return(EXECUTION_SUCCESS);
    	}

	builtin_error("%s: bad sleep interval", list->word->word);
	return (EXECUTION_FAILURE);
}

static char *sleep_doc[] = {
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
