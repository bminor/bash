#! /bin/sh
CC=cc
export CC

if [ -f /usr/include/string.h ]; then
	STRINGH='<string.h>'
elif [ -f /usr/include/strings.h ]; then
	STRINGH='<strings.h>'
else
	exit 1
fi

cat > x.c << EOF
#include $STRINGH

#ifndef strchr
extern char *strchr();
#endif

char *x = "12345";

main()
{
	char	*s;

	s = strchr(x, '2');
	if (s)
		exit(0);
	exit(1);
}
EOF

if ${CC} x.c >/dev/null 2>&1
then
	if ./a.out
	then
		echo '#define HAVE_STRCHR'
	fi
fi

rm -f x.c x.o a.out

cat > x.c << EOF
extern char *strerror();

main()
{
	char	*s;

	s = strerror(2);
	if (s)
		exit(0);
	exit(1);
}
EOF

if ${CC} x.c >/dev/null 2>&1
then
	if ./a.out
	then
		echo '#define HAVE_STRERROR'
	fi
fi

rm -f x.c x.o a.out


cat > x.c << EOF

main()
{
	if (strcasecmp("abc", "AbC") == 0)
		exit(0);
	exit(1);
}
EOF

if ${CC} x.c >/dev/null 2>&1
then
	if ./a.out
	then
		echo '#define HAVE_STRCASECMP'
	fi
fi

rm -f x.c x.o a.out
exit 0
