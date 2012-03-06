#include <stdio.h>

main()
{

	union {
		int l;
		char c[sizeof(int)];
	} u;
	int x0, x1, x2, x3;

	u.l = 0x012345678;
	x0 = u.c[0];
	x3 = u.c[sizeof (int) - 1];
	printf ("x0 = 0x%x x3 = 0x%x (%s)\n", x0, x3, x3 == 0x78 ? "bigendian" : "littleendian");
	x0 = (u.l >> 24) & 0xff;
	x1 = (u.l >> 16) & 0xff;
	x2 = (u.l >> 8)  & 0xff;
	x3 = u.l & 0xff;
	printf ("big endian x0:x3: %x %x %x %x\n", x0, x1, x2, x3);
}
