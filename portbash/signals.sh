#! /bin/sh
#
CC=cc
export CC

cat > x.c <<EOF
#include <signal.h>

main()
{
}
EOF

${CC} -E x.c > x.i || { rm -f x.c x.i ; exit 1; }

if egrep 'void.*signal' x.i >/dev/null 2>&1
then
	echo '#define VOID_SIGHANDLER'
fi
rm -f x.c x.i

cat > x.c << EOF
#include <signal.h>
sigset_t set, oset;
main()
{
	sigemptyset(&set);
	sigemptyset(&oset);
	sigaddset(&set, 2);
	sigprocmask(SIG_BLOCK, &set, &oset);
}
EOF
if ${CC} x.c >/dev/null 2>&1; then
	echo '#define HAVE_POSIX_SIGNALS'
else
	cat > x.c << EOF
#include <signal.h>
main()
{
	long omask = sigblock(sigmask(2));
	sigsetmask(omask);
}
EOF
	if ${CC} x.c >/dev/null 2>&1; then
		echo '#define HAVE_BSD_SIGNALS'
	else
		cat > x.c << EOF
#include <signal.h>
main()
{
	int n;
	n = sighold(2);
	sigrelse(2);
}
EOF
		if ${CC} x.c >/dev/null 2>&1; then
			echo '#define HAVE_USG_SIGHOLD'
		fi
	fi
fi

rm -f x.c x.o a.out

exit 0
