/* print_command -- A way to make readable commands from a command tree. */

/* Copyright (C) 1989-2025 Free Software Foundation, Inc.

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

#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include <stdarg.h>

#include "bashansi.h"
#include "bashintl.h"

#define NEED_XTRACE_SET_DECL

#include "shell.h"
#include "flags.h"
#include <y.tab.h>	/* use <...> so we pick it up from the build directory */
#include "input.h"

#include "shmbutil.h"

#include "builtins/common.h"

#if !HAVE_DECL_PRINTF
extern int printf (const char *, ...);	/* Yuck.  Double yuck. */
#endif

static int indentation;
static int indentation_amount = 4;

typedef void PFUNC (const char *, ...);

static void cprintf (const char *, ...)  __attribute__((__format__ (printf, 1, 2)));
static void xprintf (const char *, ...)  __attribute__((__format__ (printf, 1, 2)));

static void uw_reset_locals (void *);

static void newline (char *);
static void indent (int);
static void semicolon (void);
static void the_printed_command_resize (size_t);

static void make_command_string_internal (COMMAND *);
static void _print_word_list (WORD_LIST *, char *, PFUNC *);
static void command_print_word_list (WORD_LIST *, char *);
static void print_case_clauses (PATTERN_LIST *);
static void print_redirection_list (REDIRECT *);
static void print_redirection (REDIRECT *);
static void print_heredoc_header (REDIRECT *);
static void print_heredoc_body (REDIRECT *);
static void print_heredocs (REDIRECT *);
static void print_heredoc_bodies (REDIRECT *);
static void print_deferred_heredocs (const char *);

static void print_for_command (FOR_COM *);
#if defined (ARITH_FOR_COMMAND)
static void print_arith_for_command (ARITH_FOR_COM *);
#endif
#if defined (SELECT_COMMAND)
static void print_select_command (SELECT_COM *);
#endif
static void print_group_command (GROUP_COM *);
static void print_case_command (CASE_COM *);
static void print_while_command (WHILE_COM *);
static void print_until_command (WHILE_COM *);
static void print_until_or_while (WHILE_COM *, char *);
static void print_if_command (IF_COM *);
#if defined (COND_COMMAND)
static void print_cond_node (COND_COM *);
#endif
static void print_function_def (FUNCTION_DEF *);

#ifdef DEBUG
void debug_print_word_list (char *, WORD_LIST *, char *);
void debug_print_cond_command (COND_COM *);
#endif

#define PRINTED_COMMAND_INITIAL_SIZE 64
#define PRINTED_COMMAND_GROW_SIZE 128

char *the_printed_command = (char *)NULL;
size_t the_printed_command_size = 0;
int command_string_index = 0;

int xtrace_fd = -1;
FILE *xtrace_fp = 0;

#define CHECK_XTRACE_FP	xtrace_fp = (xtrace_fp ? xtrace_fp : stderr)

/* shell expansion characters: used in print_redirection_list */
#define EXPCHAR(c) ((c) == '{' || (c) == '~' || (c) == '$' || (c) == '`')

#define PRINT_DEFERRED_HEREDOCS(x) \
  do { \
    if (deferred_heredocs) \
      print_deferred_heredocs (x); \
  } while (0)

/* Non-zero means the stuff being printed is inside of a function def. */
static int inside_function_def;
static int skip_this_indent;
static int was_heredoc;
static int printing_connection;
static int printing_comsub;
static REDIRECT *deferred_heredocs;

/* The depth of the group commands that we are currently printing.  This
   includes the group command that is a function body. */
static int group_command_nesting;

/* A buffer to indicate the indirection level (PS4) when set -x is enabled. */
static char *indirection_string = 0;
static size_t indirection_stringsiz = 0;

/* Print COMMAND (a command tree) on standard output. */
void
print_command (COMMAND *command)
{
  command_string_index = 0;
  printf ("%s", make_command_string (command));
}

/* Make a string which is the printed representation of the command
   tree in COMMAND.  We return this string.  However, the string is
   not consed, so you have to do that yourself if you want it to
   remain around. */
char *
make_command_string (COMMAND *command)
{
  command_string_index = was_heredoc = 0;
  deferred_heredocs = 0;
  make_command_string_internal (command);
  return (the_printed_command);
}

/* Print a command substitution after parsing it in parse_comsub to turn it
   back into an external representation without turning newlines into `;'.
   Placeholder for other changes, if any are necessary. */
char *
print_comsub (COMMAND *command)
{
  char *ret;

  printing_comsub++;
  ret = make_command_string (command);
  printing_comsub--;
  return ret;
}

/* The internal function.  This is the real workhorse. */
static void
make_command_string_internal (COMMAND *command)
{
  char s[3];

  if (command == 0)
    cprintf ("");
  else
    {
      if (skip_this_indent)
	skip_this_indent--;
      else
	indent (indentation);

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

#if defined (ARITH_FOR_COMMAND)
	case cm_arith_for:
	  print_arith_for_command (command->value.ArithFor);
	  break;
#endif

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
	  print_arith_command (command->value.Arith->exp);
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
	  printing_connection++;
	  make_command_string_internal (command->value.Connection->first);

	  switch (command->value.Connection->connector)
	    {
	    case '&':
	    case '|':
	      {
		char c = command->value.Connection->connector;

		s[0] = ' ';
		s[1] = c;
		s[2] = '\0';
		
		print_deferred_heredocs (s);

		if (c != '&' || command->value.Connection->second)
		  {
		    cprintf (" ");
		    skip_this_indent++;
		  }
	      }
	      break;

	    case AND_AND:
	      print_deferred_heredocs (" && ");
	      if (command->value.Connection->second)
		skip_this_indent++;
	      break;

	    case OR_OR:
	      print_deferred_heredocs (" || ");
	      if (command->value.Connection->second)
		skip_this_indent++;
	      break;

	    case ';':
	    case '\n':				/* special case this */
	      {
		char c = command->value.Connection->connector;
		int was_newline;

		s[0] = printing_comsub ? c : ';';
		s[1] = '\0';

		was_newline = deferred_heredocs == 0 && was_heredoc == 0 && c == '\n';
		if (deferred_heredocs == 0)
		  {
		    if (was_heredoc == 0)
		      cprintf ("%s", s);	/* inside_function_def? */
		    else
		      was_heredoc = 0;
		  }
		else
		  /* print_deferred_heredocs special-cases `;' */
		  print_deferred_heredocs (inside_function_def ? "" : ";");

		if (inside_function_def)
		  cprintf ("\n");
		else if (printing_comsub && c == '\n' && was_newline == 0)
		  cprintf ("\n");	/* preserve newlines in comsubs but don't double them */
		else
		  {
		    if (c == ';')
		      cprintf (" ");
		    if (command->value.Connection->second)
		      skip_this_indent++;
		  }
		break;
	      }

	    default:
	      cprintf (_("print_command: bad connector `%d'"),
		       command->value.Connection->connector);
	      break;
	    }

	  make_command_string_internal (command->value.Connection->second);
	  /* If this is a recursive call to make_command_string_internal to
	     print a connection with more than two components, defer printing
	     the here-document bodies until our caller can print the
	     connector. Remember that the parser builds lists to be left-side
	     heavy. */
	  if (printing_connection == 1)
	    PRINT_DEFERRED_HEREDOCS ("");
	  printing_connection--;	  	  
	  break;

	case cm_function_def:
	  print_function_def (command->value.Function_def);
	  break;

	case cm_group:
	  print_group_command (command->value.Group);
	  break;

	case cm_subshell:
	  cprintf ("( ");
	  skip_this_indent++;
	  make_command_string_internal (command->value.Subshell->command);
	  PRINT_DEFERRED_HEREDOCS ("");
	  cprintf (" )");
	  was_heredoc = 0;	/* last wasn't heredoc/newline */
	  break;

	case cm_coproc:
	  cprintf ("coproc ");
	  if (command->value.Coproc->command->type != cm_simple)
	    cprintf ("%s ", command->value.Coproc->name);
	  skip_this_indent++;
	  make_command_string_internal (command->value.Coproc->command);
	  break;

	default:
	  command_error ("print_command", CMDERR_BADTYPE, command->type, 0);
	  break;
	}


      if (command->redirects)
	{
	  cprintf (" ");
	  print_redirection_list (command->redirects);
	}
    }
}

static void
_print_word_list (WORD_LIST *list, char *separator, PFUNC *pfunc)
{
  WORD_LIST *w;

  for (w = list; w; w = w->next)
    (*pfunc) ("%s%s", w->word->word, w->next ? separator : "");
}

void
print_word_list (WORD_LIST *list, char *separator)
{
  _print_word_list (list, separator, xprintf);
}

void
xtrace_set (int fd, FILE *fp)
{
  if (fd >= 0 && sh_validfd (fd) == 0)
    {
      internal_error (_("xtrace_set: %d: invalid file descriptor"), fd);
      return;
    }
  if (fp == 0)
    {
      internal_error (_("xtrace_set: NULL file pointer"));
      return;
    }
  if (fd >= 0 && fileno (fp) != fd)
    internal_warning (_("xtrace fd (%d) != fileno xtrace fp (%d)"), fd, fileno (fp));
  
  xtrace_fd = fd;
  xtrace_fp = fp;
}

void
xtrace_init (void)
{
  xtrace_set (-1, stderr);
}

void
xtrace_reset (void)
{
  if (xtrace_fd >= 0 && xtrace_fp)
    {
      fflush (xtrace_fp);
      fclose (xtrace_fp);
    }
  else if (xtrace_fd >= 0)
    close (xtrace_fd);

  xtrace_fd = -1;
  xtrace_fp = stderr;
}

void
xtrace_fdchk (int fd)
{
  if (fd == xtrace_fd)
    xtrace_reset ();
}

/* Return a string denoting what our indirection level is. */

char *
indirection_level_string (void)
{
  register int i, j;
  char *ps4;
  char ps4_firstc[MB_LEN_MAX+1];
  size_t ps4_firstc_len, ps4_len, ineed;
  int old;
  DECLARE_MBSTATE;

  ps4 = get_string_value ("PS4");
  if (indirection_string == 0)
    indirection_string = xmalloc (indirection_stringsiz = 100);
  indirection_string[0] = '\0';

  if (ps4 == 0 || *ps4 == '\0')
    return (indirection_string);

  old = change_flag ('x', FLAG_OFF);
  ps4 = decode_prompt_string (ps4, 1);
  if (old)
    change_flag ('x', FLAG_ON);

  if (ps4 == 0 || *ps4 == '\0')
    {
      FREE (ps4);
      return (indirection_string);
    }

#if defined (HANDLE_MULTIBYTE)
  ps4_len = strnlen (ps4, MB_CUR_MAX);
  ps4_firstc_len = MBRLEN (ps4, ps4_len, &state);
  if (ps4_firstc_len == 1 || ps4_firstc_len == 0 || ps4_firstc_len < 0)
    {
      ps4_firstc[0] = ps4[0];
      ps4_firstc[ps4_firstc_len = 1] = '\0';
    }
  else
    memcpy (ps4_firstc, ps4, ps4_firstc_len);
#else
  ps4_firstc[0] = ps4[0];
  ps4_firstc[ps4_firstc_len = 1] = '\0';
#endif

  /* Dynamically resize indirection_string so we have room for everything
     and we don't have to truncate ps4 */
  ineed = (ps4_firstc_len * indirection_level) + strlen (ps4);
  if (ineed + 1 > indirection_stringsiz)
    {
      indirection_stringsiz = ineed + 1;
      indirection_string = xrealloc (indirection_string, indirection_stringsiz);
    }

  for (i = j = 0; ps4_firstc[0] && j < indirection_level && i < indirection_stringsiz - 1; i += ps4_firstc_len, j++)
    {
      if (ps4_firstc_len == 1)
	indirection_string[i] = ps4_firstc[0];
      else
	memcpy (indirection_string+i, ps4_firstc, ps4_firstc_len);
    }      

  for (j = ps4_firstc_len; *ps4 && ps4[j] && i < indirection_stringsiz - 1; i++, j++)
    indirection_string[i] = ps4[j];

  indirection_string[i] = '\0';
  free (ps4);
  return (indirection_string);
}

void
xtrace_print_assignment (char *name, char *value, int assign_list, int xflags)
{
  char *nval;

  CHECK_XTRACE_FP;

  if (xflags)
    fprintf (xtrace_fp, "%s", indirection_level_string ());

  /* VALUE should not be NULL when this is called. */
  if (*value == '\0' || assign_list)
    nval = value;
  else if (ansic_shouldquote (value))
    nval = ansic_quote (value, 0, (int *)0);
  else if (sh_contains_shell_metas (value))
    nval = sh_single_quote (value);
  else
    nval = value;

  if (assign_list)
    fprintf (xtrace_fp, "%s=(%s)\n", name, nval);
  else
    fprintf (xtrace_fp, "%s=%s\n", name, nval);

  if (nval != value)
    FREE (nval);

  fflush (xtrace_fp);
}

/* A function to print the words of a simple command when set -x is on.  Also used to
   print the word list in a for or select command header; in that case, we suppress
   quoting the words because they haven't been expanded yet.  XTFLAGS&1 means to
   print $PS4; XTFLAGS&2 means to suppress quoting the words in LIST. */
void
xtrace_print_word_list (WORD_LIST *list, int xtflags)
{
  WORD_LIST *w;
  char *t, *x;

  CHECK_XTRACE_FP;

  if (xtflags&1)
    fprintf (xtrace_fp, "%s", indirection_level_string ());

  for (w = list; w; w = w->next)
    {
      t = w->word->word;
      if (t == 0 || *t == '\0')
	fprintf (xtrace_fp, "''%s", w->next ? " " : "");
      else if (xtflags & 2)
	fprintf (xtrace_fp, "%s%s", t, w->next ? " " : "");
      else if (ansic_shouldquote (t))
	{
	  x = ansic_quote (t, 0, (int *)0);
	  fprintf (xtrace_fp, "%s%s", x, w->next ? " " : "");
	  free (x);
	}
      else if (sh_contains_shell_metas (t))
	{
	  x = sh_single_quote (t);
	  fprintf (xtrace_fp, "%s%s", x, w->next ? " " : "");
	  free (x);
	}
      else
	fprintf (xtrace_fp, "%s%s", t, w->next ? " " : "");
    }
  fprintf (xtrace_fp, "\n");
  fflush (xtrace_fp);
}

static void
command_print_word_list (WORD_LIST *list, char *separator)
{
  WORD_LIST *nlist;

  /* A start at removing CTLESC from commands that haven't been expanded. */
  if (pretty_print_mode)
    {
      nlist = list ? dequote_list (list) : list;
      _print_word_list (nlist, separator, cprintf);
      /* we're just pretty-printing, so this can be destructive */
    }
  else
    _print_word_list (list, separator, cprintf);
}

void
print_for_command_head (FOR_COM *for_command)
{
  cprintf ("for %s in ", for_command->name->word);
  command_print_word_list (for_command->map_list, " ");
}

void
xtrace_print_for_command_head (FOR_COM *for_command)
{
  CHECK_XTRACE_FP;
  fprintf (xtrace_fp, "%s", indirection_level_string ());
  fprintf (xtrace_fp, "for %s in ", for_command->name->word);
  xtrace_print_word_list (for_command->map_list, 2);
}

static void
print_for_command (FOR_COM *for_command)
{
  print_for_command_head (for_command);
  cprintf (";");
  newline ("do\n");

  indentation += indentation_amount;
  make_command_string_internal (for_command->action);
  PRINT_DEFERRED_HEREDOCS ("");
  semicolon ();
  indentation -= indentation_amount;

  newline ("done");
}

#if defined (ARITH_FOR_COMMAND)
static void
print_arith_for_command (ARITH_FOR_COM *arith_for_command)
{
  cprintf ("for ((");
  command_print_word_list (arith_for_command->init, " ");
  cprintf ("; ");
  command_print_word_list (arith_for_command->test, " ");
  cprintf ("; ");
  command_print_word_list (arith_for_command->step, " ");
  cprintf ("))");
  newline ("do\n");
  indentation += indentation_amount;
  make_command_string_internal (arith_for_command->action);
  PRINT_DEFERRED_HEREDOCS ("");
  semicolon ();
  indentation -= indentation_amount;
  newline ("done");
}
#endif /* ARITH_FOR_COMMAND */

#if defined (SELECT_COMMAND)
void
print_select_command_head (SELECT_COM *select_command)
{
  WORD_DESC *w;

  w = (pretty_print_mode) ? dequote_word (select_command->name) : select_command->name;
  /* we're just pretty-printing, so this can be destructive */

  cprintf ("select %s in ", w->word);
  command_print_word_list (select_command->map_list, " ");
}

void
xtrace_print_select_command_head (SELECT_COM *select_command)
{
  CHECK_XTRACE_FP;
  fprintf (xtrace_fp, "%s", indirection_level_string ());
  fprintf (xtrace_fp, "select %s in ", select_command->name->word);
  xtrace_print_word_list (select_command->map_list, 2);
}

static void
print_select_command (SELECT_COM *select_command)
{
  print_select_command_head (select_command);

  cprintf (";");
  newline ("do\n");
  indentation += indentation_amount;
  make_command_string_internal (select_command->action);
  PRINT_DEFERRED_HEREDOCS ("");
  semicolon ();
  indentation -= indentation_amount;
  newline ("done");
}
#endif /* SELECT_COMMAND */

static void
print_group_command (GROUP_COM *group_command)
{
  group_command_nesting++;
  cprintf ("{ ");

  if (inside_function_def == 0 /* && pretty_print_mode == 0 */)
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
  PRINT_DEFERRED_HEREDOCS ("");

  if (inside_function_def /* || pretty_print_mode */)
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
  was_heredoc = 0;	/* last wasn't heredoc/newline */

  group_command_nesting--;
}

void
print_case_command_head (CASE_COM *case_command)
{
  WORD_DESC *w;

  w = (pretty_print_mode) ? dequote_word (case_command->word) : case_command->word;
  /* we're just pretty-printing, so this can be destructive */

  cprintf ("case %s in ", w->word);
}

void
xtrace_print_case_command_head (CASE_COM *case_command)
{
  CHECK_XTRACE_FP;
  fprintf (xtrace_fp, "%s", indirection_level_string ());
  fprintf (xtrace_fp, "case %s in\n", case_command->word->word);
}

static void
print_case_command (CASE_COM *case_command)
{
  print_case_command_head (case_command);

  if (case_command->clauses)
    print_case_clauses (case_command->clauses);
  newline ("esac");
}

static void
print_case_clauses (PATTERN_LIST *clauses)
{
  int first = 1;

  indentation += indentation_amount;
  while (clauses)
    {
      /* If we're printing a comsub, the result will be reparsed later, so
	 we don't want to insert a newline after the `in': that could cause
	 the parser to parse a reserved word in error, since the newline
	 inserts a token after the `in'. */
      if (printing_comsub == 0 || first == 0)
	newline ("");
      first = 0;
      /* "The grammar shows that reserved words can be used as patterns,
	 even if one is the first word on a line. Obviously, the reserved
	 word esac cannot be used in this manner." */
      /* If the first word of the pattern list is literal "esac", the only
	 way it could have gotten through the parser is to have been
	 preceded by a left paren. */
      if (STREQ (clauses->patterns->word->word, "esac"))
	cprintf("(");
      command_print_word_list (clauses->patterns, " | ");
      cprintf (")\n");
      indentation += indentation_amount;
      make_command_string_internal (clauses->action);
      indentation -= indentation_amount;
      PRINT_DEFERRED_HEREDOCS ("");
      if (clauses->flags & CASEPAT_FALLTHROUGH)
	newline (";&");
      else if (clauses->flags & CASEPAT_TESTNEXT)
	newline (";;&");
      else
	newline (";;");
      clauses = clauses->next;
    }
  indentation -= indentation_amount;
}

static void
print_while_command (WHILE_COM *while_command)
{
  print_until_or_while (while_command, "while");
}

static void
print_until_command (WHILE_COM *while_command)
{
  print_until_or_while (while_command, "until");
}

static void
print_until_or_while (WHILE_COM *while_command, char *which)
{
  cprintf ("%s ", which);
  skip_this_indent++;
  make_command_string_internal (while_command->test);
  PRINT_DEFERRED_HEREDOCS ("");
  semicolon ();
  if (was_heredoc)
    {
      indent (indentation);
      cprintf ("do\n");
      was_heredoc = 0;
    }
  else
    cprintf (" do\n");	/* was newline ("do\n"); */
  indentation += indentation_amount;
  make_command_string_internal (while_command->action);
  PRINT_DEFERRED_HEREDOCS ("");
  indentation -= indentation_amount;
  semicolon ();
  newline ("done");
}

static void
print_if_command (IF_COM *if_command)
{
  cprintf ("if ");
  skip_this_indent++;
  make_command_string_internal (if_command->test);
  PRINT_DEFERRED_HEREDOCS ("");
  semicolon ();
  if (was_heredoc)
    {
      indent (indentation_amount);
      cprintf ("then\n");
      was_heredoc = 0;
    }
  else
    cprintf (" then\n");
  indentation += indentation_amount;
  make_command_string_internal (if_command->true_case);
  PRINT_DEFERRED_HEREDOCS ("");
  indentation -= indentation_amount;

  if (if_command->false_case)
    {
      semicolon ();
      newline ("else\n");
      indentation += indentation_amount;
      make_command_string_internal (if_command->false_case);
      PRINT_DEFERRED_HEREDOCS ("");
      indentation -= indentation_amount;
    }
  semicolon ();
  newline ("fi");
}

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
void
print_arith_command (WORD_LIST *arith_cmd_list)
{
  cprintf ("((");
  command_print_word_list (arith_cmd_list, " ");
  cprintf ("))");
}
#endif

#if defined (COND_COMMAND)
static void
print_cond_node (COND_COM *cond)
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
      cprintf ("%s", cond->op->word);
      cprintf (" ");
      print_cond_node (cond->left);
    }
  else if (cond->type == COND_BINARY)
    {
      print_cond_node (cond->left);
      cprintf (" ");
      cprintf ("%s", cond->op->word);
      cprintf (" ");
      print_cond_node (cond->right);
    }
  else if (cond->type == COND_TERM)
    {
      cprintf ("%s", cond->op->word);		/* need to add quoting here */
    }
}

void
print_cond_command (COND_COM *cond)
{
  cprintf ("[[ ");
  print_cond_node (cond);
  cprintf (" ]]");
}

#ifdef DEBUG
void
debug_print_word_list (char *s, WORD_LIST *list, char *sep)
{
  WORD_LIST *w;

  if (s)
    fprintf (stderr, "%s: ", s);
  for (w = list; w; w = w->next)
    fprintf (stderr, "%s%s", w->word->word, w->next ? sep : "");
  fprintf (stderr, "\n");
}

void
debug_print_cond_command (COND_COM *cond)
{
  fprintf (stderr, "DEBUG: ");
  command_string_index = 0;
  print_cond_command (cond);
  fprintf (stderr, "%s\n", the_printed_command);
}
#endif

void
xtrace_print_cond_term (int type, int invert, WORD_DESC *op, char *arg1, char *arg2)
{
  CHECK_XTRACE_FP;
  command_string_index = 0;
  fprintf (xtrace_fp, "%s", indirection_level_string ());
  fprintf (xtrace_fp, "[[ ");
  if (invert)
    fprintf (xtrace_fp, "! ");

  if (type == COND_UNARY)
    {
      fprintf (xtrace_fp, "%s ", op->word);
      fprintf (xtrace_fp, "%s", (arg1 && *arg1) ? arg1 : "''");
    }
  else if (type == COND_BINARY)
    {
      fprintf (xtrace_fp, "%s", (arg1 && *arg1) ? arg1 : "''");
      fprintf (xtrace_fp, " %s ", op->word);
      fprintf (xtrace_fp, "%s", (arg2 && *arg2) ? arg2 : "''");
    }

  fprintf (xtrace_fp, " ]]\n");

  fflush (xtrace_fp);
}	  
#endif /* COND_COMMAND */

#if defined (DPAREN_ARITHMETIC) || defined (ARITH_FOR_COMMAND)
/* A function to print the words of an arithmetic command when set -x is on. */
void
xtrace_print_arith_cmd (WORD_LIST *list)
{
  WORD_LIST *w;

  CHECK_XTRACE_FP;
  fprintf (xtrace_fp, "%s", indirection_level_string ());
  fprintf (xtrace_fp, "(( ");
  for (w = list; w; w = w->next)
    fprintf (xtrace_fp, "%s%s", w->word->word, w->next ? " " : "");
  fprintf (xtrace_fp, " ))\n");

  fflush (xtrace_fp);
}
#endif

void
print_simple_command (SIMPLE_COM *simple_command)
{
  if (simple_command->words)
    command_print_word_list (simple_command->words, " ");
  else
    cprintf ("");

  if (simple_command->redirects)
    {
      if (simple_command->words)
	cprintf (" ");
      print_redirection_list (simple_command->redirects);
    }
}

static void
print_heredocs (REDIRECT *heredocs)
{
  REDIRECT *hdtail;

  cprintf (" "); 
  for (hdtail = heredocs; hdtail; hdtail = hdtail->next)
    {
      print_redirection (hdtail);
      cprintf ("\n");
    }
  was_heredoc = 1;
}

static void
print_heredoc_bodies (REDIRECT *heredocs)
{
  REDIRECT *hdtail;

  cprintf ("\n"); 
  for (hdtail = heredocs; hdtail; hdtail = hdtail->next)
    {
      print_heredoc_body (hdtail);
      cprintf ("\n");
    }
  was_heredoc = 1;
}

/* Print heredocs that are attached to the command before the connector
   represented by CSTRING.  The parsing semantics require us to print the
   here-doc delimiters, then the connector (CSTRING), then the here-doc
   bodies.  We print the here-doc delimiters in print_redirection_list
   and print the connector and the bodies here. We don't print the connector
   if it's a `;', but we use it to note not to print an extra space after the
   last heredoc body and newline. */
static void
print_deferred_heredocs (const char *cstring)
{
  /* We now print the heredoc headers in print_redirection_list */
  if (cstring && cstring[0] && (cstring[0] != ';' || cstring[1]))
    cprintf ("%s", cstring); 
  if (deferred_heredocs)
    {
      print_heredoc_bodies (deferred_heredocs);
      if (cstring && cstring[0] && (cstring[0] != ';' || cstring[1]))
	cprintf (" ");	/* make sure there's at least one space */
      dispose_redirects (deferred_heredocs);
      was_heredoc = 1;
    }
  deferred_heredocs = (REDIRECT *)NULL;
}
      
static void
print_redirection_list (REDIRECT *redirects)
{
  REDIRECT *heredocs, *hdtail, *newredir;

  heredocs = (REDIRECT *)NULL;
  hdtail = heredocs;

  was_heredoc = 0;
  while (redirects)
    {
      /* Defer printing the here document bodies until we've printed the rest of the
         redirections, but print the headers in the order they're given.  */
      if (redirects->instruction == r_reading_until || redirects->instruction == r_deblank_reading_until)
	{
	  newredir = copy_redirect (redirects);
	  newredir->next = (REDIRECT *)NULL;

	  print_heredoc_header (newredir);

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
     print the here documents.  If we're printing a connection, we wait until
     we print the connector symbol, then we print the here document bodies */
  if (heredocs && printing_connection)
    deferred_heredocs = heredocs;
  else if (heredocs)
    {
      print_heredoc_bodies (heredocs);
      dispose_redirects (heredocs);
    }
}

static void
print_heredoc_header (REDIRECT *redirect)
{
  int kill_leading;
  char *x;

  kill_leading = redirect->instruction == r_deblank_reading_until;

  /* Here doc header */
  if (redirect->rflags & REDIR_VARASSIGN)
    cprintf ("{%s}", redirect->redirector.filename->word);
  else if (redirect->redirector.dest != 0)
    cprintf ("%d", redirect->redirector.dest);

  /* If the here document delimiter is quoted, single-quote it. */
  if (redirect->redirectee.filename->flags & W_QUOTED)
    {
      x = sh_single_quote (redirect->here_doc_eof);
      cprintf ("<<%s%s", kill_leading ? "-" : "", x);
      free (x);
    }
  else
    cprintf ("<<%s%s", kill_leading ? "-" : "", redirect->here_doc_eof);
}

static void
print_heredoc_body (REDIRECT *redirect)
{
  /* Here doc body */
  cprintf ("%s%s", redirect->redirectee.filename->word, redirect->here_doc_eof);
}

static void
print_redirection (REDIRECT *redirect)
{
  int redirector, redir_fd;
  WORD_DESC *redirectee, *redir_word;

  redirectee = redirect->redirectee.filename;
  redir_fd = redirect->redirectee.dest;

  redir_word = redirect->redirector.filename;
  redirector = redirect->redirector.dest;

  switch (redirect->instruction)
    {
    case r_input_direction:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}", redir_word->word);
      else if (redirector != 0)
	cprintf ("%d", redirector);
      cprintf ("< %s", redirectee->word);
      break;

    case r_output_direction:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}", redir_word->word);
      else if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf ("> %s", redirectee->word);
      break;

    case r_inputa_direction:	/* Redirection created by the shell. */
      cprintf ("&");
      break;

    case r_output_force:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}", redir_word->word);
      else if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf (">| %s", redirectee->word);
      break;

    case r_appending_to:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}", redir_word->word);
      else if (redirector != 1)
	cprintf ("%d", redirector);
      cprintf (">> %s", redirectee->word);
      break;

    case r_input_output:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}", redir_word->word);
      else if (redirector != 0)
	cprintf ("%d", redirector);
      cprintf ("<> %s", redirectee->word);
      break;

    case r_deblank_reading_until:
    case r_reading_until:
      print_heredoc_header (redirect);
      cprintf ("\n");
      print_heredoc_body (redirect);
      break;

    case r_reading_string:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}", redir_word->word);
      else if (redirector != 0)
	cprintf ("%d", redirector);
#if 0
      /* Don't need to check whether or not to requote, since original quotes
         are still intact.  The only thing that has happened is that $'...'
         has been replaced with 'expanded ...'. */
      if (ansic_shouldquote (redirect->redirectee.filename->word))
	{
	  char *x;
	  x = ansic_quote (redirect->redirectee.filename->word, 0, (int *)0);
	  cprintf ("<<< %s", x);
	  free (x);
	}
      else
#endif
	cprintf ("<<< %s", redirect->redirectee.filename->word);
      break;

    case r_duplicating_input:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}<&%d", redir_word->word, redir_fd);
      else
	cprintf ("%d<&%d", redirector, redir_fd);
      break;

    case r_duplicating_output:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}>&%d", redir_word->word, redir_fd);
      else
	cprintf ("%d>&%d", redirector, redir_fd);
      break;

    case r_duplicating_input_word:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}<&%s", redir_word->word, redirectee->word);
      else if (redirector == 0)
	cprintf ("<&%s", redirectee->word);
      else
	cprintf ("%d<&%s", redirector, redirectee->word);
      break;

    case r_duplicating_output_word:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}>&%s", redir_word->word, redirectee->word);
      else if (redirector == 1)
	cprintf (">&%s", redirectee->word);
      else
	cprintf ("%d>&%s", redirector, redirectee->word);
      break;

    case r_move_input:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}<&%d-", redir_word->word, redir_fd);
      else
	cprintf ("%d<&%d-", redirector, redir_fd);
      break;

    case r_move_output:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}>&%d-", redir_word->word, redir_fd);
      else
	cprintf ("%d>&%d-", redirector, redir_fd);
      break;

    case r_move_input_word:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}<&%s-", redir_word->word, redirectee->word);
      else
	cprintf ("%d<&%s-", redirector, redirectee->word);
      break;

    case r_move_output_word:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}>&%s-", redir_word->word, redirectee->word);
      else
	cprintf ("%d>&%s-", redirector, redirectee->word);
      break;

    case r_close_this:
      if (redirect->rflags & REDIR_VARASSIGN)
	cprintf ("{%s}>&-", redir_word->word);
      else
	cprintf ("%d>&-", redirector);
      break;

    case r_err_and_out:
      cprintf ("&> %s", redirectee->word);
      break;

    case r_append_err_and_out:
      cprintf ("&>> %s", redirectee->word);
      break;
    }
}

static void
uw_reset_locals (void *ignore)
{
  inside_function_def = 0;
  indentation = 0;
  printing_connection = 0;
  deferred_heredocs = 0;
  printing_comsub = 0;
}

static void
print_function_def (FUNCTION_DEF *func)
{
  COMMAND *cmdcopy;
  REDIRECT *func_redirects;
  WORD_DESC *w;
  int pflags;

  pflags = 0;
  if (posixly_correct)
    {
      pflags |= 4;	/* no reserved words */
#if POSIX_RESTRICT_FUNCNAME
      pflags |= 1;	/* function names must be valid identifiers */
#endif
    }

  w = pretty_print_mode ? dequote_word (func->name) : func->name;
  /* we're just pretty-printing, so this can be destructive */      

  func_redirects = NULL;
  /* When in posix mode, print functions as posix specifies them, but prefix
     `function' to names that are not valid posix function names, as
     determined by valid_function_name(). */
  if (posixly_correct == 0)
    cprintf ("function %s () \n", w->word);
  else if (valid_function_name (w->word, pflags) == 0)
    cprintf ("function %s () \n", w->word);
  else
    cprintf ("%s () \n", w->word);

  begin_unwind_frame ("function-def");
  add_unwind_protect (uw_reset_locals, 0);

  indent (indentation);
  cprintf ("{ \n");	/* } */

  inside_function_def++;
  indentation += indentation_amount;

  cmdcopy = func->command;
  unwind_protect_pointer (cmdcopy);
  if (cmdcopy->type == cm_group)
    {
      func_redirects = cmdcopy->redirects;
      cmdcopy->redirects = (REDIRECT *)NULL;
    }
  make_command_string_internal (cmdcopy->type == cm_group
					? cmdcopy->value.Group->command
					: cmdcopy);
  PRINT_DEFERRED_HEREDOCS ("");

  indentation -= indentation_amount;
  inside_function_def--;

  if (func_redirects)
    { /* { */
      newline ("} ");
      print_redirection_list (func_redirects);
      cmdcopy->redirects = func_redirects;
    }
  else
    {
      /* { */
      newline ("}");
      was_heredoc = 0;		/* not printing any here-documents now */
    }

  discard_unwind_frame ("function-def");
}

/* Return the string representation of the named function.
   NAME is the name of the function.
   COMMAND is the function body.  It should be a GROUP_COM.
   flags&FUNC_MULTILINE is non-zero to pretty-print, or zero for all on one line.
   flags&FUNC_EXTERNAL means convert from internal to external form
  */
char *
named_function_string (char *name, COMMAND *command, int flags)
{
  char *result;
  int old_indent, old_amount;
  COMMAND *cmdcopy;
  REDIRECT *func_redirects;
  int pflags;

  pflags = 0;
  if (posixly_correct)
    {
      pflags |= 4;	/* no reserved words */
#if POSIX_RESTRICT_FUNCNAME
      pflags |= 1;	/* function names must be valid identifiers */
#endif
    }

  old_indent = indentation;
  old_amount = indentation_amount;
  command_string_index = was_heredoc = 0;
  deferred_heredocs = 0;
  printing_comsub = 0;

  if (name && *name)
    {
      if (valid_function_name (name, pflags) == 0)
	cprintf ("function ");
      cprintf ("%s ", name);
    }

  cprintf ("() ");

  if ((flags & FUNC_MULTILINE) == 0)
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

  cprintf ((flags & FUNC_MULTILINE) ? "{ \n" : "{ ");	/* }} */

  cmdcopy = command;
  unwind_protect_pointer (cmdcopy);

  /* Take any redirections specified in the function definition (which should
     apply to the function as a whole) and save them for printing later. */
  func_redirects = (REDIRECT *)NULL;
  if (cmdcopy->type == cm_group)
    {
      func_redirects = cmdcopy->redirects;
      cmdcopy->redirects = (REDIRECT *)NULL;
    }
  make_command_string_internal (cmdcopy->type == cm_group
					? cmdcopy->value.Group->command
					: cmdcopy);
  PRINT_DEFERRED_HEREDOCS ("");

  indentation = old_indent;
  indentation_amount = old_amount;
  inside_function_def--;

  if (func_redirects)
    { /* { */
      newline ("} ");
      print_redirection_list (func_redirects);
      cmdcopy->redirects = func_redirects;
    }
  else
    {	/* { */
      newline ("}");
      was_heredoc = 0;
    }

  remove_unwind_protect ();	/* unwind_protect_pointer */
  result = the_printed_command;

  if ((flags & FUNC_MULTILINE) == 0)
    {
      if (result[2] == '\n')
	memmove (result + 2, result + 3, strlen (result) - 2);	
    }

  if (flags & FUNC_EXTERNAL)
    result = remove_quoted_escapes (result);

  return (result);
}

static void
newline (char *string)
{
  cprintf ("\n");
  indent (indentation);
  if (string && *string)
    cprintf ("%s", string);
}

static char *indentation_string;
static size_t indentation_size;

static void
indent (int amount)
{
  register int i;

  RESIZE_MALLOCED_BUFFER (indentation_string, 0, amount, indentation_size, 16);

  for (i = 0; amount > 0; amount--)
    indentation_string[i++] = ' ';
  indentation_string[i] = '\0';
  cprintf ("%s", indentation_string);
}

static void
semicolon (void)
{
  if ((command_string_index > 0 &&
	the_printed_command[command_string_index - 1] == '\n') ||
      (command_string_index > 1 && 
	the_printed_command[command_string_index - 1] == '&' &&
	the_printed_command[command_string_index - 2] == ' '))
    return;
  cprintf (";");
}

/* How to make the string. */
static void
cprintf (const char *control, ...)
{
  const char *s;
  char char_arg[2], *argp, intbuf[INT_STRLEN_BOUND (unsigned int) + 1];
  int digit_arg, c;
  size_t arg_len;
  va_list args;

  va_start (args, control);

  arg_len = strlen (control);
  the_printed_command_resize (arg_len + 1);

  char_arg[1] = '\0';
  s = control;
  while (s && *s)
    {
      c = *s++;
      argp = (char *)NULL;
      if (c != '%' || !*s)
	{
	  char_arg[0] = c;
	  argp = char_arg;
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
	      /* Represent an out-of-range file descriptor with an out-of-range
		 integer value.  We can do this because the only use of `%d' in
		 the calls to cprintf is to output a file descriptor number for
		 a redirection. */
	      digit_arg = va_arg (args, int);
	      if (digit_arg < 0)
		{
		  sprintf (intbuf, "%u", (unsigned int)-1);
		  argp = intbuf;
		}
	      else
		argp = inttostr (digit_arg, intbuf, sizeof (intbuf));
	      arg_len = strlen (argp);
	      break;

	    case 'c':
	      char_arg[0] = va_arg (args, int);
	      argp = char_arg;
	      arg_len = 1;
	      break;

	    default:
	      programming_error (_("cprintf: `%c': invalid format character"), c);
	      /*NOTREACHED*/
	    }
	}

      if (argp && arg_len)
	{
	  the_printed_command_resize (arg_len + 1);
	  FASTCOPY (argp, the_printed_command + command_string_index, arg_len);
	  command_string_index += arg_len;
	}
    }

  va_end (args);

  the_printed_command[command_string_index] = '\0';
}

/* Ensure that there is enough space to stuff LENGTH characters into
   THE_PRINTED_COMMAND. */
static void
the_printed_command_resize (size_t length)
{
  if (the_printed_command == 0)
    {
      the_printed_command_size = (length + PRINTED_COMMAND_INITIAL_SIZE - 1) & ~(PRINTED_COMMAND_INITIAL_SIZE - 1);
      the_printed_command = (char *)xmalloc (the_printed_command_size);
      command_string_index = 0;
    }
  else if ((command_string_index + length) >= the_printed_command_size)
    {
      size_t new;
      new = command_string_index + length + 1;

      /* Round up to the next multiple of PRINTED_COMMAND_GROW_SIZE. */
      new = (new + PRINTED_COMMAND_GROW_SIZE - 1) & ~(PRINTED_COMMAND_GROW_SIZE - 1);
      the_printed_command_size = new;

      the_printed_command = (char *)xrealloc (the_printed_command, the_printed_command_size);
    }
}

static void
xprintf (const char *format, ...)
{
  va_list args;

  va_start (args, format);

  vfprintf (stdout, format, args);
  va_end (args);
}
