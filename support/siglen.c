#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <signal.h>

int
main(int argc, char **argv)
{
	int	i, l, longest, verb;
	size_t	len;
	char	*msg;

	verb = (argc > 1 && argv[1] && strcmp (argv[1], "-l") == 0);

	longest = 0;
	for (i = 0; i < NSIG; i++) {
		msg = strsignal (i);
		len = msg ? strlen (msg) : 0;
		l = len;
		if (msg && verb)
			printf ("%d\t%s\n", l, msg);
		if (l > longest)
			longest = l;
	}
	printf ("%d\n", longest);
	exit (0);
}
