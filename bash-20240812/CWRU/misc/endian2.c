#include <stdio.h>
 
int
main()
{
	int i = 0x12345678;
	char *x;

	x = (char *)&i;
	printf ("0x%x\n", *x);
	printf ((*x == 0x78) ? "little endian\n" : "big endian\n");
	return 0;
}
