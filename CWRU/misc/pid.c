#include <stdio.h>

main()
{
	fprintf(stderr, "%d\n", getpid());
	exit(0);
}
