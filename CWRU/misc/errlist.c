/*
 * If necessary, link with lib/sh/libsh.a
 */

#include <stdio.h>
#include <errno.h>

extern char *strerror();

extern int sys_nerr;

int
main(c, v)
int	c;
char	**v;
{
	int	i, n;

	if (c == 1) {
		for (i = 1; i < sys_nerr; i++)
			printf("%d --> %s\n", i, strerror(i));
	} else {
		for (i = 1; i < c; i++) {
			n = atoi(v[i]);
			printf("%d --> %s\n", n, strerror(n));
		}
	}
	exit (0);
}

programming_error(a, b)
char	*a;
int	b;
{
}

fatal_error()
{
}
