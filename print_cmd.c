/* print_command -- A way to make readable commands from a command tree. */
/* Copyright (C) 1989 Free Software Foundation, Inc.

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

#include "config.h"

#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#if defined (PREFER_STDARG)
#  include <stdarg.h>
#else
#  if defined (PREFER_VARARGS)
#    include <varargs.h>
#  endif
#endif

#include "bashansi.h"

#include "shell.h"
#include <y.tab.h>	/* use <...> so we pick it up from the build directory */
#include "stdc.h"
#include "builtins/common.h"

#if !defined (PRINTF_DECLARED)
extern int printf __P((const char *, ...));	/* Yuck.  Double yuck. */
#endif

static int indentation;
static int indentation_amount = 4;

#if defined (PREFER_STDARG)
static void cprintf __P((char *, ...));
#else
static void cprintf ();
#endif

static void newline (), indent (), the_printed_command_resize ();
static void semicolon ();
static void xprintf ();

static void make_command_string_internal ();
static void command_print_word_list ();
static void print_case_clauses ();
static void print_redirection_list ();
static void print_redirection ();

static void print_for_command ();
#if defined (SELECT_COMMAND)
static void print_select_command ();
#endif
static void print_group_command ();
static void print_case_command ();
static void print_while_command ();
static void print_until_command ();
static void print_until_or_while ();
static void print_if_command ();
static void print_function_def ();
#if defined (DPAREN_ARITHMETIC)
static void print_arith_command ();
#endif
#if defined (COND_COMMAND)
static void print_cond_node ();
static void print_cond_command ();
#endif

#define PRINTED_COMMAND_INITIAL_SIZE 64
#define PRINTED_COMMAND_GROW_SIZE 128

char *the_printed_command = (char *)NULL;
int the_printed_command_size = 0;
int command_string_index = 0;

/* Non-zero means the stuff being printed is inside of a function def. */
static int inside_function_def;
static int skip_this_indent;
static int was_heredoc;

/* The depth of the group commands that we are currently printing.  This
   includes the group command that is a function body. */
static int group_command_nesting;

/* Print COMMAND (a command tree) on standard output. */
void
print_command (command)
     COMMAND *command;
{
  command_string_index = 0;
  printf ("%s", make_command_string (command));
}

/* Make a string which is the printed representation of the command
   tree in COMMAND.  We return this string.  However, the string is
   not consed, so you have to do that yourself if you want it to
   remain around. */
char *
make_command_string (command)
     COMMAND *command;
{
  command_string_index = was_heredoc = 0;
  make_command_string_internal (command);
  return (the_printed_command);
}

/* The internal function.  This is the real workhorse. */
static void
make_command_string_internal (command)
     COMMAND *command;
{
  if (command == 0)
    cprintf ("");
  else
    {
      if (skip_this_indent)
	skip_this_indent--;
      else
	indent (indentation);

      if (command->flags & CMD_WANT_SUBSHELL)
	cprintf ("( ");

      if (command->flags & CMD_TIME_PIPELINE)
	{
	  cprintf ("time ");
	  if (command->flags & CMD_TIME_POSIX)
	    cprintf ("-p ");
	}

      if (command->flags & CMD_INVERT_RETURN)
	cprintf ("! ");

      switch (command->type)
	{
	case cm_for:
	  print_for_command (command->value.For);
	  break;

#if defined (SELECT_COMMAND)
	case cm_select:
	  print_select_command (command->value.Select);
	  break;
#endif

	case cm_case:
	  print_case_command (command->value.Case);
	  break;

	case cm_while:
	  print_while_command (command->value.While);
	  break;

	case cm_until:
	  print_until_command (command->value.While);
	  break;

	case cm_if:
	  print_if_command (command->value.If);
	  break;

#if defined (DPAREN_ARITHMETIC)
	case cm_arith:
	  print_arith_command (command->value.Arith);
	  break;
#endif

#if defined (COND_COMMAND)
	case cm_cond:
	  print_cond_command (command->value.Cond);
	  break;
#endif

	case cm_simple:
	  print_simple_command (command->value.Simple);
	  break;

	case cm_connection:

	  skip_this_indent++;
	  make_command_string_internal (command->value.Connection->first);

	  switch (command->value.Connection->connector)
	    {
	    case '&':
	    case '|':
	      {
		char c = command->value.Connection->connector;
		cprintf (" %c", c);
		if (c != '&' || command->value.Connection->second)
		  {
		    cprintf (" ");
		    skip_this_indent++;
		  }
	      }
	      break;

	    case AND_AND:
	      cprintf (" && ");
	      if (command->value.Connection->second)
		skip_this_indent++;
	      break;

	    case OR_OR:
	      cprintf (" || ");
	      if (command->value.Connection->second)
		skip_this_indent++;
	      break;

	    case ';':
	      if (was_heredoc == 0)
		cprintf (";");
	      else
		was_heredoc = 0;

	      if (inside_function_def)
		cprintf ("\n");
	      else
		{
		  cprintf (" ");
		  if (command->value.Connection->second)
		    skip_this_indent++;
		}
	      break;

	    default:
	      cprintf ("print_command: bad connector `%d'",
		       command->value.Connection->connector);
	      break;
	    }

	  make_command_string_internal (command->value.Connection->second);
	  break;

	case cm_function_def:
	  print_function_def (command->value.Function_def);
	  break;

	case cm_group:
	  print_group_command (command->value.Group);
	  break;

	default:
	  programming_error ("print_command: bad command type `%d'", command->type);
	  break;
	}

      if (command->flags & CMD_WANT_SUBSHELL)
	cprintf (" )");

      if (command->redirects)
	print_redirection_list (command->redirects);
    }
}

static void
_print_word_list (list, separator, pfunc)
     WORD_LIST *list;
     char *separator;
     VFunction *pfunc;
{
  WORD_LIST *w;

  for (w = list; w; w = w->next)
    (*pfunc) ("%s%s", w->word->word, w->next ? separator : "");
}

void
print_word_list (list, separator)
     WORD_LIST *list;
     char *separator;
{
  _print_word_list (list, separator, xprintf);
}

/* A function to print the words of a simple command when set -x is on. */
void
xtrace_print_word_list (list)
     WORD_LIST *list;
{
  WORD_LIST *w;
  char *t, *x;

  fprintf (stderr, "%s", indirection_level_string ());
  for (w = list; w; w = w->next)
    {
      t = w->word->word;
      if (t == 0 || *t == '\0')
	fprintf (stderr, "''%s", w->next ? " " : "");
      else if (contains_shell_metas (t))
	{
	  x = single_quote (t);
	  fprintf (stderr, "%s%s", x, w->next ? " " : "");
	  free (x);
	}
      else
	fprintf (stderr, "%s%s", t, w->next ? " " : "");
    }
  fprintf (stderr, "\n");
}

static void
command_print_word_list (list, separator)
     WORD_LIST *list;
     char *separator;
{
  _print_word_list (list, separator, cprintf);
}

static void
print_for_command (for_command)
     FOR_COM *for_command;
{
  cprintf ("for %s in ", for_command->name->word);
  command_print_word_list (for_command->map_list, " ");
  cprintf (";");
  newline ("do\n");
  indentation += indentation_amount;
  make_command_string_internal (for_command->action);
  semicolon ();
  indentation -= indentation_amount;
  newline ("done");
}

#if defined (SELECT_COMMAND)
static void
print_select_command (select_command)
     SELECT_COM *select_command;
{
  cprintf ("select %s in ", select_command->name->word);
  command_print_word_list (select_command->map_list, " ");
  cprintf (";");
  newline ("do\n");
  indentation += indentation_amount;
  make_command_string_internal (select_command->action);
  semicolon ();
  indentation -= indentation_amount;
  newline ("done");
}
#endif /* SELECT_COMMAND */

static void
print_group_command (group_command)
     GROUP_COM *group_command;
{
  group_command_nesting++;
  cprintf ("{ ");

  if (inside_function_def == 0)
    skip_this_indent++;
  else
    {
      /* This is a group command { ... } inside of a function
	 definition, and should be printed as a multiline group
	 command, using the current indentation. */
      cprintf ("\n");
      indentation += indentation_amount;
    }

  make_command_string_internal (group_command->command);

  if (inside_function_def)
    {
      cprintf ("\n");
      indentation -= indentation_amount;
      indent (indentation);
    }
  else
    {
      semicolon ();
      cprintf (" ");
    }

  cprintf ("}");

  group_command_nesting--;
}

static void
print_case_command (case_command)
     CASE_COM *case_command;
{
  cprintf ("case %s in ", case_command->word->word);
  if (case_command->clauses)
    print_case_clauses (case_command->clauses);
  newline ("esac");
}

static void
print_case_clauses (clauses)
     PATTERN_LIST *clauses;
{
  indentation += indentation_amount;
  while (clauses)
    {
      newline ("");
      command_print_word_list (clauses->patterns, " | ");
      cprintf (")\n");
      indentation += indentation_amount;
      make_command_string_internal (clauses->action);
      indentation -= indentation_amount;
      newline (";;");
      clauses = clauses->next;
    }
  indentation -= indentation_amount;
}

static void
print_while_command (while_command)
     WHILE_COM *while_command;
{
  print_until_or_while (while_command, "while");
}

static void
print_until_command (while_command)
     WHILE_COM *while_command;
{
  print_until_or_while (while_command, "until");
}

static void
print_until_or_while (while_command, which)
     WHILE_COM *while_command;
     char *which;
{
  cprintf ("%s ", which);
  skip_this_indent++;
  make_command_string_internal (while_command->test);
  semicolon ();
  cprintf (" do\n");	/* was newline ("do\n"); */
  indentation += indentation_amount;
  make_command_string_internal (while_command->action);
  indentation -= indentation_amount;
  semicolon ();
  newline ("done");
}

static void
print_if_command (if_command)
     IF_COM *if_command;
{
  cprintf ("if ");
  skip_this_indent++;
  make_command_string_internal (if_command->test);
  semicolon ();
  cprintf (" then\n");
  indentation += indentation_amount;
  make_command_string_internal (if_command->true_case);
  indentation -= indentation_amount;

  if (if_command->false_case)
    {
      semicolon ();
      newline ("else\n");
      indentation += indentation_amount;
      make_command_string_internal (if_command->false_case);
      indentation -= indentation_amount;
    }
  semicolon ();
  newline ("fi");
}

#if defined (DPAREN_ARITHMETIC)
static void
print_arith_command (arith_command)
     ARITH_COM *arith_command;
{
  cprintf ("(( ");
  command_print_word_list (arith_command->exp, " ");
  cprintf (" ))");
}

#if defined (COND_COMMAND)
static void
print_cond_node (cond)
     COND_COM *cond;
{
  if (cond->flags & CMD_INVERT_RETURN)
    cprintf ("! ");

  if (cond->type == COND_EXPR)
    {
      cprintf ("( ");
      print_cond_node (cond->left);
      cprintf (" )");
    }
  else if (cond->type == COND_AND)
    {
      print_cond_node (cond->left);
      cprintf (" && ");
      print_cond_node (cond->right);
    }
  else if (cond->type == COND_OR)
    {
      print_cond_node (cond->left);
      cprintf (" || ");
      print_cond_node (cond->right);
    }
  else if (cond->type == COND_UNARY)
    {
      cprintf (cond->op->word);
      cprintf (" ");
      print_cond_node (cond->left);
    }
  else if (cond->type == COND_BINARY)
    {
      print_cond_node (cond->left);
      cprintf (" ");
      cprintf (cond->op->word);
      cprintf (" ");
      print_cond_node (cond->right);
    }
  else if (cond->type == COND_TERM)
    {
      cprintf (cond->op->word);		/* need to add quoting here */
    }
}

static void
print_cond_command (cond)
     COND_COM *cond;
{
  cprintf ("[[ ");
  print_cond_node (cond);
  cprintf (" ]]");
}

void
debug_print_cond_command (cond)
     COND_COM *cond;
{
  fprintf (stderr, "DEBUG: ");
  command_string_index = 0;
  print_cond_command (cond);
  fprintf (stderr, "%s\n", the_printed_command);
}

void
xtrace_print_cond_term (type, invert, op, arg1, arg2)
     int type, invert;
     WORD_DESC *op;
     char *arg1, *arg2;
{
  command_string_index = 0;
  fprintf (stderr, "%s", indirection_level_string ());
  fprintf (stderr, "[[ ");
  if (invert)
    fprintf (stderr, "! ");

  if (type == COND_UNARY)
    {
      fprintf (stderr, "%s ", op->word);
      fprintf (stderr, "%s", (arg1 && *arg1) ? arg1 : "''");
    }
  else if (type == COND_BINARY)
    {
      fprintf (stderr, "%s", (arg1 && *arg1) ? arg1 : "''");
      fprintf (stderr, " %s ", op->word);
      fprintf (stderr, "%s", (arg2 && *arg2) ? arg2 : "''");
    }

  fprintf (stderr, " ]]\n");
}	  
#endif /* COND_COMMAND */

/* A function to print the words of an arithmetic command when set -x is on. */
void
xtrace_print_arith_cmd (list)
     WORD_LIST *list;
{
  WORD_LIST *w;

  fprintf (stderr, "%s", indirection_level_string ());
  fprintf (stderr, "(( ");
  for (w = list; w; w = w->next)
    fprintf (stderr, "%s%s", w->word->word, w->next ? " " : "");
  fprintf (stderr, " ))\n");
}
#endif

void
print_simple_command (simple_command)
     SIMPLE_COM *simple_command;
{
  command_print_word_list (simple_command->words, " ");

  if (simple_command->redirects)
    {
      cprintf (" ");
      print_redirection_list (simple_command->redirects);
    }
}

static void
print_redirection_list (redirects)
     REDIRECT *redirects;
{
  REDIRECT *heredocs, *hdtail, *newredir;

  heredocs = (REDIRECT *)NULL;
  hdtail = heredocs;

  was_heredoc = 0;
  while (redirects)
    {
      /* Defer printing the here documents until we've printed the
	 rest of the redirections. */
      if (redirects->instruction == r_reading_until || redirects->instruction == r_deblank_reading_until)
	{
	  newredir = copy_redirect (redirects);
	  newredir->next = (REDIRECT *)NULL;
	  if (heredocs)
	    {
	      hdtail->next = newredir;
	      hdtail = newredir;
	    }
	  else
	    hdtail = heredocs = newredir;
	}
      else
	print_redirection (redirects);

      redirects = redirects->next;
      if (redirects)
	cprintf (" ");
    }

  /* Now that we've printed all the other redirections (on one line),
     print the here documents. */
  if (heredocs)
    {
      cprintf (" "); 
      for (hdtail = heredocs; hdtail; hdtail = hdtail->next)
        {
	  print_redirection (hdtail);
	  cprintf ("\n");
        }
      dispose_redirects (heredocs);
      was_heredoc = 1;
    }
}

static void
print_redirection (redirect)
     REDIRECT *redirect;
{
  int kill_leading, redirector, redir_fd;
  WORD_DESC *redirectee;

  kill_leading = 0;
  redirectee = redirect->redirectee.filename;
  redirector = redirect->redirector;
  redir_fd = redirect->redirectee.dest;

  switch (redirect->instruction)
    {
    case r_output_direction:
      if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf (">%s", redirectee->word);
      break;

    case r_input_direction:
      if (redirector != 0)
	cprintf ("%d", redirector);
      cprintf ("<%s", redirectee->word);
      break;

    case r_inputa_direction:	/* Redirection created by the shell. */
      cprintf ("&");
      break;

    case r_appending_to:
      if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf (">>%s", redirectee->word);
      break;

    case r_deblank_reading_until:
      kill_leading++;
      /* ... */
    case r_reading_until:
      if (redirector != 0)
	cprintf ("%d", redirector);
      /* If the here document delimiter is quoted, single-quote it. */
      if (redirect->redirectee.filename->flags & W_QUOTED)
        {
          char *x;
          x = single_quote (redirect->here_doc_eof);
	  cprintf ("<<%s%s\n", kill_leading? "-" : "", x);
          free (x);
        }
      else
	cprintf ("<<%s%s\n", kill_leading? "-" : "", redirect->here_doc_eof);
      cprintf ("%s%s",
	       redirect->redirectee.filename->word, redirect->here_doc_eof);
      break;

    case r_duplicating_input:
      cprintf ("%d<&%d", redirector, redir_fd);
      break;

    case r_duplicating_output:
      cprintf ("%d>&%d", redirector, redir_fd);
      break;

    case r_duplicating_input_word:
      cprintf ("%d<&%s", redirector, redirectee->word);
      break;

    case r_duplicating_output_word:
      cprintf ("%d>&%s", redirector, redirectee->word);
      break;

    case r_close_this:
      cprintf ("%d>&-", redirector);
      break;

    case r_err_and_out:
      cprintf (">&%s", redirectee->word);
      break;

    case r_input_output:
      if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf ("<>%s", redirectee->word);
      break;

    case r_output_force:
      if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf (">|%s", redirectee->word);
      break;
    }
}

static void
reset_locals ()
{
  inside_function_def = 0;
  indentation = 0;
}

static void
print_function_def (func)
     FUNCTION_DEF *func;
{
  cprintf ("function %s () \n", func->name->word);
  add_unwind_protect (reset_locals, 0);

  indent (indentation);
  cprintf ("{ \n");

  inside_function_def++;
  indentation += indentation_amount;

  make_command_string_internal (func->command->type == cm_group
					? func->command->value.Group->command
					: func->command);

  remove_unwind_protect ();
  indentation -= indentation_amount;
  inside_function_def--;

  newline ("}");
}

/* Return the string representation of the named function.
   NAME is the name of the function.
   COMMAND is the function body.  It should be a GROUP_COM.
   MULTI_LINE is non-zero to pretty-print, or zero for all on one line.
  */
char *
named_function_string (name, command, multi_line)
     char *name;
     COMMAND *command;
     int multi_line;
{
  char *result;
  int old_indent, old_amount;

  old_indent = indentation;
  old_amount = indentation_amount;
  command_string_index = was_heredoc = 0;

  if (name && *name)
    cprintf ("%s ", name);

  cprintf ("() ");

  if (multi_line == 0)
    {
      indentation = 1;
      indentation_amount = 0;
    }
  else
    {
      cprintf ("\n");
      indentation += indentation_amount;
    }

  inside_function_def++;

  cprintf (multi_line ? "{ \n" : "{ ");

  make_command_string_internal (command->type == cm_group
					? command->value.Group->command
					: command);

  indentation = old_indent;
  indentation_amount = old_amount;
  inside_function_def--;

  newline ("}");

  result = the_printed_command;

  if (!multi_line)
    {
#if 0
      register int i;
      for (i = 0; result[i]; i++)
	if (result[i] == '\n')
	  {
	    strcpy (result + i, result + i + 1);
	    --i;
	  }
#else
      if (result[2] == '\n')	/* XXX -- experimental */
        strcpy (result + 2, result + 3);
#endif
    }

  return (result);
}

static void
newline (string)
     char *string;
{
  cprintf ("\n");
  indent (indentation);
  if (string && *string)
    cprintf ("%s", string);
}

static char *indentation_string;
static int indentation_size;

static void
indent (amount)
     int amount;
{
  register int i;

  RESIZE_MALLOCED_BUFFER (indentation_string, 0, amount, indentation_size, 16);

  for (i = 0; amount > 0; amount--)
    indentation_string[i++] = ' ';
  indentation_string[i] = '\0';
  cprintf (indentation_string);
}

static void
semicolon ()
{
  if (command_string_index > 0 && the_printed_command[command_string_index - 1] == '&')
    return;
  cprintf (";");
}

#if !defined (USE_VARARGS)
/* How to make the string. */
static void
cprintf (format, arg1, arg2)
     char *format, *arg1, *arg2;
{
  register char *s;
  char char_arg[2], *argp, *args[2];
  int arg_len, c, arg_index;

  args[arg_index = 0] = arg1;
  args[1] = arg2;

  arg_len = strlen (format);
  the_printed_command_resize (arg_len + 1);

  char_arg[1] = '\0';
  s = format;
  while (s && *s)
    {
      int free_argp = 0;
      c = *s++;
      if (c != '%' || !*s)
	{
	  argp = s;
	  arg_len = 1;
	}
      else
	{
	  c = *s++;
	  switch (c)
	    {
	    case '%':
	      char_arg[0] = c;
	      argp = char_arg;
	      arg_len = 1;
	      break;

	    case 's':
	      argp = (char *)args[arg_index++];
	      arg_len = strlen (argp);
	      break;

	    case 'd':
	      argp = itos (pointer_to_int (args[arg_index]));
	      arg_index++;
	      arg_len = strlen (argp);
	      free_argp = 1;
	      break;

	    case 'c':
	      char_arg[0] = pointer_to_int (args[arg_index]);
	      arg_index++;
	      argp = char_arg;
	      arg_len = 1;
	      break;

	    default:
	      programming_error ("cprintf: bad `%%' argument (%c)", c);
	    }
	}
      if (argp)
	{
	  the_printed_command_resize (arg_len + 1);
	  FASTCOPY (argp, the_printed_command + command_string_index, arg_len);
	  command_string_index += arg_len;
	  if (free_argp)
	    free (argp);
	}
    }

  the_printed_command[command_string_index] = '\0';
}

#else /* We have support for varargs. */

/* How to make the string. */
static void
#if defined (PREFER_STDARG)
cprintf (char *control, ...)
#else
cprintf (control, va_alist)
     char *control;
     va_dcl
#endif
{
  register char *s;
  char char_arg[2], *argp;
  int digit_arg, arg_len, c;
  va_list args;

#if defined (PREFER_STDARG)
  va_start (args, control);
#else
  va_start (args);
#endif

  arg_len = strlen (control);
  the_printed_command_resize (arg_len + 1);

  char_arg[1] = '\0';
  s = control;
  while (s && *s)
    {
      int free_argp;
      free_argp = 0;
      c = *s++;
      argp = (char *)NULL;
      if (c != '%' || !*s)
	{
	  argp = s - 1;
	  arg_len = 1;
	}
      else
	{
	  c = *s++;
	  switch (c)
	    {
	    case '%':
	      char_arg[0] = c;
	      argp = char_arg;
	      arg_len = 1;
	      break;

	    case 's':
	      argp = va_arg (args, char *);
	      arg_len = strlen (argp);
	      break;

	    case 'd':
	      digit_arg = va_arg (args, int);
	      argp = itos (digit_arg);
	      arg_len = strlen (argp);
	      free_argp = 1;
	      break;

	    case 'c':
	      char_arg[0] = va_arg (args, int);
	      argp = char_arg;
	      arg_len = 1;
	      break;

	    default:
	      programming_error ("cprintf: bad `%%' argument (%c)", c);
	      /*NOTREACHED*/
	    }
	}

      if (argp && arg_len)
	{
	  the_printed_command_resize (arg_len + 1);
	  FASTCOPY (argp, the_printed_command + command_string_index, arg_len);
	  command_string_index += arg_len;
	  if (free_argp)
	    free (argp);
	}
    }

  the_printed_command[command_string_index] = '\0';
}
#endif /* HAVE_VARARGS_H */

/* Ensure that there is enough space to stuff LENGTH characters into
   THE_PRINTED_COMMAND. */
static void
the_printed_command_resize (length)
     int length;
{
  if (the_printed_command == 0)
    {
      the_printed_command_size = (length + PRINTED_COMMAND_INITIAL_SIZE - 1) & ~(PRINTED_COMMAND_INITIAL_SIZE - 1);
      the_printed_command = xmalloc (the_printed_command_size);
      command_string_index = 0;
    }
  else if ((command_string_index + length) >= the_printed_command_size)
    {
      int new;
      new = command_string_index + length + 1;
#if 1
      /* Round up to the next multiple of PRINTED_COMMAND_GROW_SIZE. */
      new = (new + PRINTED_COMMAND_GROW_SIZE - 1) & ~(PRINTED_COMMAND_GROW_SIZE - 1);
#else
      new = new + 2 * PRINTED_COMMAND_GROW_SIZE - 1;
      new -= new % PRINTED_COMMAND_GROW_SIZE;
#endif
      the_printed_command_size = new;
      the_printed_command = xrealloc (the_printed_command, the_printed_command_size);
    }
}

#if defined (HAVE_VFPRINTF)

static void
#if defined (PREFER_STDARG)
xprintf (const char *format, ...)
#else
xprintf (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  va_list args;

#if defined (PREFER_STDARG)
  va_start (args, format);
#else
  va_start (args);
#endif

  vfprintf (stdout, format, args);
  va_end (args);
}

#else

static void
xprintf (format, arg1, arg2, arg3, arg4, arg5)
     char *format;
{
  printf (format, arg1, arg2, arg3, arg4, arg5);
}

#endif /* !HAVE_VFPRINTF */
