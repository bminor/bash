/*
 * If this system has a BSD-style getpgrp() call which takes a pid
 * as an argument, output a -DBSD_GETPGRP.
 */
#include <stdio.h>
#include <sys/types.h>

int     pid;
int     pg1, pg2, pg3, pg4;
int     ng, np, s, child;

main()
{
	pid = getpid();
	pg1 = getpgrp(0);
	pg2 = getpgrp();
	pg3 = getpgrp(pid);
	pg4 = getpgrp(1);

	/*
	 * If all of these values are the same, it's pretty sure that
	 * we're on a system that ignores getpgrp's first argument.
	 */
	if (pg2 == pg4 && pg1 == pg3 && pg2 == pg3)
		exit(0);

	child = fork();
	if (child < 0)
		exit(1);
	else if (child == 0) {
		np = getpid();
		/*
		 * If this is Sys V, this will not work; pgrp will be
		 * set to np because setpgrp just changes a pgrp to be
		 * the same as the pid.
		 */
		setpgrp(np, pg1);
		ng = getpgrp(0);	/* Same result for Sys V and BSD */
		if (ng == pg1) {
			printf("-DBSD_GETPGRP\n");
			exit(0);
		} else
			exit(1);
	} else {
		wait(&s);
		exit(s>>8);
	}
}
