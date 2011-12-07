#include <stdio.h>
#include <locale.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static void
usage()
{
	fprintf(stderr, "rangecmp: usage: rangecmp [-v] start test end\n");
}

int
main(c, v)
int	c;
char	**v;
{
	int	i, verbose, r1, r2;
	char	*dlocale;

	verbose = 0;
	while ((i = getopt(c, v, "v")) != -1) {
		switch (i) {
		case 'v':
			verbose = 1; break;
		case '?':
		default:
			usage();
			exit(2);
		}
	}
	c -= optind;
	v += optind;

	dlocale = setlocale(LC_ALL, "");
	if (verbose)
		printf("default locale = %s\n", dlocale ? dlocale : "''");
	r1 = strcoll (v[1], v[0]);
	printf("strcoll (%s, %s) -> %d\n", v[1], v[0], r1);
	r2 = strcoll (v[1], v[2]);
	printf("strcoll (%s, %s) -> %d\n", v[1], v[2], r2);

	exit(0);
}
