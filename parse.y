/* Yacc grammar for bash. */

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
   with Bash; see the file LICENSE.  If not, write to the Free Software
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

%{
#include "config.h"

#include "bashtypes.h"
#include "bashansi.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#if defined (HAVE_LOCALE_H)
#  include <locale.h>
#endif

#include <stdio.h>
#include <signal.h>

#include "memalloc.h"

#include "shell.h"
#include "trap.h"
#include "flags.h"
#include "parser.h"
#include "mailcheck.h"
#include "builtins/common.h"
#include "builtins/builtext.h"

#if defined (READLINE)
#  include "bashline.h"
#  include <readline/readline.h>
#endif /* READLINE */

#if defined (HISTORY)
#  include "bashhist.h"
#  include <readline/history.h>
#endif /* HISTORY */

#if defined (JOB_CONTROL)
#  include "jobs.h"
#endif /* JOB_CONTROL */

#if defined (ALIAS)
#  include "alias.h"
#endif /* ALIAS */

#if defined (PROMPT_STRING_DECODE)
#  ifndef _MINIX
#    include <sys/param.h>
#  endif
#  include <time.h>
#  include "maxpath.h"
#endif /* PROMPT_STRING_DECODE */

#define RE_READ_TOKEN	-99
#define NO_EXPANSION	-100

#define YYDEBUG 0

#if defined (EXTENDED_GLOB)
#define PATTERN_CHAR(c) \
	((c) == '@' || (c) == '*' || (c) == '+' || (c) == '?' || (c) == '!')

extern int extended_glob;
#endif

extern int eof_encountered;
extern int no_line_editing, running_under_emacs;
extern int current_command_number;
extern int interactive, interactive_shell, login_shell;
extern int sourcelevel;
extern int posixly_correct;
extern int last_command_exit_value;
extern int interrupt_immediately;
extern char *shell_name, *current_host_name;
extern char *dist_version;
extern int patch_level;
extern int dump_translatable_strings, dump_po_strings;
extern Function *last_shell_builtin, *this_shell_builtin;
#if defined (BUFFERED_INPUT)
extern int bash_input_fd_changed;
#endif

/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

static char *ansiexpand ();
static char *localeexpand ();
static int reserved_word_acceptable ();
static int read_token ();
static int yylex ();
static int parse_arith_cmd ();
#if defined (COND_COMMAND)
static COMMAND *parse_cond_command ();
#endif
static int read_token_word ();
static void discard_parser_constructs ();

static void report_syntax_error ();
static void handle_eof_input_unit ();
static void prompt_again ();
#if 0
static void reset_readline_prompt ();
#endif
static void print_prompt ();

extern int yyerror ();

/* Default prompt strings */
char *primary_prompt = PPROMPT;
char *secondary_prompt = SPROMPT;

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* Non-zero means we expand aliases in commands. */
int expand_aliases = 0;

/* If non-zero, the decoded prompt string undergoes parameter and
   variable substitution, command substitution, arithmetic substitution,
   string expansion, process substitution, and quote removal in
   decode_prompt_string. */
int promptvars = 1;

/* The decoded prompt string.  Used if READLINE is not defined or if
   editing is turned off.  Analogous to current_readline_prompt. */
static char *current_decoded_prompt;

/* The number of lines read from input while creating the current command. */
int current_command_line_count;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
static REDIRECT *redir_stack[10];
int need_here_doc;

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
static char *shell_input_line = (char *)NULL;
static int shell_input_line_index;
static int shell_input_line_size;	/* Amount allocated for shell_input_line. */
static int shell_input_line_len;	/* strlen (shell_input_line) */

/* Either zero or EOF. */
static int shell_input_line_terminator;

/* The line number in a script on which a function definition starts. */
static int function_dstart;

/* The line number in a script on which a function body starts. */
static int function_bstart;

static REDIRECTEE redir;
%}

%union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT element;
  PATTERN_LIST *pattern;
}

/* Reserved words.  Members of the first group are only recognized
   in the case that they are preceded by a list_terminator.  Members
   of the second group are for [[...]] commands.  Members of the
   third group are recognized only under special circumstances. */
%token IF THEN ELSE ELIF FI CASE ESAC FOR SELECT WHILE UNTIL DO DONE FUNCTION
%token COND_START COND_END COND_ERROR
%token IN BANG TIME TIMEOPT 

/* More general tokens. yylex () knows how to make these. */
%token <word> WORD ASSIGNMENT_WORD
%token <number> NUMBER
%token <word_list> ARITH_CMD
%token <command> COND_CMD
%token AND_AND OR_OR GREATER_GREATER LESS_LESS LESS_AND
%token GREATER_AND SEMI_SEMI LESS_LESS_MINUS AND_GREATER LESS_GREATER
%token GREATER_BAR

/* The types that the various syntactical units return. */

%type <command> inputunit command pipeline pipeline_command
%type <command> list list0 list1 compound_list simple_list simple_list1
%type <command> simple_command shell_command
%type <command> for_command select_command case_command group_command
%type <command> arith_command
%type <command> cond_command
%type <command> function_def if_command elif_clause subshell
%type <redirect> redirection redirection_list
%type <element> simple_command_element
%type <word_list> word_list pattern
%type <pattern> pattern_list case_clause_sequence case_clause
%type <number> timespec

%start inputunit

%left '&' ';' '\n' yacc_EOF
%left AND_AND OR_OR
%right '|'
%%

inputunit:	simple_list '\n'
			{
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  global_command = $1;
			  eof_encountered = 0;
			  discard_parser_constructs (0);
			  YYACCEPT;
			}
	|	'\n'
			{
			  /* Case of regular command, but not a very
			     interesting one.  Return a NULL command. */
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			}
	|	error '\n'
			{
			  /* Error during parsing.  Return NULL command. */
			  global_command = (COMMAND *)NULL;
			  eof_encountered = 0;
			  discard_parser_constructs (1);
			  if (interactive)
			    {
			      YYACCEPT;
			    }
			  else
			    {
			      YYABORT;
			    }
			}
	|	yacc_EOF
			{
			  /* Case of EOF seen by itself.  Do ignoreeof or
			     not. */
			  global_command = (COMMAND *)NULL;
			  handle_eof_input_unit ();
			  YYACCEPT;
			}
	;

word_list:	WORD
			{ $$ = make_word_list ($1, (WORD_LIST *)NULL); }
	|	word_list WORD
			{ $$ = make_word_list ($2, $1); }
	;

redirection:	'>' WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (1, r_output_direction, redir);
			}
	|	'<' WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (0, r_input_direction, redir);
			}
	|	NUMBER '>' WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_output_direction, redir);
			}
	|	NUMBER '<' WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_input_direction, redir);
			}
	|	GREATER_GREATER WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (1, r_appending_to, redir);
			}
	|	NUMBER GREATER_GREATER WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_appending_to, redir);
			}
	|	LESS_LESS WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (0, r_reading_until, redir);
			  redir_stack[need_here_doc++] = $$;
			}
	|	NUMBER LESS_LESS WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_reading_until, redir);
			  redir_stack[need_here_doc++] = $$;
			}
	|	LESS_AND NUMBER
			{
			  redir.dest = $2;
			  $$ = make_redirection (0, r_duplicating_input, redir);
			}
	|	NUMBER LESS_AND NUMBER
			{
			  redir.dest = $3;
			  $$ = make_redirection ($1, r_duplicating_input, redir);
			}
	|	GREATER_AND NUMBER
			{
			  redir.dest = $2;
			  $$ = make_redirection (1, r_duplicating_output, redir);
			}
	|	NUMBER GREATER_AND NUMBER
			{
			  redir.dest = $3;
			  $$ = make_redirection ($1, r_duplicating_output, redir);
			}
	|	LESS_AND WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (0, r_duplicating_input_word, redir);
			}
	|	NUMBER LESS_AND WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_duplicating_input_word, redir);
			}
	|	GREATER_AND WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (1, r_duplicating_output_word, redir);
			}
	|	NUMBER GREATER_AND WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_duplicating_output_word, redir);
			}
	|	LESS_LESS_MINUS WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection
			    (0, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = $$;
			}
	|	NUMBER LESS_LESS_MINUS WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection
			    ($1, r_deblank_reading_until, redir);
			  redir_stack[need_here_doc++] = $$;
			}
	|	GREATER_AND '-'
			{
			  redir.dest = 0L;
			  $$ = make_redirection (1, r_close_this, redir);
			}
	|	NUMBER GREATER_AND '-'
			{
			  redir.dest = 0L;
			  $$ = make_redirection ($1, r_close_this, redir);
			}
	|	LESS_AND '-'
			{
			  redir.dest = 0L;
			  $$ = make_redirection (0, r_close_this, redir);
			}
	|	NUMBER LESS_AND '-'
			{
			  redir.dest = 0L;
			  $$ = make_redirection ($1, r_close_this, redir);
			}
	|	AND_GREATER WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (1, r_err_and_out, redir);
			}
	|	NUMBER LESS_GREATER WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_input_output, redir);
			}
	|	LESS_GREATER WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (0, r_input_output, redir);
			}
	|	GREATER_BAR WORD
			{
			  redir.filename = $2;
			  $$ = make_redirection (1, r_output_force, redir);
			}
	|	NUMBER GREATER_BAR WORD
			{
			  redir.filename = $3;
			  $$ = make_redirection ($1, r_output_force, redir);
			}
	;

simple_command_element: WORD
			{ $$.word = $1; $$.redirect = 0; }
	|	ASSIGNMENT_WORD
			{ $$.word = $1; $$.redirect = 0; }
	|	redirection
			{ $$.redirect = $1; $$.word = 0; }
	;

redirection_list: redirection
			{
			  $$ = $1;
			}
	|	redirection_list redirection
			{
			  register REDIRECT *t;

			  for (t = $1; t->next; t = t->next)
			    ;
			  t->next = $2;
			  $$ = $1;
			}
	;

simple_command:	simple_command_element
			{ $$ = make_simple_command ($1, (COMMAND *)NULL); }
	|	simple_command simple_command_element
			{ $$ = make_simple_command ($2, $1); }
	;

command:	simple_command
			{ $$ = clean_simple_command ($1); }
	|	shell_command
			{ $$ = $1; }
	|	shell_command redirection_list
			{
			  COMMAND *tc;

			  tc = $1;
			  /* According to Posix.2 3.9.5, redirections
			     specified after the body of a function should
			     be attached to the function and performed when
			     the function is executed, not as part of the
			     function definition command. */
			  if (tc->type == cm_function_def)
			    {
			      tc = tc->value.Function_def->command;
			      if (tc->type == cm_group)
			        tc = tc->value.Group->command;
			    }
			  if (tc->redirects)
			    {
			      register REDIRECT *t;
			      for (t = tc->redirects; t->next; t = t->next)
				;
			      t->next = $2;
			    }
			  else
			    tc->redirects = $2;
			  $$ = $1;
			}
	;

shell_command:	for_command
			{ $$ = $1; }
	|	case_command
			{ $$ = $1; }
 	|	WHILE compound_list DO compound_list DONE
			{ $$ = make_while_command ($2, $4); }
	|	UNTIL compound_list DO compound_list DONE
			{ $$ = make_until_command ($2, $4); }
	|	select_command
			{ $$ = $1; }
	|	if_command
			{ $$ = $1; }
	|	subshell
			{ $$ = $1; }
	|	group_command
			{ $$ = $1; }
	|	arith_command
			{ $$ = $1; }
	|	cond_command
			{ $$ = $1; }
	|	function_def
			{ $$ = $1; }
	;

for_command:	FOR WORD newline_list DO compound_list DONE
			{ $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $5); }
	|	FOR WORD newline_list '{' compound_list '}'
			{ $$ = make_for_command ($2, add_string_to_list ("$@", (WORD_LIST *)NULL), $5); }
	|	FOR WORD ';' newline_list DO compound_list DONE
			{ $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6); }
	|	FOR WORD ';' newline_list '{' compound_list '}'
			{ $$ = make_for_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6); }
	|	FOR WORD newline_list IN word_list list_terminator newline_list DO compound_list DONE
			{ $$ = make_for_command ($2, REVERSE_LIST ($5, WORD_LIST *), $9); }
	|	FOR WORD newline_list IN word_list list_terminator newline_list '{' compound_list '}'
			{ $$ = make_for_command ($2, REVERSE_LIST ($5, WORD_LIST *), $9); }
	;

select_command:	SELECT WORD newline_list DO list DONE
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $5);
			}
	|	SELECT WORD newline_list '{' list '}'
			{
			  $$ = make_select_command ($2, add_string_to_list ("$@", (WORD_LIST *)NULL), $5);
			}
	|	SELECT WORD ';' newline_list DO list DONE
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6);
			}
	|	SELECT WORD ';' newline_list '{' list '}'
			{
			  $$ = make_select_command ($2, add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $6);
			}
	|	SELECT WORD newline_list IN word_list list_terminator newline_list DO list DONE
			{
			  $$ = make_select_command ($2, (WORD_LIST *)reverse_list ($5), $9);
			}
	|	SELECT WORD newline_list IN word_list list_terminator newline_list '{' list '}'
			{
			  $$ = make_select_command ($2, (WORD_LIST *)reverse_list ($5), $9);
			}
	;

case_command:	CASE WORD newline_list IN newline_list ESAC
			{ $$ = make_case_command ($2, (PATTERN_LIST *)NULL); }
	|	CASE WORD newline_list IN case_clause_sequence newline_list ESAC
			{ $$ = make_case_command ($2, $5); }
	|	CASE WORD newline_list IN case_clause ESAC
			{ $$ = make_case_command ($2, $5); }
	;

function_def:	WORD '(' ')' newline_list group_command
			{ $$ = make_function_def ($1, $5, function_dstart, function_bstart); }


	|	FUNCTION WORD '(' ')' newline_list group_command
			{ $$ = make_function_def ($2, $6, function_dstart, function_bstart); }

	|	FUNCTION WORD newline_list group_command
			{ $$ = make_function_def ($2, $4, function_dstart, function_bstart); }
	;

subshell:	'(' compound_list ')'
			{ $2->flags |= CMD_WANT_SUBSHELL; $$ = $2; }
	;

if_command:	IF compound_list THEN compound_list FI
			{ $$ = make_if_command ($2, $4, (COMMAND *)NULL); }
	|	IF compound_list THEN compound_list ELSE compound_list FI
			{ $$ = make_if_command ($2, $4, $6); }
	|	IF compound_list THEN compound_list elif_clause FI
			{ $$ = make_if_command ($2, $4, $5); }
	;


group_command:	'{' list '}'
			{ $$ = make_group_command ($2); }
	;

arith_command:	ARITH_CMD
			{ $$ = make_arith_command ($1); }
	;

cond_command:	COND_START COND_CMD COND_END
			{ $$ = $2; }
	; 

elif_clause:	ELIF compound_list THEN compound_list
			{ $$ = make_if_command ($2, $4, (COMMAND *)NULL); }
	|	ELIF compound_list THEN compound_list ELSE compound_list
			{ $$ = make_if_command ($2, $4, $6); }
	|	ELIF compound_list THEN compound_list elif_clause
			{ $$ = make_if_command ($2, $4, $5); }
	;

case_clause:	pattern_list
	|	case_clause_sequence pattern_list
			{ $2->next = $1; $$ = $2; }
	;

pattern_list:	newline_list pattern ')' compound_list
			{ $$ = make_pattern_list ($2, $4); }
	|	newline_list pattern ')' newline_list
			{ $$ = make_pattern_list ($2, (COMMAND *)NULL); }
	|	newline_list '(' pattern ')' compound_list
			{ $$ = make_pattern_list ($3, $5); }
	|	newline_list '(' pattern ')' newline_list
			{ $$ = make_pattern_list ($3, (COMMAND *)NULL); }
	;

case_clause_sequence:  pattern_list SEMI_SEMI
	|	case_clause_sequence pattern_list SEMI_SEMI
			{ $2->next = $1; $$ = $2; }
	;

pattern:	WORD
			{ $$ = make_word_list ($1, (WORD_LIST *)NULL); }
	|	pattern '|' WORD
			{ $$ = make_word_list ($3, $1); }
	;

/* A list allows leading or trailing newlines and
   newlines as operators (equivalent to semicolons).
   It must end with a newline or semicolon.
   Lists are used within commands such as if, for, while.  */

list:		newline_list list0
			{
			  $$ = $2;
			  if (need_here_doc)
			    gather_here_documents ();
			 }
	;

compound_list:	list
	|	newline_list list1
			{
			  $$ = $2;
			}
	;

list0:  	list1 '\n' newline_list
	|	list1 '&' newline_list
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, (COMMAND *)NULL, '&');
			  else
			    $$ = command_connect ($1, (COMMAND *)NULL, '&');
			}
	|	list1 ';' newline_list

	;

list1:		list1 AND_AND newline_list list1
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	list1 OR_OR newline_list list1
			{ $$ = command_connect ($1, $4, OR_OR); }
	|	list1 '&' newline_list list1
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, $4, '&');
			  else
			    $$ = command_connect ($1, $4, '&');
			}
	|	list1 ';' newline_list list1
			{ $$ = command_connect ($1, $4, ';'); }
	|	list1 '\n' newline_list list1
			{ $$ = command_connect ($1, $4, ';'); }
	|	pipeline_command
			{ $$ = $1; }
	;

list_terminator:'\n'
	|	';'
	|	yacc_EOF
	;

newline_list:
	|	newline_list '\n'
	;

/* A simple_list is a list that contains no significant newlines
   and no leading or trailing newlines.  Newlines are allowed
   only following operators, where they are not significant.

   This is what an inputunit consists of.  */

simple_list:	simple_list1
			{
			  $$ = $1;
			  if (need_here_doc)
			    gather_here_documents ();
			}
	|	simple_list1 '&'
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, (COMMAND *)NULL, '&');
			  else
			    $$ = command_connect ($1, (COMMAND *)NULL, '&');
			  if (need_here_doc)
			    gather_here_documents ();
			}
	|	simple_list1 ';'
			{
			  $$ = $1;
			  if (need_here_doc)
			    gather_here_documents ();
			}
	;

simple_list1:	simple_list1 AND_AND newline_list simple_list1
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	simple_list1 OR_OR newline_list simple_list1
			{ $$ = command_connect ($1, $4, OR_OR); }
	|	simple_list1 '&' simple_list1
			{
			  if ($1->type == cm_connection)
			    $$ = connect_async_list ($1, $3, '&');
			  else
			    $$ = command_connect ($1, $3, '&');
			}
	|	simple_list1 ';' simple_list1
			{ $$ = command_connect ($1, $3, ';'); }

	|	pipeline_command
			{ $$ = $1; }
	;

pipeline_command: pipeline
			{ $$ = $1; }
	|	BANG pipeline
			{
			  $2->flags |= CMD_INVERT_RETURN;
			  $$ = $2;
			}
	|	timespec pipeline
			{
			  $2->flags |= $1;
			  $$ = $2;
			}
	|	timespec BANG pipeline
			{
			  $3->flags |= $1;
			  $$ = $3;
			}
	|	BANG timespec pipeline
			{
			  $3->flags |= $2|CMD_INVERT_RETURN;
			  $$ = $3;
			}
	;

pipeline:
		pipeline '|' newline_list pipeline
			{ $$ = command_connect ($1, $4, '|'); }
	|	command
			{ $$ = $1; }
	;

timespec:	TIME
			{ $$ = CMD_TIME_PIPELINE; }
	|	TIME TIMEOPT
			{ $$ = CMD_TIME_PIPELINE|CMD_TIME_POSIX; }
	;
%%

/* Possible states for the parser that require it to do special things. */
#define PST_CASEPAT	0x001		/* in a case pattern list */
#define PST_ALEXPNEXT	0x002		/* expand next word for aliases */
#define PST_ALLOWOPNBRC	0x004		/* allow open brace for function def */
#define PST_NEEDCLOSBRC	0x008		/* need close brace */
#define PST_DBLPAREN	0x010		/* double-paren parsing */
#define PST_SUBSHELL	0x020		/* ( ... ) subshell */
#define PST_CMDSUBST	0x040		/* $( ... ) command substitution */
#define PST_CASESTMT	0x080		/* parsing a case statement */
#define PST_CONDCMD	0x100		/* parsing a [[...]] command */
#define PST_CONDEXPR	0x200		/* parsing the guts of [[...]] */

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_INITIAL_SIZE 496
#define TOKEN_DEFAULT_GROW_SIZE 512

/* Shell meta-characters that, when unquoted, separate words. */
#define shellmeta(c)	(strchr (shell_meta_chars, (c)) != 0)
#define shellbreak(c)	(strchr (shell_break_chars, (c)) != 0)
#define shellquote(c)	((c) == '"' || (c) == '`' || (c) == '\'')
#define shellexp(c)	((c) == '$' || (c) == '<' || (c) == '>')

char *shell_meta_chars = "()<>;&|";
char *shell_break_chars = "()<>;&| \t\n";

/* The token currently being read. */
static int current_token;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
static int last_read_token;

/* The token read prior to last_read_token. */
static int token_before_that;

/* The token read prior to token_before_that. */
static int two_tokens_ago;

/* If non-zero, it is the token that we want read_token to return
   regardless of what text is (or isn't) present to be read.  This
   is reset by read_token.  If token_to_read == WORD or
   ASSIGNMENT_WORD, yylval.word should be set to word_desc_to_read. */
static int token_to_read;
static WORD_DESC *word_desc_to_read;

/* The current parser state. */
static int parser_state;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

void
debug_parser (i)
     int i;
{
#if YYDEBUG != 0
  yydebug = i;
#endif
}

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget, type, location) makes the function GET the
   installed function for getting the next character, makes UNGET the
   installed function for un-getting a character, sets the type of stream
   (either string or file) from TYPE, and makes LOCATION point to where
   the input is coming from. */

/* Unconditionally returns end-of-file. */
int
return_EOF ()
{
  return (EOF);
}

/* Variable containing the current get and unget functions.
   See ./input.h for a clearer description. */
BASH_INPUT bash_input;

/* Set all of the fields in BASH_INPUT to NULL.  Free bash_input.name if it
   is non-null, avoiding a memory leak. */
void
initialize_bash_input ()
{
  bash_input.type = st_none;
  FREE (bash_input.name);
  bash_input.name = (char *)NULL;
  bash_input.location.file = (FILE *)NULL;
  bash_input.location.string = (char *)NULL;
  bash_input.getter = (Function *)NULL;
  bash_input.ungetter = (Function *)NULL;
}

/* Set the contents of the current bash input stream from
   GET, UNGET, TYPE, NAME, and LOCATION. */
void
init_yy_io (get, unget, type, name, location)
     Function *get, *unget;
     enum stream_type type;
     char *name;
     INPUT_STREAM location;
{
  bash_input.type = type;
  FREE (bash_input.name);
  bash_input.name = name ? savestring (name) : (char *)NULL;

  /* XXX */
#if defined (CRAY)
  memcpy((char *)&bash_input.location.string, (char *)&location.string, sizeof(location));
#else
  bash_input.location = location;
#endif
  bash_input.getter = get;
  bash_input.ungetter = unget;
}

/* Call this to get the next character of input. */
int
yy_getc ()
{
  return (*(bash_input.getter)) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
int
yy_ungetc (c)
     int c;
{
  return (*(bash_input.ungetter)) (c);
}

#if defined (BUFFERED_INPUT)
int
input_file_descriptor ()
{
  switch (bash_input.type)
    {
    case st_stream:
      return (fileno (bash_input.location.file));
    case st_bstream:
      return (bash_input.location.buffered_fd);
    case st_stdin:
    default:
      return (fileno (stdin));
    }
}
#endif /* BUFFERED_INPUT */

/* **************************************************************** */
/*								    */
/*		  Let input be read from readline ().		    */
/*								    */
/* **************************************************************** */

#if defined (READLINE)
char *current_readline_prompt = (char *)NULL;
char *current_readline_line = (char *)NULL;
int current_readline_line_index = 0;

static int
yy_readline_get ()
{
  SigHandler *old_sigint;
  int line_len, c;

  if (!current_readline_line)
    {
      if (!bash_readline_initialized)
	initialize_readline ();

#if defined (JOB_CONTROL)
      if (job_control)
	give_terminal_to (shell_pgrp);
#endif /* JOB_CONTROL */

      if (signal_is_ignored (SIGINT) == 0)
	{
	  old_sigint = (SigHandler *)set_signal_handler (SIGINT, sigint_sighandler);
	  interrupt_immediately++;
	}

      current_readline_line = readline (current_readline_prompt ?
      					  current_readline_prompt : "");

      if (signal_is_ignored (SIGINT) == 0)
	{
	  interrupt_immediately--;
	  set_signal_handler (SIGINT, old_sigint);
	}

#if 0
      /* Reset the prompt to the decoded value of prompt_string_pointer. */
      reset_readline_prompt ();
#endif

      if (current_readline_line == 0)
	return (EOF);

      current_readline_line_index = 0;
      line_len = strlen (current_readline_line);

      current_readline_line = xrealloc (current_readline_line, 2 + line_len);
      current_readline_line[line_len++] = '\n';
      current_readline_line[line_len] = '\0';
    }

  if (current_readline_line[current_readline_line_index] == 0)
    {
      free (current_readline_line);
      current_readline_line = (char *)NULL;
      return (yy_readline_get ());
    }
  else
    {
      c = (unsigned char)current_readline_line[current_readline_line_index++];
      return (c);
    }
}

static int
yy_readline_unget (c)
     int c;
{
  if (current_readline_line_index && current_readline_line)
    current_readline_line[--current_readline_line_index] = c;
  return (c);
}

void
with_input_from_stdin ()
{
  INPUT_STREAM location;

  if (bash_input.type != st_stdin && stream_on_stack (st_stdin) == 0)
    {
      location.string = current_readline_line;
      init_yy_io (yy_readline_get, yy_readline_unget,
		  st_stdin, "readline stdin", location);
    }
}

#else  /* !READLINE */

void
with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}
#endif	/* !READLINE */

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

static int
yy_string_get ()
{
  register char *string;
  register int c;

  string = bash_input.location.string;
  c = EOF;

  /* If the string doesn't exist, or is empty, EOF found. */
  if (string && *string)
    {
      c = *(unsigned char *)string++;
      bash_input.location.string = string;
    }
  return (c);
}

static int
yy_string_unget (c)
     int c;
{
  *(--bash_input.location.string) = c;
  return (c);
}

void
with_input_from_string (string, name)
     char *string, *name;
{
  INPUT_STREAM location;

  location.string = string;
  init_yy_io (yy_string_get, yy_string_unget, st_string, name, location);
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

static int
yy_stream_get ()
{
  int result = EOF;

  if (bash_input.location.file)
    {
#if !defined (HAVE_RESTARTABLE_SYSCALLS)
      result = getc_with_restart (bash_input.location.file);
#else /* HAVE_RESTARTABLE_SYSCALLS */
      result = getc (bash_input.location.file);
      result = (feof (bash_input.location.file)) ? EOF : (unsigned char)result;
#endif /* HAVE_RESTARTABLE_SYSCALLS */
    }
  return (result);
}

static int
yy_stream_unget (c)
     int c;
{
#if !defined (HAVE_RESTARTABLE_SYSCALLS)
  return (ungetc_with_restart (c, bash_input.location.file));
#else /* HAVE_RESTARTABLE_SYSCALLS */
  return (ungetc (c, bash_input.location.file));
#endif /* HAVE_RESTARTABLE_SYSCALLS */
}

void
with_input_from_stream (stream, name)
     FILE *stream;
     char *name;
{
  INPUT_STREAM location;

  location.file = stream;
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, name, location);
}

typedef struct stream_saver {
  struct stream_saver *next;
  BASH_INPUT bash_input;
  int line;
#if defined (BUFFERED_INPUT)
  BUFFERED_STREAM *bstream;
#endif /* BUFFERED_INPUT */
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

#if defined (COND_COMMAND)
static int cond_lineno;
static int cond_token;
#endif

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

void
push_stream (reset_lineno)
     int reset_lineno;
{
  STREAM_SAVER *saver = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));

  xbcopy ((char *)&bash_input, (char *)&(saver->bash_input), sizeof (BASH_INPUT));

#if defined (BUFFERED_INPUT)
  saver->bstream = (BUFFERED_STREAM *)NULL;
  /* If we have a buffered stream, clear out buffers[fd]. */
  if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
    saver->bstream = set_buffered_stream (bash_input.location.buffered_fd,
    					  (BUFFERED_STREAM *)NULL);
#endif /* BUFFERED_INPUT */

  saver->line = line_number;
  bash_input.name = (char *)NULL;
  saver->next = stream_list;
  stream_list = saver;
  EOF_Reached = 0;
  if (reset_lineno)
    line_number = 0;
}

void
pop_stream ()
{
  if (!stream_list)
    EOF_Reached = 1;
  else
    {
      STREAM_SAVER *saver = stream_list;

      EOF_Reached = 0;
      stream_list = stream_list->next;

      init_yy_io (saver->bash_input.getter,
		  saver->bash_input.ungetter,
		  saver->bash_input.type,
		  saver->bash_input.name,
		  saver->bash_input.location);

#if defined (BUFFERED_INPUT)
      /* If we have a buffered stream, restore buffers[fd]. */
      /* If the input file descriptor was changed while this was on the
	 save stack, update the buffered fd to the new file descriptor and
	 re-establish the buffer <-> bash_input fd correspondence. */
      if (bash_input.type == st_bstream && bash_input.location.buffered_fd >= 0)
        {
          if (bash_input_fd_changed)
	    {
	      bash_input_fd_changed = 0;
	      if (default_buffered_input >= 0)
		{
		  bash_input.location.buffered_fd = default_buffered_input;
		  saver->bstream->b_fd = default_buffered_input;
		}
	    }
	  set_buffered_stream (bash_input.location.buffered_fd, saver->bstream);
        }
#endif /* BUFFERED_INPUT */

      line_number = saver->line;

      FREE (saver->bash_input.name);
      free (saver);
    }
}

/* Return 1 if a stream of type TYPE is saved on the stack. */
int
stream_on_stack (type)
     enum stream_type type;
{
  register STREAM_SAVER *s;

  for (s = stream_list; s; s = s->next)
    if (s->bash_input.type == type)
      return 1;
  return 0;
}

/*
 * This is used to inhibit alias expansion and reserved word recognition
 * inside case statement pattern lists.  A `case statement pattern list' is:
 *
 *	everything between the `in' in a `case word in' and the next ')'
 *	or `esac'
 *	everything between a `;;' and the next `)' or `esac'
 */

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)

#if !defined (ALIAS)
typedef void *alias_t;
#endif

#define END_OF_ALIAS 0

/*
 * Pseudo-global variables used in implementing token-wise alias expansion.
 */

/*
 * Pushing and popping strings.  This works together with shell_getc to
 * implement alias expansion on a per-token basis.
 */

typedef struct string_saver {
  struct string_saver *next;
  int expand_alias;  /* Value to set expand_alias to when string is popped. */
  char *saved_line;
#if defined (ALIAS)
  alias_t *expander;   /* alias that caused this line to be pushed. */
#endif
  int saved_line_size, saved_line_index, saved_line_terminator;
} STRING_SAVER;

STRING_SAVER *pushed_string_list = (STRING_SAVER *)NULL;

/*
 * Push the current shell_input_line onto a stack of such lines and make S
 * the current input.  Used when expanding aliases.  EXPAND is used to set
 * the value of expand_next_token when the string is popped, so that the
 * word after the alias in the original line is handled correctly when the
 * alias expands to multiple words.  TOKEN is the token that was expanded
 * into S; it is saved and used to prevent infinite recursive expansion.
 */
static void
push_string (s, expand, ap)
     char *s;
     int expand;
     alias_t *ap;
{
  STRING_SAVER *temp = (STRING_SAVER *) xmalloc (sizeof (STRING_SAVER));

  temp->expand_alias = expand;
  temp->saved_line = shell_input_line;
  temp->saved_line_size = shell_input_line_size;
  temp->saved_line_index = shell_input_line_index;
  temp->saved_line_terminator = shell_input_line_terminator;
#if defined (ALIAS)
  temp->expander = ap;
#endif
  temp->next = pushed_string_list;
  pushed_string_list = temp;

#if defined (ALIAS)
  if (ap)
    ap->flags |= AL_BEINGEXPANDED;
#endif

  shell_input_line = s;
  shell_input_line_size = strlen (s);
  shell_input_line_index = 0;
  shell_input_line_terminator = '\0';
  parser_state &= ~PST_ALEXPNEXT;
}

/*
 * Make the top of the pushed_string stack be the current shell input.
 * Only called when there is something on the stack.  Called from shell_getc
 * when it thinks it has consumed the string generated by an alias expansion
 * and needs to return to the original input line.
 */
static void
pop_string ()
{
  STRING_SAVER *t;

  FREE (shell_input_line);
  shell_input_line = pushed_string_list->saved_line;
  shell_input_line_index = pushed_string_list->saved_line_index;
  shell_input_line_size = pushed_string_list->saved_line_size;
  shell_input_line_terminator = pushed_string_list->saved_line_terminator;

  if (pushed_string_list->expand_alias)
    parser_state |= PST_ALEXPNEXT;
  else
    parser_state &= ~PST_ALEXPNEXT;

  t = pushed_string_list;
  pushed_string_list = pushed_string_list->next;

#if defined (ALIAS)
  if (t->expander)
    t->expander->flags &= ~AL_BEINGEXPANDED;
#endif

  free ((char *)t);
}

static void
free_string_list ()
{
  register STRING_SAVER *t, *t1;

  for (t = pushed_string_list; t; )
    {
      t1 = t->next;
      FREE (t->saved_line);
#if defined (ALIAS)
      if (t->expander)
	t->expander->flags &= ~AL_BEINGEXPANDED;
#endif
      free ((char *)t);
      t = t1;
    }
  pushed_string_list = (STRING_SAVER *)NULL;
}

#endif /* ALIAS || DPAREN_ARITHMETIC */

/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL.  If REMOVE_QUOTED_NEWLINE
   is non-zero, we remove unquoted \<newline> pairs.  This is used by
   read_secondary_line to read here documents. */
static char *
read_a_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  static char *line_buffer = (char *)NULL;
  static int buffer_size = 0;
  int indx = 0, c, peekc, pass_next;

#if defined (READLINE)
  if (interactive && bash_input.type != st_string && no_line_editing)
#else
  if (interactive && bash_input.type != st_string)
#endif
    print_prompt ();

  pass_next = 0;
  while (1)
    {
      c = yy_getc ();

      /* Allow immediate exit if interrupted during input. */
      QUIT;

      if (c == 0)
	continue;

      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  if (interactive && bash_input.type == st_stream)
	    clearerr (stdin);
	  if (indx == 0)
	    return ((char *)NULL);
	  c = '\n';
	}

      /* `+2' in case the final character in the buffer is a newline. */
      RESIZE_MALLOCED_BUFFER (line_buffer, indx, 2, buffer_size, 128);

      /* IF REMOVE_QUOTED_NEWLINES is non-zero, we are reading a
	 here document with an unquoted delimiter.  In this case,
	 the line will be expanded as if it were in double quotes.
	 We allow a backslash to escape the next character, but we
	 need to treat the backslash specially only if a backslash
	 quoting a backslash-newline pair appears in the line. */
      if (pass_next)
        {
	  line_buffer[indx++] = c;
	  pass_next = 0;
        }
      else if (c == '\\' && remove_quoted_newline)
	{
	  peekc = yy_getc ();
	  if (peekc == '\n')
	    continue;	/* Make the unquoted \<newline> pair disappear. */
	  else
	    {
	      yy_ungetc (peekc);
	      pass_next = 1;
	      line_buffer[indx++] = c;		/* Preserve the backslash. */
	    }
	}
      else
	line_buffer[indx++] = c;

      if (c == '\n')
	{
	  line_buffer[indx] = '\0';
	  return (line_buffer);
	}
    }
}

/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt.  This is used to read the lines of a here
   document.  REMOVE_QUOTED_NEWLINE is non-zero if we should remove
   newlines quoted with backslashes while reading the line.  It is
   non-zero unless the delimiter of the here document was quoted. */
char *
read_secondary_line (remove_quoted_newline)
     int remove_quoted_newline;
{
  prompt_string_pointer = &ps2_prompt;
  prompt_again ();
  return (read_a_line (remove_quoted_newline));
}

/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command. */
STRING_INT_ALIST word_token_alist[] = {
  { "if", IF },
  { "then", THEN },
  { "else", ELSE },
  { "elif", ELIF },
  { "fi", FI },
  { "case", CASE },
  { "esac", ESAC },
  { "for", FOR },
#if defined (SELECT_COMMAND)
  { "select", SELECT },
#endif
  { "while", WHILE },
  { "until", UNTIL },
  { "do", DO },
  { "done", DONE },
  { "in", IN },
  { "function", FUNCTION },
#if defined (COMMAND_TIMING)
  { "time", TIME },
#endif
  { "{", '{' },
  { "}", '}' },
  { "!", BANG },
#if defined (COND_COMMAND)
  { "[[", COND_START },
  { "]]", COND_END },
#endif
  { (char *)NULL, 0}
};

/* XXX - we should also have an alist with strings for other tokens, so we
         can give more descriptive error messages.  Look at y.tab.h for the
         other tokens. */

/* These are used by read_token_word, but appear up here so that shell_getc
   can use them to decide when to add otherwise blank lines to the history. */

/* The primary delimiter stack. */
struct dstack dstack = {  (char *)NULL, 0, 0 };

/* A temporary delimiter stack to be used when decoding prompt strings.
   This is needed because command substitutions in prompt strings (e.g., PS2)
   can screw up the parser's quoting state. */
static struct dstack temp_dstack = { (char *)NULL, 0, 0 };

/* Macro for accessing the top delimiter on the stack.  Returns the
   delimiter or zero if none. */
#define current_delimiter(ds) \
  (ds.delimiter_depth ? ds.delimiters[ds.delimiter_depth - 1] : 0)

#define push_delimiter(ds, character) \
  do \
    { \
      if (ds.delimiter_depth + 2 > ds.delimiter_space) \
	ds.delimiters = xrealloc \
	  (ds.delimiters, (ds.delimiter_space += 10) * sizeof (char)); \
      ds.delimiters[ds.delimiter_depth] = character; \
      ds.delimiter_depth++; \
    } \
  while (0)

#define pop_delimiter(ds)	ds.delimiter_depth--

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line.  This is called by read_token when the shell is
   processing normal command input. */

static int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  register int i;
  int c;
  static int mustpop = 0;

  QUIT;

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If shell_input_line[shell_input_line_index] == 0, but there is
     something on the pushed list of strings, then we don't want to go
     off and get another line.  We let the code down below handle it. */

  if (!shell_input_line || ((!shell_input_line[shell_input_line_index]) &&
			    (pushed_string_list == (STRING_SAVER *)NULL)))
#else /* !ALIAS && !DPAREN_ARITHMETIC */
  if (!shell_input_line || !shell_input_line[shell_input_line_index])
#endif /* !ALIAS && !DPAREN_ARITHMETIC */
    {
      line_number++;

    restart_read:

      /* Allow immediate exit if interrupted during input. */
      QUIT;

      i = 0;
      shell_input_line_terminator = 0;

#if defined (JOB_CONTROL)
      /* This can cause a problem when reading a command as the result
	 of a trap, when the trap is called from flush_child.  This call
	 had better not cause jobs to disappear from the job table in
	 that case, or we will have big trouble. */
      notify_and_cleanup ();
#else /* !JOB_CONTROL */
      cleanup_dead_jobs ();
#endif /* !JOB_CONTROL */

#if defined (READLINE)
      if (interactive && bash_input.type != st_string && no_line_editing)
#else
      if (interactive && bash_input.type != st_string)
#endif
	print_prompt ();

      if (bash_input.type == st_stream)
	clearerr (stdin);

      while (c = yy_getc ())
	{
	  /* Allow immediate exit if interrupted during input. */
	  QUIT;

	  RESIZE_MALLOCED_BUFFER (shell_input_line, i, 2, shell_input_line_size, 256);

	  if (c == EOF)
	    {
	      if (bash_input.type == st_stream)
		clearerr (stdin);

	      if (i == 0)
		shell_input_line_terminator = EOF;

	      shell_input_line[i] = '\0';
	      break;
	    }

	  shell_input_line[i++] = c;

	  if (c == '\n')
	    {
	      shell_input_line[--i] = '\0';
	      current_command_line_count++;
	      break;
	    }
	}
      shell_input_line_index = 0;
      shell_input_line_len = i;		/* == strlen (shell_input_line) */

#if defined (HISTORY)
      if (remember_on_history && shell_input_line && shell_input_line[0])
	{
	  char *expansions;
#  if defined (BANG_HISTORY)
	  int old_hist;

	  /* If the current delimiter is a single quote, we should not be
	     performing history expansion, even if we're on a different
	     line from the original single quote. */
	  old_hist = history_expansion_inhibited;
	  if (current_delimiter (dstack) == '\'')
	    history_expansion_inhibited = 1;
#  endif
	  expansions = pre_process_line (shell_input_line, 1, 1);
#  if defined (BANG_HISTORY)
	  history_expansion_inhibited = old_hist;
#  endif
	  if (expansions != shell_input_line)
	    {
	      free (shell_input_line);
	      shell_input_line = expansions;
	      shell_input_line_len = shell_input_line ?
					strlen (shell_input_line) : 0;
	      if (!shell_input_line_len)
		current_command_line_count--;

	      /* We have to force the xrealloc below because we don't know
	         the true allocated size of shell_input_line anymore. */
	      shell_input_line_size = shell_input_line_len;
	    }
	}
      /* XXX - this is grotesque */
      else if (remember_on_history && shell_input_line &&
	       shell_input_line[0] == '\0' &&
	       current_command_line_count > 1 && current_delimiter (dstack))
	{
	  /* We know shell_input_line[0] == 0 and we're reading some sort of
	     quoted string.  This means we've got a line consisting of only
	     a newline in a quoted string.  We want to make sure this line
	     gets added to the history. */
	  maybe_add_history (shell_input_line);
	}

#endif /* HISTORY */

      if (shell_input_line)
	{
	  /* Lines that signify the end of the shell's input should not be
	     echoed. */
	  if (echo_input_at_read && (shell_input_line[0] ||
				     shell_input_line_terminator != EOF))
	    fprintf (stderr, "%s\n", shell_input_line);
	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_string_pointer = &current_prompt_string;
	  prompt_again ();
	  goto restart_read;
	}

      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  if (shell_input_line_len + 3 > shell_input_line_size)
	    shell_input_line = xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  shell_input_line[shell_input_line_len] = '\n';
	  shell_input_line[shell_input_line_len + 1] = '\0';
	}
    }

  c = shell_input_line[shell_input_line_index];

  if (c)
    shell_input_line_index++;

  if (c == '\\' && remove_quoted_newline &&
      shell_input_line[shell_input_line_index] == '\n')
    {
	prompt_again ();
	line_number++;
	goto restart_read;
    }

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  /* If C is NULL, we have reached the end of the current input string.  If
     pushed_string_list is non-empty, it's time to pop to the previous string
     because we have fully consumed the result of the last alias expansion.
     Do it transparently; just return the next character of the string popped
     to. */
  if (!c && (pushed_string_list != (STRING_SAVER *)NULL))
    {
      if (mustpop)
        {
          pop_string ();
          c = shell_input_line[shell_input_line_index];
	  if (c)
	    shell_input_line_index++;
	  mustpop--;
        }
      else
        {
          mustpop++;
          c = ' ';
        }
    }
#endif /* ALIAS || DPAREN_ARITHMETIC */

  if (!c && shell_input_line_terminator == EOF)
    return ((shell_input_line_index != 0) ? '\n' : EOF);

  return ((unsigned char)c);
}

/* Put C back into the input for the shell. */
static void
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
}

static void
shell_ungetchar ()
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line_index--;
}

/* Discard input until CHARACTER is seen, then push that character back
   onto the input stream. */
static void
discard_until (character)
     int character;
{
  int c;

  while ((c = shell_getc (0)) != EOF && c != character)
    ;

  if (c != EOF)
    shell_ungetc (c);
}

void
execute_prompt_command (command)
     char *command;
{
  Function *temp_last, *temp_this;
  char *last_lastarg;
  int temp_exit_value, temp_eof_encountered;

  temp_last = last_shell_builtin;
  temp_this = this_shell_builtin;
  temp_exit_value = last_command_exit_value;
  temp_eof_encountered = eof_encountered;
  last_lastarg = get_string_value ("_");
  if (last_lastarg)
    last_lastarg = savestring (last_lastarg);

  parse_and_execute (savestring (command), "PROMPT_COMMAND", SEVAL_NONINT|SEVAL_NOHIST);

  last_shell_builtin = temp_last;
  this_shell_builtin = temp_this;
  last_command_exit_value = temp_exit_value;
  eof_encountered = temp_eof_encountered;

  bind_variable ("_", last_lastarg);
  FREE (last_lastarg);

  if (token_to_read == '\n')	/* reset_parser was called */
    token_to_read = 0;
}

/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
static char *token = (char *)NULL;

/* Current size of the token buffer. */
static int token_buffer_size;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */
static int
yylex ()
{
  if (interactive && (current_token == 0 || current_token == '\n'))
    {
      /* Before we print a prompt, we might have to check mailboxes.
	 We do this only if it is time to do so. Notice that only here
	 is the mail alarm reset; nothing takes place in check_mail ()
	 except the checking of mail.  Please don't change this. */
      if (prompt_is_ps1 && time_to_check_mail ())
	{
	  check_mail ();
	  reset_mail_timer ();
	}

      /* Avoid printing a prompt if we're not going to read anything, e.g.
	 after resetting the parser with read_token (RESET). */
      if (token_to_read == 0 && interactive)
	prompt_again ();
    }

  two_tokens_ago = token_before_that;
  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int esacs_needed_count;

void
gather_here_documents ()
{
  int r = 0;
  while (need_here_doc)
    {
      make_here_document (redir_stack[r++]);
      need_here_doc--;
    }
}

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_count;

#define command_token_position(token) \
  (((token) == ASSIGNMENT_WORD) || \
   ((token) != SEMI_SEMI && reserved_word_acceptable(token)))

#define assignment_acceptable(token) command_token_position(token) && \
					((parser_state & PST_CASEPAT) == 0)

/* Check to see if TOKEN is a reserved word and return the token
   value if it is. */
#define CHECK_FOR_RESERVED_WORD(tok) \
  do { \
    if (!dollar_present && !quoted && \
	reserved_word_acceptable (last_read_token)) \
      { \
	int i; \
	for (i = 0; word_token_alist[i].word != (char *)NULL; i++) \
	  if (STREQ (tok, word_token_alist[i].word)) \
	    { \
	      if ((parser_state & PST_CASEPAT) && (word_token_alist[i].token != ESAC)) \
		break; \
	      if (word_token_alist[i].token == TIME) \
		break; \
	      if (word_token_alist[i].token == ESAC) \
		parser_state &= ~(PST_CASEPAT|PST_CASESTMT); \
	      else if (word_token_alist[i].token == CASE) \
		parser_state |= PST_CASESTMT; \
	      else if (word_token_alist[i].token == COND_END) \
		parser_state &= ~(PST_CONDCMD|PST_CONDEXPR); \
	      else if (word_token_alist[i].token == COND_START) \
		parser_state |= PST_CONDCMD; \
	      else if (word_token_alist[i].token == '{') \
		open_brace_count++; \
	      else if (word_token_alist[i].token == '}' && open_brace_count) \
		open_brace_count--; \
	      return (word_token_alist[i].token); \
	    } \
      } \
  } while (0)

#if defined (ALIAS)

    /* OK, we have a token.  Let's try to alias expand it, if (and only if)
       it's eligible.

       It is eligible for expansion if the shell is in interactive mode, and
       the token is unquoted and the last token read was a command
       separator (or expand_next_token is set), and we are currently
       processing an alias (pushed_string_list is non-empty) and this
       token is not the same as the current or any previously
       processed alias.

       Special cases that disqualify:
	 In a pattern list in a case statement (parser_state & PST_CASEPAT). */
static int
alias_expand_token (token)
     char *token;
{
  char *expanded;
  alias_t *ap;

  if (((parser_state & PST_ALEXPNEXT) || command_token_position (last_read_token)) &&
	(parser_state & PST_CASEPAT) == 0)
    {
      ap = find_alias (token);

      /* Currently expanding this token. */
      if (ap && (ap->flags & AL_BEINGEXPANDED))
	return (NO_EXPANSION);

      expanded = ap ? savestring (ap->value) : (char *)NULL;
      if (expanded)
	{
	  push_string (expanded, ap->flags & AL_EXPANDNEXT, ap);
	  return (RE_READ_TOKEN);
	}
      else
	/* This is an eligible token that does not have an expansion. */
	return (NO_EXPANSION);
    }
  return (NO_EXPANSION);
}
#endif /* ALIAS */

static int
time_command_acceptable ()
{
#if defined (COMMAND_TIMING)
  switch (last_read_token)
    {
    case 0:
    case ';':
    case '\n':
    case AND_AND:
    case OR_OR:
    case '&':
    case DO:
    case THEN:
    case ELSE:
    case '{':
    case '(':
      return 1;
    default:
      return 0;
    }
#else
  return 0;
#endif /* COMMAND_TIMING */
}

/* Handle special cases of token recognition:
	IN is recognized if the last token was WORD and the token
	before that was FOR or CASE or SELECT.

	DO is recognized if the last token was WORD and the token
	before that was FOR or SELECT.

	ESAC is recognized if the last token caused `esacs_needed_count'
	to be set

	`{' is recognized if the last token as WORD and the token
	before that was FUNCTION.

	`}' is recognized if there is an unclosed `{' prsent.

	`-p' is returned as TIMEOPT if the last read token was TIME.

	']]' is returned as COND_END if the parser is currently parsing
	a conditional expression ((parser_state & PST_CONDEXPR) != 0)

	`time' is returned as TIME if and only if it is immediately
	preceded by one of `;', `\n', `||', `&&', or `&'.
*/

static int
special_case_tokens (token)
     char *token;
{
  if ((last_read_token == WORD) &&
#if defined (SELECT_COMMAND)
      ((token_before_that == FOR) || (token_before_that == CASE) || (token_before_that == SELECT)) &&
#else
      ((token_before_that == FOR) || (token_before_that == CASE)) &&
#endif
      (token[0] == 'i' && token[1] == 'n' && token[2] == 0))
    {
      if (token_before_that == CASE)
	{
	  parser_state |= PST_CASEPAT;
	  esacs_needed_count++;
	}
      return (IN);
    }

  if (last_read_token == WORD &&
#if defined (SELECT_COMMAND)
      (token_before_that == FOR || token_before_that == SELECT) &&
#else
      (token_before_that == FOR) &&
#endif
      (token[0] == 'd' && token[1] == 'o' && token[2] == '\0'))
    return (DO);

  /* Ditto for ESAC in the CASE case.
     Specifically, this handles "case word in esac", which is a legal
     construct, certainly because someone will pass an empty arg to the
     case construct, and we don't want it to barf.  Of course, we should
     insist that the case construct has at least one pattern in it, but
     the designers disagree. */
  if (esacs_needed_count)
    {
      esacs_needed_count--;
      if (STREQ (token, "esac"))
	{
	  parser_state &= ~PST_CASEPAT;
	  return (ESAC);
	}
    }

  /* The start of a shell function definition. */
  if (parser_state & PST_ALLOWOPNBRC)
    {
      parser_state &= ~PST_ALLOWOPNBRC;
      if (token[0] == '{' && token[1] == '\0')		/* } */
	{
	  open_brace_count++;
	  function_bstart = line_number;
	  return ('{');					/* } */
	}
    }

  if (open_brace_count && reserved_word_acceptable (last_read_token) && token[0] == '}' && !token[1])
    {
      open_brace_count--;		/* { */
      return ('}');
    }

#if defined (COMMAND_TIMING)
  /* Handle -p after `time'. */
  if (last_read_token == TIME && token[0] == '-' && token[1] == 'p' && !token[2])
    return (TIMEOPT);
#endif

#if defined (COMMAND_TIMING)
  if (STREQ (token, "time") && ((parser_state & PST_CASEPAT) == 0) && time_command_acceptable ())
    return (TIME);
#endif /* COMMAND_TIMING */

#if defined (COND_COMMAND) /* [[ */
  if ((parser_state & PST_CONDEXPR) && token[0] == ']' && token[1] == ']' && token[2] == '\0')
    return (COND_END);
#endif

  return (-1);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
void
reset_parser ()
{
  dstack.delimiter_depth = 0;	/* No delimiters found so far. */
  open_brace_count = 0;

  parser_state = 0;

#if defined (ALIAS) || defined (DPAREN_ARITHMETIC)
  if (pushed_string_list)
    free_string_list ();
#endif /* ALIAS || DPAREN_ARITHMETIC */

  if (shell_input_line)
    {
      free (shell_input_line);
      shell_input_line = (char *)NULL;
      shell_input_line_size = shell_input_line_index = 0;
    }

  FREE (word_desc_to_read);
  word_desc_to_read = (WORD_DESC *)NULL;

  last_read_token = '\n';
  token_to_read = '\n';
}

/* Read the next token.  Command can be READ (normal operation) or
   RESET (to normalize state). */
static int
read_token (command)
     int command;
{
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */

  if (command == RESET)
    {
      reset_parser ();
      return ('\n');
    }

  if (token_to_read)
    {
      result = token_to_read;
      if (token_to_read == WORD || token_to_read == ASSIGNMENT_WORD)
	{
	  yylval.word = word_desc_to_read;
	  word_desc_to_read = (WORD_DESC *)NULL;
	}
      token_to_read = 0;
      return (result);
    }

#if defined (COND_COMMAND)
  if ((parser_state & (PST_CONDCMD|PST_CONDEXPR)) == PST_CONDCMD)
    {
      cond_lineno = line_number;
      parser_state |= PST_CONDEXPR;
      yylval.command = parse_cond_command ();
      if (cond_token != COND_END)
	{
	  if (EOF_Reached && cond_token != COND_ERROR)		/* [[ */
	    parser_error (cond_lineno, "unexpected EOF while looking for `]]'");
	  else if (cond_token != COND_ERROR)
	    parser_error (cond_lineno, "syntax error in conditional expression");
	  return (-1);
	}
      token_to_read = COND_END;
      parser_state &= ~(PST_CONDEXPR|PST_CONDCMD);
      return (COND_CMD);
    }
#endif

#if defined (ALIAS)
  /* This is a place to jump back to once we have successfully expanded a
     token with an alias and pushed the string with push_string () */
 re_read_token:
#endif /* ALIAS */

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && whitespace (character))
    ;

  if (character == EOF)
    {
      EOF_Reached = 1;
      return (yacc_EOF);
    }

  if (character == '#' && (!interactive || interactive_comments))
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      discard_until ('\n');
      shell_getc (0);
      character = '\n';	/* this will take the next if statement and return. */
    }

  if (character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	gather_here_documents ();

#if defined (ALIAS)
      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      return (character);
    }

  /* Shell meta-characters. */
  if (shellmeta (character) && ((parser_state & PST_DBLPAREN) == 0))
    {
#if defined (ALIAS)
      /* Turn off alias tokenization iff this character sequence would
	 not leave us ready to read a command. */
      if (character == '<' || character == '>')
	parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */

      peek_char = shell_getc (1);
      if (character == peek_char)
	{
	  switch (character)
	    {
	    case '<':
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	      peek_char = shell_getc (1);
	      if (peek_char == '-')
		return (LESS_LESS_MINUS);
	      else
		{
		  shell_ungetc (peek_char);
		  return (LESS_LESS);
		}

	    case '>':
	      return (GREATER_GREATER);

	    case ';':
	      parser_state |= PST_CASEPAT;
#if defined (ALIAS)
	      parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	      return (SEMI_SEMI);

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);

#if defined (DPAREN_ARITHMETIC)
	    case '(':		/* ) */
	      if (reserved_word_acceptable (last_read_token))
		{
		  int cmdtyp, sline;
		  char *wval;
		  WORD_DESC *wd;

		  sline = line_number;
		  cmdtyp = parse_arith_cmd (&wval);
		  if (cmdtyp == 1)	/* arithmetic command */
		    {
		      wd = make_word (wval);
		      wd->flags = W_QUOTED;
		      yylval.word_list = make_word_list (wd, (WORD_LIST *)NULL);
		      free (wval);	/* make_word copies it */
		      return (ARITH_CMD);
		    }
		  else if (cmdtyp == 0)	/* nested subshell */
		    {
		      push_string (wval, 0, (alias_t *)NULL);
		      if ((parser_state & PST_CASEPAT) == 0)
			parser_state |= PST_SUBSHELL;
		      return (character);
		    }
		  else			/* ERROR */
		    return -1;
		}
	      break;
#endif
	    }
	}
      else if (character == '<' && peek_char == '&')
	return (LESS_AND);
      else if (character == '>' && peek_char == '&')
	return (GREATER_AND);
      else if (character == '<' && peek_char == '>')
	return (LESS_GREATER);
      else if (character == '>' && peek_char == '|')
	return (GREATER_BAR);
      else if (peek_char == '>' && character == '&')
	return (AND_GREATER);

      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if (character == ')' && last_read_token == '(' && token_before_that == WORD)
	{
	  parser_state |= PST_ALLOWOPNBRC;
#if defined (ALIAS)
	  parser_state &= ~PST_ALEXPNEXT;
#endif /* ALIAS */
	  function_dstart = line_number;
	}

      /* case pattern lists may be preceded by an optional left paren.  If
	 we're not trying to parse a case pattern list, the left paren
	 indicates a subshell. */
      if (character == '(' && (parser_state & PST_CASEPAT) == 0) /* ) */
	parser_state |= PST_SUBSHELL;
      /*(*/
      else if ((parser_state & PST_CASEPAT) && character == ')')
        parser_state &= ~PST_CASEPAT;
      /*(*/
      else if ((parser_state & PST_SUBSHELL) && character == ')')
	parser_state &= ~PST_SUBSHELL;

#if defined (PROCESS_SUBSTITUTION)
      /* Check for the constructs which introduce process substitution.
	 Shells running in `posix mode' don't do process substitution. */
      if (posixly_correct ||
	  ((character != '>' && character != '<') || peek_char != '('))
#endif /* PROCESS_SUBSTITUTION */
	return (character);
    }

  /* Hack <&- (close stdin) case. */
  if (character == '-' && (last_read_token == LESS_AND || last_read_token == GREATER_AND))
    return (character);

  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  result = read_token_word (character);
#if defined (ALIAS)
  if (result == RE_READ_TOKEN)
    goto re_read_token;
#endif
  return result;
}

/* Match a $(...) or other grouping construct.  This has to handle embedded
   quoted strings ('', ``, "") and nested constructs.  It also must handle
   reprompting the user, if necessary, after reading a newline, and returning
   correct error values if it reads EOF. */

#define P_FIRSTCLOSE	0x01

static char matched_pair_error;
static char *
parse_matched_pair (qc, open, close, lenp, flags)
     int qc;	/* `"' if this construct is within double quotes */
     int open, close;
     int *lenp, flags;
{
  int count, ch, was_dollar;
  int pass_next_character, nestlen, start_lineno;
  char *ret, *nestret;
  int retind, retsize;

  count = 1;
  pass_next_character = was_dollar = 0;

  ret = xmalloc (retsize = 64);
  retind = 0;

  start_lineno = line_number;
  while (count)
    {
      ch = shell_getc (qc != '\'' && pass_next_character == 0);
      if (ch == EOF)
	{
	  free (ret);
	  parser_error (start_lineno, "unexpected EOF while looking for matching `%c'", close);
	  EOF_Reached = 1;	/* XXX */
	  return (&matched_pair_error);
	}

      /* Possible reprompting. */
      if (ch == '\n' && interactive &&
	    (bash_input.type == st_stdin || bash_input.type == st_stream))
	prompt_again ();

      if (pass_next_character)		/* last char was backslash */
	{
	  pass_next_character = 0;
	  if (qc != '\'' && ch == '\n')	/* double-quoted \<newline> disappears. */
	    {
	      if (retind > 0) retind--;	/* swallow previously-added backslash */
	      continue;
	    }

	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  if (ch == CTLESC || ch == CTLNUL)
	    ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if (ch == CTLESC || ch == CTLNUL)	/* special shell escapes */
	{
	  RESIZE_MALLOCED_BUFFER (ret, retind, 2, retsize, 64);
	  ret[retind++] = CTLESC;
	  ret[retind++] = ch;
	  continue;
	}
      else if (ch == close)		/* ending delimiter */
	count--;
      else if (((flags & P_FIRSTCLOSE) == 0) && ch == open)		/* nested begin */
	count++;

      /* Add this character. */
      RESIZE_MALLOCED_BUFFER (ret, retind, 1, retsize, 64);
      ret[retind++] = ch;

      if (open == '\'')			/* '' inside grouping construct */
	continue;

      if (ch == '\\')			/* backslashes */
	pass_next_character++;

      if (open != close)		/* a grouping construct */
	{
	  if (shellquote (ch))
	    {
	      /* '', ``, or "" inside $(...) or other grouping construct. */
	      push_delimiter (dstack, ch);
	      nestret = parse_matched_pair (ch, ch, ch, &nestlen, 0);
	      pop_delimiter (dstack);
	      if (nestret == &matched_pair_error)
		{
		  free (ret);
		  return &matched_pair_error;
		}
	      if (nestlen)
		{
		  RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
		  strcpy (ret + retind, nestret);
		  retind += nestlen;
		}
	      FREE (nestret);
	    }
	}
      /* Parse an old-style command substitution within double quotes as a
	 single word. */
      /* XXX - sh and ksh93 don't do this - XXX */
      else if (open == '"' && ch == '`')
	{
	  nestret = parse_matched_pair (0, '`', '`', &nestlen, 0);
	  if (nestret == &matched_pair_error)
	    {
	      free (ret);
	      return &matched_pair_error;
	    }
	  if (nestlen)
	    {
	      RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
	      strcpy (ret + retind, nestret);
	      retind += nestlen;
	    }
	  FREE (nestret);
	}
      else if (was_dollar && (ch == '(' || ch == '{' || ch == '['))	/* ) } ] */
	/* check for $(), $[], or ${} inside quoted string. */
	{
	  if (open == ch)	/* undo previous increment */
	    count--;
	  if (ch == '(')		/* ) */
	    nestret = parse_matched_pair (0, '(', ')', &nestlen, 0);
	  else if (ch == '{')		/* } */
	    nestret = parse_matched_pair (0, '{', '}', &nestlen, P_FIRSTCLOSE);
	  else if (ch == '[')		/* ] */
	    nestret = parse_matched_pair (0, '[', ']', &nestlen, 0);
	  if (nestret == &matched_pair_error)
	    {
	      free (ret);
	      return &matched_pair_error;
	    }
	  if (nestlen)
	    {
	      RESIZE_MALLOCED_BUFFER (ret, retind, nestlen, retsize, 64);
	      strcpy (ret + retind, nestret);
	      retind += nestlen;
	    }
	  FREE (nestret);
	}
      was_dollar = (ch == '$');
    }

  ret[retind] = '\0';
  if (lenp)
    *lenp = retind;
  return ret;
}

#if defined (DPAREN_ARITHMETIC)
/* We've seen a `(('.  Look for the matching `))'.  If we get it, return 1.
   If not, assume it's a nested subshell for backwards compatibility and
   return 0.  In any case, put the characters we've consumed into a locally-
   allocated buffer and make *ep point to that buffer.  Return -1 on an
   error, for example EOF. */
static int
parse_arith_cmd (ep)
     char **ep;
{
  int exp_lineno, rval, c;
  char *ttok, *token;
  int ttoklen;

  exp_lineno = line_number;
  ttok = parse_matched_pair (0, '(', ')', &ttoklen, 0);
  rval = 1;
  if (ttok == &matched_pair_error)
    return -1;
  /* Check that the next character is the closing right paren.  If
     not, this is a syntax error. ( */
  if ((c = shell_getc (0)) != ')')
    rval = 0;

  token = xmalloc(ttoklen + 4);

  /* (( ... )) -> "..." */
  token[0] = (rval == 1) ? '"' : '(';
  strncpy (token + 1, ttok, ttoklen - 1);	/* don't copy the final `)' */
  if (rval == 1)
    {
      token[ttoklen] = '"';
      token[ttoklen+1] = '\0';
    }
  else
    {
      token[ttoklen] = ')';
      token[ttoklen+1] = c;
      token[ttoklen+2] = '\0';
    }
  *ep = token;
  FREE (ttok);
  return rval;
}
#endif /* DPAREN_ARITHMETIC */

#if defined (COND_COMMAND)
static COND_COM *cond_term ();
static COND_COM *cond_and ();
static COND_COM *cond_or ();
static COND_COM *cond_expr ();

static COND_COM *
cond_expr ()
{
  return (cond_or ());  
}

static COND_COM *
cond_or ()
{
  COND_COM *l, *r;

  l = cond_and ();
  if (cond_token == OR_OR)
    {
      r = cond_or ();
      l = make_cond_node (COND_OR, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static COND_COM *
cond_and ()
{
  COND_COM *l, *r;

  l = cond_term ();
  if (cond_token == AND_AND)
    {
      r = cond_and ();
      l = make_cond_node (COND_AND, (WORD_DESC *)NULL, l, r);
    }
  return l;
}

static int
cond_skip_newlines ()
{
  while ((cond_token = read_token (READ)) == '\n')
    {
      if (interactive && (bash_input.type == st_stdin || bash_input.type == st_stream))
	prompt_again ();
    }
  return (cond_token);
}

#define COND_RETURN_ERROR() \
  do { cond_token = COND_ERROR; return ((COND_COM *)NULL); } while (0)

static COND_COM *
cond_term ()
{
  WORD_DESC *op;
  COND_COM *term, *tleft, *tright;
  int tok, lineno;

  /* Read a token.  It can be a left paren, a `!', a unary operator, or a
     word that should be the first argument of a binary operator.  Start by
     skipping newlines, since this is a compound command. */
  tok = cond_skip_newlines ();
  lineno = line_number;
  if (tok == COND_END)
    {
      COND_RETURN_ERROR ();
    }
  else if (tok == '(')
    {
      term = cond_expr ();
      if (cond_token != ')')
	{
	  if (term)
	    dispose_cond_node (term);		/* ( */
	  parser_error (lineno, "expected `)'");
	  COND_RETURN_ERROR ();
	}
      term = make_cond_node (COND_EXPR, (WORD_DESC *)NULL, term, (COND_COM *)NULL);
      (void)cond_skip_newlines ();
    }
  else if (tok == BANG || (tok == WORD && (yylval.word->word[0] == '!' && yylval.word->word[1] == '\0')))
    {
      if (tok == WORD)
	dispose_word (yylval.word);	/* not needed */
      term = cond_term ();
      if (term)
	term->flags |= CMD_INVERT_RETURN;
    }
  else if (tok == WORD && test_unop (yylval.word->word))
    {
      op = yylval.word;
      tok = read_token (READ);
      if (tok == WORD)
	{
	  tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	}
      else
	{
	  dispose_word (op);
	  parser_error (line_number, "unexpected argument to conditional unary operator");
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  else		/* left argument to binary operator */
    {
      /* lhs */
      tleft = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);

      /* binop */
      tok = read_token (READ);
      if (tok == WORD && test_binop (yylval.word->word))
        op = yylval.word;
      else if (tok == '<' || tok == '>')
        op = make_word_from_token (tok);
      else if (tok == COND_END || tok == AND_AND || tok == OR_OR)
	{
	  /* Special case.  [[ x ]] is equivalent to [[ -n x ]], just like
	     the test command.  Similarly for [[ x && expr ]] or
	     [[ x || expr ]] */
	  op = make_word ("-n");
	  term = make_cond_node (COND_UNARY, op, tleft, (COND_COM *)NULL);
	  cond_token = tok;
	  return (term);
	}
      else
	{
	  parser_error (line_number, "conditional binary operator expected");
	  dispose_cond_node (tleft);
	  COND_RETURN_ERROR ();
	}

      /* rhs */
      tok = read_token (READ);
      if (tok == WORD)
	{
	  tright = make_cond_node (COND_TERM, yylval.word, (COND_COM *)NULL, (COND_COM *)NULL);
	  term = make_cond_node (COND_BINARY, op, tleft, tright);
	}
      else
	{
	  parser_error (line_number, "unexpected argument to conditional binary operator");
	  dispose_cond_node (tleft);
	  dispose_word (op);
	  COND_RETURN_ERROR ();
	}

      (void)cond_skip_newlines ();
    }
  return (term);
}      

/* This is kind of bogus -- we slip a mini recursive-descent parser in
   here to handle the conditional statement syntax. */
static COMMAND *
parse_cond_command ()
{
  COND_COM *cexp;

  cexp = cond_expr ();
  return (make_cond_command (cexp));
}
#endif

static int
read_token_word (character)
     int character;
{
  /* The value for YYLVAL when a WORD is read. */
  WORD_DESC *the_word;

  /* Index into the token that we are building. */
  int token_index;

  /* ALL_DIGITS becomes zero when we see a non-digit. */
  int all_digits;

  /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
  int dollar_present;

  /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
  int quoted;

  /* Non-zero means to ignore the value of the next character, and just
     to add it no matter what. */
 int pass_next_character;

  /* The current delimiting character. */
  int cd;
  int result, peek_char;
  char *ttok, *ttrans;
  int ttoklen, ttranslen;

  if (token_buffer_size < TOKEN_DEFAULT_INITIAL_SIZE)
    token = xrealloc (token, token_buffer_size = TOKEN_DEFAULT_INITIAL_SIZE);

  token_index = 0;
  all_digits = digit (character);
  dollar_present = quoted = pass_next_character = 0;

  for (;;)
    {
      if (character == EOF)
	goto got_token;

      if (pass_next_character)
	{
	  pass_next_character = 0;
	  goto got_character;
	}

      cd = current_delimiter (dstack);

      /* Handle backslashes.  Quote lots of things when not inside of
	 double-quotes, quote some things inside of double-quotes. */
      if (character == '\\')
	{
	  peek_char = shell_getc (0);

	  /* Backslash-newline is ignored in all cases except
	     when quoted with single quotes. */
	  if (peek_char == '\n')
	    {
	      character = '\n';
	      goto next_character;
	    }
	  else
	    {
	      shell_ungetc (peek_char);

	      /* If the next character is to be quoted, note it now. */
	      if (cd == 0 || cd == '`' ||
		  (cd == '"' && member (peek_char, slashify_in_quotes)))
		pass_next_character++;

	      quoted = 1;
	      goto got_character;
	    }
	}

      /* Parse a matched pair of quote characters. */
      if (shellquote (character))
	{
	  push_delimiter (dstack, character);
	  ttok = parse_matched_pair (character, character, character, &ttoklen, 0);
	  pop_delimiter (dstack);
	  if (ttok == &matched_pair_error)
	    return -1;		/* Bail immediately. */
	  RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				  token_buffer_size, TOKEN_DEFAULT_GROW_SIZE);
	  token[token_index++] = character;
	  strcpy (token + token_index, ttok);
	  token_index += ttoklen;
	  all_digits = 0;
	  quoted = 1;
	  dollar_present |= (character == '"' && strchr (ttok, '$') != 0);
	  FREE (ttok);
	  goto next_character;
	}

#ifdef EXTENDED_GLOB
      /* Parse a ksh-style extended pattern matching specification. */
      if (extended_glob && PATTERN_CHAR(character))
	{
	  peek_char = shell_getc (1);
	  if (peek_char == '(')		/* ) */
	    {
	      push_delimiter (dstack, peek_char);
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	      pop_delimiter (dstack);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = all_digits = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif /* EXTENDED_GLOB */

      /* If the delimiter character is not single quote, parse some of
	 the shell expansions that must be read as a single word. */
#if defined (PROCESS_SUBSTITUTION)
      if (character == '$' || character == '<' || character == '>')
#else
      if (character == '$')
#endif /* !PROCESS_SUBSTITUTION */
	{
	  peek_char = shell_getc (1);
	  /* $(...), <(...), >(...), $((...)), ${...}, and $[...] constructs */
	  if (peek_char == '(' ||
		((peek_char == '{' || peek_char == '[') && character == '$'))	/* ) ] } */
	    {
	      if (peek_char == '{')		/* } */
	        ttok = parse_matched_pair (cd, '{', '}', &ttoklen, P_FIRSTCLOSE);
	      else if (peek_char == '(')		/* ) */
		{
		  /* XXX - push and pop the `(' as a delimiter for use by
		     the command-oriented-history code.  This way newlines
		     appearing in the $(...) string get added to the
		     history literally rather than causing a possibly-
		     incorrect `;' to be added. */
		  push_delimiter (dstack, peek_char);
		  ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
		  pop_delimiter (dstack);
		}
	      else
		ttok = parse_matched_pair (cd, '[', ']', &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      dollar_present = 1;
	      all_digits = 0;
	      goto next_character;
	    }
	  /* This handles $'...' and $"..." new-style quoted strings. */
	  else if (character == '$' && (peek_char == '\'' || peek_char == '"'))
	    {
	      int first_line;

	      first_line = line_number;
	      ttok = parse_matched_pair (peek_char, peek_char, peek_char, &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;
	      if (peek_char == '\'')
		ttrans = ansiexpand (ttok, 0, ttoklen - 1, &ttranslen);
	      else
		ttrans = localeexpand (ttok, 0, ttoklen - 1, first_line, &ttranslen);
	      free (ttok);
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttranslen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttrans);
	      token_index += ttranslen;
	      token[token_index++] = peek_char;
	      FREE (ttrans);
	      quoted = 1;
	      all_digits = 0;
	      goto next_character;
	    }
	  /* This could eventually be extended to recognize all of the
	     shell's single-character parameter expansions, and set flags.*/
	  else if (character == '$' && peek_char == '$')
	    {
	      ttok = xmalloc (3);
	      ttok[0] = ttok[1] = '$';
	      ttok[2] = '\0';
	      RESIZE_MALLOCED_BUFFER (token, token_index, 3,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      strcpy (token + token_index, ttok);
	      token_index += 2;
	      dollar_present = 1;
	      all_digits = 0;
	      FREE (ttok);
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}

#if defined (ARRAY_VARS)
      /* Identify possible compound array variable assignment. */
      else if (character == '=' && token_index > 0)
	{
	  peek_char = shell_getc (1);
	  if (peek_char == '(')		/* ) */
	    {
	      ttok = parse_matched_pair (cd, '(', ')', &ttoklen, 0);
	      if (ttok == &matched_pair_error)
		return -1;		/* Bail immediately. */
	      if (ttok[0] == '(')	/* ) */
		{
		  FREE (ttok);
		  return -1;
		}
	      RESIZE_MALLOCED_BUFFER (token, token_index, ttoklen + 2,
				      token_buffer_size,
				      TOKEN_DEFAULT_GROW_SIZE);
	      token[token_index++] = character;
	      token[token_index++] = peek_char;
	      strcpy (token + token_index, ttok);
	      token_index += ttoklen;
	      FREE (ttok);
	      all_digits = 0;
	      goto next_character;
	    }
	  else
	    shell_ungetc (peek_char);
	}
#endif

      /* When not parsing a multi-character word construct, shell meta-
	 characters break words. */
      if (shellbreak (character))
	{
	  shell_ungetc (character);
	  goto got_token;
	}

    got_character:

      all_digits &= digit (character);
      dollar_present |= character == '$';

      if (character == CTLESC || character == CTLNUL)
	token[token_index++] = CTLESC;

      token[token_index++] = character;

      RESIZE_MALLOCED_BUFFER (token, token_index, 1, token_buffer_size,
			      TOKEN_DEFAULT_GROW_SIZE);

    next_character:
      if (character == '\n' && interactive &&
	(bash_input.type == st_stdin || bash_input.type == st_stream))
	prompt_again ();

      /* We want to remove quoted newlines (that is, a \<newline> pair)
	 unless we are within single quotes or pass_next_character is
	 set (the shell equivalent of literal-next). */
      cd = current_delimiter (dstack);
      character = shell_getc (cd != '\'' && pass_next_character == 0);
    }	/* end for (;;) */

got_token:

  token[token_index] = '\0';

  /* Check to see what thing we should return.  If the last_read_token
     is a `<', or a `&', or the character which ended this token is
     a '>' or '<', then, and ONLY then, is this input token a NUMBER.
     Otherwise, it is just a word, and should be returned as such. */
  if (all_digits && (character == '<' || character == '>' ||
		    last_read_token == LESS_AND ||
		    last_read_token == GREATER_AND))
      {
	yylval.number = atoi (token);
	return (NUMBER);
      }

  /* Check for special case tokens. */
  result = special_case_tokens (token);
  if (result >= 0)
    return result;

#if defined (ALIAS)
  /* Posix.2 does not allow reserved words to be aliased, so check for all
     of them, including special cases, before expanding the current token
     as an alias. */
  if (posixly_correct)
    CHECK_FOR_RESERVED_WORD (token);

  /* Aliases are expanded iff EXPAND_ALIASES is non-zero, and quoting
     inhibits alias expansion. */
  if (expand_aliases && quoted == 0)
    {
      result = alias_expand_token (token);
      if (result == RE_READ_TOKEN)
	return (RE_READ_TOKEN);
      else if (result == NO_EXPANSION)
	parser_state &= ~PST_ALEXPNEXT;
    }

  /* If not in Posix.2 mode, check for reserved words after alias
     expansion. */
  if (posixly_correct == 0)
#endif
    CHECK_FOR_RESERVED_WORD (token);

  the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
  the_word->word = xmalloc (1 + token_index);
  the_word->flags = 0;
  strcpy (the_word->word, token);
  if (dollar_present)
    the_word->flags |= W_HASDOLLAR;
  if (quoted)
    the_word->flags |= W_QUOTED;
  /* A word is an assignment if it appears at the beginning of a
     simple command, or after another assignment word.  This is
     context-dependent, so it cannot be handled in the grammar. */
  if (assignment (token))
    {
      the_word->flags |= W_ASSIGNMENT;
      /* Don't perform word splitting on assignment statements. */
      if (assignment_acceptable (last_read_token))
	the_word->flags |= W_NOSPLIT;
    }

  yylval.word = the_word;

  result = ((the_word->flags & (W_ASSIGNMENT|W_NOSPLIT)) == (W_ASSIGNMENT|W_NOSPLIT))
		? ASSIGNMENT_WORD : WORD;

  if (last_read_token == FUNCTION)
    {
      parser_state |= PST_ALLOWOPNBRC;
      function_dstart = line_number;
    }

  return (result);
}

/* $'...' ANSI-C expand the portion of STRING between START and END and
   return the result.  The result cannot be longer than the input string. */
static char *
ansiexpand (string, start, end, lenp)
     char *string;
     int start, end, *lenp;
{
  char *temp, *t;
  int len, tlen;

  temp = xmalloc (end - start + 1);
  for (tlen = 0, len = start; len < end; )
    temp[tlen++] = string[len++];
  temp[tlen] = '\0';

  if (*temp)
    {
      t = ansicstr (temp, tlen, (int *)NULL, lenp);
      free (temp);
      return (t);
    }
  else
    {
      if (lenp)
	*lenp = 0;
      return (temp);
    }
}

/* $"..." -- Translate the portion of STRING between START and END
   according to current locale using gettext (if available) and return
   the result.  The caller will take care of leaving the quotes intact.
   The string will be left without the leading `$' by the caller.
   If translation is performed, the translated string will be double-quoted
   by the caller.  The length of the translated string is returned in LENP,
   if non-null. */
static char *
localeexpand (string, start, end, lineno, lenp)
     char *string;
     int start, end, lineno, *lenp;
{
  int len, tlen;
  char *temp, *t;

  temp = xmalloc (end - start + 1);
  for (tlen = 0, len = start; len < end; )
    temp[tlen++] = string[len++];
  temp[tlen] = '\0';

  /* If we're just dumping translatable strings, don't do anything. */
  if (dump_translatable_strings)
    {
      if (dump_po_strings)
	printf ("#: %s:%d\nmsgid \"%s\"\nmsgstr \"\"\n",
		(bash_input.name ? bash_input.name : "stdin"), lineno, temp);
      else
	printf ("\"%s\"\n", temp);
      if (lenp)
	*lenp = tlen;
      return (temp);
    }
  else if (*temp)
    {
      t = localetrans (temp, tlen, &len);
      free (temp);
      if (lenp)
	*lenp = len;
      return (t);
    }
  else
    {
      if (lenp)
	*lenp = 0;
      return (temp);
    }
}

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (token)
     int token;
{
  if (token == '\n' || token == ';' || token == '(' || token == ')' ||
      token == '|' || token == '&' || token == '{' ||
      token == '}' ||			/* XXX */
      token == AND_AND ||
      token == BANG ||
      token == TIME || token == TIMEOPT ||
      token == DO ||
      token == ELIF ||
      token == ELSE ||
      token == FI ||
      token == IF ||
      token == OR_OR ||
      token == SEMI_SEMI ||
      token == THEN ||
      token == UNTIL ||
      token == WHILE ||
      token == DONE ||		/* XXX these two are experimental */
      token == ESAC ||
      token == 0)
    return (1);
  else
    return (0);
}

/* Return the index of TOKEN in the alist of reserved words, or -1 if
   TOKEN is not a shell reserved word. */
int
find_reserved_word (token)
     char *token;
{
  int i;
  for (i = 0; word_token_alist[i].word; i++)
    if (STREQ (token, word_token_alist[i].word))
      return i;
  return -1;
}

#if 0
#if defined (READLINE)
/* Called after each time readline is called.  This insures that whatever
   the new prompt string is gets propagated to readline's local prompt
   variable. */
static void
reset_readline_prompt ()
{
  char *temp_prompt;

  if (prompt_string_pointer)
    {
      temp_prompt = (*prompt_string_pointer)
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

      if (temp_prompt == 0)
	{
	  temp_prompt = xmalloc (1);
	  temp_prompt[0] = '\0';
	}

      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
}
#endif /* READLINE */
#endif /* 0 */

#if defined (HISTORY)
/* A list of tokens which can be followed by newlines, but not by
   semi-colons.  When concatenating multiple lines of history, the
   newline separator for such tokens is replaced with a space. */
static int no_semi_successors[] = {
  '\n', '{', '(', ')', ';', '&', '|',
  CASE, DO, ELSE, IF, SEMI_SEMI, THEN, UNTIL, WHILE, AND_AND, OR_OR, IN,
  0
};

/* If we are not within a delimited expression, try to be smart
   about which separators can be semi-colons and which must be
   newlines.  Returns the string that should be added into the
   history entry. */
char *
history_delimiting_chars ()
{
  register int i;

  if (dstack.delimiter_depth != 0)
    return ("\n");
    
  /* First, handle some special cases. */
  /*(*/
  /* If we just read `()', assume it's a function definition, and don't
     add a semicolon.  If the token before the `)' was not `(', and we're
     not in the midst of parsing a case statement, assume it's a
     parenthesized command and add the semicolon. */
  /*)(*/
  if (token_before_that == ')')
    {
      if (two_tokens_ago == '(')	/*)*/	/* function def */
	return " ";
      /* This does not work for subshells inside case statement
	 command lists.  It's a suboptimal solution. */
      else if (parser_state & PST_CASESTMT)	/* case statement pattern */
        return " ";
      else	
        return "; ";				/* (...) subshell */
    }
  else if (token_before_that == WORD && two_tokens_ago == FUNCTION)
    return " ";		/* function def using `function name' without `()' */

  for (i = 0; no_semi_successors[i]; i++)
    {
      if (token_before_that == no_semi_successors[i])
	return (" ");
    }

  return ("; ");
}
#endif /* HISTORY */

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
static void
prompt_again ()
{
  char *temp_prompt;

  if (!interactive)	/* XXX */
    return;

  ps1_prompt = get_string_value ("PS1");
  ps2_prompt = get_string_value ("PS2");

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  temp_prompt = *prompt_string_pointer
			? decode_prompt_string (*prompt_string_pointer)
			: (char *)NULL;

  if (temp_prompt == 0)
    {
      temp_prompt = xmalloc (1);
      temp_prompt[0] = '\0';
    }

  current_prompt_string = *prompt_string_pointer;
  prompt_string_pointer = &ps2_prompt;

#if defined (READLINE)
  if (!no_line_editing)
    {
      FREE (current_readline_prompt);
      current_readline_prompt = temp_prompt;
    }
  else
#endif	/* READLINE */
    {
      FREE (current_decoded_prompt);
      current_decoded_prompt = temp_prompt;
    }
}

static void
print_prompt ()
{
  fprintf (stderr, "%s", current_decoded_prompt);
  fflush (stderr);
}

/* Return a string which will be printed as a prompt.  The string
   may contain special characters which are decoded as follows:

	\a	bell (ascii 07)
	\e	escape (ascii 033)
	\d	the date in Day Mon Date format
	\h	the hostname up to the first `.'
	\H	the hostname
	\n	CRLF
	\s	the name of the shell
	\t	the time in 24-hour hh:mm:ss format
	\T	the time in 12-hour hh:mm:ss format
	\@	the time in 12-hour am/pm format
	\v	the version of bash (e.g., 2.00)
	\V	the release of bash, version + patchlevel (e.g., 2.00.0)
	\w	the current working directory
	\W	the last element of $PWD
	\u	your username
	\#	the command number of this command
	\!	the history number of this command
	\$	a $ or a # if you are root
	\nnn	character code nnn in octal
	\\	a backslash
	\[	begin a sequence of non-printing chars
	\]	end a sequence of non-printing chars
*/
#define PROMPT_GROWTH 48
char *
decode_prompt_string (string)
     char *string;
{
  WORD_LIST *list;
  char *result, *t;
  struct dstack save_dstack;
#if defined (PROMPT_STRING_DECODE)
  int result_size, result_index;
  int c, n;
  char *temp, octal_string[4];
  time_t the_time;

  result = xmalloc (result_size = PROMPT_GROWTH);
  result[result_index = 0] = 0;
  temp = (char *)NULL;

  while (c = *string++)
    {
      if (posixly_correct && c == '!')
	{
	  if (*string == '!')
	    {
	      temp = savestring ("!");
	      goto add_string;
	    }
	  else
	    {
#if !defined (HISTORY)
		temp = savestring ("1");
#else /* HISTORY */
		temp = itos (history_number ());
#endif /* HISTORY */
		string--;	/* add_string increments string again. */
		goto add_string;
	    }
	}
      if (c == '\\')
	{
	  c = *string;

	  switch (c)
	    {
	    case '0':
	    case '1':
	    case '2':
	    case '3':
	    case '4':
	    case '5':
	    case '6':
	    case '7':
	      strncpy (octal_string, string, 3);
	      octal_string[3] = '\0';

	      n = read_octal (octal_string);
	      temp = xmalloc (3);

	      if (n == CTLESC || n == CTLNUL)
		{
		  string += 3;
		  temp[0] = CTLESC;
		  temp[1] = n;
		  temp[2] = '\0';
		}
	      else if (n == -1)
		{
		  temp[0] = '\\';
		  temp[1] = '\0';
		}
	      else
		{
		  string += 3;
		  temp[0] = n;
		  temp[1] = '\0';
		}

	      c = 0;
	      goto add_string;

	    case 't':
	    case 'd':
	    case 'T':
	    case '@':
	      /* Make the current time/date into a string. */
	      the_time = time (0);
	      temp = ctime (&the_time);

	      temp = (c != 'd') ? savestring (temp + 11) : savestring (temp);
	      temp[(c != 'd') ? 8 : 10] = '\0';

	      /* quick and dirty conversion to 12-hour time */
	      if (c == 'T' || c == '@')
		{
		  if (c == '@')
		    {
		      temp[5] = 'a';	/* am/pm format */
		      temp[6] = 'm';
		      temp[7] = '\0';
		    }
		  c = temp[2];
		  temp[2] = '\0';
		  n = atoi (temp);
		  temp[2] = c;
		  n -= 12;
		  if (n > 0)
		    {
		      temp[0] = (n / 10) + '0';
		      temp[1] = (n % 10) + '0';
		    }
		  if (n >= 0 && temp[5] == 'a')
		    temp[5] = 'p';
		}
	      goto add_string;

	    case 'r':
	      temp = xmalloc (2);
	      temp[0] = '\r';
	      temp[1] = '\0';
	      goto add_string;

	    case 'n':
	      temp = xmalloc (3);
	      temp[0] = no_line_editing ? '\n' : '\r';
	      temp[1] = no_line_editing ? '\0' : '\n';
	      temp[2] = '\0';
	      goto add_string;

	    case 's':
	      temp = base_pathname (shell_name);
	      temp = savestring (temp);
	      goto add_string;

	    case 'v':
	    case 'V':
	      temp = xmalloc (8);
	      if (c == 'v')
		strcpy (temp, dist_version);
	      else
		sprintf (temp, "%s.%d", dist_version, patch_level);
	      goto add_string;

	    case 'w':
	    case 'W':
	      {
		/* Use the value of PWD because it is much more efficient. */
		char t_string[PATH_MAX];
		int tlen;

		temp = get_string_value ("PWD");

		if (temp == 0)
		  {
		    if (getcwd (t_string, sizeof(t_string)) == 0)
		      {
		        t_string[0] = '.';
			tlen = 1;
		      }
		    else
		      tlen = strlen (t_string);
		  }
		else
		  {
		    tlen = sizeof (t_string) - 1;
		    strncpy (t_string, temp, tlen);
		  }
		t_string[tlen] = '\0';

		if (c == 'W')
		  {
		    t = strrchr (t_string, '/');
		    if (t && t != t_string)
		      strcpy (t_string, t + 1);
		  }
		else
		  /* polite_directory_format is guaranteed to return a string
		     no longer than PATH_MAX - 1 characters. */
		  strcpy (t_string, polite_directory_format (t_string));

		/* If we're going to be expanding the prompt string later,
		   quote the directory name. */
		if (promptvars || posixly_correct)
		  temp = backslash_quote (t_string);
		else
		  temp = savestring (t_string);

		goto add_string;
	      }

	    case 'u':
	      temp = savestring (current_user.user_name);
	      goto add_string;

	    case 'h':
	    case 'H':
	      temp = savestring (current_host_name);
	      if (c == 'h' && (t = (char *)strchr (temp, '.')))
		*t = '\0';
	      goto add_string;

	    case '#':
	      temp = itos (current_command_number);
	      goto add_string;

	    case '!':
#if !defined (HISTORY)
	      temp = savestring ("1");
#else /* HISTORY */
	      temp = itos (history_number ());
#endif /* HISTORY */
	      goto add_string;

	    case '$':
	      t = temp = xmalloc (3);
	      if ((promptvars || posixly_correct) && (current_user.euid != 0))
		*t++ = '\\';
	      *t++ = current_user.euid == 0 ? '#' : '$';
	      *t = '\0';
	      goto add_string;

#if defined (READLINE)
	    case '[':
	    case ']':
	      temp = xmalloc (3);
	      temp[0] = '\001';
	      temp[1] = (c == '[') ? RL_PROMPT_START_IGNORE : RL_PROMPT_END_IGNORE;
	      temp[2] = '\0';
	      goto add_string;
#endif /* READLINE */

	    case '\\':
	      temp = xmalloc (2);
	      temp[0] = c;
	      temp[1] = '\0';
	      goto add_string;

	    case 'a':
	    case 'e':
	      temp = xmalloc (2);
	      temp[0] = (c == 'a') ? '\07' : '\033';
	      temp[1] = '\0';
	      goto add_string;

	    default:
	      temp = xmalloc (3);
	      temp[0] = '\\';
	      temp[1] = c;
	      temp[2] = '\0';

	    add_string:
	      if (c)
		string++;
	      result =
		sub_append_string (temp, result, &result_index, &result_size);
	      temp = (char *)NULL; /* Freed in sub_append_string (). */
	      result[result_index] = '\0';
	      break;
	    }
	}
      else
	{
	  RESIZE_MALLOCED_BUFFER (result, result_index, 3, result_size, PROMPT_GROWTH);
	  result[result_index++] = c;
	  result[result_index] = '\0';
	}
    }
#else /* !PROMPT_STRING_DECODE */
  result = savestring (string);
#endif /* !PROMPT_STRING_DECODE */

  /* Save the delimiter stack and point `dstack' to temp space so any
     command substitutions in the prompt string won't result in screwing
     up the parser's quoting state. */
  save_dstack = dstack;
  dstack = temp_dstack;
  dstack.delimiter_depth = 0;

  /* Perform variable and parameter expansion and command substitution on
     the prompt string. */
  if (promptvars || posixly_correct)
    {
      list = expand_string_unsplit (result, Q_DOUBLE_QUOTES);
      free (result);
      result = string_list (list);
      dispose_words (list);
    }
  else
    {
      t = dequote_string (result);
      free (result);
      result = t;
    }

  dstack = save_dstack;

  return (result);
}

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
int
yyerror ()
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
  return (0);
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
static void
report_syntax_error (message)
     char *message;
{
  char *msg, *t;
  int token_end, i;
  char msg2[2];

  if (message)
    {
      parser_error (line_number, "%s", message);
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
      last_command_exit_value = EX_USAGE;
      return;
    }

  /* If the line of input we're reading is not null, try to find the
     objectionable token. */
  if (shell_input_line && *shell_input_line)
    {
      t = shell_input_line;
      i = shell_input_line_index;
      token_end = 0;

      if (i && t[i] == '\0')
	i--;

      while (i && (whitespace (t[i]) || t[i] == '\n'))
	i--;

      if (i)
	token_end = i + 1;

      while (i && (member (t[i], " \n\t;|&") == 0))
	i--;

      while (i != token_end && (whitespace (t[i]) || t[i] == '\n'))
	i++;

      /* Print the offending token. */
      if (token_end || (i == 0 && token_end == 0))
	{
	  if (token_end)
	    {
	      msg = xmalloc (1 + (token_end - i));
	      strncpy (msg, t + i, token_end - i);
	      msg[token_end - i] = '\0';
	    }
	  else	/* one-character token */
	    {
	      msg2[0] = t[i];
	      msg2[1] = '\0';
	      msg = msg2;
	    }

	  parser_error (line_number, "syntax error near unexpected token `%s'", msg);

	  if (msg != msg2)
	    free (msg);
	}

      /* If not interactive, print the line containing the error. */
      if (interactive == 0)
	{
	  msg = savestring (shell_input_line);
	  token_end = strlen (msg);
	  while (token_end && msg[token_end - 1] == '\n')
	    msg[--token_end] = '\0';

	  parser_error (line_number, "`%s'", msg);
	  free (msg);
	}
    }
  else
    {
      msg = EOF_Reached ? "syntax error: unexpected end of file" : "syntax error";
      parser_error (line_number, "%s", msg);
      /* When the shell is interactive, this file uses EOF_Reached
	 only for error reporting.  Other mechanisms are used to
	 decide whether or not to exit. */
      if (interactive && EOF_Reached)
	EOF_Reached = 0;
    }
  last_command_exit_value = EX_USAGE;
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command. */
static void
discard_parser_constructs (error_p)
     int error_p;
{
}

/* Do that silly `type "bye" to exit' stuff.  You know, "ignoreeof". */

/* A flag denoting whether or not ignoreeof is set. */
int ignoreeof = 0;

/* The number of times that we have encountered an EOF character without
   another character intervening.  When this gets above the limit, the
   shell terminates. */
int eof_encountered = 0;

/* The limit for eof_encountered. */
int eof_encountered_limit = 10;

/* If we have EOF as the only input unit, this user wants to leave
   the shell.  If the shell is not interactive, then just leave.
   Otherwise, if ignoreeof is set, and we haven't done this the
   required number of times in a row, print a message. */
static void
handle_eof_input_unit ()
{
  if (interactive)
    {
      /* shell.c may use this to decide whether or not to write out the
	 history, among other things.  We use it only for error reporting
	 in this file. */
      if (EOF_Reached)
	EOF_Reached = 0;

      /* If the user wants to "ignore" eof, then let her do so, kind of. */
      if (ignoreeof)
	{
	  if (eof_encountered < eof_encountered_limit)
	    {
	      fprintf (stderr, "Use \"%s\" to leave the shell.\n",
		       login_shell ? "logout" : "exit");
	      eof_encountered++;
	      /* Reset the prompt string to be $PS1. */
	      prompt_string_pointer = (char **)NULL;
	      prompt_again ();
	      last_read_token = current_token = '\n';
	      return;
	    }
	}

      /* In this case EOF should exit the shell.  Do it now. */
      reset_parser ();
      exit_builtin ((WORD_LIST *)NULL);
    }
  else
    {
      /* We don't write history files, etc., for non-interactive shells. */
      EOF_Reached = 1;
    }
}
