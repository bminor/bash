/* GNU test program (ksb and mjb) */

/* Modified to run with the GNU shell Apr 25, 1988 by bfox. */

/* Copyright (C) 1987, 1988, 1989, 1990, 1991 Free Software Foundation, Inc.

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

/* Define PATTERN_MATCHING to get the csh-like =~ and !~ pattern-matching
   binary operators. */
/* #define PATTERN_MATCHING */

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif

#include <stdio.h>

#include "bashtypes.h"

#if defined (HAVE_LIMITS_H)
#  include <limits.h>
#else
#  include <sys/param.h>
#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if !defined (_POSIX_VERSION)
#  include <sys/file.h>
#endif /* !_POSIX_VERSION */
#include "posixstat.h"
#include "filecntl.h"

#include "shell.h"
#include "builtins/common.h"

#if !defined (STRLEN)
#  define STRLEN(s) ((s)[0] ? ((s)[1] ? ((s)[2] ? strlen(s) : 2) : 1) : 0)
#endif

#include <errno.h>
#if !defined (errno)
extern int errno;
#endif /* !errno */

#if !defined (STREQ)
#  define STREQ(a, b) ((a)[0] == (b)[0] && strcmp (a, b) == 0)
#endif /* !STREQ */

#if !defined (member)
#  define member(c, s) (int)((c) ? (char *)strchr ((s), (c)) : 0)
#endif /* !member */

#if !defined (R_OK)
#define R_OK 4
#define W_OK 2
#define X_OK 1
#define F_OK 0
#endif /* R_OK */

#define EQ	0
#define NE	1
#define LT	2
#define GT	3
#define LE	4
#define GE	5

#define NT	0
#define OT	1
#define EF	2

/* The following few defines control the truth and false output of each stage.
   TRUE and FALSE are what we use to compute the final output value.
   SHELL_BOOLEAN is the form which returns truth or falseness in shell terms.
   Default is TRUE = 1, FALSE = 0, SHELL_BOOLEAN = (!value). */
#define TRUE 1
#define FALSE 0
#define SHELL_BOOLEAN(value) (!(value))

static procenv_t test_exit_buf;
static int test_error_return;
#define test_exit(val) \
	do { test_error_return = val; longjmp (test_exit_buf, 1); } while (0)

#if defined (AFS)
  /* We have to use access(2) for machines running AFS, because it's
     not a Unix file system.  This may produce incorrect answers for
     non-AFS files.  I hate AFS. */
#  define EACCESS(path, mode)	access(path, mode)
#else
#  define EACCESS(path, mode)	test_eaccess(path, mode)
#endif /* AFS */

static int pos;		/* The offset of the current argument in ARGV. */
static int argc;	/* The number of arguments present in ARGV. */
static char **argv;	/* The argument list. */
static int noeval;

static int unop ();
static int binop ();
static int unary_operator ();
static int binary_operator ();
static int two_arguments ();
static int three_arguments ();
static int posixtest ();

static int expr ();
static int term ();
static int and ();
static int or ();

static void beyond ();

static void
test_syntax_error (format, arg)
     char *format, *arg;
{
  extern int interactive_shell;
  extern char *get_name_for_error ();
  if (interactive_shell == 0)
    fprintf (stderr, "%s: ", get_name_for_error ());
  fprintf (stderr, "%s: ", argv[0]);
  fprintf (stderr, format, arg);
  fprintf (stderr, "\n");
  fflush (stderr);
  test_exit (SHELL_BOOLEAN (FALSE));
}

/* A wrapper for stat () which disallows pathnames that are empty strings
   and handles /dev/fd emulation on systems that don't have it. */
static int
test_stat (path, finfo)
     char *path;
     struct stat *finfo;
{
  if (*path == '\0')
    {
      errno = ENOENT;
      return (-1);
    }
  if (path[0] == '/' && path[1] == 'd' && strncmp (path, "/dev/fd/", 8) == 0)
    {
#if !defined (HAVE_DEV_FD)
      long fd;
      if (legal_number (path + 8, &fd))
	return (fstat ((int)fd, finfo));
      else
	{
	  errno = EBADF;
	  return (-1);
	}
#else
  /* If HAVE_DEV_FD is defined, DEV_FD_PREFIX is defined also, and has a
     trailing slash.  Make sure /dev/fd/xx really uses DEV_FD_PREFIX/xx.
     On most systems, with the notable exception of linux, this is
     effectively a no-op. */
      char pbuf[32];
      strcpy (pbuf, DEV_FD_PREFIX);
      strcat (pbuf, path + 8);
      return (stat (pbuf, finfo));
#endif /* !HAVE_DEV_FD */
    }
  return (stat (path, finfo));
}

/* Do the same thing access(2) does, but use the effective uid and gid,
   and don't make the mistake of telling root that any file is
   executable. */
static int
test_eaccess (path, mode)
     char *path;
     int mode;
{
  struct stat st;

  if (test_stat (path, &st) < 0)
    return (-1);

  if (current_user.euid == 0)
    {
      /* Root can read or write any file. */
      if (mode != X_OK)
	return (0);

      /* Root can execute any file that has any one of the execute
	 bits set. */
      if (st.st_mode & S_IXUGO)
	return (0);
    }

  if (st.st_uid == current_user.euid)        /* owner */
    mode <<= 6;
  else if (group_member (st.st_gid))
    mode <<= 3;

  if (st.st_mode & mode)
    return (0);

  return (-1);
}

/* Increment our position in the argument list.  Check that we're not
   past the end of the argument list.  This check is supressed if the
   argument is FALSE.  Made a macro for efficiency. */
#define advance(f) do { ++pos; if (f && pos >= argc) beyond (); } while (0)
#define unary_advance() do { advance (1); ++pos; } while (0)

/*
 * beyond - call when we're beyond the end of the argument list (an
 *	error condition)
 */
static void
beyond ()
{
  test_syntax_error ("argument expected", (char *)NULL);
}

/* Syntax error for when an integer argument was expected, but
   something else was found. */
static void
integer_expected_error (pch)
     char *pch;
{
  test_syntax_error ("%s: integer expression expected", pch);
}

/*
 * term - parse a term and return 1 or 0 depending on whether the term
 *	evaluates to true or false, respectively.
 *
 * term ::=
 *	'-'('a'|'b'|'c'|'d'|'e'|'f'|'g'|'h'|'p'|'r'|'s'|'u'|'w'|'x') filename
 *	'-'('G'|'L'|'O'|'S') filename
 * 	'-t' [int]
 *	'-'('z'|'n') string
 *	string
 *	string ('!='|'='|'==') string
 *	<int> '-'(eq|ne|le|lt|ge|gt) <int>
 *	file '-'(nt|ot|ef) file
 *	'(' <expr> ')'
 * int ::=
 *	positive and negative integers
 */
static int
term ()
{
  int value;

  if (pos >= argc)
    beyond ();

  /* Deal with leading `not's. */
  if (argv[pos][0] == '!' && argv[pos][1] == '\0')
    {
      value = 0;
      while (pos < argc && argv[pos][0] == '!' && argv[pos][1] == '\0')
	{
	  advance (1);
	  value = 1 - value;
	}

      return (value ? !term() : term());
    }

  /* A paren-bracketed argument. */
  if (argv[pos][0] == '(' && argv[pos][1] == '\0')
    {
      advance (1);
      value = expr ();
      if (argv[pos] == 0)
        test_syntax_error ("`)' expected", (char *)NULL);
      else if (argv[pos][0] != ')' || argv[pos][1])
	test_syntax_error ("`)' expected, found %s", argv[pos]);
      advance (0);
      return (value);
    }

#if 1
  /* are there enough arguments left that this could be dyadic? */
  if ((pos + 3 <= argc) && binop (argv[pos + 1]))
    value = binary_operator ();
#else
  /* If this is supposed to be a binary operator, make sure there are
     enough arguments and fail if there are not. */
  if ((pos + 1 < argc) && binop (argv[pos+1]))
    {
      if (pos + 3 <= argc)
        value = binary_operator ();
      else
        beyond ();
    }
#endif

  /* Might be a switch type argument */
  else if (argv[pos][0] == '-' && argv[pos][2] == '\0')
    {
      if (unop (argv[pos][1]))
	value = unary_operator ();
      else
	test_syntax_error ("%s: unary operator expected", argv[pos]);
    }
  else
    {
      value = argv[pos][0] != '\0';
      advance (0);
    }

  return (value);
}

static int
filecomp (s, t, op)
     char *s, *t;
     int op;
{
  struct stat st1, st2;

  if (test_stat (s, &st1) < 0 || test_stat (t, &st2) < 0)
    return (FALSE);
  switch (op)
    {
    case OT: return (st1.st_mtime < st2.st_mtime);
    case NT: return (st1.st_mtime > st2.st_mtime);
    case EF: return ((st1.st_dev == st2.st_dev) && (st1.st_ino == st2.st_ino));
    }
  return (FALSE);
}

static int
arithcomp (s, t, op)
     char *s, *t;
     int op;
{
  long l, r;

  if (legal_number (s, &l) == 0)
    integer_expected_error (s);
  if (legal_number (t, &r) == 0)
    integer_expected_error (t);
  switch (op)
    {
    case EQ: return (l == r);
    case NE: return (l != r);
    case LT: return (l < r);
    case GT: return (l > r);
    case LE: return (l <= r);
    case GE: return (l >= r);
    }
  return (FALSE);
}

#if defined (PATTERN_MATCHING)
static int
patcomp (string, pat, op)
     char *string, *pat;
     int op;
{
  int m;

  m = fnmatch (pat, string, 0);
  switch (op)
    {
    case EQ: return (m == 0);
    case NE: return (m != 0);
    }
}
#endif /* PATTERN_MATCHING */

static int
binary_operator ()
{
  int value;
  char *w;

  w = argv[pos + 1];
  if (w[0] == '=' && (w[1] == '\0' || (w[1] == '=' && w[2] == '\0')))
    {
      value = STREQ (argv[pos], argv[pos + 2]);
      pos += 3;
      return (value);
    }
  if ((w[0] == '>' || w[0] == '<') && w[1] == '\0')
    {
      value = (w[0] == '>') ? strcmp (argv[pos], argv[pos + 2]) > 0
			    : strcmp (argv[pos], argv[pos + 2]) < 0;
      pos += 3;
      return (value);
    }
#if defined (PATTERN_MATCHING)
  if ((w[0] == '=' || w[0] == '!') && w[1] == '~' && w[2] == '\0')
    {
      value = patcomp (argv[pos], argv[pos + 2], w[0] == '=' ? EQ : NE);
      pos += 3;
      return (value);
    }
#endif
  if (w[0] == '!' && w[1] == '=' && w[2] == '\0')
    {
      value = STREQ (argv[pos], argv[pos + 2]) == 0;
      pos += 3;
      return (value);
    }

  if (w[0] != '-' || w[3] != '\0')
    {
      test_syntax_error ("%s: binary operator expected", w);
      /* NOTREACHED */
      return (FALSE);
    }

  w++;
  if (w[1] == 't')
    {
      switch (w[0])
	{
        case 'n': value = filecomp (argv[pos], argv[pos + 2], NT); break;
        case 'o': value = filecomp (argv[pos], argv[pos + 2], OT); break;
	case 'l': value = arithcomp (argv[pos], argv[pos + 2], LT); break;
	case 'g': value = arithcomp (argv[pos], argv[pos + 2], GT); break;
	default: test_syntax_error ("-%s: binary operator expected", w);
	}
    }
  else if (w[0] == 'e')
    {
      switch (w[1])
	{
	case 'q': value = arithcomp (argv[pos], argv[pos + 2], EQ); break;
	case 'f': value = filecomp (argv[pos], argv[pos + 2], EF); break;
	default: test_syntax_error ("-%s: binary operator expected", w);
	}
    }
  else if (w[1] == 'e')
    {
      switch (w[0])
	{
	case 'n': value = arithcomp (argv[pos], argv[pos + 2], NE); break;
	case 'g': value = arithcomp (argv[pos], argv[pos + 2], GE); break;
	case 'l': value = arithcomp (argv[pos], argv[pos + 2], LE); break;
	default: test_syntax_error ("-%s: binary operator expected", w);
	}
    }
  else
    test_syntax_error ("-%s: binary operator expected", w);

  pos += 3;
  return value;
}

static int
unary_operator ()
{
  long r;
  struct stat stat_buf;

  switch (argv[pos][1])
    {
    default:
      return (FALSE);

      /* All of the following unary operators use unary_advance (), which
	 checks to make sure that there is an argument, and then advances
	 pos right past it.  This means that pos - 1 is the location of the
	 argument. */

    case 'a':			/* file exists in the file system? */
    case 'e':
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0);

    case 'r':			/* file is readable? */
      unary_advance ();
      return (EACCESS (argv[pos - 1], R_OK) == 0);

    case 'w':			/* File is writeable? */
      unary_advance ();
      return (EACCESS (argv[pos - 1], W_OK) == 0);

    case 'x':			/* File is executable? */
      unary_advance ();
      return (EACCESS (argv[pos - 1], X_OK) == 0);

    case 'O':			/* File is owned by you? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      (uid_t) current_user.euid == (uid_t) stat_buf.st_uid);

    case 'G':			/* File is owned by your group? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      (gid_t) current_user.egid == (gid_t) stat_buf.st_gid);

    case 'f':			/* File is a file? */
      unary_advance ();
      if (test_stat (argv[pos - 1], &stat_buf) < 0)
	return (FALSE);

      /* -f is true if the given file exists and is a regular file. */
#if defined (S_IFMT)
      return (S_ISREG (stat_buf.st_mode) || (stat_buf.st_mode & S_IFMT) == 0);
#else
      return (S_ISREG (stat_buf.st_mode));
#endif /* !S_IFMT */

    case 'd':			/* File is a directory? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      (S_ISDIR (stat_buf.st_mode)));

    case 's':			/* File has something in it? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      stat_buf.st_size > (off_t) 0);

    case 'S':			/* File is a socket? */
#if !defined (S_ISSOCK)
      return (FALSE);
#else
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      S_ISSOCK (stat_buf.st_mode));
#endif /* S_ISSOCK */

    case 'c':			/* File is character special? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      S_ISCHR (stat_buf.st_mode));

    case 'b':			/* File is block special? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      S_ISBLK (stat_buf.st_mode));

    case 'p':			/* File is a named pipe? */
      unary_advance ();
#ifndef S_ISFIFO
      return (FALSE);
#else
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      S_ISFIFO (stat_buf.st_mode));
#endif /* S_ISFIFO */

    case 'L':			/* Same as -h  */
    case 'h':			/* File is a symbolic link? */
      unary_advance ();
#if !defined (S_ISLNK) || !defined (HAVE_LSTAT)
      return (FALSE);
#else
      return ((argv[pos - 1][0] != '\0') &&
	      (lstat (argv[pos - 1], &stat_buf) == 0) &&
	      S_ISLNK (stat_buf.st_mode));
#endif /* S_IFLNK && HAVE_LSTAT */

    case 'u':			/* File is setuid? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      (stat_buf.st_mode & S_ISUID) != 0);

    case 'g':			/* File is setgid? */
      unary_advance ();
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      (stat_buf.st_mode & S_ISGID) != 0);

    case 'k':			/* File has sticky bit set? */
      unary_advance ();
#if !defined (S_ISVTX)
      /* This is not Posix, and is not defined on some Posix systems. */
      return (FALSE);
#else
      return (test_stat (argv[pos - 1], &stat_buf) == 0 &&
	      (stat_buf.st_mode & S_ISVTX) != 0);
#endif

    case 't':	/* File fd is a terminal?  fd defaults to stdout. */
      advance (0);
      if (pos < argc && legal_number (argv[pos], &r))
	{
	  advance (0);
	  return (isatty ((int)r));
	}
      return (isatty (1));

    case 'n':			/* True if arg has some length. */
      unary_advance ();
      return (argv[pos - 1][0] != '\0');

    case 'z':			/* True if arg has no length. */
      unary_advance ();
      return (argv[pos - 1][0] == '\0');

    case 'o':
      unary_advance ();
      return (minus_o_option_value (argv[pos - 1]) == 1);
    }
}

/*
 * and:
 *	term
 *	term '-a' and
 */
static int
and ()
{
  int value, v2;

  value = term ();
  while (pos < argc && argv[pos][0] == '-' && argv[pos][1] == 'a' && !argv[pos][2])
    {
      advance (0);
      v2 = and ();
      return (value && v2);
    }
  return (value);
}

/*
 * or:
 *	and
 *	and '-o' or
 */
static int
or ()
{
  int value, v2;

  value = and ();
  while (pos < argc && argv[pos][0] == '-' && argv[pos][1] == 'o' && !argv[pos][2])
    {
      advance (0);
      v2 = or ();
      return (value || v2);
    }

  return (value);
}

/*
 * expr:
 *	or
 */
static int
expr ()
{
  if (pos >= argc)
    beyond ();

  return (FALSE ^ or ());		/* Same with this. */
}

/* Return TRUE if S is one of the test command's binary operators. */
static int
binop (s)
     char *s;
{
  char *t;

  if (s[0] == '=' && s[1] == '\0')
    return (1);		/* '=' */
  else if ((s[0] == '<' || s[0] == '>') && s[1] == '\0')  /* string <, > */
    return (1);
  else if ((s[0] == '=' || s[0] == '!') && s[1] == '=' && s[2] == '\0')
    return (1);		/* `==' and `!=' */
#if defined (PATTERN_MATCHING)
  else if (s[2] == '\0' && s[1] == '~' && (s[0] == '=' || s[0] == '!'))
    return (1);
#endif
  else if (s[0] != '-' || s[2] == '\0' || s[3] != '\0')
    return (0);
  else
    {
      t = s + 1;
      if (t[1] == 't')
	switch (t[0])
	  {
	    case 'n':		/* -nt */
	    case 'o':		/* -ot */
	    case 'l':		/* -lt */
	    case 'g':		/* -gt */
	      return (1);
	    default:
	      return (0);
	  }
      else if (t[0] == 'e')
	switch (t[1])
	  {
	    case 'q':		/* -eq */
	    case 'f':		/* -ef */
	      return (1);
	    default:
	      return (0);
	  }
      else if (t[1] == 'e')
	switch (t[0])
	  {
	    case 'n':		/* -ne */
	    case 'l':		/* -le */
	    case 'g':		/* -ge */
	      return (1);
	    default:
	      return (0);
	  }
      else
        return (0);
    }
}

/* Return non-zero if OP is one of the test command's unary operators. */
static int
unop (op)
     int op;
{
  switch (op)
    {
    case 'a': case 'b': case 'c': case 'd': case 'e':
    case 'f': case 'g': case 'h': case 'k': case 'n':
    case 'p': case 'r': case 's': case 't': case 'u':
    case 'w': case 'x': case 'z':
    case 'G': case 'L': case 'O': case 'S':
    case 'o':
      return (1);
    }
  return (0);
}

static int
two_arguments ()
{
  if (argv[pos][0] == '!' && argv[pos][1] == '\0')
    return (argv[pos + 1][0] == '\0');
  else if (argv[pos][0] == '-' && argv[pos][2] == '\0')
    {
      if (unop (argv[pos][1]))
	return (unary_operator ());
      else
	test_syntax_error ("%s: unary operator expected", argv[pos]);
    }
  else
    test_syntax_error ("%s: unary operator expected", argv[pos]);

  return (0);
}

#define ANDOR(s)  (s[0] == '-' && !s[2] && (s[1] == 'a' || s[1] == 'o'))

#define ONE_ARG_TEST(s)		((s)[0] != '\0')

static int
three_arguments ()
{
  int value;

  if (binop (argv[pos+1]))
    {
      value = binary_operator ();
      pos = argc;
    }
  else if (ANDOR (argv[pos+1]))
    {
      if (argv[pos+1][1] == 'a')
        value = ONE_ARG_TEST(argv[pos]) && ONE_ARG_TEST(argv[pos+2]);
      else
        value = ONE_ARG_TEST(argv[pos]) || ONE_ARG_TEST(argv[pos+2]);
      pos = argc;
    }
  else if (argv[pos][0] == '!' && !argv[pos][1])
    {
      advance (1);
      value = !two_arguments ();
    }
  else if (argv[pos][0] == '(' && argv[pos+2][0] == ')')
    {
      value = ONE_ARG_TEST(argv[pos+1]);
      pos = argc;
    }
  else
    test_syntax_error ("%s: binary operator expected", argv[pos+1]);

  return (value);
}

/* This is an implementation of a Posix.2 proposal by David Korn. */
static int
posixtest ()
{
  int value;

  switch (argc - 1)	/* one extra passed in */
    {
      case 0:
	value = FALSE;
	pos = argc;
	break;

      case 1:
	value = ONE_ARG_TEST(argv[1]);
	pos = argc;
	break;

      case 2:
	value = two_arguments ();
	pos = argc;
	break;

      case 3:
	value = three_arguments ();
	break;

      case 4:
	if (argv[pos][0] == '!' && argv[pos][1] == '\0')
	  {
	    advance (1);
	    value = !three_arguments ();
	    break;
	  }
	/* FALLTHROUGH */
      default:
	value = expr ();
    }

  return (value);
}

/*
 * [:
 *	'[' expr ']'
 * test:
 *	test expr
 */
int
test_command (margc, margv)
     int margc;
     char **margv;
{
  int value;

  int code;

  code = setjmp (test_exit_buf);

  if (code)
    return (test_error_return);

  argv = margv;

  if (margv[0] && margv[0][0] == '[' && margv[0][1] == '\0')
    {
      --margc;

      if (margc < 2)
	test_exit (SHELL_BOOLEAN (FALSE));

      if (margv[margc] && (margv[margc][0] != ']' || margv[margc][1]))
	test_syntax_error ("missing `]'", (char *)NULL);
    }

  argc = margc;
  pos = 1;

  if (pos >= argc)
    test_exit (SHELL_BOOLEAN (FALSE));

  noeval = 0;
  value = posixtest ();

  if (pos != argc)
    test_syntax_error ("too many arguments", (char *)NULL);

  test_exit (SHELL_BOOLEAN (value));
}
