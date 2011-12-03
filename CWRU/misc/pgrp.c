#include <stdio.h>
#include <sys/types.h>

int     pid;
int     pg1;

main()
{
	pid = getpid();
	pg1 = getpgrp(0);

	printf("pid = %d, pgrp = %d\n", pid, pg1);
}
