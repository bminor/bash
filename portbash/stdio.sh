#! /bin/sh
#
# test certain aspects of stdio
CC=cc
export CC

cat > x.c << EOF
#include <stdio.h>
#include <varargs.h>

xp(va_alist)
va_dcl
{
	va_list args;
	va_start (args);
	vfprintf(stdout, "abcde", args);
}

main()
{
	xp();
	exit(0);
}
EOF

if ${CC} x.c >/dev/null 2>&1
then
	echo '#define HAVE_VFPRINTF'
	rm -f x.c x.o a.out
else

	cat > x.c << EOF
#include <stdio.h>

main()
{
	_doprnt();
}
EOF

	if ${CC} x.c >/dev/null 2>&1
	then
		echo '#define USE_VFPRINTF_EMULATION'
		rm -f x.c x.o a.out
	fi
fi

cat > x.c << EOF
#include <stdio.h>
main()
{
	setlinebuf(stdout);
}
EOF

if ${CC} x.c > /dev/null 2>&1
then
	rm -f x.c x.o a.out
	echo '#define HAVE_SETLINEBUF'
else
	# check for setvbuf
	# If this compiles, the system has setvbuf.  If this segfaults while
	# running, non-reversed systems get a seg violation

	cat > x.c << EOF
#include <stdio.h>

main()
{
	setvbuf(stdout, _IOLBF, (char *)0, BUFSIZ);	/* reversed */
	exit(0);		/* non-reversed systems segv */
}
EOF

	if ${CC} x.c >/dev/null 2>&1 ; then
		echo '#define HAVE_SETVBUF'
		if a.out; then
			:
		else
			rm -f core
			echo '#define REVERSED_SETVBUF_ARGS'
		fi
	fi
fi

rm -f x.c x.o a.out
exit 0
