/* pathexp.c -- The shell interface to the globbing library. */

/* Copyright (C) 1995-2024 Free Software Foundation, Inc.

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

#include "config.h"

#include "bashtypes.h"
#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "posixstat.h"
#include "stat-time.h"

#include "bashansi.h"

#include "shell.h"
#include "pathexp.h"
#include "flags.h"

#include "shmbutil.h"
#include "bashintl.h"


#include <glob/strmatch.h>

static int glob_name_is_acceptable (const char *);
static void ignore_globbed_names (char **, sh_ignore_func_t *);
static char *split_ignorespec (char *, int *);
static void sh_sortglob (char **);
	       
#include <glob/glob.h>

/* Control whether * matches .files in globbing. */
int glob_dot_filenames;

/* Control whether the extended globbing features are enabled. */
int extended_glob = EXTGLOB_DEFAULT;

/* Control enabling special handling of `**' */
int glob_star = 0;

/* Return nonzero if STRING has any unquoted special globbing chars in it.
   This is supposed to be called when pathname expansion is performed, so
   it implements the rules in Posix 2.13.3, specifically that an unquoted
   slash cannot appear in a bracket expression. */
int
unquoted_glob_pattern_p (char *string)
{
  register int c;
  char *send;
  int open;

  DECLARE_MBSTATE;

  open = 0;
  send = string + strlen (string);

  while (c = *string++)
    {
      switch (c)
	{
	case '?':
	case '*':
	  return (1);

	case '[':
	  open++;
	  continue;

	case ']':
	  if (open)		/* XXX - if --open == 0? */
	    return (1);
	  continue;

	case '/':
	  if (open)
	    open = 0;
	  continue;

	case '+':
	case '@':
	case '!':
	  if (extended_glob && *string == '(')	/*)*/
	    return (1);
	  continue;

	case '\\':
	  if (*string == CTLESC)
	    {
	      string++;
	      /* If the CTLESC was quoting a CTLESC, skip it so that it's not
		 treated as a quoting character */
	      if (*string == CTLESC)
		string++;
	    }
	  else
	  /*FALLTHROUGH*/
   	case CTLESC:
	  if (*string++ == '\0')
	    return (0);
	}

      /* Advance one fewer byte than an entire multibyte character to
	 account for the auto-increment in the loop above. */
#ifdef HANDLE_MULTIBYTE
      string--;
      ADVANCE_CHAR_P (string, send - string);
      string++;
#else
      ADVANCE_CHAR_P (string, send - string);
#endif
    }

  return (0);
}

/* Return 1 if C is a character that is `special' in a POSIX ERE and needs to
   be quoted to match itself. */
static inline int
ere_char (int c)
{
  switch (c)
    {
    case '.':
    case '[':
    case '\\':
    case '(':
    case ')':
    case '*':
    case '+':
    case '?':
    case '{':
    case '|':
    case '^':
    case '$':
      return 1;
    default: 
      return 0;
    }
  return (0);
}

/* This is only used to determine whether to backslash-quote a character. */
int
glob_char_p (const char *s)
{
  switch (*s)
    {
#if defined (EXTENDED_GLOB)
    case '+':
    case '@':
      return (s[1] == '('); /*)*/
    case '(':
    case '|':
    case ')':
#endif
    case '!':
    case '^':
    case '-':
    case '.':
    case ':':
    case '=':
    case '*':
    case '[':
    case ']':
    case '?':
    case '\\':
      return 1;
    }
  return 0;
}

static inline int
glob_quote_char (const char *s)
{
  return (glob_char_p (s) || (*s == '%') || (*s == '#'));
}

/* PATHNAME can contain characters prefixed by CTLESC; this indicates
   that the character is to be quoted.  We quote it here in the style
   that the glob library recognizes.  If flags includes QGLOB_CVTNULL,
   we change quoted null strings (pathname[0] == CTLNUL) into empty
   strings (pathname[0] == 0).  If this is called after quote removal
   is performed, (flags & QGLOB_CVTNULL) should be 0; if called when quote
   removal has not been done (for example, before attempting to match a
   pattern while executing a case statement), flags should include
   QGLOB_CVTNULL.  If flags includes QGLOB_CTLESC, we need to remove CTLESC
   quoting CTLESC or CTLNUL (as if dequote_string were called).  If flags
   includes QGLOB_FILENAME, appropriate quoting to match a filename should be
   performed.  QGLOB_REGEXP means we're quoting for a Posix ERE (for
   [[ string =~ pat ]]) and that requires some special handling. */
char *
quote_string_for_globbing (const char *pathname, int qflags)
{
  char *temp;
  register int i, j;
  int cclass, collsym, equiv, c, last_was_backslash;
  int savei, savej;
  unsigned char cc;

  temp = (char *)xmalloc (2 * strlen (pathname) + 1);

  if ((qflags & QGLOB_CVTNULL) && QUOTED_NULL (pathname))
    {
      temp[0] = '\0';
      return temp;
    }

  cclass = collsym = equiv = last_was_backslash = 0;
  for (i = j = 0; pathname[i]; i++)
    {
      /* Fix for CTLESC at the end of the string? */
      if (pathname[i] == CTLESC && pathname[i+1] == '\0')
	{
	  temp[j++] = pathname[i++];
	  break;
	}
      /* If we are parsing regexp, turn CTLESC CTLESC into CTLESC. It's not an
	 ERE special character, so we should just be able to pass it through. */
      else if ((qflags & (QGLOB_REGEXP|QGLOB_CTLESC)) && pathname[i] == CTLESC && (pathname[i+1] == CTLESC || pathname[i+1] == CTLNUL))
	{
	  i++;
	  temp[j++] = pathname[i];
	  continue;
	}
      else if (pathname[i] == CTLESC)
	{
convert_to_backslash:
	  cc = pathname[i+1];

	  if ((qflags & QGLOB_FILENAME) && pathname[i+1] == '/')
	    continue;

	  /* What to do if preceding char is backslash? */

	  /* We don't have to backslash-quote non-special ERE characters if
	     we're quoting a regexp. */
	  if (cc != CTLESC && (qflags & QGLOB_REGEXP) && ere_char (cc) == 0)
	    continue;

	  /* We don't have to backslash-quote non-special BRE characters if
	     we're quoting a glob pattern. */
	  if (cc != CTLESC && (qflags & QGLOB_REGEXP) == 0 && glob_quote_char (pathname+i+1) == 0)
	    continue;

	  /* If we're in a multibyte locale, don't bother quoting multibyte
	     characters. It matters if we're going to convert NFD to NFC on
	     macOS, and doesn't make a difference on other systems. */
	  if (cc != CTLESC && locale_utf8locale && UTF8_SINGLEBYTE (cc) == 0)
	    continue;	/* probably don't need to check for UTF-8 locale */

	  temp[j++] = '\\';
	  i++;
	  if (pathname[i] == '\0')
	    break;
	}
      else if ((qflags & QGLOB_REGEXP) && (i == 0 || pathname[i-1] != CTLESC) && pathname[i] == '[')	/*]*/
	{
	  temp[j++] = pathname[i++];	/* open bracket */
	  savej = j;
	  savei = i;
	  c = pathname[i++];	/* c == char after open bracket */
	  if (c == '^')		/* ignore pattern negation */
	    {
	      temp[j++] = c;
	      c = pathname[i++];
	    }
	  if (c == ']')		/* ignore right bracket if first char */
	    {
	      temp[j++] = c;
	      c = pathname[i++];
	    }
	  do
	    {
	      if (c == 0)
		goto endpat;
	      else if (c == CTLESC)
		{
		  /* skip c, check for EOS, let assignment at end of loop */
		  /* pathname[i] == backslash-escaped character */
		  if (pathname[i] == 0)
		    goto endpat;
		  temp[j++] = pathname[i++];
		}
	      else if (c == '[' && pathname[i] == ':')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  cclass = 1;
		}
	      else if (cclass && c == ':' && pathname[i] == ']')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  cclass = 0;
		}
	      else if (c == '[' && pathname[i] == '=')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  if (pathname[i] == ']')
		    temp[j++] = pathname[i++];		/* right brack can be in equiv */
		  equiv = 1;
		}
	      else if (equiv && c == '=' && pathname[i] == ']')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  equiv = 0;
		}
	      else if (c == '[' && pathname[i] == '.')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  if (pathname[i] == ']')
		    temp[j++] = pathname[i++];		/* right brack can be in collsym */
		  collsym = 1;
		}
	      else if (collsym && c == '.' && pathname[i] == ']')
		{
		  temp[j++] = c;
		  temp[j++] = pathname[i++];
		  collsym = 0;
		}
	      else
		temp[j++] = c;
	    }
	  while (((c = pathname[i++]) != ']') && c != 0);

	  /* If we don't find the closing bracket before we hit the end of
	     the string, rescan string without treating it as a bracket
	     expression (has implications for backslash and special ERE
	     chars) */
	  if (c == 0)
	    {
	      i = savei - 1;	/* -1 for autoincrement above */
	      j = savej;
	      continue;
	    }

	  temp[j++] = c;	/* closing right bracket */
	  i--;			/* increment will happen above in loop */
	  continue;		/* skip double assignment below */
	}
      else if (pathname[i] == '\\' && (qflags & QGLOB_REGEXP) == 0)
	{
	  /* XXX - if not quoting regexp, use backslash as quote char. Should
	     We just pass it through without treating it as special? That is
	     what ksh93 seems to do. */

	  /* If we want to pass through backslash unaltered, comment out these
	     lines. */
	  temp[j++] = '\\';

	  i++;
	  if (pathname[i] == '\0')
	    break;
	  /* If we are turning CTLESC CTLESC into CTLESC, we need to do that
	     even when the first CTLESC is preceded by a backslash. */
	  if ((qflags & QGLOB_CTLESC) && pathname[i] == CTLESC && (pathname[i+1] == CTLESC || pathname[i+1] == CTLNUL))
	    i++;	/* skip over the CTLESC */
	  else if ((qflags & QGLOB_CTLESC) && pathname[i] == CTLESC)
	    /* A little more general: if there is an unquoted backslash in the
	       pattern and we are handling quoted characters in the pattern,
	       convert the CTLESC to backslash and add the next character on
	       the theory that the backslash will quote the next character
	       but it would be inconsistent not to replace the CTLESC with
	       another backslash here. We can't tell at this point whether the
	       CTLESC comes from a backslash or other form of quoting in the
	       original pattern. */
	    goto convert_to_backslash;
	}
      else if (pathname[i] == '\\' && (qflags & QGLOB_REGEXP))
        last_was_backslash = 1;
#if 0
      /* TAG:bash-5.4 Takaaki Konno <re_c25@yahoo.co.jp> 6/23/2025 */
      else if (pathname[i] == CTLNUL && (qflags & QGLOB_CVTNULL)
				     && (qflags & QGLOB_CTLESC))
	/* If we have an unescaped CTLNUL in the string, and QFLAGS says
	   we want to remove those (QGLOB_CVTNULL) but the string is quoted
	   (QGLOB_CVTNULL and QGLOB_CTLESC), we need to remove it. This can
	   happen when the pattern contains a quoted null string adjacent
	   to non-null characters, and it is not removed by quote removal. */
	continue;
#endif

      temp[j++] = pathname[i];
    }
endpat:
  temp[j] = '\0';

  return (temp);
}

char *
quote_globbing_chars (const char *string)
{
  size_t slen;
  char *temp, *t;
  const char *s, *send;
  DECLARE_MBSTATE;

  slen = strlen (string);
  send = string + slen;

  temp = (char *)xmalloc (slen * 2 + 1);
  for (t = temp, s = string; *s; )
    {
      if (glob_char_p (s))
	*t++ = '\\';

      /* Copy a single (possibly multibyte) character from s to t,
	 incrementing both. */
      COPY_CHAR_P (t, s, send);
    }
  *t = '\0';
  return temp;
}

/* Call the glob library to do globbing on PATHNAME, honoring all the shell
   variables that control globbing. */
char **
shell_glob_filename (const char *pathname, int qflags)
{
  char *temp, **results;
  int gflags, quoted_pattern;

  noglob_dot_filenames = glob_dot_filenames == 0;

  temp = quote_string_for_globbing (pathname, QGLOB_FILENAME|qflags);
  gflags = glob_star ? GX_GLOBSTAR : 0;
  results = glob_filename (temp, gflags);
  free (temp);

  if (results && ((GLOB_FAILED (results)) == 0))
    {
      if (should_ignore_glob_matches ())
	ignore_glob_matches (results);
      if (results && results[0])
        sh_sortglob (results);
      else
	{
	  FREE (results);
	  results = (char **)&glob_error_return;
	}
    }

  return (results);
}

#if defined (READLINE) && defined (PROGRAMMABLE_COMPLETION)
char **
noquote_glob_filename (char *pathname)
{
  char **results;
  int gflags;

  noglob_dot_filenames = glob_dot_filenames == 0;
  gflags = glob_star ? GX_GLOBSTAR : 0;

  results = glob_filename (pathname, gflags);

  if (results && GLOB_FAILED (results))
    results = (char **)NULL;

  if (results && results[0])
    sh_sortglob (results);

  return (results);
}
#endif

/* Stuff for GLOBIGNORE. */

static struct ignorevar globignore =
{
  "GLOBIGNORE",
  (struct ign *)0,
  0,
  (char *)0,
  (sh_iv_item_func_t *)0,
};

/* Set up to ignore some glob matches because the value of GLOBIGNORE
   has changed.  If GLOBIGNORE is being unset, we also need to disable
   the globbing of filenames beginning with a `.'. */
void
setup_glob_ignore (const char *name)
{
  char *v;

  v = get_string_value (name);
  setup_ignore_patterns (&globignore);

  if (globignore.num_ignores)
    glob_dot_filenames = 1;
  else if (v == 0)
    glob_dot_filenames = 0;
}

int
should_ignore_glob_matches (void)
{
  return globignore.num_ignores;
}

/* Return 0 if NAME matches a pattern in the globignore.ignores list. */
static int
glob_name_is_acceptable (const char *name)
{
  struct ign *p;
  char *n;
  int flags;

  /* . and .. are never matched. We extend this to the terminal component of a
     pathname. */
  n = strrchr (name, '/');
  if (n == 0 || n[1] == 0)
    n = (char *)name;
  else
    n++;

  if (n[0] == '.' && (n[1] == '\0' || (n[1] == '.' && n[2] == '\0')))
    return (0);

  flags = FNM_PATHNAME | FNMATCH_EXTFLAG | FNMATCH_NOCASEGLOB;
  for (p = globignore.ignores; p->val; p++)
    {
      if (strmatch (p->val, (char *)name, flags) != FNM_NOMATCH)
	return (0);
    }
  return (1);
}

/* Internal function to test whether filenames in NAMES should be
   ignored.  NAME_FUNC is a pointer to a function to call with each
   name.  It returns non-zero if the name is acceptable to the particular
   ignore function which called _ignore_names; zero if the name should
   be removed from NAMES. */

static void
ignore_globbed_names (char **names, sh_ignore_func_t *name_func)
{
  char **newnames;
  size_t n, i;

  for (i = 0; names[i]; i++)
    ;
  newnames = strvec_create (i + 1);

  for (n = i = 0; names[i]; i++)
    {
      if ((*name_func) (names[i]))
	newnames[n++] = names[i];
      else
	free (names[i]);
    }

  newnames[n] = (char *)NULL;

  if (n == 0)
    {
      names[0] = (char *)NULL;
      free (newnames);
      return;
    }

  /* Copy the acceptable names from NEWNAMES back to NAMES and set the
     new array end. */
  for (n = 0; newnames[n]; n++)
    names[n] = newnames[n];
  names[n] = (char *)NULL;
  free (newnames);
}

void
ignore_glob_matches (char **names)
{
  if (globignore.num_ignores == 0)
    return;

  ignore_globbed_names (names, glob_name_is_acceptable);
}

static char *
split_ignorespec (char *s, int *ip)
{
  char *t;
  int n, i;

  if (s == 0)
    return 0;

  i = *ip;
  if (s[i] == 0)
    return 0;

  n = skip_to_delim (s, i, ":", SD_NOJMP|SD_EXTGLOB|SD_GLOB);
  t = substring (s, i, n);

  if (s[n] == ':')
    n++;  
  *ip = n;  
  return t;
}
  
void
setup_ignore_patterns (struct ignorevar *ivp)
{
  int numitems, maxitems, ptr;
  char *colon_bit, *this_ignoreval;
  struct ign *p;

  this_ignoreval = get_string_value (ivp->varname);

  /* If nothing has changed then just exit now. */
  if ((this_ignoreval && ivp->last_ignoreval && STREQ (this_ignoreval, ivp->last_ignoreval)) ||
      (!this_ignoreval && !ivp->last_ignoreval))
    return;

  /* Oops.  The ignore variable has changed.  Re-parse it. */
  ivp->num_ignores = 0;

  if (ivp->ignores)
    {
      for (p = ivp->ignores; p->val; p++)
	free(p->val);
      free (ivp->ignores);
      ivp->ignores = (struct ign *)NULL;
    }

  if (ivp->last_ignoreval)
    {
      free (ivp->last_ignoreval);
      ivp->last_ignoreval = (char *)NULL;
    }

  if (this_ignoreval == 0 || *this_ignoreval == '\0')
    return;

  ivp->last_ignoreval = savestring (this_ignoreval);

  numitems = maxitems = ptr = 0;

  while (colon_bit = split_ignorespec (this_ignoreval, &ptr))
    {
      if (numitems + 1 >= maxitems)
	{
	  maxitems += 10;
	  ivp->ignores = (struct ign *)xrealloc (ivp->ignores, maxitems * sizeof (struct ign));
	}
      ivp->ignores[numitems].val = colon_bit;
      ivp->ignores[numitems].len = strlen (colon_bit);
      ivp->ignores[numitems].flags = 0;
      if (ivp->item_func)
	(*ivp->item_func) (&ivp->ignores[numitems]);
      numitems++;
    }
  ivp->ignores[numitems].val = (char *)NULL;
  ivp->num_ignores = numitems;
}

/* Functions to handle sorting glob results in different ways depending on
   the value of the GLOBSORT variable. */

static int glob_sorttype = SORT_NONE;

static STRING_INT_ALIST sorttypes[] = {
  { "name",	SORT_NAME },
  { "size",	SORT_SIZE },
  { "mtime",	SORT_MTIME },
  { "atime",	SORT_ATIME },
  { "ctime",	SORT_CTIME },
  { "blocks",	SORT_BLOCKS },
  { "numeric",	SORT_NUMERIC },
  { "nosort",	SORT_NOSORT },
  { (char *)NULL,	-1 }
};

/* A subset of the fields in the posix stat struct -- the ones we need --
   normalized to using struct timespec. */
struct globstat {
  off_t size;
  struct timespec mtime;
  struct timespec atime;
  struct timespec ctime;
  int blocks;
};
  
struct globsort_t {
  char *name;
  struct globstat st;
};

static struct globstat glob_nullstat = { -1, { -1, -1 }, { -1, -1 }, { -1, -1 }, -1 };

static inline int
glob_findtype (char *t)
{
  int type;

  type = find_string_in_alist (t, sorttypes, 0);
  return (type == -1 ? SORT_NONE : type);
}

void
setup_globsort (const char *varname)
{
  char *val;
  int r, t;

  glob_sorttype = SORT_NONE;
  val = get_string_value (varname);
  if (val == 0 || *val == 0)
    return;

  t = r = 0;
  while (*val && whitespace (*val))
    val++;			/* why not? */
  if (*val == '+')
    val++;			/* allow leading `+' but ignore it */
  else if (*val == '-')
    {
      r = SORT_REVERSE;		/* leading `-' reverses sort order */
      val++;
    }

  if (*val == 0)
    {
      /* A bare `+' means the default sort by name in ascending order; a bare
         `-' means to sort by name in descending order. */
      glob_sorttype = SORT_NAME | r;
      return;
    }

  t = glob_findtype (val);
  /* any other value is equivalent to the historical behavior */
  glob_sorttype = (t == SORT_NONE) ? t : t | r;
}

static int
globsort_namecmp (char **s1, char **s2)
{
  return ((glob_sorttype < SORT_REVERSE) ? strvec_posixcmp (s1, s2) : strvec_posixcmp (s2, s1));
}

/* Generic transitive comparison of two numeric values for qsort */
/* #define GENCMP(a,b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0)) */
/* A clever idea from gnulib */
#define GENCMP(a,b) (((a) > (b)) - ((a) < (b)))

static int
globsort_sizecmp (struct globsort_t *g1, struct globsort_t *g2)
{
  int x;

  x = (glob_sorttype < SORT_REVERSE) ? GENCMP(g1->st.size, g2->st.size) : GENCMP(g2->st.size, g1->st.size);
  return (x == 0) ? (globsort_namecmp (&g1->name, &g2->name)) : x;
}

static int
globsort_timecmp (struct globsort_t *g1, struct globsort_t *g2)
{
  int t, x;
  struct timespec t1, t2;

  t = (glob_sorttype < SORT_REVERSE) ? glob_sorttype : glob_sorttype - SORT_REVERSE;
  if (t == SORT_MTIME)
    {
      t1 = g1->st.mtime;
      t2 = g2->st.mtime;
    }
  else if (t == SORT_ATIME)
    {
      t1 = g1->st.atime;
      t2 = g2->st.atime;
    }
  else
    {
      t1 = g1->st.ctime;
      t2 = g2->st.ctime;
    }

  x = (glob_sorttype < SORT_REVERSE) ? timespec_cmp (t1, t2) : timespec_cmp (t2, t1);
  return (x == 0) ? (globsort_namecmp (&g1->name, &g2->name)) : x;
}

static int
globsort_blockscmp (struct globsort_t *g1, struct globsort_t *g2)
{
  int x;

  x = (glob_sorttype < SORT_REVERSE) ? GENCMP(g1->st.blocks, g2->st.blocks) : GENCMP(g2->st.blocks, g1->st.blocks);
  return (x == 0) ? (globsort_namecmp (&g1->name, &g2->name)) : x;
}

static inline int
gs_checknum (char *string, intmax_t *val)
{
  int v;
  intmax_t i;

  v = all_digits (string);
  if (v)
    *val = strtoimax (string, (char **)NULL, 10);
  return v;
}

static int
globsort_numericcmp (struct globsort_t *g1, struct globsort_t *g2)
{
  intmax_t i1, i2;
  int v1, v2, x;

  /* like valid_number but doesn't allow leading/trailing whitespace or sign */
  v1 = gs_checknum (g1->name, &i1);
  v2 = gs_checknum (g2->name, &i2);

  if (v1 && v2)		/* both valid numbers */
    /* Don't need to fall back to name comparison here */
    return (glob_sorttype < SORT_REVERSE) ? GENCMP(i1, i2) : GENCMP(i2, i1);
  else if (v1 == 0 && v2 == 0)	/* neither valid numbers */
    return (globsort_namecmp (&g1->name, &g2->name));
  else if (v1 != 0 && v2 == 0)
    return (glob_sorttype < SORT_REVERSE) ? -1 : 1;
  else
    return (glob_sorttype < SORT_REVERSE) ? 1 : -1;
}

#undef GENCMP

static struct globsort_t *
globsort_buildarray (char **array, size_t len)
{
  struct globsort_t *ret;
  int i;
  struct stat st;

  ret = (struct globsort_t *)xmalloc (len * sizeof (struct globsort_t));

  for (i = 0; i < len; i++)
    {
      ret[i].name = array[i];
      if (stat (array[i], &st) != 0)
        ret[i].st = glob_nullstat;
      else
        {
          ret[i].st.size = st.st_size;
          ret[i].st.mtime = get_stat_mtime (&st);
          ret[i].st.atime = get_stat_atime (&st);
          ret[i].st.ctime = get_stat_ctime (&st);
          ret[i].st.blocks = st.st_blocks;
        }
    }

  return ret;
}  
          
static inline void
globsort_sortbyname (char **results)
{
  qsort (results, strvec_len (results), sizeof (char *), (QSFUNC *)globsort_namecmp);
}

static void
globsort_sortarray (struct globsort_t *garray, size_t len)
{
  int t;
  QSFUNC *sortfunc;

  t = (glob_sorttype < SORT_REVERSE) ? glob_sorttype : glob_sorttype - SORT_REVERSE;

  switch (t)
    {
    case SORT_SIZE:
      sortfunc = (QSFUNC *)globsort_sizecmp;
      break;
    case SORT_ATIME:
    case SORT_MTIME:
    case SORT_CTIME:
      sortfunc = (QSFUNC *)globsort_timecmp;
      break;
    case SORT_BLOCKS:
      sortfunc = (QSFUNC *)globsort_blockscmp;
      break;
    case SORT_NUMERIC:
      sortfunc = (QSFUNC *)globsort_numericcmp;
      break;
    default:
      internal_error (_("invalid glob sort type"));
      break;
    }

  qsort (garray, len, sizeof (struct globsort_t), sortfunc);
}

static void
sh_sortglob (char **results)
{
  size_t rlen;
  struct globsort_t *garray;

  if (glob_sorttype == SORT_NOSORT || glob_sorttype == (SORT_NOSORT|SORT_REVERSE))
    return;

  if (glob_sorttype == SORT_NONE || glob_sorttype == SORT_NAME)
    globsort_sortbyname (results);	/* posix sort */
  else if (glob_sorttype == (SORT_NAME|SORT_REVERSE))
    globsort_sortbyname (results);	/* posix sort reverse order */
  else
    {
      int i;

      rlen = strvec_len (results);
      /* populate an array of name/statinfo, sort it appropriately, copy the
	 names from the sorted array back to RESULTS, and free the array */
      garray = globsort_buildarray (results, rlen);
      globsort_sortarray (garray, rlen);
      for (i = 0; i < rlen; i++)
        results[i] = garray[i].name;
      free (garray);
    }
}
