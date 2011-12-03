#include <stdio.h>
#include <unistd.h>

main(c, v)
int	c;
char	**v;
{
	char	*p;
	long	l;

	p = v[1] ? v[1] : "/";
	l = pathconf (p, _PC_PIPE_BUF);
	printf ("%ld\n", l);
	exit(0);
}
