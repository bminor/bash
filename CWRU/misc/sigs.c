#include <signal.h>
#include <stdio.h>

extern char	*sys_siglist[];

typedef void	sighandler();

main(argc, argv)
int	argc;
char	**argv;
{
	register int	i;
	sighandler	*h;

	for (i = 1; i < NSIG; i++) {
		h = signal(i, SIG_DFL);
		if (h != SIG_DFL) {
			if (h == SIG_IGN)
				fprintf(stderr, "%d: ignored (%s)\n", i, sys_siglist[i]);
			else
				fprintf(stderr, "%d: caught (%s)\n", i, sys_siglist[i]);
		}
	}
	exit(0);
}

		
