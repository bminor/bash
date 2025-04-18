/* fltexpr.c -- floating-point arithmetic expression evaluation. */

/* A thinly-edited version of expr.c/builtins/let.def */

/* Copyright (C) 2025 Free Software Foundation, Inc.

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

/*
 All arithmetic is done as double-precision floating point numbers
 with some checking for overflow (though division by 0 is caught and
 flagged as an error).

 The following operators are handled, grouped into a set of levels in
 order of decreasing precedence.

	"id++", "id--"		[post-increment and post-decrement]
	"++id", "--id"		[pre-increment and pre-decrement]
	"-", "+"		[(unary operators)]
	"!"
	"**"			[(exponentiation)]
	"*", "/"
	"+", "-"
	"<=", ">=", "<", ">"
	"==", "!="
	"&&"
	"||"
	"expr ? expr : expr"
	"=", "*=", "/=", "+=", "-="
	,			[comma]

 This is a subset of the operators available for integer expressions.

 Sub-expressions within parentheses have a precedence level greater than
 all of the above levels and are evaluated first.  Within a single prece-
 dence group, evaluation is left-to-right, except for the arithmetic
 assignment operator (`='), which is evaluated right-to-left (as in C).

 The expression evaluator returns the value of the expression (assignment
 statements have as a value what is returned by the RHS).  The `fltexpr'
 builtin, on the other hand, returns 0 if the last expression evaluates to
 a non-zero, and 1 otherwise.

 Implementation is a recursive-descent parser.

 Chet Ramey
 chet.ramey@case.edu
*/

#include "config.h"

#include <stdio.h>
#include "bashansi.h"

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include <errno.h>
#include <float.h>
#include <math.h>

#include "chartypes.h"
#include "bashintl.h"

#include "loadables.h"

#include "arrayfunc.h"
#include "execute_cmd.h"
#include "flags.h"
#include "subst.h"
#include "typemax.h"		/* INTMAX_MAX, INTMAX_MIN */

typedef double sh_float_t;

/* Because of the $((...)) construct, expressions may include newlines.
   Here is a macro which accepts newlines, tabs and spaces as whitespace. */
#define cr_whitespace(c) (whitespace(c) || ((c) == '\n'))

/* Size be which the expression stack grows when necessary. */
#define EXPR_STACK_GROW_SIZE 10

/* Maximum amount of recursion allowed.  This prevents a non-integer
   variable such as "num=num+2" from infinitely adding to itself when
   "let num=num+2" is given. */
#define MAX_EXPR_RECURSION_LEVEL 1024

/* The Tokens.  Singing "The Lion Sleeps Tonight". */

#define EQEQ	1	/* "==" */
#define NEQ	2	/* "!=" */
#define LEQ	3	/* "<=" */
#define GEQ	4	/* ">=" */
#define STR	5	/* string */
#define NUM	6	/* number */
#define LAND	7	/* "&&" Logical AND */
#define LOR	8	/* "||" Logical OR */
#define OP_ASSIGN 11	/* op= expassign as in Posix.2 */
#define COND	12	/* exp1 ? exp2 : exp3 */
#define POWER	13	/* exp1**exp2 */
#define PREINC	14	/* ++var */
#define PREDEC	15	/* --var */
#define POSTINC	16	/* var++ */
#define POSTDEC	17	/* var-- */
#define FUNC	18	/* function call */
#define EQ	'='
#define GT	'>'
#define LT	'<'
#define PLUS	'+'
#define MINUS	'-'
#define MUL	'*'
#define DIV	'/'
#define NOT	'!'
#define LPAR	'('
#define RPAR	')'
#define QUES	'?'
#define COL	':'
#define COMMA	','

/* This should be the function corresponding to the operator with the
   lowest precedence. */
#define EXP_LOWEST	expcomma

#define SHFLOAT_STRLEN_BOUND	63		/* For now */
#define SHFLOAT_BUFSIZE_BOUND	(SHFLOAT_STRLEN_BOUND+1)

/* These are valid when sh_float_t == double */
#define SHFLOAT_MAX	DBL_MAX
#define SHFLOAT_MIN	DBL_MIN
#define SHFLOAT_DIG	DBL_DIG
#define SHFLOAT_MANT_DIG	DBL_MANT_DIG
#define SHFLOAT_LENGTH_MODIFIER	'l';
#define SHFLOAT_STRTOD		strtod

#ifndef M_EGAMMA
#define M_EGAMMA 0.57721566490153286060651209008240243
#endif

struct lvalue
{
  char *tokstr;		/* possibly-rewritten lvalue if not NULL */
  sh_float_t tokval;	/* expression evaluated value */
  SHELL_VAR *tokvar;	/* variable described by array or var reference */
  arrayind_t ind;	/* array index if not -1 */
};

/* A structure defining a single expression context. */
typedef struct
{
  int curtok, lasttok;
  char *expression, *tp, *lasttp;
  sh_float_t tokval;
  char *tokstr;
  int noeval;
  struct lvalue lval;
} FLTEXPR_CONTEXT;

static char	*expression;	/* The current expression */
static char	*tp;		/* token lexical position */
static char	*lasttp;	/* pointer to last token position */
static int	curtok;		/* the current token */
static int	lasttok;	/* the previous token */
static int	assigntok;	/* the OP in OP= */
static char	*tokstr;	/* current token string */
static sh_float_t	tokval;		/* current token value */
static int	noeval;		/* set to 1 if no assignment to be done */
static procenv_t evalbuf;

/* set to 1 if the expression has already been run through word expansion */
static int	already_expanded;

static struct lvalue curlval = {0, 0, 0, -1};
static struct lvalue lastlval = {0, 0, 0, -1};

/* Function equivalents for POSIX math.h macros. */
static int xfpclassify(sh_float_t d) { return fpclassify(d); }
static int xisinf(sh_float_t d) { return isinf(d); }
static int xisnan(sh_float_t d) { return isnan(d); }
static int xisnormal(sh_float_t d) { return isnormal(d); }
static int xisfinite(sh_float_t d) { return isfinite(d); }
static int xsignbit(sh_float_t d) { return signbit(d); }

static int xisgreater(sh_float_t d1, sh_float_t d2) { return isgreater(d1, d2); }
static int xisgreaterequal(sh_float_t d1, sh_float_t d2) { return isgreaterequal(d1, d2); }
static int xisless(sh_float_t d1, sh_float_t d2) { return isless(d1, d2); }
static int xislessequal(sh_float_t d1, sh_float_t d2) { return islessequal(d1, d2); }
static int xislessgreater(sh_float_t d1, sh_float_t d2) { return islessgreater(d1, d2); }
static int xisunordered(sh_float_t d1, sh_float_t d2) { return isunordered(d1, d2); }

static int xisinfinite(sh_float_t d) { return (fpclassify(d) == FP_INFINITE); }
static int xissubnormal(sh_float_t d) { return (fpclassify(d) == FP_SUBNORMAL); }
static int xiszero(sh_float_t d) { return (fpclassify(d) == FP_ZERO); }

/* Function replacements for some math functions that don't conform to the
   supported prototypes. */
static sh_float_t xscalbn(sh_float_t d1, sh_float_t d2) { int x = d2; return (scalbn (d1, x)); }
static sh_float_t xjn(sh_float_t d1, sh_float_t d2) { int x = d1; return (jn (x, d2)); }
static sh_float_t xyn(sh_float_t d1, sh_float_t d2) { int x = d1; return (yn (x, d2)); }
static sh_float_t xldexp(sh_float_t d1, sh_float_t d2) { int x = d2; return (ldexp (d1, x)); }

/* Some additional math functions that aren't in libm */
static sh_float_t xcot(sh_float_t d) { return (1.0 / tan(d)); }
static sh_float_t xcoth(sh_float_t d) { return (cosh(d) / sinh(d)); }

static sh_float_t xroundp(sh_float_t d1, sh_float_t d2)
{
  sh_float_t m, r;
  int prec = d2;

  m = pow(10.0, prec);
  r = round(d1 * m) / m;
  return r;
}

typedef int imathfunc1(sh_float_t);
typedef int imathfunc2(sh_float_t, sh_float_t);
typedef sh_float_t mathfunc1(sh_float_t);
typedef sh_float_t mathfunc2(sh_float_t, sh_float_t);
typedef sh_float_t mathfunc3(sh_float_t, sh_float_t, sh_float_t);

typedef struct
{
  char *name;
  int nargs;		/* > 0, function returns double; < 0, function returns int */
  union
    {
      mathfunc1 *func1;
      mathfunc2 *func2;
      mathfunc3 *func3;
      imathfunc1 *ifunc1;
      imathfunc2 *ifunc2;
    } f;
} FLTEXPR_MATHFUN;

/* Not implemented yet: functions that don't fit one of the supported
   calling prototypes, with a couple of exceptions */
FLTEXPR_MATHFUN mathfuncs[] =
{
  { "abs",	1,	{ .func1 = fabs }	},
  { "acos",	1,	{ .func1 = acos }	},
  { "acosh",	1,	{ .func1 = acosh }	},
  { "asin",	1,	{ .func1 = asin }	},
  { "asinh",	1,	{ .func1 = asinh }	},
  { "atan",	1,	{ .func1 = atan }	},
  { "atanh",	1,	{ .func1 = atanh }	},
  { "cbrt",	1,	{ .func1 = cbrt }	},
  { "ceil",	1,	{ .func1 = ceil }	},
  { "cos",	1,	{ .func1 = cos }	},  
  { "cosh",	1,	{ .func1 = cosh }	},
  { "cot",	1,	{ .func1 = xcot }	},
  { "coth",	1,	{ .func1 = xcoth }	},
  { "erf",	1,	{ .func1 = erf }	},
  { "erfc",	1,	{ .func1 = erfc }	},
  { "exp",	1,	{ .func1 = exp }	},
  { "exp2",	1,	{ .func1 = exp2 }	},
  { "expm1",	1,	{ .func1 = expm1 }	},
  { "fabs",	1,	{ .func1 = fabs }	},
  { "floor",	1,	{ .func1 = floor }	},
  { "j0",	1,	{ .func1 = j0 }		},
  { "j1",	1,	{ .func1 = j1 }		},
  { "lgamma",	1,	{ .func1 = lgamma }	},
  { "log",	1,	{ .func1 = log }	},
  { "log10",	1,	{ .func1 = log10 }	},
  { "log1p",	1,	{ .func1 = log1p }	},
  { "log2",	1,	{ .func1 = log2 }	},
  { "logb",	1,	{ .func1 = logb }	},
  { "nearbyint",1,	{ .func1 = nearbyint }	},
  { "rint",	1,	{ .func1 = rint }	},
  { "round",	1,	{ .func1 = round }	},
  { "sin",	1,	{ .func1 = sin }	},
  { "sinh",	1,	{ .func1 = sinh }	},
  { "sqrt",	1,	{ .func1 = sqrt }	},
  { "tan",	1,	{ .func1 = tan }	},
  { "tanh",	1,	{ .func1 = tanh }	},
  { "tgamma",	1,	{ .func1 = tgamma }	},
  { "trunc",	1,	{ .func1 = trunc }	},
  { "y0",	1,	{ .func1 = y0 }		},
  { "y1",	1,	{ .func1 = y1 }		},

  { "atan2",	2,	{ .func2 = atan2 }	},
  { "copysign",	2,	{ .func2 = copysign }	},
  { "fdim",	2,	{ .func2 = fdim }	},
  { "fmax",	2,	{ .func2 = fmax }	},
  { "fmin",	2,	{ .func2 = fmin }	},
  { "fmod",	2,	{ .func2 = fmod }	},
  { "hypot",	2,	{ .func2 = hypot }	},
  { "nextafter",2,	{ .func2 = nextafter }	},
  { "pow",	2,	{ .func2 = pow }	},
  { "remainder",2,	{ .func2 = remainder }	},
  { "roundp",	2,	{ .func2 = xroundp }	},
  { "ldexp",	2,	{ .func2 = xldexp }	},
  { "jn",	2,	{ .func2 = xjn }	},
  { "scalbn",	2,	{ .func2 = xscalbn }	},
  { "yn",	2,	{ .func2 = xyn }	},

  { "fma",	3,	{ .func3 = fma }	},
 
  { "fpclassify",-1,	{ .ifunc1 = xfpclassify }	},
  { "isfinite",	-1,	{ .ifunc1 = xisfinite }		},
  { "isinf",	-1,	{ .ifunc1 = xisinf }		},
  { "isinfinite",-1,	{ .ifunc1 = xisinfinite }	},
  { "isnan",	-1,	{ .ifunc1 = xisnan }		},
  { "isnormal",	-1,	{ .ifunc1 = xisnormal }		},
  { "issubnormal",-1,	{ .ifunc1 = xissubnormal }	},
  { "iszero",	-1,	{ .ifunc1 = xiszero }		},
  { "ilogb",	-1,	{ .ifunc1 = ilogb }		},
  { "signbit",	-1,	{ .ifunc1 = xsignbit }		},
 
  { "isgreater",-2,	{ .ifunc2 = xisgreater }	},
  { "isgreaterequal",-2,{ .ifunc2 = xisgreaterequal }	},
  { "isless",	-2,	{ .ifunc2 = xisless }		},
  { "islessequal", -2,	{ .ifunc2 = xislessequal }	},
  { "islessgreater",-2,	{ .ifunc2 = xislessgreater }	},
  { "isunordered",-2,	{ .ifunc2 = xisunordered }	},

  { NULL, 	0,	NULL	}
};

static sh_float_t nanval, infval;

static int	is_arithop (int);
static int	is_multiop (int);
static void	readtok (void);	/* lexical analyzer */

static void	init_lvalue (struct lvalue *);
static struct lvalue *alloc_lvalue (void);
static void	free_lvalue (struct lvalue *);

static sh_float_t fltexpr_streval (char *, int, struct lvalue *);

static int fltexpr_findfunc (char *);
static sh_float_t fltexpr_funeval (char *, struct lvalue *);
static sh_float_t expfunc (int);

static void	evalerror (const char *);

static sh_float_t fltexpr_strtod (const char *, char **);
static char	*fltexpr_format (sh_float_t);

#if defined (ARRAYS)
static int	fltexpr_skipsubscript (char *, char *);
#endif

static void	pushexp (void);
static void	popexp (void);
static void	fltexpr_unwind (void);
static void	fltexpr_bind_variable (char *, char *);
#if defined (ARRAY_VARS)
static void	fltexpr_bind_array_element (char *, arrayind_t, char *);
#endif

static sh_float_t fltexp_subexpr (const char *);

static sh_float_t expcomma (void);
static sh_float_t expassign (void);
static sh_float_t expcond (void);
static sh_float_t explor (void);
static sh_float_t expland (void);
static sh_float_t expeq (void);
static sh_float_t expcompare (void);
static sh_float_t expshift (void);
static sh_float_t expaddsub (void);
static sh_float_t expmuldiv (void);
static sh_float_t exppower (void);
static sh_float_t expunary (void);
static sh_float_t exp0 (void);

/* Global var which contains the stack of expression contexts. */
static FLTEXPR_CONTEXT **expr_stack;
static int expr_depth;		   /* Location in the stack. */
static size_t expr_stack_size;	   /* Number of slots already allocated. */

#if defined (ARRAY_VARS)
extern const char * const bash_badsub_errmsg;
#endif

#define SAVETOK(X) \
  do { \
    (X)->curtok = curtok; \
    (X)->lasttok = lasttok; \
    (X)->tp = tp; \
    (X)->lasttp = lasttp; \
    (X)->tokval = tokval; \
    (X)->tokstr = tokstr; \
    (X)->noeval = noeval; \
    (X)->lval = curlval; \
  } while (0)

#define RESTORETOK(X) \
  do { \
    curtok = (X)->curtok; \
    lasttok = (X)->lasttok; \
    tp = (X)->tp; \
    lasttp = (X)->lasttp; \
    tokval = (X)->tokval; \
    tokstr = (X)->tokstr; \
    noeval = (X)->noeval; \
    curlval = (X)->lval; \
  } while (0)

/* Push and save away the contents of the globals describing the
   current expression context. */
static void
pushexp (void)
{
  FLTEXPR_CONTEXT *context;

  if (expr_depth >= MAX_EXPR_RECURSION_LEVEL)
    evalerror (_("expression recursion level exceeded"));

  if (expr_depth >= expr_stack_size)
    {
      expr_stack_size += EXPR_STACK_GROW_SIZE;
      expr_stack = (FLTEXPR_CONTEXT **)xrealloc (expr_stack, expr_stack_size * sizeof (FLTEXPR_CONTEXT *));
    }

  context = (FLTEXPR_CONTEXT *)xmalloc (sizeof (FLTEXPR_CONTEXT));

  context->expression = expression;
  SAVETOK(context);

  expr_stack[expr_depth++] = context;
}

/* Pop the the contents of the expression context stack into the
   globals describing the current expression context. */
static void
popexp (void)
{
  FLTEXPR_CONTEXT *context;

  if (expr_depth <= 0)
    {
      /* See the comment at the top of evalexp() for an explanation of why
	 this is done. */
      expression = lasttp = 0;
      evalerror (_("recursion stack underflow"));
    }

  context = expr_stack[--expr_depth];

  expression = context->expression;
  RESTORETOK (context);

  free (context);
}

static void
fltexpr_unwind (void)
{
  while (--expr_depth > 0)
    {
      if (expr_stack[expr_depth]->tokstr)
	free (expr_stack[expr_depth]->tokstr);

      if (expr_stack[expr_depth]->expression)
	free (expr_stack[expr_depth]->expression);

      free (expr_stack[expr_depth]);
    }
  if (expr_depth == 0)
    free (expr_stack[expr_depth]);	/* free the allocated FLTEXPR_CONTEXT */

  noeval = 0;	/* XXX */
}

static sh_float_t
fltexpr_strtod (const char *nptr, char **ep)
{
  sh_float_t r;
  char *xp;

  errno = 0;
  r = SHFLOAT_STRTOD (nptr, &xp);
  if (errno == ERANGE)
    evalerror ("number out of range");
  else if (r == 0 && *ep == nptr)
    evalerror ("invalid number");
  if (ep)
    *ep = xp;
  return r;
}

/* Convert from internal format (double) to external format (char *).
   Code adapted from gnulib. */
   
static char *
fltexpr_format (sh_float_t val)
{
  int r;
  char ret[SHFLOAT_BUFSIZE_BOUND];	/* XXX */
  char format[8], *p;
  size_t retsize;
  int prec, n;
  sh_float_t abs_val;

  abs_val = val < 0 ? -val : val;

  /* There are better ways to do this, but this is an example */

  /* Construct the format, this is where we tinker */
  p = format;

  *p++ = '%';
  *p++ = '.';
  *p++ = '*';
  *p++ = SHFLOAT_LENGTH_MODIFIER;
  *p++ = 'g';
  *p = '\0';

  retsize = sizeof (ret);

  /* Use a loop to get the minimal representation but make sure we have the
     minimum number of digits required to round-trip a sh_float_t. */
  for (prec = abs_val < SHFLOAT_MIN ? 1 : SHFLOAT_DIG; ; prec++)
    {
      n = snprintf (ret, retsize, format, prec, val);
      if (n < 0 ||
	  prec >= SHFLOAT_MANT_DIG ||
	  (n < retsize && SHFLOAT_STRTOD (ret, NULL) == val))
	break;
    }
    
  return savestring (ret);
}

static void
fltexpr_bind_variable (char *lhs, char *rhs)
{
  SHELL_VAR *v;
  int aflags;

  if (lhs == 0 || *lhs == 0)
    return;		/* XXX */

#if defined (ARRAY_VARS)
  aflags = ASS_NOEXPAND|ASS_ALLOWALLSUB;		/* allow assoc[@]=value */;
#else
  aflags = 0;
#endif
  v = builtin_bind_variable (lhs, rhs, aflags);
  if (v && ASSIGN_DISALLOWED (v, 0))
    sh_longjmp (evalbuf, 1);	/* variable assignment error */
  stupidly_hack_special_variables (lhs);
}

#if defined (ARRAY_VARS)
/* This is similar to the logic in arrayfunc.c:valid_array_reference when
   you pass VA_NOEXPAND. */
static int
fltexpr_skipsubscript (char *vp, char *cp)
{
  int flags, isassoc, noexp;
  SHELL_VAR *entry;

  isassoc = 0;
  entry = 0;

  *cp = '\0';
  isassoc = valid_identifier (vp) && (entry = find_variable (vp)) && assoc_p (entry);
  *cp = '[';	/* ] */

  /* We're not doing any evaluation here, we should suppress expansion when
     skipping over the subscript */
  flags = isassoc ? VA_NOEXPAND : 0;
  return (skipsubscript (cp, 0, flags));
}

/* Rewrite tok, which is of the form vname[expression], to vname[ind], where
   IND is the already-calculated value of expression. */
static void
fltexpr_bind_array_element (char *tok, arrayind_t ind, char *rhs)
{
  char *lhs, *vname;
  size_t llen;
  char ibuf[INT_STRLEN_BOUND (arrayind_t) + 1], *istr;

  istr = fmtumax (ind, 10, ibuf, sizeof (ibuf), 0);
  vname = array_variable_name (tok, 0, (char **)NULL, (int *)NULL);

  llen = strlen (vname) + sizeof (ibuf) + 3;
  lhs = xmalloc (llen);

  sprintf (lhs, "%s[%s]", vname, istr);		/* XXX */

/*itrace("expr_bind_array_element: %s=%s", lhs, rhs);*/
  fltexpr_bind_variable (lhs, rhs);
  free (vname);
  free (lhs);
}
#endif /* ARRAY_VARS */

/* Evaluate EXPR, and return the arithmetic result.  If VALIDP is
   non-null, a zero is stored into the location to which it points
   if the expression is invalid, non-zero otherwise.  If a non-zero
   value is returned in *VALIDP, the return value of evalexp() may
   be used.

   The `while' loop after the longjmp is caught relies on the above
   implementation of pushexp and popexp leaving in expr_stack[0] the
   values that the variables had when the program started.  That is,
   the first things saved are the initial values of the variables that
   were assigned at program startup or by the compiler.  Therefore, it is
   safe to let the loop terminate when expr_depth == 0, without freeing up
   any of the expr_depth[0] stuff. */
sh_float_t
fltexpr_evalexp (const char *expr, int flags, int *validp)
{
  sh_float_t val;
  int c;
  procenv_t oevalbuf;

  val = 0;
  noeval = 0;
  already_expanded = (flags&EXP_EXPANDED);

  FASTCOPY (evalbuf, oevalbuf, sizeof (evalbuf));

  c = setjmp_nosigs (evalbuf);

  if (c)
    {
      FREE (tokstr);
      FREE (expression);
      tokstr = expression = (char *)NULL;

      fltexpr_unwind ();
      expr_depth = 0;	/* XXX - make sure */

      /* We copy in case we've called evalexp recursively */
      FASTCOPY (oevalbuf, evalbuf, sizeof (evalbuf));

      if (validp)
	*validp = 0;
      return (0);
    }

  val = fltexp_subexpr (expr);

  if (validp)
    *validp = 1;

  FASTCOPY (oevalbuf, evalbuf, sizeof (evalbuf));

  return (val);
}

static sh_float_t
fltexp_subexpr (const char *expr)
{
  sh_float_t val;
  const char *p;

  for (p = expr; p && *p && cr_whitespace (*p); p++)
    ;

  if (p == NULL || *p == '\0')
    return (0);

  pushexp ();
  expression = savestring (expr);
  tp = expression;

  curtok = lasttok = 0;
  tokstr = (char *)NULL;
  tokval = 0;
  init_lvalue (&curlval);
  lastlval = curlval;

  readtok ();

  val = EXP_LOWEST ();

  if (curtok != 0)
    evalerror (_("arithmetic syntax error in expression"));

  FREE (tokstr);
  FREE (expression);

  popexp ();

  return val;
}

static sh_float_t
expcomma (void)
{
  register sh_float_t value;

  value = expassign ();
  while (curtok == COMMA)
    {
      readtok ();
      value = expassign ();
    }

  return value;
}
  
static sh_float_t
expassign (void)
{
  register sh_float_t value;
  char *lhs, *rhs;
  arrayind_t lind;
#if defined (HAVE_IMAXDIV)
  imaxdiv_t idiv;
#endif

  value = expcond ();
  if (curtok == EQ || curtok == OP_ASSIGN)
    {
      int special, op;
      sh_float_t lvalue;

      special = curtok == OP_ASSIGN;

      if (lasttok != STR)
	evalerror (_("attempted assignment to non-variable"));

      if (special)
	{
	  op = assigntok;		/* a OP= b */
	  lvalue = value;
	}

      if (tokstr == 0)
	evalerror (_("arithmetic syntax error in variable assignment"));

      /* XXX - watch out for pointer aliasing issues here */
      lhs = savestring (tokstr);
      /* save ind in case rhs is string var and evaluation overwrites it */
      lind = curlval.ind;
      readtok ();
      value = expassign ();

      if (special)
	{
	  if (op == DIV && value == 0)
	    {
	      if (noeval == 0)
		evalerror (_("division by 0"));
	      else
	        value = 1;
	    }

	  switch (op)
	    {
	    case MUL:
	      lvalue *= value;
	      break;
	    case DIV:
	      lvalue = lvalue / value;
	      break;
	    case PLUS:
	      lvalue += value;
	      break;
	    case MINUS:
	      lvalue -= value;
	      break;
	    default:
	      free (lhs);
	      evalerror (_("bug: bad expassign token"));
	      break;
	    }
	  value = lvalue;
	}

      rhs = fltexpr_format (value);
      if (noeval == 0)
	{
#if defined (ARRAY_VARS)
	  if (lind != -1)
	    fltexpr_bind_array_element (lhs, lind, rhs);
	  else
#endif
	    fltexpr_bind_variable (lhs, rhs);
	}
      if (curlval.tokstr && curlval.tokstr == tokstr)
	init_lvalue (&curlval);

      free (rhs);
      free (lhs);
      FREE (tokstr);
      tokstr = (char *)NULL;		/* For freeing on errors. */
    }

  return (value);
}

/* Conditional expression (expr?expr:expr) */
static sh_float_t
expcond (void)
{
  sh_float_t cval, val1, val2, rval;
  int set_noeval;

  set_noeval = 0;
  rval = cval = explor ();
  if (curtok == QUES)		/* found conditional expr */
    {
      if (cval == 0)
	{
	  set_noeval = 1;
	  noeval++;
	}

      readtok ();
      if (curtok == 0 || curtok == COL)
	evalerror (_("expression expected"));

      val1 = EXP_LOWEST ();

      if (set_noeval)
	noeval--;
      if (curtok != COL)
	evalerror (_("`:' expected for conditional expression"));

      set_noeval = 0;
      if (cval)
 	{
 	  set_noeval = 1;
	  noeval++;
 	}

      readtok ();
      if (curtok == 0)
	evalerror (_("expression expected"));
      val2 = expcond ();

      if (set_noeval)
	noeval--;
      rval = cval ? val1 : val2;
      lasttok = COND;
    }
  return rval;
}

/* Logical OR. */
static sh_float_t
explor (void)
{
  register sh_float_t val1, val2;
  int set_noeval;

  val1 = expland ();

  while (curtok == LOR)
    {
      set_noeval = 0;
      if (val1 != 0)
	{
	  noeval++;
	  set_noeval = 1;
	}
      readtok ();
      val2 = expland ();
      if (set_noeval)
	noeval--;
      val1 = val1 || val2;
      lasttok = LOR;
    }

  return (val1);
}

/* Logical AND. */
static sh_float_t
expland (void)
{
  register sh_float_t val1, val2;
  int set_noeval;

  val1 = expeq ();		/* XXX */

  while (curtok == LAND)
    {
      set_noeval = 0;
      if (val1 == 0)
	{
	  set_noeval = 1;
	  noeval++;
	}
      readtok ();
      val2 = expeq ();
      if (set_noeval)
	noeval--;
      val1 = val1 && val2;
      lasttok = LAND;
    }

  return (val1);
}

static sh_float_t
expeq (void)
{
  register sh_float_t val1, val2;

  val1 = expcompare ();

  while ((curtok == EQEQ) || (curtok == NEQ))
    {
      int op = curtok;

      readtok ();
      val2 = expcompare ();
      if (op == EQEQ)
	val1 = (val1 == val2);
      else if (op == NEQ)
	val1 = (val1 != val2);
      lasttok = NUM;
    }
  return (val1);
}

static sh_float_t
expcompare (void)
{
  register sh_float_t val1, val2;

  val1 = expaddsub ();
  while ((curtok == LEQ) ||
	 (curtok == GEQ) ||
	 (curtok == LT) ||
	 (curtok == GT))
    {
      int op = curtok;

      readtok ();
      val2 = expaddsub ();

      if (op == LEQ)
	val1 = val1 <= val2;
      else if (op == GEQ)
	val1 = val1 >= val2;
      else if (op == LT)
	val1 = val1 < val2;
      else			/* (op == GT) */
	val1 = val1 > val2;
      lasttok = NUM;
    }
  return (val1);
}

static sh_float_t
expaddsub (void)
{
  register sh_float_t val1, val2;

  val1 = expmuldiv ();

  while ((curtok == PLUS) || (curtok == MINUS))
    {
      int op = curtok;

      readtok ();
      val2 = expmuldiv ();

      if (op == PLUS)
	val1 += val2;
      else if (op == MINUS)
	val1 -= val2;
      lasttok = NUM;
    }
  return (val1);
}

static sh_float_t
expmuldiv (void)
{
  register sh_float_t val1, val2;

  val1 = exppower ();

  while ((curtok == MUL) || (curtok == DIV))
    {
      int op = curtok;
      char *stp, *sltp;

      stp = tp;
      readtok ();

      val2 = exppower ();

      /* Handle division by 0 and twos-complement arithmetic overflow */
      if (op == DIV && val2 == 0)
	{
	  if (noeval == 0)
	    {
	      sltp = lasttp;
	      lasttp = stp;
	      while (lasttp && *lasttp && whitespace (*lasttp))
		lasttp++;
	      evalerror (_("division by 0"));
	      lasttp = sltp;
	    }
	  else
	    val2 = 1;
	}

      if (op == MUL)
	val1 *= val2;
      else if (op == DIV)
	val1 = val1 / val2;
      lasttok = NUM;
    }
  return (val1);
}

static sh_float_t
exppower (void)
{
  register sh_float_t val1, val2;

  val1 = expunary ();
  while (curtok == POWER)
    {
      readtok ();
      val2 = exppower ();	/* exponentiation is right-associative */
      lasttok = NUM;
      if (noeval == 0)
	{
	  if (val2 == 0)
	    return (1);
	  val1 = pow (val1, val2);
	}
      else
	val1 = 1;
    }
  return (val1);
}

static sh_float_t
expunary (void)
{
  register sh_float_t val;

  if (curtok == NOT)
    {
      readtok ();
      val = !expunary ();
      lasttok = NUM;
    }
  else if (curtok == MINUS)
    {
      readtok ();
      val = - expunary ();
      lasttok = NUM;
    }
  else if (curtok == PLUS)
    {
      readtok ();
      val = expunary ();
      lasttok = NUM;
    }
  else
    val = exp0 ();

  return (val);
}

static sh_float_t
exp0 (void)
{
  sh_float_t val, v2;
  char *vincdec;
  int stok;
  FLTEXPR_CONTEXT ec;

  val = 0;
  /* XXX - might need additional logic here to decide whether or not
	   pre-increment or pre-decrement is legal at this point. */
  if (curtok == PREINC || curtok == PREDEC)
    {
      stok = lasttok = curtok;
      readtok ();
      if (curtok != STR)
	/* readtok() catches this */
	evalerror (_("identifier expected after pre-increment or pre-decrement"));

      v2 = tokval + ((stok == PREINC) ? 1 : -1);
      vincdec = fltexpr_format (v2);
      if (noeval == 0)
	{
#if defined (ARRAY_VARS)
	  if (curlval.ind != -1)
	    fltexpr_bind_array_element (curlval.tokstr, curlval.ind, vincdec);
	  else
#endif
	    if (tokstr)
	      fltexpr_bind_variable (tokstr, vincdec);
	}
      free (vincdec);
      val = v2;

      curtok = NUM;	/* make sure --x=7 is flagged as an error */
      readtok ();
    }
  else if (curtok == LPAR)
    {
      /* XXX - save curlval here?  Or entire expression context? */
      readtok ();
      val = EXP_LOWEST ();

      if (curtok != RPAR) /* ( */
	evalerror (_("missing `)'"));

      /* Skip over closing paren. */
      readtok ();
    }
  else if (curtok == NUM)
    {
      val = tokval;
      readtok ();
    }
  else if (curtok == STR)
    {
      val = tokval;
      SAVETOK (&ec);
      tokstr = (char *)NULL;	/* keep it from being freed */
      noeval = 1;
      readtok ();
      stok = curtok;

      /* post-increment or post-decrement */
      if (stok == POSTINC || stok == POSTDEC)
	{
 	  /* restore certain portions of EC */
 	  tokstr = ec.tokstr;
 	  noeval = ec.noeval;
 	  curlval = ec.lval;
 	  lasttok = STR;	/* ec.curtok */

	  v2 = val + ((stok == POSTINC) ? 1 : -1);
	  vincdec = fltexpr_format (v2);
	  if (noeval == 0)
	    {
#if defined (ARRAY_VARS)
	      if (curlval.ind != -1)
		fltexpr_bind_array_element (curlval.tokstr, curlval.ind, vincdec);
	      else
#endif
		fltexpr_bind_variable (tokstr, vincdec);
	    }
          free (vincdec);
          curtok = NUM;	/* make sure x++=7 is flagged as an error */
	}
      else
	{
	  /* XXX - watch out for pointer aliasing issues here */
	  if (stok == STR)	/* free new tokstr before old one is restored */
	    FREE (tokstr);
	  RESTORETOK (&ec);
 	}

      readtok ();
    }
  else if (curtok == FUNC)
    {
      val = expfunc (tokval);
      lasttok = FUNC;
      curtok = NUM;

      readtok ();	/* skip over closing right paren, expfunc checks syntax */
    }
  else
    evalerror (_("arithmetic syntax error: operand expected"));

  return (val);
}

/* Evaluate a math function call with some minimal error checking. */
static sh_float_t
expfunc (int ind)
{
  FLTEXPR_MATHFUN func;
  sh_float_t arg1, arg2, arg3, val;
  int nargs, ival;

  func = mathfuncs[ind];
  /* If func.nargs > 0, the function returns double and takes func.nargs arguments;
     if func.nargs < 0, the function returns int and takes -func.nargs arguments. */
  nargs = (func.nargs > 0) ? func.nargs : -func.nargs;

  readtok();
  if (curtok != LPAR)
    evalerror (_("function call: expected left paren"));

  readtok ();
  arg1 = expassign ();
  if (nargs > 1)
    {
      if (curtok != COMMA)
	evalerror (_("function call: expected comma"));
      readtok ();		/* consume the comma */
      arg2 = expassign ();
    }
  if (nargs > 2)
    {
      if (curtok != COMMA)
	evalerror (_("function call: expected comma"));
      readtok ();		/* consume the comma */
      arg3 = expassign ();
    }

  if (curtok != RPAR)
    evalerror (_("function call: expected right paren"));

  switch (func.nargs)
    {
    case 1:
      val = (*func.f.func1) (arg1); break;
    case 2:
      val = (*func.f.func2) (arg1, arg2); break;
    case 3:
      val = (*func.f.func3) (arg1, arg2, arg3); break;
    case -1:
      ival = (*func.f.ifunc1) (arg1); val = ival; break;
    case -2:
      ival = (*func.f.ifunc2) (arg1, arg2); val = ival; break;
    }

  return val;
}

static void
init_lvalue (struct lvalue *lv)
{
  lv->tokstr = 0;
  lv->tokvar = 0;
  lv->tokval = -1;
  lv->ind = -1;
}

static struct lvalue *
alloc_lvalue (void)
{
  struct lvalue *lv;

  lv = xmalloc (sizeof (struct lvalue));
  init_lvalue (lv);
  return (lv);
}

static void
free_lvalue (struct lvalue *lv)
{
  free (lv);		/* should be inlined */
}

static int
fltexpr_findfunc (char *name)
{
  int i;

  for (i = 0; mathfuncs[i].name; i++)
    if (STREQ (name, mathfuncs[i].name))
      return i;
  return -1;
}

static sh_float_t
fltexpr_streval (char *tok, int e, struct lvalue *lvalue)
{
  SHELL_VAR *v;
  char *value;
  sh_float_t tval;
  int initial_depth;
#if defined (ARRAY_VARS)
  arrayind_t ind;
  int tflag, aflag;
  array_eltstate_t es;
#endif

/*itrace("fltexpr_streval: %s: noeval = %d expanded=%d", tok, noeval, already_expanded);*/
  /* If we are suppressing evaluation, just short-circuit here instead of
     going through the rest of the evaluator. */
  if (noeval)
    return (0);

  initial_depth = expr_depth;

#if defined (ARRAY_VARS)
  tflag = AV_NOEXPAND;	/* for a start */
#endif

  /* [[[[[ */
#if defined (ARRAY_VARS)
  aflag = tflag;	/* use a different variable for now */
  if (shell_compatibility_level > 51)
    aflag |= AV_ATSTARKEYS;
  v = (e == ']') ? array_variable_part (tok, tflag, (char **)0, (int *)0) : find_variable (tok);
#else
  v = find_variable (tok);
#endif
  if (v == 0 && e != ']')
    v = find_variable_last_nameref (tok, 0);  

  if ((v == 0 || invisible_p (v)) && unbound_vars_is_error)
    {
#if defined (ARRAY_VARS)
      value = (e == ']') ? array_variable_name (tok, tflag, (char **)0, (int *)0) : tok;
#else
      value = tok;
#endif

      set_exit_status (EXECUTION_FAILURE);
      err_unboundvar (value);

#if defined (ARRAY_VARS)
      if (e == ']')
	FREE (value);	/* array_variable_name returns new memory */
#endif

      if (no_longjmp_on_fatal_error && interactive_shell)
	sh_longjmp (evalbuf, 1);

      if (interactive_shell)
	{
	  fltexpr_unwind ();
	  top_level_cleanup ();
	  jump_to_top_level (DISCARD);
	}
      else
	jump_to_top_level (FORCE_EOF);
    }

#if defined (ARRAY_VARS)
  init_eltstate (&es);
  es.ind = -1;
  /* If the second argument to get_array_value doesn't include AV_ALLOWALL,
     we don't allow references like array[@].  In this case, get_array_value
     is just like get_variable_value in that it does not return newly-allocated
     memory or quote the results.  AFLAG is set above and is either AV_NOEXPAND
     or 0. */
  value = (e == ']') ? get_array_value (tok, aflag, &es) : get_variable_value (v);
  ind = es.ind;
  flush_eltstate (&es);
#else
  value = get_variable_value (v);
#endif

  if (expr_depth < initial_depth)
    {
      if (no_longjmp_on_fatal_error && interactive_shell)
	sh_longjmp (evalbuf, 1);
      return (0);
    }

  tval = (value && *value) ? fltexp_subexpr (value) : 0;

  if (lvalue)
    {
      lvalue->tokstr = tok;	/* XXX */
      lvalue->tokval = tval;
      lvalue->tokvar = v;	/* XXX */
#if defined (ARRAY_VARS)
      lvalue->ind = ind;
#else
      lvalue->ind = -1;
#endif
    }
	  
  return (tval);
}

static inline int
is_multiop (int c)
{
  switch (c)
    {
    case EQEQ:
    case NEQ:
    case LEQ:
    case GEQ:
    case LAND:
    case LOR:
    case OP_ASSIGN:
    case COND:
    case POWER:
    case PREINC:
    case PREDEC:
    case POSTINC:
    case POSTDEC:
      return 1;
    default:
      return 0;
    }
}

static inline int
is_arithop (int c)
{
  switch (c)
    {
    case EQ:
    case GT:
    case LT:
    case PLUS:
    case MINUS:
    case MUL:
    case DIV:
    case NOT:
    case LPAR:
    case RPAR:
      return 1;		/* operator tokens */
    case QUES:
    case COL:
    case COMMA:
      return 1;		/* questionable */
    default:
      return 0;		/* anything else is invalid */
    }
}

/* Lexical analyzer/token reader for the expression evaluator.  Reads the
   next token and puts its value into curtok, while advancing past it.
   Updates value of tp.  May also set tokval (for number) or tokstr (for
   string). */
static void
readtok (void)
{
  char *cp, *xp;
  unsigned char c, c1;
  int e;

  /* Skip leading whitespace. */
  cp = tp;
  c = 0;
  e = 0;
  while (cp && (c = *cp) && (cr_whitespace (c)))
    cp++;

  if (c)
    cp++;

  if (c == '\0')
    {
      lasttok = curtok;
      curtok = 0;
      tp = cp;
      return;
    }
  lasttp = tp = cp - 1;

  /* check for Inf, Nan here */
  if (strncasecmp (tp, "INF", 3) == 0 && (isalnum (tp[3]) == 0))
    {
      cp = tp + 3;
      tokval = infval;
      lasttok = curtok;
      curtok = NUM;
    }
  else if (strncasecmp (tp, "NAN", 3) == 0 && (isalnum (tp[3]) == 0))
    {
      cp = tp + 3;
      tokval = nanval;
      lasttok = curtok;
      curtok = NUM;
    }
  else if (strncasecmp (tp, "DBL_MIN", 7) == 0 && (isalnum (tp[7]) == 0))
    {
      cp = tp + 7;
      tokval = SHFLOAT_MIN;
      lasttok = curtok;
      curtok = NUM;
    }
  else if (strncasecmp (tp, "DBL_MAX", 7) == 0 && (isalnum (tp[7]) == 0))
    {
      cp = tp + 7;
      tokval = SHFLOAT_MAX;
      lasttok = curtok;
      curtok = NUM;
    }
  else if (strncasecmp (tp, "PI", 2) == 0 && (isalnum (tp[2]) == 0))
    {
      cp = tp + 2;
      tokval = M_PI;
      lasttok = curtok;
      curtok = NUM;
    }
  else if (strncasecmp (tp, "GAMMA", 2) == 0 && (isalnum (tp[5]) == 0))
    {
      cp = tp + 5;
      tokval = M_EGAMMA;
      lasttok = curtok;
      curtok = NUM;
    }
  else if ((tp[0] == 'E' || tp[0] == 'e') && (isalnum (tp[1]) == 0))
    {
      cp = tp + 1;
      tokval = M_E;
      lasttok = curtok;
      curtok = NUM;
    }
  else if (legal_variable_starter (c))
    {
      /* variable names not preceded with a dollar sign are shell variables. */
      char *savecp;
      FLTEXPR_CONTEXT ec;
      int peektok, ind;

      while (legal_variable_char (c))
	c = *cp++;

      c = *--cp;

#if defined (ARRAY_VARS)
      if (c == '[')
	{
	  e = fltexpr_skipsubscript (tp, cp);		/* XXX - was skipsubscript */
	  if (cp[e] == ']')
	    {
	      cp += e + 1;
	      c = *cp;
	      e = ']';
	    }
	  else
	    evalerror (_(bash_badsub_errmsg));
	}
#endif /* ARRAY_VARS */

      *cp = '\0';
      /* XXX - watch out for pointer aliasing issues here */
      if (curlval.tokstr && curlval.tokstr == tokstr)
	init_lvalue (&curlval);

      FREE (tokstr);
      tokstr = savestring (tp);
      *cp = c;

      /* XXX - make peektok part of saved token state? */
      SAVETOK (&ec);
      tokstr = (char *)NULL;	/* keep it from being freed */
      tp = savecp = cp;
      noeval = 1;
      curtok = STR;
      readtok ();
      peektok = curtok;
      if (peektok == STR)	/* free new tokstr before old one is restored */
	FREE (tokstr);
      RESTORETOK (&ec);
      cp = savecp;

      ind = -1;
      if ((ind = fltexpr_findfunc (tokstr)) != -1 && peektok == LPAR)
	{
	  lasttok = curtok;
	  curtok = FUNC;
	  tokval = ind;		/* overload this here for expfunc */
	  tp = cp;
	  return;		/* XXX */
	}
      else if (ind == -1 && peektok == LPAR)
	evalerror (_("unrecognized function name"));
	
      /* The tests for PREINC and PREDEC aren't strictly correct, but they
	 preserve old behavior if a construct like --x=9 is given. */
      if (lasttok == PREINC || lasttok == PREDEC || peektok != EQ)
        {
          lastlval = curlval;
	  tokval = fltexpr_streval (tokstr, e, &curlval);
        }
      else
	tokval = 0;

      lasttok = curtok;
      curtok = STR;
    }
  else if (DIGIT(c))
    {
      /* Let strtod figure out where to end the floating-point value and let
	 the parser figure out what's valid. */
      tokval = fltexpr_strtod (tp, &cp);
      lasttok = curtok;
      curtok = NUM;
    }
  else
    {
      c1 = *cp++;
      if ((c == EQ) && (c1 == EQ))
	c = EQEQ;
      else if ((c == NOT) && (c1 == EQ))
	c = NEQ;
      else if ((c == GT) && (c1 == EQ))
	c = GEQ;
      else if ((c == LT) && (c1 == EQ))
	c = LEQ;
      else if ((c == '*') && (c1 == '*'))
	c = POWER;
      else if ((c == '-' || c == '+') && c1 == c && curtok == STR)
	c = (c == '-') ? POSTDEC : POSTINC;
      else if ((c == '-' || c == '+') && c1 == c && curtok == NUM)
	{
	  /* This catches something like --FOO++ */
	  if (c == '-')
	    evalerror (_("--: assignment requires lvalue"));
	  else
	    evalerror (_("++: assignment requires lvalue"));
	}
      else if ((c == '-' || c == '+') && c1 == c)
	{
	  /* Quickly scan forward to see if this is followed by optional
	     whitespace and an identifier. */
	  xp = cp;
	  while (xp && *xp && cr_whitespace (*xp))
	    xp++;
	  if (legal_variable_starter ((unsigned char)*xp))
	    c = (c == '-') ? PREDEC : PREINC;
	  else
	    {
	      /* Posix says unary plus and minus have higher priority than
		 preinc and predec. */
	      /* This catches something like --4++ */
	      if (c == '-')
		evalerror (_("--: assignment requires lvalue"));
	      else
		evalerror (_("++: assignment requires lvalue"));
	    }
	}
      else if (c1 == EQ && member (c, "*/+-"))
	{
	  assigntok = c;	/* a OP= b */
	  c = OP_ASSIGN;
	}
      else if (is_arithop (c) == 0)
	{
	  cp--;
	  /* use curtok, since it hasn't been copied to lasttok yet */
	  if (curtok == 0 || is_arithop (curtok) || is_multiop (curtok))
	    evalerror (_("arithmetic syntax error: operand expected"));
	  else
	    evalerror (_("arithmetic syntax error: invalid arithmetic operator"));
	}
      else
	cp--;			/* `unget' the character */

      /* Should check here to make sure that the current character is one
	 of the recognized operators and flag an error if not.  Could create
	 a character map the first time through and check it on subsequent
	 calls. */
      lasttok = curtok;
      curtok = c;
    }
  tp = cp;
}

static void
evalerror (const char *msg)
{
  char *name, *t;

  name = this_command_name;
  for (t = expression; t && whitespace (*t); t++)
    ;
  internal_error (_("%s%s%s: %s (error token is \"%s\")"),
		   name ? name : "", name ? ": " : "",
		   t ? t : "", msg, (lasttp && *lasttp) ? lasttp : "");
  sh_longjmp (evalbuf, 1);
}

int
fltexpr_builtin (WORD_LIST *list)
{
  sh_float_t ret;
  int expok, opt, pflag;
  char *str;

  pflag = 0;

  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "p")) != -1)
    {
      switch (opt)
        {
          case 'p':
	    pflag = 1;
	    break;
	  CASE_HELPOPT;
	  default:
	    builtin_usage ();
	    return (EX_USAGE);
        }
    }

  list = loptend;

  if (list == 0)
    {
      builtin_error (_("expression expected"));
      return (EXECUTION_FAILURE);
    }
             	                
  ret = fltexpr_evalexp (list->word->word, EXP_EXPANDED, &expok);

  if (expok == 0)
    return (EXECUTION_FAILURE);

  if (pflag)
    {
      str = fltexpr_format (ret);
      printf ("%s\n", str);
      free (str);
    }

  return ((ret == 0) ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

int
fltexpr_builtin_load (char *s)
{
  /* Internal representations of Inf and NaN here */
  nanval = strtod ("NAN", NULL);
  infval = strtod ("INF", NULL);

  return 1;
}

void
fltexpr_builtin_unload (char *s)
{
}

char *fltexpr_doc[] =
{
  "Evaluate floating-point arithmetic expression.",
  "",
  "Evaluate EXPRESSION as a floating-point arithmetic expression and,",
  "if the -p option is supplied, print the value to the standard output.",
  "",
  "Exit Status:",
  "If the EXPRESSION evaluates to 0, the return status is 1; 0 otherwise.",
  (char *)NULL
};

struct builtin fltexpr_struct =
{
  "fltexpr",		/* builtin name */
  fltexpr_builtin,	/* function implementing the builtin */
  BUILTIN_ENABLED,	/* initial flags for builtin */
  fltexpr_doc,		/* array of long documentation strings. */
  "fltexpr [-p] expression",	/* usage synopsis; becomes short_doc */
  0			/* reserved for internal use */
};
