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

#include <errno.h>
#include "shell.h"

#include "bashansi.h"

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

	if (!list) {
		list_optarg = (char *)NULL;
		loptend = (WORD_LIST *)NULL;	/* No non-option arguments */
		return -1;
	}

	if (list != lhead || !lhead) {
		/* Hmmm.... called with a different word list.  Reset. */
		sp = 1;
		lcurrent = lhead = list;
		loptend = (WORD_LIST *)NULL;
	}

	if (sp == 1) {
		if (!lcurrent ||
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

	if (*++cp == ':') {
		/* Option requires an argument. */
		/* We allow -l2 as equivalent to -l 2 */
		if (lcurrent->word->word[sp+1] != '\0') {
			list_optarg = &(lcurrent->word->word[sp+1]);
			lcurrent = lcurrent->next;
		} else if (lcurrent->next == NULL) {
			ERR("option requires an argument: -", c);
			sp = 1;
			list_optarg = (char *)NULL;
			return('?');
		} else {
			lcurrent = lcurrent->next;
			list_optarg = lcurrent->word->word;
			lcurrent = lcurrent->next;
		}
		sp = 1;
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

void
report_bad_option ()
{
	char s[3];

	s[0] = '-';
	s[1] = list_optopt;
	s[2] = '\0';
	bad_option (s);
}
