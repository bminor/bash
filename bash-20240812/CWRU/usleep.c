#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

char *progname;

int
main(int argc, char **argv)
{
	unsigned long int length;

	length = 0;
	progname = argv[0];

	if (argc != 2) {
		fprintf(stderr, "%s: must be given a duration\n", progname);
		return 1;
	}

	length = strtol(argv[1], NULL, 10);
	if (length == LONG_MAX) {
		fprintf(stderr, "%s: number is too large: %s\n", progname, argv[1]);
       	        return 1;
        }

	usleep(length);

	return 0;
}
