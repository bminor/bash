#include <unistd.h>
#include <limits.h>
#include <stdio.h>

#ifndef INTMAX_MAX
#  ifdef LLONG_MAX
#    define INTMAX_MAX LLONG_MAX
#  else
#    define INTMAX_MAX LONG_MAX
#  endif
#endif

main(int c, char **v)
{
	printf ("%d\n", INT_MAX);
	printf ("%ld\n", LONG_MAX);
	printf ("%lld\n", INTMAX_MAX);
	exit(0);
}
