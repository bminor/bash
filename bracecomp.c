/* bracecomp.c -- Complete a filename with the possible completions enclosed
   in csh-style braces such that the list of completions is available to the
   shell. */

/* Original version by tromey@cns.caltech.edu,  Fri Feb  7 1992. */

/* Copyright (C) 1993 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2, or (at your option) any later
   version.

   Bash is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
   for more details.

   You should have received a copy of the GNU General Public License along
   with Bash; see the file COPYING.  If not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include "config.h"
#if defined (BRACE_EXPANSION) && defined (READLINE)

#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include "bashansi.h"

#include "shell.h"
#include <readline/readline.h>

extern char *backslash_quote ();

/* Find greatest common prefix of two strings. */
static int
string_gcd (s1, s2)
     char *s1, *s2;
{
  register int i;

  if (s1 == NULL || s2 == NULL)
    return (0);

  for (i = 0; *s1 && *s2; ++s1, ++s2, ++i)
    {
      if (*s1 != *s2)
	break;
    }

  return (i);
}

static char *
really_munge_braces (array, real_start, real_end, gcd_zero)
     char **array;
     int real_start, real_end, gcd_zero;
{
  int start, end, gcd;
  char *result, *subterm, *x;
  int result_size, flag, tlen;

  flag = 0;

  if (real_start == real_end)
    {
      x = array[real_start] ? backslash_quote (array[real_start] + gcd_zero)
 			    : backslash_quote (array[0]);
      return x;
    }

  result = xmalloc (result_size = 16);
  *result = '\0';

  for (start = real_start; start < real_end; start = end + 1)
    {
      gcd = strlen (array[start]);
      for (end = start + 1; end < real_end; end++)
	{
	  int temp;

	  temp = string_gcd (array[start], array[end]);

	  if (temp <= gcd_zero)
	    break;

	  gcd = temp;
	}
      end--;

      if (gcd_zero == 0 && start == real_start && end != (real_end - 1))
	{
	  /* In this case, add in a leading '{', because we are at
	     top level, and there isn't a consistent prefix. */
	  result_size += 1;
	  result = xrealloc (result, result_size);
	  result[0] = '{'; result[1] = '\0';
	  flag++;
	}

      /* Make sure we backslash quote every substring we insert into the
	 resultant brace expression.  This is so the default filename
	 quoting function won't inappropriately quote the braces. */
      if (start == end)
	{
	  x = savestring (array[start] + gcd_zero);
	  subterm = backslash_quote (x);
	  free (x);
	}
      else
	{
	  /* If there is more than one element in the subarray,
	     insert the (quoted) prefix and an opening brace. */
	  tlen = gcd - gcd_zero;
	  x = xmalloc (tlen + 1);
	  strncpy (x, array[start] + gcd_zero, tlen);
	  x[tlen] = '\0';
	  subterm = backslash_quote (x);
	  free (x);
	  result_size += strlen (subterm) + 1;
	  result = xrealloc (result, result_size);
	  strcat (result, subterm);
	  free (subterm);
	  strcat (result, "{");
	  subterm = really_munge_braces (array, start, end + 1, gcd);
	  subterm[strlen (subterm) - 1] = '}';
	}

      result_size += strlen (subterm) + 1;
      result = xrealloc (result, result_size);
      strcat (result, subterm);
      strcat (result, ",");
      free (subterm);
    }

  if (gcd_zero == 0)
    result[strlen (result) - 1] = flag ? '}' : '\0';
  return (result);
}

static void
hack_braces_completion (names)
     char **names;
{
  register int i;
  char *temp;

  temp = really_munge_braces (names, 1, array_len (names), 0);

  for (i = 0; names[i]; ++i)
    {
      free (names[i]);
      names[i] = NULL;
    }
  names[0] = temp;
}

/* We handle quoting ourselves within hack_braces_completion, so we turn off
   rl_filename_quoting_desired and rl_filename_quoting_function. */
void
bash_brace_completion ()
{
  Function *orig_ignore_func;
  Function *orig_entry_func;
  CPFunction *orig_quoting_func;
  CPPFunction *orig_attempt_func;
  int orig_quoting_desired;

  orig_ignore_func = rl_ignore_some_completions_function;
  orig_attempt_func = rl_attempted_completion_function;
  orig_entry_func = rl_completion_entry_function;
  orig_quoting_func = rl_filename_quoting_function;
  orig_quoting_desired = rl_filename_quoting_desired;

  rl_completion_entry_function = (Function *) filename_completion_function;
  rl_attempted_completion_function = NULL;
  rl_ignore_some_completions_function = (Function *) hack_braces_completion;
  rl_filename_quoting_function = NULL;
  rl_filename_quoting_desired = 0;

  rl_complete_internal (TAB);

  rl_ignore_some_completions_function = orig_ignore_func;
  rl_attempted_completion_function = orig_attempt_func;
  rl_completion_entry_function = orig_entry_func;
  rl_filename_quoting_function = orig_quoting_func;
  rl_filename_quoting_desired = orig_quoting_desired;
}
#endif /* BRACE_EXPANSION && READLINE */
