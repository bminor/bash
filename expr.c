/* expr.c -- arithmetic expression evaluation. */

/* Copyright (C) 1990, 1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

/*
 All arithmetic is done as long integers with no checking for overflow
 (though division by 0 is caught and flagged as an error).

 The following operators are handled, grouped into a set of levels in
 order of decreasing precedence.

	"-", "+"		[(unary operators)]
	"!", "~"
	"**"			[(exponentiation)]
	"*", "/", "%"
	"+", "-"
	"<<", ">>"
	"<=", ">=", "<", ">"
	"==", "!="
	"&"
	"^"
	"|"
	"&&"
	"||"
	"expr ? expr : expr"
	"=", "*=", "/=", "%=", "+=", "-=", "<<=", ">>=", "&=", "^=", "|="

 (Note that most of these operators have special meaning to bash, and an
 entire expression should be quoted, e.g. "a=$a+1" or "a=a+1" to ensure
 that it is passed intact to the evaluator when using `let'.  When using
 the $[] or $(( )) forms, the text between the `[' and `]' or `((' and `))'
 is treated as if in double quotes.)

 Sub-expressions within parentheses have a precedence level greater than
 all of the above levels and are evaluated first.  Within a single prece-
 dence group, evaluation is left-to-right, except for the arithmetic
 assignment operator (`='), which is evaluated right-to-left (as in C).

 The expression evaluator returns the value of the expression (assignment
 statements have as a value what is returned by the RHS).  The `let'
 builtin, on the other hand, returns 0 if the last expression evaluates to
 a non-zero, and 1 otherwise.

 Implementation is a recursive-descent parser.

 Chet Ramey
 chet@ins.CWRU.Edu
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

#include "shell.h"

/* Because of the $((...)) construct, expressions may include newlines.
   Here is a macro which accepts newlines, tabs and spaces as whitespace. */
#define cr_whitespace(c) (whitespace(c) || ((c) == '\n'))

/* Size be which the expression stack grows when neccessary. */
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
#define LSH	9	/* "<<" Left SHift */
#define RSH    10	/* ">>" Right SHift */
#define OP_ASSIGN 11	/* op= expassign as in Posix.2 */
#define COND	12	/* exp1 ? exp2 : exp3 */
#define POWER	13	/* exp1**exp2 */
#define EQ	'='
#define GT	'>'
#define LT	'<'
#define PLUS	'+'
#define MINUS	'-'
#define MUL	'*'
#define DIV	'/'
#define MOD	'%'
#define NOT	'!'
#define LPAR	'('
#define RPAR	')'
#define BAND	'&'	/* Bitwise AND */
#define BOR	'|'	/* Bitwise OR. */
#define BXOR	'^'	/* Bitwise eXclusive OR. */
#define BNOT	'~'	/* Bitwise NOT; Two's complement. */
#define QUES	'?'
#define COL	':'

static char	*expression;	/* The current expression */
static char	*tp;		/* token lexical position */
static char	*lasttp;	/* pointer to last token position */
static int	curtok;		/* the current token */
static int	lasttok;	/* the previous token */
static int	assigntok;	/* the OP in OP= */
static char	*tokstr;	/* current token string */
static int	tokval;		/* current token value */
static int	noeval;		/* set to 1 if no assignment to be done */
static procenv_t evalbuf;

static void	readtok ();	/* lexical analyzer */
static long	subexpr (), expassign (), exp0 (), exp1 (), exp2 (), exp3 (),
		exp4 (), exp5 (), expshift (), expland (), explor (),
		expband (), expbor (), expbxor (), expcond (), exppower ();
static long	strlong ();
static void	evalerror ();

/* A structure defining a single expression context. */
typedef struct {
  int curtok, lasttok;
  char *expression, *tp;
  int tokval;
  char *tokstr;
} EXPR_CONTEXT;

/* Global var which contains the stack of expression contexts. */
static EXPR_CONTEXT **expr_stack;
static int expr_depth;		   /* Location in the stack. */
static int expr_stack_size;	   /* Number of slots already allocated. */

extern char *this_command_name;

/* Push and save away the contents of the globals describing the
   current expression context. */
static void
pushexp ()
{
  EXPR_CONTEXT *context;

  if (expr_depth >= MAX_EXPR_RECURSION_LEVEL)
    evalerror ("expression recursion level exceeded");

  if (expr_depth >= expr_stack_size)
    {
      expr_stack = (EXPR_CONTEXT **)
	xrealloc (expr_stack, (expr_stack_size += EXPR_STACK_GROW_SIZE)
		  * sizeof (EXPR_CONTEXT *));
    }

  context = (EXPR_CONTEXT *)xmalloc (sizeof (EXPR_CONTEXT));

  context->curtok = curtok;
  context->lasttok = lasttok;
  context->expression = expression;
  context->tp = tp;
  context->tokval = tokval;
  context->tokstr = tokstr;
  expr_stack[expr_depth++] = context;
}

/* Pop the the contents of the expression context stack into the
   globals describing the current expression context. */
static void
popexp ()
{
  EXPR_CONTEXT *context;

  if (expr_depth == 0)
    evalerror ("recursion stack underflow");

  context = expr_stack[--expr_depth];
  curtok = context->curtok;
  lasttok = context->lasttok;
  expression = context->expression;
  tp = context->tp;
  tokval = context->tokval;
  tokstr = context->tokstr;
  free (context);
}

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
long
evalexp (expr, validp)
     char *expr;
     int *validp;
{
  long val;
#if 0
  procenv_t old_evalbuf;
#endif

  val = 0L;

#if 0
  /* Save the value of evalbuf to protect it around possible recursive
     calls to evalexp (). */
  COPY_PROCENV (evalbuf, old_evalbuf);
#endif

  if (setjmp (evalbuf))
    {
      FREE (tokstr);
      FREE (expression);
      tokstr = expression = (char *)NULL;

      while (--expr_depth > 0)
	{
	  if (expr_stack[expr_depth]->tokstr)
	    free (expr_stack[expr_depth]->tokstr);

	  if (expr_stack[expr_depth]->expression)
	    free (expr_stack[expr_depth]->expression);

	  free (expr_stack[expr_depth]);
	}
      free (expr_stack[expr_depth]);	/* free the allocated EXPR_CONTEXT */

      if (validp)
	*validp = 0;
      return (0L);
    }

  val = subexpr (expr);

#if 0
  /* Restore the value of evalbuf so that any subsequent longjmp calls
     will have a valid location to jump to. */
  COPY_PROCENV (old_evalbuf, evalbuf);
#endif

  if (validp)
    *validp = 1;

  return (val);
}

static long
subexpr (expr)
     char *expr;
{
  long val;
  char *p;

  for (p = expr; p && *p && cr_whitespace (*p); p++)
    ;

  if (p == NULL || *p == '\0')
    return (0L);

  pushexp ();
  curtok = lasttok = 0;
  expression = savestring (expr);
  tp = expression;

  tokstr = (char *)NULL;
  tokval = 0L;

  readtok ();

  val = expassign ();

  if (curtok != 0)
    evalerror ("syntax error in expression");

  FREE (tokstr);
  FREE (expression);

  popexp ();

  return val;
}

/* Bind/create a shell variable with the name LHS to the RHS.
   This creates or modifies a variable such that it is an integer.

   This should really be in variables.c, but it is here so that all of the
   expression evaluation stuff is localized.  Since we don't want any
   recursive evaluation from bind_variable() (possible without this code,
   since bind_variable() calls the evaluator for variables with the integer
   attribute set), we temporarily turn off the integer attribute for each
   variable we set here, then turn it back on after binding as necessary. */

void
bind_int_variable (lhs, rhs)
     char *lhs, *rhs;
{
  register SHELL_VAR *v;
  int isint = 0;

  v = find_variable (lhs);
  if (v)
    {
      isint = integer_p (v);
      v->attributes &= ~att_integer;
    }

  v = bind_variable (lhs, rhs);
  if (isint)
    v->attributes |= att_integer;
}

static long
expassign ()
{
  register long	value;
  char *lhs, *rhs;

  value = expcond ();
  if (curtok == EQ || curtok == OP_ASSIGN)
    {
      int special, op;
      long lvalue;

      special = curtok == OP_ASSIGN;

      if (lasttok != STR)
	evalerror ("attempted assignment to non-variable");

      if (special)
	{
	  op = assigntok;		/* a OP= b */
	  lvalue = value;
	}

      lhs = savestring (tokstr);
      readtok ();
      value = expassign ();

      if (special)
	{
	  switch (op)
	    {
	    case MUL:
	      lvalue *= value;
	      break;
	    case DIV:
	      lvalue /= value;
	      break;
	    case MOD:
	      lvalue %= value;
	      break;
	    case PLUS:
	      lvalue += value;
	      break;
	    case MINUS:
	      lvalue -= value;
	      break;
	    case LSH:
	      lvalue <<= value;
	      break;
	    case RSH:
	      lvalue >>= value;
	      break;
	    case BAND:
	      lvalue &= value;
	      break;
	    case BOR:
	      lvalue |= value;
	      break;
	    default:
	      free (lhs);
	      evalerror ("bug: bad expassign token");
	      break;
	    }
	  value = lvalue;
	}

      rhs = itos (value);
      if (noeval == 0)
	bind_int_variable (lhs, rhs);
      free (rhs);
      free (lhs);
      FREE (tokstr);
      tokstr = (char *)NULL;		/* For freeing on errors. */
    }
  return (value);
}

/* Conditional expression (expr?expr:expr) */
static long
expcond ()
{
  long cval, val1, val2, rval;
  int set_noeval;

  set_noeval = 0;
  rval = cval = explor ();
  if (curtok == QUES)		/* found conditional expr */
    {
      readtok ();
      if (curtok == 0 || curtok == COL)
	evalerror ("expression expected");
      if (cval == 0)
	{
	  set_noeval = 1;
	  noeval++;
	}
#if 0
      val1 = explor ();
#else
      val1 = expassign ();
#endif
      if (set_noeval)
        noeval--;
      if (curtok != COL)
        evalerror ("`:' expected for conditional expression");
      readtok ();
      if (curtok == 0)
	evalerror ("expression expected");
      set_noeval = 0;
      if (cval)
 	{
 	  set_noeval = 1;
	  noeval++;
 	}
      val2 = explor ();
      if (set_noeval)
        noeval--;
      rval = cval ? val1 : val2;
      lasttok = COND;
    }
  return rval;
}

/* Logical OR. */
static long
explor ()
{
  register long val1, val2;
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
static long
expland ()
{
  register long val1, val2;
  int set_noeval;

  val1 = expbor ();

  while (curtok == LAND)
    {
      set_noeval = 0;
      if (val1 == 0)
	{
	  set_noeval = 1;
	  noeval++;
	}
      readtok ();
      val2 = expbor ();
      if (set_noeval)
	noeval--;
      val1 = val1 && val2;
      lasttok = LAND;
    }

  return (val1);
}

/* Bitwise OR. */
static long
expbor ()
{
  register long val1, val2;

  val1 = expbxor ();

  while (curtok == BOR)
    {
      readtok ();
      val2 = expbxor ();
      val1 = val1 | val2;
    }

  return (val1);
}

/* Bitwise XOR. */
static long
expbxor ()
{
  register long val1, val2;

  val1 = expband ();

  while (curtok == BXOR)
    {
      readtok ();
      val2 = expband ();
      val1 = val1 ^ val2;
    }

  return (val1);
}

/* Bitwise AND. */
static long
expband ()
{
  register long val1, val2;

  val1 = exp5 ();

  while (curtok == BAND)
    {
      readtok ();
      val2 = exp5 ();
      val1 = val1 & val2;
    }

  return (val1);
}

static long
exp5 ()
{
  register long val1, val2;

  val1 = exp4 ();

  while ((curtok == EQEQ) || (curtok == NEQ))
    {
      int op = curtok;

      readtok ();
      val2 = exp4 ();
      if (op == EQEQ)
	val1 = (val1 == val2);
      else if (op == NEQ)
	val1 = (val1 != val2);
    }
  return (val1);
}

static long
exp4 ()
{
  register long val1, val2;

  val1 = expshift ();
  while ((curtok == LEQ) ||
	 (curtok == GEQ) ||
	 (curtok == LT) ||
	 (curtok == GT))
    {
      int op = curtok;

      readtok ();
      val2 = expshift ();

      if (op == LEQ)
	val1 = val1 <= val2;
      else if (op == GEQ)
	val1 = val1 >= val2;
      else if (op == LT)
	val1 = val1 < val2;
      else			/* (op == GT) */
	val1 = val1 > val2;
    }
  return (val1);
}

/* Left and right shifts. */
static long
expshift ()
{
  register long val1, val2;

  val1 = exp3 ();

  while ((curtok == LSH) || (curtok == RSH))
    {
      int op = curtok;

      readtok ();
      val2 = exp3 ();

      if (op == LSH)
	val1 = val1 << val2;
      else
	val1 = val1 >> val2;
    }

  return (val1);
}

static long
exp3 ()
{
  register long val1, val2;

  val1 = exp2 ();

  while ((curtok == PLUS) || (curtok == MINUS))
    {
      int op = curtok;

      readtok ();
      val2 = exp2 ();

      if (op == PLUS)
	val1 += val2;
      else if (op == MINUS)
	val1 -= val2;
    }
  return (val1);
}

static long
exp2 ()
{
  register long val1, val2;

  val1 = exppower ();

  while ((curtok == MUL) ||
         (curtok == DIV) ||
         (curtok == MOD))
    {
      int op = curtok;

      readtok ();

      val2 = exppower ();

      if (((op == DIV) || (op == MOD)) && (val2 == 0))
	evalerror ("division by 0");

      if (op == MUL)
        val1 *= val2;
      else if (op == DIV)
        val1 /= val2;
      else if (op == MOD)
        val1 %= val2;
    }
  return (val1);
}

static long
exppower ()
{
  register long val1, val2, c;

  val1 = exp1 ();
  if (curtok == POWER)
    {
      readtok ();
      val2 = exp1 ();
      if (val2 == 0)
	return (1L);
      for (c = 1; val2--; c *= val1)
	;
      val1 = c;
    }
  return (val1);
}

static long
exp1 ()
{
  register long val;

  if (curtok == NOT)
    {
      readtok ();
      val = !exp1 ();
    }
  else if (curtok == BNOT)
    {
      readtok ();
      val = ~exp1 ();
    }
  else
    val = exp0 ();

  return (val);
}

static long
exp0 ()
{
  register long val = 0L;

  if (curtok == MINUS)
    {
      readtok ();
      val = - exp0 ();
    }
  else if (curtok == PLUS)
    {
      readtok ();
      val = exp0 ();
    }
  else if (curtok == LPAR)
    {
      readtok ();
      val = expassign ();

      if (curtok != RPAR)
	evalerror ("missing `)'");

      /* Skip over closing paren. */
      readtok ();
    }
  else if ((curtok == NUM) || (curtok == STR))
    {
      val = tokval;
      readtok ();
    }
  else
    evalerror ("syntax error: operand expected");

  return (val);
}

/* Lexical analyzer/token reader for the expression evaluator.  Reads the
   next token and puts its value into curtok, while advancing past it.
   Updates value of tp.  May also set tokval (for number) or tokstr (for
   string). */
static void
readtok ()
{
  register char *cp;
  register int c, c1, e;

  /* Skip leading whitespace. */
  cp = tp;
  c = e = 0;
  while (cp && (c = *cp) && (cr_whitespace (c)))
    cp++;

  if (c)
    cp++;

  lasttp = tp = cp - 1;

  if (c == '\0')
    {
      lasttok = curtok;
      curtok = 0;
      tp = cp;
      return;
    }

  if (legal_variable_starter (c))
    {
      /* Semi-bogus ksh compatibility feature -- variable names
	 not preceded with a dollar sign are shell variables. */
      char *value;

      while (legal_variable_char (c))
	c = *cp++;

      c = *--cp;

#if defined (ARRAY_VARS)
      if (c == '[')
	{
	  e = skipsubscript (cp, 0);
	  if (cp[e] == ']')
	    {
	      cp += e + 1;
	      c = *cp;
	      e = ']';
	    }
	  else
	    evalerror ("bad array subscript");
	}
#endif /* ARRAY_VARS */

      *cp = '\0';

      FREE (tokstr);
      tokstr = savestring (tp);

#if defined (ARRAY_VARS)
      value = (e == ']') ? get_array_value (tokstr, 0) : get_string_value (tokstr);
#else
      value = get_string_value (tokstr);
#endif

      tokval = (value && *value) ? subexpr (value) : 0;

#if defined (ARRAY_VARS)
      if (e == ']')
	FREE (value);	/* get_array_value returns newly-allocated memory */
#endif

      *cp = c;
      lasttok = curtok;
      curtok = STR;
    }
  else if (digit(c))
    {
      while (digit (c) || isletter (c) || c == '#' || c == '@' || c == '_')
	c = *cp++;

      c = *--cp;
      *cp = '\0';

      tokval = strlong (tp);
      *cp = c;
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
      else if ((c == LT) && (c1 == LT))
	{
	  if (*cp == '=')	/* a <<= b */
	    {
	      assigntok = LSH;
	      c = OP_ASSIGN;
	      cp++;
	    }
	  else
	    c = LSH;
	}
      else if ((c == GT) && (c1 == GT))
	{
	  if (*cp == '=')
	    {
	      assigntok = RSH;	/* a >>= b */
	      c = OP_ASSIGN;
	      cp++;
	    }
	  else
	    c = RSH;
	}
      else if ((c == BAND) && (c1 == BAND))
	c = LAND;
      else if ((c == BOR) && (c1 == BOR))
	c = LOR;
      else if ((c == '*') && (c1 == '*'))
        c = POWER;
      else if (c1 == EQ && member(c, "*/%+-&^|"))
	{
	  assigntok = c;	/* a OP= b */
	  c = OP_ASSIGN;
	}
      else
	cp--;			/* `unget' the character */
      lasttok = curtok;
      curtok = c;
    }
  tp = cp;
}

static void
evalerror (msg)
     char *msg;
{
  char *name, *t;

  name = this_command_name;
  for (t = expression; whitespace (*t); t++)
    ;
  internal_error ("%s%s%s: %s (error token is \"%s\")",
		   name ? name : "", name ? ": " : "", t,
		   msg, (lasttp && *lasttp) ? lasttp : "");
  longjmp (evalbuf, 1);
}

/* Convert a string to a long integer, with an arbitrary base.
   0nnn -> base 8
   0[Xx]nn -> base 16
   Anything else: [base#]number (this is implemented to match ksh93)

   Base may be >=2 and <=64.  If base is <= 36, the numbers are drawn
   from [0-9][a-zA-Z], and lowercase and uppercase letters may be used
   interchangably.  If base is > 36 and <= 64, the numbers are drawn
   from [0-9][a-z][A-Z]_@ (a = 10, z = 35, A = 36, Z = 61, _ = 62, @ = 63 --
   you get the picture). */

static long
strlong (num)
     char *num;
{
  register char *s;
  register int c;
  int base, foundbase;
  long val = 0L;

  s = num;
  if (s == NULL || *s == '\0')
    return 0L;

  base = 10;
  foundbase = 0;
  if (*s == '0')
    {
      s++;

      if (s == NULL || *s == '\0')
	return 0L;

       /* Base 16? */
      if (*s == 'x' || *s == 'X')
	{
	  base = 16;
	  s++;
	}
      else
	base = 8;
      foundbase++;
    }

  val = 0L;
  for (c = *s++; c; c = *s++)
    {
      if (c == '#')
	{
	  if (foundbase)
	    evalerror ("bad number");

	  base = (int)val;

	  /* Illegal base specifications raise an evaluation error. */
	  if (base < 2 || base > 64)
	    evalerror ("illegal arithmetic base");

	  val = 0L;
	  foundbase++;
	}
      else if (isletter(c) || digit(c) || (c == '_') || (c == '@'))
	{
	  if (digit(c))
	    c = digit_value(c);
	  else if (c >= 'a' && c <= 'z')
	    c -= 'a' - 10;
	  else if (c >= 'A' && c <= 'Z')
	    c -= 'A' - ((base <= 36) ? 10 : 36);
	  else if (c == '_')
	    c = 62;
	  else if (c == '@')
	    c = 63;

	  if (c >= base)
	    evalerror ("value too great for base");

	  val = (val * base) + c;
	}
      else
	break;
    }
  return (val);
}

#if defined (EXPR_TEST)
char *
xmalloc (n)
     int n;
{
  return (malloc (n));
}

char *
xrealloc (s, n)
     char *s;
     int n;
{
  return (realloc (s, n));
}

SHELL_VAR *find_variable () { return 0;}
SHELL_VAR *bind_variable () { return 0; }

char *get_string_value () { return 0; }

procenv_t top_level;

main (argc, argv)
     int argc;
     char **argv;
{
  register int i;
  long v;
  int expok;

  if (setjmp (top_level))
    exit (0);

  for (i = 1; i < argc; i++)
    {
      v = evalexp (argv[i], &expok);
      if (expok == 0)
        fprintf (stderr, "%s: expression error\n", argv[i]);
      else
        printf ("'%s' -> %ld\n", argv[i], v);
    }
  exit (0);
}

int
builtin_error (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  fprintf (stderr, "expr: ");
  fprintf (stderr, format, arg1, arg2, arg3, arg4, arg5);
  fprintf (stderr, "\n");
  return 0;
}

char *
itos (n)
     int n;
{
  return ("42");
}

#endif /* EXPR_TEST */
