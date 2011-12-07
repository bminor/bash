#include <stdlib.h>
#include <stdio.h>
#include <errno.h>


#ifndef errno
extern int errno;
#endif

main(c, v)
int	c;
char	**v;
{
	double	dv, dv2;
	char	*sv, *ep;
	int	r;

	sv = "4.2";
	dv = 4.2;

	errno = 0;
	dv2 = strtod(sv, &ep);
	if (*ep)
		exit(1);
	else if (errno == ERANGE)
		exit(1);
	if (dv != dv2)
		exit(1);
	exit(0);
}

	
	
