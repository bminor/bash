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

#include <stdio.h>

#if defined (HAVE_VARARGS_H)
#  include <varargs.h>
#endif

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#include "shell.h"
#include "y.tab.h"
#include "stdc.h"
#include "builtins/common.h"

#if defined (__GNUC__) || defined (ardent)
extern int printf __P((const char *, ...));	/* Yuck.  Double yuck. */
#endif

static int indentation = 0;
static int indentation_amount = 4;

static void cprintf (), newline (), indent (), the_printed_command_resize ();
static void semicolon ();

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

#define PRINTED_COMMAND_GROW_SIZE 1024

char *the_printed_command = (char *)NULL;
int the_printed_command_size = 0;
int command_string_index = 0;

/* Non-zero means the stuff being printed is inside of a function def. */
static int inside_function_def = 0;
static int skip_this_indent = 0;

/* The depth of the group commands that we are currently printing.  This
   includes the group command that is a function body. */
static int group_command_nesting = 0;

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
  command_string_index = 0;
  make_command_string_internal (command);
  return (the_printed_command);
}

/* The internal function.  This is the real workhorse. */
static void
make_command_string_internal (command)
     COMMAND *command;
{
  if (!command)
    cprintf ("");
  else
    {
      if (skip_this_indent)
	skip_this_indent--;
      else
	indent (indentation);

      if (command->flags & CMD_WANT_SUBSHELL)
	cprintf ("( ");

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
	      cprintf (";");

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
  while (list)
    {
      (*pfunc) ("%s", list->word->word);
      list = list->next;
      if (list)
	(*pfunc) ("%s", separator);
    }
}

void print_word_list (list, separator)
     WORD_LIST *list;
     char *separator;
{
  _print_word_list (list, separator, (VFunction *)printf);
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

  if (!inside_function_def)
    skip_this_indent++;
  else
    {
      /* This is a group command { ... } inside of a function
	 definition, and should be handled as a `normal' group
	 command, using the current indentation. */
      cprintf ("\n");
      indentation += indentation_amount;
    }

  make_command_string_internal (group_command->command);

  cprintf ("\n");

  if (group_command_nesting)
    {
      indentation -= indentation_amount;
      indent (indentation);
      if (!indentation)
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
  while (redirects)
    {
      print_redirection (redirects);
      redirects = redirects->next;
      if (redirects)
	cprintf (" ");
    }
}

static void
print_redirection (redirect)
     REDIRECT *redirect;
{
  int kill_leading = 0;
  int redirector = redirect->redirector;
  WORD_DESC *redirectee = redirect->redirectee.filename;
  int redir_fd = redirect->redirectee.dest;

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
      if (redirect->redirectee.filename->quoted)
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

  if (func->command->type == cm_group)
    make_command_string_internal (func->command->value.Group->command);
  else
    make_command_string_internal (func->command);
    
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
  int old_indent = indentation, old_amount = indentation_amount;

  command_string_index = 0;

  if (name && *name)
    cprintf ("%s ", name);

  cprintf ("() ");

  if (!multi_line)
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

  if (multi_line)
    cprintf ("{ \n");
  else
    cprintf ("{ ");

  if (command->type == cm_group)
    make_command_string_internal (command->value.Group->command);
  else
    make_command_string_internal (command);

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

static void
indent (amount)
     int amount;
{
  while (amount-- > 0)
    cprintf (" ");
}

static void
semicolon ()
{
  if (command_string_index > 0 && the_printed_command[command_string_index - 1] == '&')
    return;
  cprintf (";");
}

#if !defined (HAVE_VARARGS_H)
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
cprintf (va_alist)
     va_dcl
{
  register char *s;
  char *control, char_arg[2], *argp;
  int digit_arg, arg_len, c;
  va_list args;

  va_start (args);
  control = va_arg (args, char *);

  arg_len = strlen (control);
  the_printed_command_resize (arg_len + 1);

  char_arg[1] = '\0';
  s = control;
  while (s && *s)
    {
      int free_argp = 0;
      c = *s++;
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
#endif /* HAVE_VARARGS_H */

/* Ensure that there is enough space to stuff LENGTH characters into
   THE_PRINTED_COMMAND. */
static void
the_printed_command_resize (length)
     int length;
{
  if (!the_printed_command)
    {
      the_printed_command_size = length + 1;
      the_printed_command = xmalloc (the_printed_command_size);
      command_string_index = 0;
    }
  else if ((command_string_index + length) >= the_printed_command_size)
    {
      int new;
      new = command_string_index + length + 1;
      new = new + 2 * PRINTED_COMMAND_GROW_SIZE - 1;
      new -= new % PRINTED_COMMAND_GROW_SIZE;
      the_printed_command_size = new;
      the_printed_command = xrealloc (the_printed_command, the_printed_command_size);
    }
}
