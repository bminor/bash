/* zecho - bare-bones echo */

#include <stdio.h>

int
main(argc, argv)
int	argc;
char	**argv;
{
	argv++;

	while (*argv) {
		(void)printf("%s", *argv);
		if (*++argv)
			putchar(' ');
	}

	putchar('\n');
	exit(0);
}
