/*
 * shmatch.c -- shell interface to posix regular expression matching.
 */

/* Copyright (C) 2003-2022 Free Software Foundation, Inc.

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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#if defined (HAVE_POSIX_REGEXP)

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif

#include "bashansi.h"

#include <stdio.h>
#include <regex.h>

#include "shell.h"
#include "variables.h"
#include "externs.h"

extern int match_ignore_case;

#if defined (ARRAY_VARS)
extern SHELL_VAR *builtin_find_indexed_array (char *, int);
#endif

static char *
strregerror (int err, const regex_t *regex_p)
{
  char *str;
  size_t size;

  size = regerror (err, regex_p, (char *)0, 0);
  str = xmalloc (size);
  (void)regerror (err, regex_p, str, size);

  return str;
}

int
sh_regmatch (const char *string, const char *pattern, int flags, char **errbuf)
{
  regex_t regex = { 0 };
  regmatch_t *matches;
  int rflags, reg_err;
#if defined (ARRAY_VARS)
  SHELL_VAR *rematch;
  ARRAY *amatch;
  size_t subexp_ind, subexp_len;
  char *subexp_str;
#endif
  int result;

#if defined (ARRAY_VARS)
  rematch = (SHELL_VAR *)NULL;
#endif

  rflags = REG_EXTENDED;
  if (match_ignore_case)
    rflags |= REG_ICASE;
#if !defined (ARRAY_VARS)
  rflags |= REG_NOSUB;
#endif

  if (reg_err = regcomp (&regex, pattern, rflags))
    {
      if (errbuf)
	*errbuf = strregerror (reg_err, &regex);
      return 2;		/* flag for printing a warning here. */
    }

#if defined (ARRAY_VARS)
  matches = (regmatch_t *)malloc (sizeof (regmatch_t) * (regex.re_nsub + 1));
#else
  matches = NULL;
#endif

  /* man regexec: NULL PMATCH ignored if NMATCH == 0 */
  if (regexec (&regex, string, matches ? regex.re_nsub + 1 : 0, matches, 0))
    /* XXX - catch errors and fill in *errbuf here? */
    result = EXECUTION_FAILURE;
  else
    result = EXECUTION_SUCCESS;		/* match */

#if defined (ARRAY_VARS)
  subexp_len = strlen (string) + 10;
  subexp_str = malloc (subexp_len + 1);

  /* Store the parenthesized subexpressions in the array BASH_REMATCH.
     Element 0 is the portion that matched the entire regexp.  Element 1
     is the part that matched the first subexpression, and so on. */
#if 0
  /* This was the pre-bash-5.3 code. */
  unbind_global_variable_noref ("BASH_REMATCH");
  rematch = make_new_array_variable ("BASH_REMATCH");
#else
  rematch = builtin_find_indexed_array ("BASH_REMATCH", 1);
#endif
  amatch = rematch ? array_cell (rematch) : (ARRAY *)0;

  if (matches && amatch && (flags & SHMAT_SUBEXP) && result == EXECUTION_SUCCESS && subexp_str)
    {
      for (subexp_ind = 0; subexp_ind <= regex.re_nsub; subexp_ind++)
	{
	  memset (subexp_str, 0, subexp_len);
	  strncpy (subexp_str, string + matches[subexp_ind].rm_so,
		     matches[subexp_ind].rm_eo - matches[subexp_ind].rm_so);
	  array_insert (amatch, subexp_ind, subexp_str);
	}
    }

#if 0
  VSETATTR (rematch, att_readonly);
#endif

  free (subexp_str);
  free (matches);
#endif /* ARRAY_VARS */

  regfree (&regex);

  return result;
}

#endif /* HAVE_POSIX_REGEXP */
