#include <stdio.h>

main()
{
	fprintf(stdout, "to stdout\n");
	fprintf(stderr, "to stderr\n");
	fprintf(stdout, "to stdout\n");
	fprintf(stderr, "to stderr\n");
	exit(0);
}

	
