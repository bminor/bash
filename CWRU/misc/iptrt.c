#include <stdio.h>

main()
{
	if (sizeof(char *) == sizeof(long))
		printf("long\n");
	else if (sizeof(char *) == sizeof(short))
		printf("short\n");
	else
		printf("int\n");
	exit(0);
}
