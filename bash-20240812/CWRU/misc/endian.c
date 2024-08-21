#include <stdio.h>

main()
{

union {
	long int l;
	char c[sizeof(long int)];
} u;
	int x0, x1, x2, x3;

	u.l = 1;
	x0 = u.c[0];
	x3 = u.c[sizeof (long int) - 1];
	printf ("x0 = %d x3 = %d (%s)\n", x0, x3, x3 == 1 ? "bigendian" : "littleendian");
	x0 = u.l >> 24;
	x1 = u.l >> 16;
	x2 = u.l >> 8;
	x3 = u.l & 0xff;
	printf ("x0:x3: %d %d %d %d\n", x0, x1, x2, x3);
}
