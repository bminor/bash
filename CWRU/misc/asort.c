#include <stdio.h>
#include <locale.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

static int
qsort_strcmp(s1, s2)
char	**s1, **s2;
{
	return (strcoll(*s1, *s2));
}

static void
usage()
{
	fprintf(stderr, "asort: usage: asort [-v] args\n");
}

int
main(c, v)
int	c;
char	**v;
{
	int	i, verbose;
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
	qsort(v, c, sizeof(char *), qsort_strcmp);
	for (i = 0; i < c; i++) {
		printf("%s ", v[i]);
	}
	printf("\n");
	exit(0);
}
