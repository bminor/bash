/* bashgetopt.c -- `getopt' for use by the builtins. */

/* Copyright (C) 1992 Free Software Foundation, Inc.

This file is part of GNU Bash, the Bourne Again SHell.

Bash is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 1, or (at your option) any later
version.

Bash is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with Bash; see the file COPYING.  If not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include <config.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "../bashansi.h"
#include <errno.h>

#include "../shell.h"
#include "common.h"

#define ERR(S, C)	builtin_error("%s%c", (S), (C))

static int	sp;

char    *list_optarg;
int	list_optopt;

static WORD_LIST *lhead = (WORD_LIST *)NULL;
WORD_LIST	*lcurrent = (WORD_LIST *)NULL;
WORD_LIST	*loptend;	/* Points to the first non-option argument in the list */

int
internal_getopt(list, opts)
WORD_LIST	*list;
char		*opts;
{
	register int c;
	register char *cp;
	int	plus;	/* nonzero means to handle +option */

	if (*opts == '+') {
		plus = 1;
		opts++;
	} else
		plus = 0;

	if (list == 0) {
		list_optarg = (char *)NULL;
		loptend = (WORD_LIST *)NULL;	/* No non-option arguments */
		return -1;
	}

	if (list != lhead || lhead == 0) {
		/* Hmmm.... called with a different word list.  Reset. */
		sp = 1;
		lcurrent = lhead = list;
		loptend = (WORD_LIST *)NULL;
	}

	if (sp == 1) {
		if (lcurrent == 0 ||
		    (lcurrent->word->word[0] != '-' || lcurrent->word->word[1] == '\0')) {
		    	lhead = (WORD_LIST *)NULL;
		    	loptend = lcurrent;
			return(-1);
		} else if (lcurrent->word->word[0] == '-' &&
			   lcurrent->word->word[1] == '-' &&
			   lcurrent->word->word[2] == 0) {
			lhead = (WORD_LIST *)NULL;
			loptend = lcurrent->next;
			return(-1);
		}
	}

	list_optopt = c = lcurrent->word->word[sp];

	if (c == ':' || (cp = strchr(opts, c)) == NULL) {
		ERR("illegal option: -", c);
		if (lcurrent->word->word[++sp] == '\0') {
			lcurrent = lcurrent->next;
			sp = 1;
		}
		list_optarg = NULL;
		if (lcurrent)
			loptend = lcurrent->next;
		return('?');
	}

	if (*++cp == ':' || *cp == ';') {
		/* `:': Option requires an argument. */
		/* `;': option argument may be missing */
		/* We allow -l2 as equivalent to -l 2 */
		if (lcurrent->word->word[sp+1]) {
			list_optarg = lcurrent->word->word + sp + 1;
			lcurrent = lcurrent->next;
		/* If the specifier is `;', don't set optarg if the next
		   argument looks like another option. */
		} else if (lcurrent->next && (*cp == ':' || lcurrent->next->word->word[0] != '-')) {
			lcurrent = lcurrent->next;
			list_optarg = lcurrent->word->word;
			lcurrent = lcurrent->next;
		} else if (*cp == ';') {
			list_optarg = (char *)NULL;
			lcurrent = lcurrent->next;
		} else {	/* lcurrent->next == NULL */
			ERR("option requires an argument: -", c);
			sp = 1;
			list_optarg = (char *)NULL;
			return('?');
		}
		sp = 1;
	} else if (*cp == '#') {
		/* optional numeric argument */
		if (lcurrent->word->word[sp+1]) {
			if (digit(lcurrent->word->word[sp+1])) {
				list_optarg = lcurrent->word->word + sp + 1;
				lcurrent = lcurrent->next;
			} else
				list_optarg = (char *)NULL;
		} else {
			if (lcurrent->next && legal_number(lcurrent->next->word->word, (long *)0)) {
				lcurrent = lcurrent->next;
				list_optarg = lcurrent->word->word;
				lcurrent = lcurrent->next;
			} else
				list_optarg = (char *)NULL;
		}

	} else {
		/* No argument, just return the option. */
		if (lcurrent->word->word[++sp] == '\0') {
			sp = 1;
			lcurrent = lcurrent->next;
		}
		list_optarg = (char *)NULL;
	}

	return(c);
}

/*
 * reset_internal_getopt -- force the in[ft]ernal getopt to reset
 */

void
reset_internal_getopt ()
{
	lhead = lcurrent = loptend = (WORD_LIST *)NULL;
	sp = 1;
}

#ifdef INCLUDE_UNUSED
void
report_bad_option ()
{
	char s[3];

	s[0] = '-';
	s[1] = list_optopt;
	s[2] = '\0';
	bad_option (s);
}
#endif
