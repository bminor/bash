/*
 * Copyright (c) 1989, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <sys/types.h>

#include <errno.h>
#include <limits.h>
#include <stdio.h>

#include "bashansi.h"
#include "shell.h"
#include "builtins.h"
#include "stdc.h"
#include "bashgetopt.h"

#if !defined (errno)
extern int errno;
#endif

#define PF(f, func) { \
	if (fieldwidth) \
		if (precision) \
			(void)fprintf(ofp, f, fieldwidth, precision, func); \
		else \
			(void)fprintf(ofp, f, fieldwidth, func); \
	else if (precision) \
		(void)fprintf(ofp, f, precision, func); \
	else \
		(void)fprintf(ofp, f, func); \
}

static int	 asciicode __P((void));
static void	 escape __P((char *));
static int	 getchr __P((void));
static double	 getdouble __P((void));
static int	 getint __P((int *));
static int	 getlong __P((long *));
static char	*getstr __P((void));
static char	*mklong __P((char *, int));
static void	 usage __P((void));

static char **gargv;

int print_builtin ();
static int printf_main ();
static int printargs ();

static FILE *ofp;

extern char *ansicstr ();
extern char *single_quote ();
extern char **make_builtin_argv ();

extern char *this_command_name;

extern int optind;

static char *print_doc[] = {
  "Output the arguments.  The -f option means to use the argument as a",
  "format string as would be supplied to printf(1).  The rest of the",
  "options are as in ksh.",
  (char *)NULL
};

struct builtin print_struct = {
	"print",
	print_builtin,
	BUILTIN_ENABLED,
	print_doc,
	"print [-Rnprs] [-u unit] [-f format] [arguments]",
	(char *)0
};

#ifndef ISOPTION
#define ISOPTION(s, c)	(s[0] == '-' && s[2] == '\0' && s[1] == c)
#endif

int
print_builtin (list)
     WORD_LIST *list;
{
  int c, r, nflag, raw, ofd, sflag;
  char **v, *pfmt, *arg;
  WORD_LIST *l;

  nflag = raw = sflag = 0;
  ofd = 1;
  pfmt = 0;

  reset_internal_getopt ();
  while ((c = internal_getopt (list, "Rnprsu:f:")) != -1)
    {
      switch (c)
	{
	case 'R':
	  raw = 2;
	  loptend = lcurrent;
	  if (loptend && ISOPTION (loptend->word->word, 'n'))
	    {
	      loptend = loptend->next;
	      nflag = 1;
	    }
	  goto opt_end;
	case 'r':
	  raw = 1;
	  break;
	case 'n':
	  nflag = 1;
	  break;
	case 's':
	  sflag = 1;
	  break;
	case 'p':
	  break;	/* NOP */
	case 'u':
	  if (all_digits (list_optarg))
	    ofd = atoi (list_optarg);
	  else
	    {
	      for (l = list; l->next && l->next != lcurrent; l = l->next);
	      lcurrent = loptend = l;
	      goto opt_end;
	    }
	  break;
	case 'f':
	  pfmt = list_optarg;
	  break;
	default:
	  builtin_usage ();
	  return (EX_USAGE);
	}
    }

opt_end:
  list = loptend;

  ofp = (ofd == 1) ? stdout : fdopen (dup (ofd), "w");

  if (pfmt)
    {
      v = word_list_to_argv (list, 0, 2, &c);
      v[0] = this_command_name;
      v[1] = pfmt;
      r = printf_main (c, v);
      free (v);
      return r;
    }

  if (raw)
    {
      for (l = list; l; l = l->next)
	{
	  fprintf (ofp, "%s", l->word->word);
	  if (l->next)
	    fprintf (ofp, " ");
	}
      if (nflag == 0)
	fprintf (ofp, "\n");
      fflush (ofp);
      return (0);	
    }
        
  r = printargs (list, ofp);
  if (r && nflag == 0)
    fprintf (ofp, "\n");
  if (ofd != 1)
    fclose (ofp);
  return 0;
}

static int printargs (list, ofp)
     WORD_LIST *list;
     FILE *ofp;
{
  WORD_LIST *l;
  char *ostr;
  int sawc;

  for (sawc = 0, l = list; l; l = l->next)
    {
      ostr = ansicstr (l->word->word, strlen (l->word->word), &sawc);
      fprintf (ofp, "%s", ostr);
      free (ostr);
      if (sawc)
        return (0);
      if (l->next)
        fprintf (ofp, " ");
    }
  return (1);
}

static int
printf_main(argc, argv)
	int argc;
	char *argv[];
{
	static char *skip1, *skip2;
	int ch, end, fieldwidth, precision;
	char convch, nextch, *format, *fmt, *start;

	while ((ch = getopt(argc, argv, "")) != EOF)
		switch (ch) {
		case '?':
		default:
			usage();
			return (1);
		}
	argc -= optind;
	argv += optind;

	if (argc < 1) {
		usage();
		return (1);
	}

	/*
	 * Basic algorithm is to scan the format string for conversion
	 * specifications -- once one is found, find out if the field
	 * width or precision is a '*'; if it is, gather up value.  Note,
	 * format strings are reused as necessary to use up the provided
	 * arguments, arguments of zero/null string are provided to use
	 * up the format string.
	 */
	skip1 = "#-+ 0";
	skip2 = "*0123456789";

	escape(fmt = format = *argv);		/* backslash interpretation */
	gargv = ++argv;
	for (;;) {
		end = 0;
		/* find next format specification */
next:		for (start = fmt;; ++fmt) {
			if (!*fmt) {
				/* avoid infinite loop */
				if (end == 1) {
					warnx("missing format character",
					    NULL, NULL);
					return (1);
				}
				end = 1;
				if (fmt > start)
					(void)printf("%s", start);
				if (!*gargv)
					return (0);
				fmt = format;
				goto next;
			}
			/* %% prints a % */
			if (*fmt == '%') {
				if (*++fmt != '%')
					break;
				*fmt++ = '\0';
				(void)printf("%s", start);
				goto next;
			}
		}

		/* skip to field width */
		for (; strchr(skip1, *fmt); ++fmt);
		if (*fmt == '*') {
			if (getint(&fieldwidth))
				return (1);
		} else
			fieldwidth = 0;

		/* skip to possible '.', get following precision */
		for (; strchr(skip2, *fmt); ++fmt);
		if (*fmt == '.')
			++fmt;
		if (*fmt == '*') {
			if (getint(&precision))
				return (1);
		} else
			precision = 0;

		/* skip to conversion char */
		for (; strchr(skip2, *fmt); ++fmt);
		if (!*fmt) {
			warnx("missing format character", NULL, NULL);
			return (1);
		}

		convch = *fmt;
		nextch = *++fmt;
		*fmt = '\0';
		switch(convch) {
		case 'c': {
			char p;

			p = getchr();
			PF(start, p);
			break;
		}
		case 's': {
			char *p;

			p = getstr();
			PF(start, p);
			break;
		}
		case 'b': {		/* expand escapes in argument */
			char *p;

			p = getstr();
			escape(p);
			PF(start, p);
			break;
		}
		case 'q': {		/* print with shell single quoting */
			char *p, *p2;

			p = getstr();
			p2 = single_quote(p);
			PF(start, p2);
			free(p2);
			break;
		}
		case 'd': case 'i': case 'o': case 'u': case 'x': case 'X': {
			long p;
			char *f;
			
			if ((f = mklong(start, convch)) == NULL)
				return (1);
			if (getlong(&p))
				return (1);
			PF(f, p);
			break;
		}
		case 'e': case 'E': case 'f': case 'g': case 'G': {
			double p;

			p = getdouble();
			PF(start, p);
			break;
		}
		default:
			warnx("illegal format character", NULL, NULL);
			return (1);
		}
		*fmt = nextch;
	}
	/* NOTREACHED */
}

static char *
mklong(str, ch)
	char *str;
	int ch;
{
	static char copy[64];
	int len;

	len = strlen(str) + 2;
	memmove(copy, str, len - 3);
	copy[len - 3] = 'l';
	copy[len - 2] = ch;
	copy[len - 1] = '\0';
	return (copy);
}

static void
escape(fmt)
	register char *fmt;
{
	register char *store;
	register int value, c;

	for (store = fmt; c = *fmt; ++fmt, ++store) {
		if (c != '\\') {
			*store = c;
			continue;
		}
		switch (*++fmt) {
		case '\0':		/* EOS, user error */
			*store = '\\';
			*++store = '\0';
			return;
		case '\\':		/* backslash */
		case '\'':		/* single quote */
			*store = *fmt;
			break;
		case 'a':		/* bell/alert */
			*store = '\7';
			break;
		case 'b':		/* backspace */
			*store = '\b';
			break;
		case 'c':
			return;
		case 'e':
		case 'E':
			*store = '\033';
			break;
		case 'f':		/* form-feed */
			*store = '\f';
			break;
		case 'n':		/* newline */
			*store = '\n';
			break;
		case 'r':		/* carriage-return */
			*store = '\r';
			break;
		case 't':		/* horizontal tab */
			*store = '\t';
			break;
		case 'v':		/* vertical tab */
			*store = '\13';
			break;
					/* octal constant */
		case '0': case '1': case '2': case '3':
		case '4': case '5': case '6': case '7':
			for (c = 3, value = 0;
			    c-- && *fmt >= '0' && *fmt <= '7'; ++fmt) {
				value <<= 3;
				value += *fmt - '0';
			}
			--fmt;
			*store = value;
			break;
		default:
			*store = *fmt;
			break;
		}
	}
	*store = '\0';
}

static int
getchr()
{
	if (!*gargv)
		return ('\0');
	return ((int)**gargv++);
}

static char *
getstr()
{
	if (!*gargv)
		return ("");
	return (*gargv++);
}

static char *Number = "+-.0123456789";
static int
getint(ip)
	int *ip;
{
	long val;

	if (getlong(&val))
		return (1);
	if (val > INT_MAX) {
		warnx("%s: %s", *gargv, strerror(ERANGE));
		return (1);
	}
	*ip = val;
	return (0);
}

static int
getlong(lp)
	long *lp;
{
	long val;
	char *ep;

	if (!*gargv) {
		*lp = 0;
		return (0);
	}
	if (strchr(Number, **gargv)) {
		errno = 0;
		val = strtol(*gargv, &ep, 0);
		if (*ep != '\0') {
			warnx("%s: illegal number", *gargv, NULL);
			return (1);
		}
		if (errno == ERANGE)
			if (val == LONG_MAX) {
				warnx("%s: %s", *gargv, strerror(ERANGE));
				return (1);
			}
			if (val == LONG_MIN) {
				warnx("%s: %s", *gargv, strerror(ERANGE));
				return (1);
			}
			
		*lp = val;
		++gargv;
		return (0);
	}
	*lp =  (long)asciicode();
	return (0);
}

static double
getdouble()
{
	if (!*gargv)
		return ((double)0);
	if (strchr(Number, **gargv))
		return (atof(*gargv++));
	return ((double)asciicode());
}

static int
asciicode()
{
	register int ch;

	ch = **gargv;
	if (ch == '\'' || ch == '"')
		ch = (*gargv)[1];
	++gargv;
	return (ch);
}

static void
usage()
{
	(void)fprintf(stderr, "usage: print [-Rnprs] [-u unit] [-f format] [arg ...]\n");
}
