/*
 * malloc test program.
 *
 * to build:
 * 	make x.o
 * 	gcc -g -o x x.o xmalloc.o lib/malloc/libmalloc.a
 */

/* Copyright (C) 2002-2003 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>

#define USING_BASH_MALLOC
#define HAVE_STRINGIZE

#include "xmalloc.h"

static char	xp[64];

main(int c, char **v)
{
	char	*p;

	malloc_set_register(1);

#if 0
	free (xp);	/* free unallocated memory */
#endif

#if 0
	p = xrealloc(xp, 128);
#endif

#if 0
	/*
	 * request an amount so that the bucket changes when the malloc
	 * overhead is added to the requested size.  This had better
	 * succeed.
	 */
	p = xmalloc(25);
	p[0] = 'a';
	p[1] = '\0';
	free(p);	/* sanity check on underflow checks in free() */
#endif

#if 0
	p = xmalloc(28);
	/* This works for little-endian machines like i386. */
	p[-4] = '\7';	/* underflow */
	p = xrealloc(p, 128);	/* realloc corrupted memory */
#endif

#if 0
	p = xmalloc(28);
	/* This works for little-endian machines like i386. */
	p[-4] = '\7';	/* underflow */
	free(p);
#endif

#if 0
	p = xmalloc(2);
	free(p);
	free(p); 	/* duplicate free */
#endif

#if 0
	p = xmalloc(32);
	free(p);
	p = xrealloc(p, 128);	/* realloc freed mem */
#endif

#if 0
	p = xmalloc(64);
	p[64] = '\0';
	p = xrealloc(p, 128);	/* overflow */
#endif

#if 0
	p = xmalloc(64);
	p[64] = '\0';
	free(p);		/* overflow */
#endif

#if 0
	p = xmalloc(64);
	p[-1] = 'a';
	free (p);		/* underflow */
#endif

#if 0
	p = xmalloc(64);
	p[-1] = 'a';
	p = xrealloc(p, 129);	/* underflow */
#endif

	mregister_dump_table();
	exit(0);
}

void
fatal_error(const char *format, ...)
{
	va_list	args;

	fprintf(stderr, "malloc-test: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);
	exit(2);
}

void
programming_error(const char *format, ...)
{
	va_list	args;

	fprintf(stderr, "malloc-test: ");
	va_start(args, format);
	vfprintf(stderr, format, args);
	fprintf(stderr, "\n");
	va_end(args);

	abort();
}

int
signal_is_trapped(int n)
{
	return 0;
}
