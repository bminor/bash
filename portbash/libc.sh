#! /bin/sh

CC=cc
export CC

cat > x.c <<EOF
extern char *alloca();

main()
{
	char *s;
	s = alloca(20);
}
EOF

if ${CC} x.c > /dev/null 2>&1; then
	:
else
	echo '#undef HAVE_ALLOCA'
fi
rm -f x.c x.o a.out

cat > x.c << EOF
#include <sys/types.h>
#include <sys/param.h>
extern char *getwd();
main()
{
	getwd();
}
EOF

if ${CC} x.c > /dev/null 2>&1; then
	echo '#define HAVE_GETWD'
else
	echo '#undef HAVE_GETWD'
	rm -f x.c x.o a.out

	cat > x.c << EOF
extern char *getcwd();

main()
{
	getcwd();
}
EOF

	if ${CC} x.c >/dev/null 2>&1; then
		echo '#define HAVE_GETCWD'
	fi
fi
rm -f a.out x.c x.o

cat > x.c << EOF
/*
 * exit 0 if we have bcopy in libc and it works as in BSD
 */

extern int bcopy();

char x[] = "12345";
char y[] = "67890";

main()
{
	bcopy(x, y, 5);
	exit(strcmp(x, y));
}
EOF

if ${CC} x.c > /dev/null 2>&1 && ./a.out ; then
	BC='-DHAVE_BCOPY'
fi

rm -f x.c x.o a.out

cat > x.c << EOF
/*
 * If this compiles, the system has uid_t and gid_t
 */

#include <sys/types.h>

uid_t	u;
gid_t	g;

main()
{
	exit(0);
}
EOF

if ${CC} x.c > /dev/null 2>&1; then
	UIDT='-DHAVE_UID_T'
fi

rm -f x.c x.o a.out

cat > x.c <<EOF
#include <signal.h>

extern char *sys_siglist[];

main()
{
	char *x;

	x = sys_siglist[3];
	write(2, x, strlen(x));
	exit(0);
}
EOF

if ${CC} ./x.c >/dev/null 2>&1; then
	echo '#define HAVE_SYS_SIGLIST'
else

	cat > x.c <<EOF
#include <signal.h>

extern char *_sys_siglist[];

main()
{
	exit(0);
}
EOF

	if ${CC} ./x.c >/dev/null 2>&1; then
		echo '#define HAVE_SYS_SIGLIST'
		SL='-Dsys_siglist=_sys_siglist'
	fi
fi

PG=
if ${CC} pgrp.c >/dev/null 2>&1; then
	PG=`./a.out`
fi

if [ -f /unix ] && [ -f /usr/ccs/lib/libc.so ]; then
	R4="-DUSGr4"
fi

touch not_a_directory
if [ -f /usr/include/dirent.h ]; then
	d='<dirent.h>'
else
	d='<sys/dir.h>'
fi

cat > x.c << EOF
/*
 * exit 0 if opendir does not check whether its argument is a directory
 */

#include $d
DIR *dir;

main()
{
	dir = opendir("not_a_directory");
	exit (dir == 0);
}
EOF

if ${CC} x.c > /dev/null 2>&1 && ./a.out ; then
	OD='-DOPENDIR_NOT_ROBUST'
fi

rm -f x.c x.o a.out pgrp.o not_a_directory
echo "#define SYSDEP_CFLAGS $BC $UIDT $SL $PG $R4 $OD"
exit 0
