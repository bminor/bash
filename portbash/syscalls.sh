#! /bin/sh
CC=cc
export CC

cat > x.c << EOF
/*
 * exit 0 if we have the getgroups system or library call.
 */

main()
{
	int	g[100], ng;

	ng = getgroups(100, g);
	if (ng)
		exit(0);
	exit(1);
}
EOF
if ${CC} x.c > /dev/null 2>&1 && ./a.out ; then
	echo '#define HAVE_GETGROUPS'
fi
rm -f x.c x.o a.out

cat > x.c << EOF
extern int dup2();
main()
{
	exit(dup2(1, 2) == -1);
}
EOF

if ${CC} x.c > /dev/null 2>&1 && ./a.out ; then
	echo '#define HAVE_DUP2'
fi
rm -f a.out x.c x.o

cat > x.c << EOF
extern int getpageesize();
main()
{
	int n = getpagesize();
}
EOF

if ${CC} x.c > /dev/null 2>&1
then
	echo '#define HAVE_GETPAGESIZE'
fi
rm -f a.out x.c x.o

cat > x.c << EOF
extern int getdtablesize();
main()
{
	int n = getdtablesize();
}
EOF

if ${CC} x.c > /dev/null 2>&1
then
	echo '#define HAVE_GETDTABLESIZE'
fi
rm -f a.out x.c x.o

cat > x.c << EOF
extern int setdtablesize();
main()
{
	int n = setdtablesize(128);
}
EOF

if ${CC} x.c > /dev/null 2>&1
then
	echo '#define HAVE_SETDTABLESIZE'
fi
rm -f a.out x.c x.o

exit 0
