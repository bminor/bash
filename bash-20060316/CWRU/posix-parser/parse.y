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
#include <stdio.h>
#include <sys/types.h>
#include <signal.h>
#include <varargs.h>
#include "shell.h"
#include "flags.h"

#define YYDEBUG 1
extern int eof_encountered;
extern int no_line_editing;
extern int interactive, interactive_shell;

extern COMMAND *new_make_simple_command ();

extern char *index ();
/* **************************************************************** */
/*								    */
/*		    "Forward" declarations			    */
/*								    */
/* **************************************************************** */

/* This is kind of sickening.  In order to let these variables be seen by
   all the functions that need them, I am forced to place their declarations
   far away from the place where they should logically be found. */

static int reserved_word_acceptable ();

/* PROMPT_STRING_POINTER points to one of these, never to an actual string. */
char *ps1_prompt, *ps2_prompt;

/* Handle on the current prompt string.  Indirectly points through
   ps1_ or ps2_prompt. */
char **prompt_string_pointer = (char **)NULL;
char *current_prompt_string;

/* The number of lines read from input while creating the current command. */
int current_command_line_count = 0;

/* Variables to manage the task of reading here documents, because we need to
   defer the reading until after a complete command has been collected. */
REDIRECT *redirection_needing_here_doc = (REDIRECT *)NULL;
int need_here_doc = 0;
%}

%union {
  WORD_DESC *word;		/* the word that we read. */
  int number;			/* the number that we read. */
  WORD_LIST *word_list;
  COMMAND *command;
  REDIRECT *redirect;
  ELEMENT *element;
  PATTERN_LIST *pattern;
  int separator_token;
}

/* Reserved words.  Members of the first group are only recognized
   in the case that they are preceded by a list_terminator.  Members
   of the second group are recognized only under special circumstances. */
%token IF THEN ELSE ELIF FI CASE ESAC FOR WHILE UNTIL DO DONE FUNCTION
%token IN BANG

/* More general tokens. yylex () knows how to make these. */
%token <word> WORD ASSIGNMENT_WORD NAME
%token <number> NUMBER
%token AND_AND OR_OR GREATER_GREATER LESS_LESS LESS_AND
%token GREATER_AND SEMI_SEMI LESS_LESS_MINUS AND_GREATER LESS_GREATER
%token GREATER_BAR NEWLINE

/* The types that the various syntactical units return. */

%type <command> complete_command
%type <command> list and_or pipeline pipe_sequence tcommand compound_command
%type <command> simple_command subshell compound_list term
%type <command> function_definition brace_group
%type <command> for_clause case_clause while_clause if_clause until_clause
%type <command> do_group else_part
%type <redirect> redirect_list redirect
%type <word_list> wordlist pattern 
%type <separator_token> separator separator_op linebreak newline_list
%type <separator_token> sequential_sep
%type <pattern> case_list case_item
%type <element> cmd_prefix cmd_suffix
%type <word> cmd_name cmd_word

%start inputunit

%left yacc_EOF
%%

inputunit:	complete_command NEWLINE
			{
			  TRACE("reduced complete_command NEWLINE to inputunit");
			  global_command = $1;
			  YYACCEPT;
			}
	|	NEWLINE
			{
			  TRACE("reduced NEWLINE to inputunit");
			  global_command = (COMMAND *)NULL;
			  YYACCEPT;
			}
	;

complete_command
	:	list separator
			{
			  TRACE("reduced 'list separator' to complete_command");
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  $$ = $1;
			}
	|	list
			{
			  TRACE("reduced list to complete_command");
			  /* Case of regular command.  Discard the error
			     safety net,and return the command just parsed. */
			  $$ = $1;
			}
	|	error
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

list	:	list separator_op and_or
			{
			  $$ = command_connect ($1, $3, $2);
			  if (need_here_doc)
			    make_here_document (redirection_needing_here_doc);
			  need_here_doc = 0;
			 }
	|	and_or
			{
			  $$ = $1;
			  if (need_here_doc)
			    make_here_document (redirection_needing_here_doc);
			  need_here_doc = 0;
			  TRACE("reduced and_or to list");
			 }
	;

and_or	:	pipeline
			{
			  TRACE("reduced pipeline to and_or");
			  $$ = $1;
			}
	|	and_or AND_AND linebreak pipeline
			{ $$ = command_connect ($1, $4, AND_AND); }
	|	and_or OR_OR linebreak pipeline
			{ $$ = command_connect ($1, $4, OR_OR); }
	;

pipeline:	pipe_sequence
			{
			  TRACE("reduced pipe_sequence to pipeline");
			  $$ = $1;
			}
	|	BANG pipe_sequence
			{
			  $2->flags |= CMD_INVERT_RETURN;
			  $$ = $2;
			}
	;

pipe_sequence:	tcommand
			{ 
			  TRACE("reduced tcommand to pipe_sequence");
			  $$ = $1;
			}
	|	pipe_sequence '|' linebreak tcommand
			{ $$ = command_connect ($1, $4, '|'); }
	;

tcommand:	simple_command
			{ 
			  TRACE("reduced simple_command to tcommand");
			  $$ = clean_simple_command ($1);
			}
	|	compound_command
			{ $$ = $1; }
	|	compound_command redirect_list
			{
			  $1->redirects = $2;
			  $$ = $1;
			}
	|	function_definition
			{ $$ = $1; }
	;

compound_command
	:	brace_group
			{ $$ = $1; }
	|	subshell
			{ $$ = $1; }
	|	for_clause
			{
			  TRACE("reduced FOR_CLAUSE to COMPOUND_COMMAND");
			  $$ = $1;
			}
	|	case_clause
			{ $$ = $1; }
	|	if_clause
			{
			  TRACE("reduced IF_CLAUSE to COMPOUND_COMMAND");
			  $$ = $1;
			}
	|	while_clause
			{
			  TRACE("reduced WHILE_CLAUSE to COMPOUND_COMMAND");
			  $$ = $1;
			}
	|	until_clause
			{ $$ = $1; }
	;

subshell:	'(' list ')'
			{ $2->flags |= CMD_WANT_SUBSHELL; $$ = $2; }
	;


compound_list
	:	term
			{ $$ = $1; }
	|	newline_list term
			{ $$ = $2; }
	|	term separator
			{ $$ = command_connect ($1, 0, $2); }
	|	newline_list term separator
			{ $$ = command_connect ($2, 0, $3); }
	;

term	:	term separator and_or
			{ $$ = command_connect ($1, $3, $2); }
	|	and_or
			{ $$ = $1; }
	;

for_clause
	:	FOR WORD do_group
			{ $$ = make_for_command ($2, (WORD_LIST *)add_string_to_list ("\"$@\"", (WORD_LIST *)NULL), $3); }
	|	FOR WORD newline_list IN wordlist sequential_sep do_group
			{ $$ = make_for_command ($2, (WORD_LIST *)reverse_list ($5), $7); }
	|	FOR WORD IN wordlist sequential_sep do_group
			{
			  TRACE("reduced FOR_CLAUSE");
			  $$ = make_for_command ($2, (WORD_LIST *)reverse_list ($4), $6);
			}
	;

wordlist:	wordlist WORD
			{ $$ = make_word_list ($2, $1); }
	|	WORD
			{ $$ = make_word_list ($1, (WORD_LIST *)NULL); }
	;

case_clause
	:	CASE WORD IN linebreak case_list ESAC
			{ $$ = make_case_command ($2, $5); }
	|	CASE WORD IN linebreak ESAC
			{ $$ = make_case_command ($2, (PATTERN_LIST *)NULL); }
	;

case_list
	:	case_list case_item
			{ $2->next = $1; $$ = $2; }
	|	case_item
			{ $$ = $1; }
	;

case_item
	:	pattern ')' linebreak SEMI_SEMI linebreak
			{ $$ = make_pattern_list ($1, (COMMAND *)NULL); }
	|	pattern ')' compound_list SEMI_SEMI linebreak
			{ $$ = make_pattern_list ($1, $3); }
	|	'(' pattern ')' compound_list SEMI_SEMI linebreak
			{ $$ = make_pattern_list ($2, $4); }
	|	'(' pattern ')' linebreak SEMI_SEMI linebreak
			{ $$ = make_pattern_list ($2, (COMMAND *)NULL); }
	;

pattern:	WORD
			{ $$ = make_word_list ($1, (WORD_LIST *)NULL); }
	|	pattern '|' WORD
			{ $$ = make_word_list ($3, $1); }
	;

if_clause:	IF compound_list THEN compound_list else_part FI
			{ $$ = make_if_command ($2, $4, $5); }
	|	IF compound_list THEN compound_list FI
			{ $$ = make_if_command ($2, $4, (COMMAND *)NULL); }
	;

else_part:	ELIF compound_list THEN else_part
			{ $$ = make_if_command ($2, $4, (COMMAND *)NULL); }
	|	ELSE compound_list
			{ $$ = $2; }
	;

while_clause:	WHILE compound_list do_group
			{
			  TRACE("reduced WHILE_CLAUSE");
			  $$ = make_while_command ($2, $3);
			}
	;

until_clause:	UNTIL compound_list do_group
			{ $$ = make_until_command ($2, $3); }
	;

function_definition
	:	WORD '(' ')' linebreak brace_group
			{ $$ = make_function_def ($1, $5); }
	;

brace_group:	'{' compound_list '}'
			{ $$ = make_group_command ($2); }
	;


do_group:	DO compound_list DONE
			{
			  TRACE("reduced DO_GROUP");
			  $$ = $2;
			}
	;

cmd_prefix:	ASSIGNMENT_WORD
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = $1;
			  temp_element->redirect = (REDIRECT *)NULL;
			  temp_element->next = (struct element *)NULL;
			  TRACE("reduced cmd_prefix to ASSIGNMENT_WORD");
			  $$ = temp_element;
			}
	|	cmd_prefix ASSIGNMENT_WORD
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = $2;
			  temp_element->redirect = (REDIRECT *)NULL;
			  temp_element->next = (struct element *)NULL;
			  $1->next = temp_element;
			  TRACE("reduced cmd_prefix to cmd_prefix ASSIGNMENT_WORD");
			  $$ = $1;
			}
	|	redirect
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = (WORD_DESC *)NULL;
			  temp_element->redirect = $1;
			  temp_element->next = (struct element *)NULL;
			  TRACE("reduced cmd_prefix to redirect");
			  $$ = temp_element;
			}
	|	cmd_prefix redirect			
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = (WORD_DESC *)NULL;
			  temp_element->redirect = $2;
			  temp_element->next = (struct element *)NULL;
			  $1->next = temp_element;
			  TRACE("reduced cmd_prefix to cmd_prefix redirect");
			  $$ = $1;
			}
	;

cmd_name:	WORD
			{
			  TRACE ("reduced WORD to cmd_name");
			  $$ = $1;
			}
	;
	
cmd_word:	WORD
			{
			  TRACE ("reduced WORD to cmd_word");
			  $$ = $1;
			}
	;

cmd_suffix:	WORD
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = $1;
			  temp_element->redirect = (REDIRECT *)NULL;
			  temp_element->next = (struct element *)NULL;
			  TRACE("reduced cmd_suffix to WORD");
			  $$ = temp_element;
			}
	|	cmd_suffix WORD
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = $2;
			  temp_element->redirect = (REDIRECT *)NULL;
			  temp_element->next = (struct element *)NULL;
			  TRACE("reduced cmd_suffix to cmd_suffix WORD");
			  $1->next = temp_element;
			  $$ = $1;
			}
	|	redirect
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = (WORD_DESC *)NULL;
			  temp_element->redirect = $1;
			  temp_element->next = (struct element *)NULL;
			  TRACE("reduced cmd_sufffix to redirect");
			  $$ = temp_element;
			}
	|	cmd_suffix redirect			
			{
			  struct element *temp_element;
			  temp_element = (ELEMENT *)xmalloc (sizeof (ELEMENT));
			  temp_element->word = (WORD_DESC *)NULL;
			  temp_element->redirect = $2;
			  temp_element->next = (struct element *)NULL;
			  $1->next = temp_element;
			  TRACE("reduced cmd_suffix to cmd_suffix redirect");
			  $$ = $1;
			}
	;

simple_command:	cmd_prefix cmd_word cmd_suffix
			{ $$ = new_make_simple_command ($2, $1, $3); }
	|	cmd_prefix cmd_word
			{ $$ = new_make_simple_command ($2, $1, (ELEMENT *)NULL); }
	|	cmd_prefix
			{ $$ = new_make_simple_command ((WORD_DESC *)NULL, $1, (ELEMENT *)NULL); }
	|	cmd_name cmd_suffix
			{ $$ = new_make_simple_command ($1, (ELEMENT *)NULL, $2); }
	|	cmd_name
			{
			  TRACE("reduced cmd_name to simple_command");
			  $$ = new_make_simple_command ($1, (ELEMENT *)NULL, (ELEMENT *)NULL);
			}
	;

redirect_list:	redirect
			{
			  $$ = $1;
			}
	|	redirect_list redirect
			{ 
			  register REDIRECT *t = $1;

			  while (t->next)
			    t = t->next;
			  t->next = $2; 
			  $$ = $1;
			}
	;

redirect:	'>' WORD
			{ $$ = make_redirection ( 1, r_output_direction, $2); }
	|	'<' WORD
			{ $$ = make_redirection ( 0, r_input_direction, $2); }
	|	NUMBER '>' WORD
			{ $$ = make_redirection ($1, r_output_direction, $3); }
	|	NUMBER '<' WORD
			{ $$ = make_redirection ($1, r_input_direction, $3); }
	|	GREATER_GREATER WORD
			{ $$ = make_redirection ( 1, r_appending_to, $2); }
	|	NUMBER GREATER_GREATER WORD
			{ $$ = make_redirection ($1, r_appending_to, $3); }

	|	LESS_AND NUMBER
			{ $$ = make_redirection ( 0, r_duplicating_input, $2); }
	|	NUMBER LESS_AND NUMBER
			{ $$ = make_redirection ($1, r_duplicating_input, $3); }
	|	GREATER_AND NUMBER
			{ $$ = make_redirection ( 1, r_duplicating_output, $2); }
	|	NUMBER GREATER_AND NUMBER
			{ $$ = make_redirection ($1, r_duplicating_output, $3); }
	|	LESS_AND WORD
			{ $$ = make_redirection ( 0, r_duplicating_input_word, $2); }
	|	NUMBER LESS_AND WORD
			{ $$ = make_redirection ($1, r_duplicating_input_word, $3); }
	|	GREATER_AND WORD
			{ $$ = make_redirection ( 1, r_duplicating_output_word, $2); }
	|	NUMBER GREATER_AND WORD
			{ $$ = make_redirection ($1, r_duplicating_output_word, $3); }
	|	GREATER_AND '-'
			{ $$ = make_redirection ( 1, r_close_this, 0); }
	|	NUMBER GREATER_AND '-'
			{ $$ = make_redirection ($1, r_close_this, 0); }
	|	LESS_AND '-'
			{ $$ = make_redirection ( 0, r_close_this, 0); }
	|	NUMBER LESS_AND '-'
			{ $$ = make_redirection ($1, r_close_this, 0); }
	|	AND_GREATER WORD
			{ $$ = make_redirection ( 1, r_err_and_out, $2); }
	|	NUMBER LESS_GREATER WORD
			{ $$ = make_redirection ( $1, r_input_output, $3); }
	|	LESS_GREATER WORD
			{
			  REDIRECT *t1, *t2;
			  extern WORD_DESC *copy_word ();

			  t1 = make_redirection ( 0, r_input_direction, $2);
			  t2 = make_redirection ( 1, r_output_direction, copy_word ($2));
			  t1->next = t2;
			  $$ = t1;
			}			  
	|	GREATER_BAR WORD
			{ $$ = make_redirection ( 1, r_output_force, $2); }
	|	NUMBER GREATER_BAR WORD
			{ $$ = make_redirection ( $1, r_output_force, $3); }
	|	LESS_LESS WORD
			{
			  $$ = make_redirection ( 0, r_reading_until, $2);
			  redirection_needing_here_doc = $$;
			  need_here_doc = 1;
			}
	|	NUMBER LESS_LESS WORD
			{
			  $$ = make_redirection ($1, r_reading_until, $3);
			  redirection_needing_here_doc = $$;
			  need_here_doc = 1;
			}
	|	LESS_LESS_MINUS WORD
			{
			  $$ = make_redirection ( 0, r_deblank_reading_until, $2);
			  redirection_needing_here_doc = $$;
			  need_here_doc = 1;
			}
	|	NUMBER LESS_LESS_MINUS WORD
			{
			  $$ = make_redirection ($1, r_deblank_reading_until, $3);
			  redirection_needing_here_doc = $$;
			  need_here_doc = 1;
			}
	;

newline_list:	NEWLINE
			{
			  TRACE("reduced NEWLINE to newline_list");
			  $$  = NEWLINE; /* XXX -- broken */
			}
	|	newline_list NEWLINE
			{
			  TRACE("reduced newline_list newline to newline_list");
			  $$ = NEWLINE; /* XXX -- broken */
			}
	;

linebreak:	newline_list
			{
			  TRACE("reduced newline_list to linebreak");
			  $$ = NEWLINE; /* XXX -- broken */
			}
	|
			{
			  TRACE("reduced empty to linebreak");
			  $$ = NEWLINE; /* XXX -- broken */
			}
	;

separator_op:	'&'
			{
			  TRACE("reducing '&' to separator_op");
			  $$ = '&'; /* XXX -- broken */
			}
	|	';'
			{
			  TRACE("reducing ';' to separator_op");
			  $$ = ';'; /* XXX -- broken */
			}
	;

separator:	separator_op linebreak
			{
			  TRACE("reduced separator_op linebreak to separator");
			  $$ = $1; /* XXX -- broken */
			}
	|	newline_list
			{
			  TRACE("reduced newline_list to separator");
			  $$ = ';'; /* XXX -- broken */
			}
	;

sequential_sep:	';' linebreak
			{
			  TRACE("reduced ';' linebreak to sequential_sep");
			  $$ = ';'; /* XXX -- broken */
			}
	|	newline_list
			{
			  TRACE("reduced newline_list to sequential_sep: ';'");
			  $$ = ';'; /* XXX -- broken */
			}
	;

%%

/* Initial size to allocate for tokens, and the
   amount to grow them by. */
#define TOKEN_DEFAULT_GROW_SIZE 512

/* The token currently being read. */
int current_token = 0;

/* The last read token, or NULL.  read_token () uses this for context
   checking. */
int last_read_token = 0;

/* The token read prior to last_read_token. */
int token_before_that = 0;

/* Global var is non-zero when end of file has been reached. */
int EOF_Reached = 0;

/* yy_getc () returns the next available character from input or EOF.
   yy_ungetc (c) makes `c' the next character to read.
   init_yy_io (get, unget), makes the function `get' the installed function
   for getting the next character, and makes `unget' the installed function
   for un-getting a character. */
return_EOF ()			/* does nothing good. */
{
  return (EOF);
}

/* Variables containing the current get and unget functions. */

/* Some stream `types'. */
#define st_stream 0
#define st_string 1

Function *get_yy_char = return_EOF;
Function *unget_yy_char = return_EOF;
int yy_input_type = st_stream;
FILE *yy_input_dev = (FILE *)NULL;

/* The current stream name.  In the case of a file, this is a filename. */
char *stream_name = (char *)NULL;

/* Function to set get_yy_char and unget_yy_char. */
init_yy_io (get_function, unget_function, type, location)
     Function *get_function, *unget_function;
     int type;
     FILE *location;
{
  get_yy_char = get_function;
  unget_yy_char = unget_function;
  yy_input_type = type;
  yy_input_dev = location;
}

/* Call this to get the next character of input. */
yy_getc ()
{
  return (*get_yy_char) ();
}

/* Call this to unget C.  That is, to make C the next character
   to be read. */
yy_ungetc (c)
{
  return (*unget_yy_char) (c);
}

with_input_from_stdin ()
{
  with_input_from_stream (stdin, "stdin");
}

/* **************************************************************** */
/*								    */
/*   Let input come from STRING.  STRING is zero terminated.	    */
/*								    */
/* **************************************************************** */

int
yy_string_get ()
{
  /* If the string doesn't exist, or is empty, EOF found. */
  if (!(char *)yy_input_dev || !*(char *)yy_input_dev)
    return (EOF);
  else
    {
      register char *temp = (char *)yy_input_dev;
      int c = *temp++;
      yy_input_dev = (FILE *)temp;
      return (c);
    }
}

int
yy_string_unget (c)
     int c;
{
  register char *temp = (char *)yy_input_dev;
  *(--temp) = c;
  yy_input_dev = (FILE *)temp;
  return (c);
}

with_input_from_string (string, name)
     char *string;
     char *name;
{
  init_yy_io (yy_string_get, yy_string_unget, st_string, (FILE *)string);
  stream_name = savestring (name);
}

/* **************************************************************** */
/*								    */
/*		     Let input come from STREAM.		    */
/*								    */
/* **************************************************************** */

int
yy_stream_get ()
{
  if (yy_input_dev)
    return (getc (yy_input_dev));
  else
    return (EOF);
}

int
yy_stream_unget (c)
     int c;
{
  return (ungetc (c, yy_input_dev));
}

with_input_from_stream (stream, name)
     FILE *stream;
     char *name;
{
  init_yy_io (yy_stream_get, yy_stream_unget, st_stream, stream);
  stream_name = savestring (name);
}

typedef struct stream_saver {
  struct stream_saver *next;
  Function *getter, *putter;
  int type, line;
  char *location, *name;
} STREAM_SAVER;

/* The globally known line number. */
int line_number = 0;

STREAM_SAVER *stream_list = (STREAM_SAVER *)NULL;

push_stream ()
{
  STREAM_SAVER *temp = (STREAM_SAVER *)xmalloc (sizeof (STREAM_SAVER));
  temp->type = yy_input_type;
  temp->location = (char *)yy_input_dev;
  temp->getter = get_yy_char;
  temp->putter = unget_yy_char;
  temp->line = line_number;
  temp->name = stream_name; stream_name = (char *)NULL;
  temp->next = stream_list;
  stream_list = temp;
  EOF_Reached = line_number = 0;
}

pop_stream ()
{
  if (!stream_list)
    {
      EOF_Reached = 1;
    }
  else
    {
      STREAM_SAVER *temp = stream_list;
    
      EOF_Reached = 0;
      stream_list = stream_list->next;

      if (stream_name)
	free (stream_name);
      stream_name = temp->name;

      init_yy_io (temp->getter, temp->putter, temp->type, (FILE *)temp->location);
      line_number = temp->line;
      free (temp);
    }
}
static int in_case_pattern_list = 0;


/* Return a line of text, taken from wherever yylex () reads input.
   If there is no more input, then we return NULL. */
char *
read_a_line ()
{
  char *line_buffer = (char *)NULL;
  int indx = 0, buffer_size = 0;
  int c;

  while (1)
    {
      c = yy_getc ();

      if (c == 0)
	continue;

      /* If there is no more input, then we return NULL. */
      if (c == EOF)
	{
	  c = '\n';
	  if (!line_buffer)
	    return ((char *)NULL);
	}

      /* `+2' in case the final (200'th) character in the buffer is a newline;
	 otherwise the code below that NULL-terminates it will write over the
	 201st slot and kill the range checking in free(). */
      if (indx + 2 > buffer_size)
	if (!buffer_size)
	  line_buffer = (char *)xmalloc (buffer_size = 200);
	else
	  line_buffer = (char *)xrealloc (line_buffer, buffer_size += 200);

      line_buffer[indx++] = c;
      if (c == '\n')
	{
	  line_buffer[indx] = '\0';
	  return (line_buffer);
	}
    }
}

/* Return a line as in read_a_line (), but insure that the prompt is
   the secondary prompt. */
char *
read_secondary_line ()
{
  prompt_string_pointer = &ps2_prompt;
  prompt_again ();
  return (read_a_line ());
}


/* **************************************************************** */
/*								    */
/*				YYLEX ()			    */
/*								    */
/* **************************************************************** */

/* Reserved words.  These are only recognized as the first word of a
   command.  TOKEN_WORD_ALIST. */
STRING_INT_ALIST word_token_alist[] = {
  {"if", IF},
  {"then", THEN},
  {"else", ELSE},
  {"elif", ELIF},
  {"fi", FI},
  {"case", CASE},
  {"esac", ESAC},
  {"for", FOR},
  {"while", WHILE},
  {"until", UNTIL},
  {"do", DO},
  {"done", DONE},
  {"in", IN},
  {"function", FUNCTION},
  {"{", '{'},
  {"}", '}'},
  {"!", BANG},
  {(char *)NULL, 0}
};

/* Where shell input comes from.  History expansion is performed on each
   line when the shell is interactive. */
char *shell_input_line = (char *)NULL;
int shell_input_line_index = 0;
int shell_input_line_size = 0;	/* Amount allocated for shell_input_line. */
int shell_input_line_len = 0;	/* strlen (shell_input_line) */

/* Either zero, or EOF. */
int shell_input_line_terminator = 0;

/* Return the next shell input character.  This always reads characters
   from shell_input_line; when that line is exhausted, it is time to
   read the next line. */
int
shell_getc (remove_quoted_newline)
     int remove_quoted_newline;
{
  int c;

  QUIT;

  if (!shell_input_line || !shell_input_line[shell_input_line_index])
    {
      register int i, l;
      char *pre_process_line (), *expansions;

      restart_read_next_line:

      line_number++;

    restart_read:

      i = 0;
      shell_input_line_terminator = 0;

      clearerr (stdin);
      while (c = yy_getc ())
	{
	  if (i + 2 > shell_input_line_size)
	    shell_input_line = (char *)
	      xrealloc (shell_input_line, shell_input_line_size += 256);

	  if (c == EOF)
	    {
	      clearerr (stdin);

	      if (!i)
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

      if (!shell_input_line || !shell_input_line[0])
	goto after_pre_process;

      if (interactive)
	{
	  expansions = pre_process_line (shell_input_line, 1, 1);

	  free (shell_input_line);
	  shell_input_line = expansions;
	  shell_input_line_len = shell_input_line ?
				 strlen (shell_input_line) :
				 0;
	  /* We have to force the xrealloc below because we don't know the
	     true allocated size of shell_input_line anymore. */
	  shell_input_line_size = shell_input_line_len;
	}

  after_pre_process:
#if 0
      if (shell_input_line)
	{
	  fprintf (stderr, "%s\n", shell_input_line);
	}
      else
	{
	  shell_input_line_size = 0;
	  prompt_string_pointer = &current_prompt_string;
	  prompt_again ();
	  goto restart_read;
	}
#endif

      /* Add the newline to the end of this string, iff the string does
	 not already end in an EOF character.  */
      if (shell_input_line_terminator != EOF)
	{
	  l = shell_input_line_len;	/* was a call to strlen */

	  if (l + 3 > shell_input_line_size)
	    shell_input_line = (char *)xrealloc (shell_input_line,
					1 + (shell_input_line_size += 2));

	  strcpy (shell_input_line + l, "\n");
	}
    }
  
  c = shell_input_line[shell_input_line_index];

  if (c)
    shell_input_line_index++;

  if (c == '\\' && remove_quoted_newline &&
      shell_input_line[shell_input_line_index] == '\n')
    {
	prompt_again ();
	goto restart_read_next_line;
    }

  if (!c && shell_input_line_terminator == EOF)
    {
      if (shell_input_line_index != 0)
	return (NEWLINE);
      else
	return (EOF);
    }

  return (c);
}

/* Put C back into the input for the shell. */
shell_ungetc (c)
     int c;
{
  if (shell_input_line && shell_input_line_index)
    shell_input_line[--shell_input_line_index] = c;
}

/* Discard input until CHARACTER is seen. */
discard_until (character)
     int character;
{
  int c;
  while ((c = shell_getc (0)) != EOF && c != character)
    ;
  if (c != EOF )
    shell_ungetc (c);
}

#if defined (HISTORY_REEDITING)
/* Tell readline () that we have some text for it to edit. */
re_edit (text)
     char *text;
{
#if defined (READLINE)
  if (strcmp (stream_name, "readline stdin") == 0)
    bash_re_edit (text);
#endif /* READLINE */
}
#endif /* HISTORY_REEDITING */

/* Non-zero means do no history expansion on this line, regardless
   of what history_expansion says. */
int history_expansion_inhibited = 0;

/* Do pre-processing on LINE.  If PRINT_CHANGES is non-zero, then
   print the results of expanding the line if there were any changes.
   If there is an error, return NULL, otherwise the expanded line is
   returned.  If ADDIT is non-zero the line is added to the history
   list after history expansion.  ADDIT is just a suggestion;
   REMEMBER_ON_HISTORY can veto, and does.
   Right now this does history expansion. */
char *
pre_process_line (line, print_changes, addit)
     char *line;
     int print_changes, addit;
{
  char *return_value;
  return_value = savestring (line);

  return (return_value);
}


/* Place to remember the token.  We try to keep the buffer
   at a reasonable size, but it can grow. */
char *token = (char *)NULL;

/* Current size of the token buffer. */
int token_buffer_size = 0;

/* Command to read_token () explaining what we want it to do. */
#define READ 0
#define RESET 1
#define prompt_is_ps1 \
      (!prompt_string_pointer || prompt_string_pointer == &ps1_prompt)

/* Function for yyparse to call.  yylex keeps track of
   the last two tokens read, and calls read_token.  */

yylex ()
{
  if (interactive && (!current_token || current_token == NEWLINE))
    {
      prompt_again ();
    }

  token_before_that = last_read_token;
  last_read_token = current_token;
  current_token = read_token (READ);
  return (current_token);
}

/* Called from shell.c when Control-C is typed at top level.  Or
   by the error rule at top level. */
reset_parser ()
{
  read_token (RESET);
}
  
/* When non-zero, we have read the required tokens
   which allow ESAC to be the next one read. */
static int allow_esac_as_next = 0;

/* When non-zero, accept single '{' as a token itself. */
static int allow_open_brace = 0;

/* DELIMITER is the value of the delimiter that is currently
   enclosing, or zero for none. */
static int delimiter = 0;
static int old_delimiter = 0;

/* When non-zero, an open-brace used to create a group is awaiting a close
   brace partner. */
static int open_brace_awaiting_satisfaction = 0;

/* If non-zero, it is the token that we want read_token to return regardless
   of what text is (or isn't) present to be read.  read_token resets this. */
int token_to_read = 0;

/* Read the next token.  Command can be READ (normal operation) or 
   RESET (to normalize state). */
read_token (command)
     int command;
{
  extern int interactive_shell;	/* Whether the current shell is interactive. */
  int character;		/* Current character. */
  int peek_char;		/* Temporary look-ahead character. */
  int result;			/* The thing to return. */
  WORD_DESC *the_word;		/* The value for YYLVAL when a WORD is read. */

  if (token_buffer_size < TOKEN_DEFAULT_GROW_SIZE)
    {
      if (token)
	free (token);
      token = (char *)xmalloc (token_buffer_size = TOKEN_DEFAULT_GROW_SIZE);
    }

  if (command == RESET)
    {
      delimiter = old_delimiter = 0;
      open_brace_awaiting_satisfaction = 0;
      in_case_pattern_list = 0;

      if (shell_input_line)
	{
	  free (shell_input_line);
	  shell_input_line = (char *)NULL;
	  shell_input_line_size = shell_input_line_index = 0;
	}
      last_read_token = NEWLINE;
      token_to_read = NEWLINE;
      return (NEWLINE);
    }

  if (token_to_read)
    {
      int rt = token_to_read;
      token_to_read = 0;
      return (rt);
    }

  /* Read a single word from input.  Start by skipping blanks. */
  while ((character = shell_getc (1)) != EOF && whitespace (character));

  if (character == EOF)
    return (yacc_EOF);

  if (character == '#' && !interactive)
    {
      /* A comment.  Discard until EOL or EOF, and then return a newline. */
      discard_until ('\n');
      shell_getc (0);

      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	make_here_document (redirection_needing_here_doc);
      need_here_doc = 0;

      return (NEWLINE);
    }

  if (character == '\n')
    {
      /* If we're about to return an unquoted newline, we can go and collect
	 the text of any pending here document. */
      if (need_here_doc)
	make_here_document (redirection_needing_here_doc);
      need_here_doc = 0;

      TRACE ("read_token: returning NEWLINE");
      return (NEWLINE);
    }

  if (member (character, "()<>;&|"))
    {
      /* Please note that the shell does not allow whitespace to
	 appear in between tokens which are character pairs, such as
	 "<<" or ">>".  I believe this is the correct behaviour. */
      if (character == (peek_char = shell_getc (1)))
	{
	  switch (character)
	    {
	      /* If '<' then we could be at "<<" or at "<<-".  We have to
		 look ahead one more character. */
	    case '<':
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
	      in_case_pattern_list = 1;
	      return (SEMI_SEMI);

	    case '&':
	      return (AND_AND);

	    case '|':
	      return (OR_OR);
	    }
	}
      else
	{
	  if (peek_char == '&')
	    {
	      switch (character)
		{
		case '<': return (LESS_AND);
		case '>': return (GREATER_AND);
		}
	    }
	  if (character == '<' && peek_char == '>')
	    return (LESS_GREATER);
	  if (character == '>' && peek_char == '|')
	    return (GREATER_BAR);
	  if (peek_char == '>' && character == '&')
	    return (AND_GREATER);
	}
      shell_ungetc (peek_char);

      /* If we look like we are reading the start of a function
	 definition, then let the reader know about it so that
	 we will do the right thing with `{'. */
      if (character == ')' &&
	  last_read_token == '(' && token_before_that == WORD)
	{
	  allow_open_brace = 1;
	}

      if (in_case_pattern_list && (character == ')'))
	in_case_pattern_list = 0;
#if defined (PROCESS_SUBSTITUTION)
      /* If we are performing process substitution, let <( and >( by
	 and make a word. */
      if (!((character == '>' || character == '<') && peek_char == '('))
#endif /* PROCESS_SUBSTITUTION */

      TRACE("read_token: returning '%c'", character);
      return (character);
    }

  /* Hack <&- (close stdin) case. */
  if (character == '-')
    {
      switch (last_read_token)
	{
	case LESS_AND:
	case GREATER_AND:
	  return (character);
	}
    }
  
  /* Okay, if we got this far, we have to read a word.  Read one,
     and then check it against the known ones. */
  {
    /* Index into the token that we are building. */
    int token_index = 0;

    /* ALL_DIGITS becomes zero when we see a non-digit. */
    int all_digits = digit (character);

    /* DOLLAR_PRESENT becomes non-zero if we see a `$'. */
    int dollar_present = 0;

    /* QUOTED becomes non-zero if we see one of ("), ('), (`), or (\). */
    int quoted = 0;

    /* Non-zero means to ignore the value of the next character, and just
       to add it no matter what. */
    int pass_next_character = 0;

    /* Non-zero means parsing a dollar-paren construct.  It is the count of
       un-quoted closes we need to see. */
    int dollar_paren_level = 0;

    /* Non-zero means parsing a dollar-bracket construct ($[...]).  It is
       the count of un-quoted `]' characters we need to see. */
    int dollar_bracket_level = 0;

    /* Another level variable.  This one is for dollar_parens inside of
       double-quotes. */
    int delimited_paren_level = 0;

    for (;;)
      {
	if (character == EOF)
	  goto got_token;

	if (pass_next_character)
	  {
	    pass_next_character = 0;
	    goto got_character;
	  }

      if (delimiter && character == '\\' && delimiter != '\'')
	{
	  peek_char = shell_getc (0);
	  if (peek_char != '\\')
	    shell_ungetc (peek_char);
	  else
	    {
	      token[token_index++] = character;
	      goto got_character;
	    }
	}

	/* Handle backslashes.  Quote lots of things when not inside of
	   double-quotes, quote some things inside of double-quotes. */
	   
	if (character == '\\' && delimiter != '\'')
	  {
	    peek_char = shell_getc (0);

	    /* Backslash-newline is ignored in all cases excepting
	       when quoted with single quotes. */
	    if (peek_char == '\n')
	      {
		character = '\n';
		goto next_character;
	      }
	    else
	      {
		shell_ungetc (peek_char);

		/* If the next character is to be quoted, do it now. */
		if (!delimiter || delimiter == '`' ||
		    ((delimiter == '"' ) &&
		     (member (peek_char, slashify_in_quotes))))
		  {
		    pass_next_character++;
		    quoted = 1;
		    goto got_character;
		  }
	      }
	  }

	/* This is a hack, in its present form.  If a backquote substitution
	   appears within double quotes, everything within the backquotes
	   should be read as part of a single word.  Jesus.  Now I see why
	   Korn introduced the $() form. */
	if (delimiter && delimiter == '"' && character == '`')
	  {
	    old_delimiter = delimiter;
	    delimiter = character;
	    goto got_character;
	  }

	if (delimiter)
	  {
	    if (character == delimiter)
	      {
		if (delimited_paren_level)
		  {
#if defined (NOTDEF)
		    report_error ("Expected ')' before %c", character);
		    return (NEWLINE);
#else
		    goto got_character;
#endif /* NOTDEF */
		  }

		delimiter = 0;

		if (old_delimiter == '"' && character == '`')
		  {
		    delimiter = old_delimiter;
		    old_delimiter = 0;
		  }

		goto got_character;
	      }
	  }

	if (!delimiter || delimiter == '`' || delimiter == '"')
	  {
#if defined (PROCESS_SUBSTITUTION)
	    if (character == '$' || character == '<' || character == '>')
#else
	    if (character == '$')
#endif /* PROCESS_SUBSTITUTION */
	      {
		peek_char = shell_getc (1);
		shell_ungetc (peek_char);
		if (peek_char == '(')
		  {
		    if (!delimiter)
		      dollar_paren_level++;
		    else
		      delimited_paren_level++;

		    pass_next_character++;
		    goto got_character;
		  }
		else if (peek_char == '[')
		  {
		    if (!delimiter)
		      dollar_bracket_level++;

		    pass_next_character++;
		    goto got_character;
		  }
	      }

	    /* If we are parsing a $() or $[] construct, we need to balance
	       parens and brackets inside the construct.  This whole function
	       could use a rewrite. */
	    if (character == '(')
	      {
		if (delimiter && delimited_paren_level)
		  delimited_paren_level++;

		if (!delimiter && dollar_paren_level)
		  dollar_paren_level++;
	      }

	    if (character == '[')
	      {
		if (!delimiter && dollar_bracket_level)
		  dollar_bracket_level++;
	      }

	    /* This code needs to take into account whether we are inside a
	       case statement pattern list, and whether this paren is supposed
	       to terminate it (hey, it could happen).  It's not as simple
	       as just using in_case_pattern_list, because we're not parsing
	       anything while we're reading a $( ) construct.  Maybe we
	       should move that whole mess into the yacc parser. */
	    if (character == ')')
	      {
		if (delimiter && delimited_paren_level)
		  delimited_paren_level--;

		if (!delimiter && dollar_paren_level)
		  {
		    dollar_paren_level--;
		    goto got_character;
		  }
	      }

	    if (character == ']')
	      {
		if (!delimiter && dollar_bracket_level)
		  {
		    dollar_bracket_level--;
		    goto got_character;
		  }
	      }
	  }

	if (!dollar_paren_level && !dollar_bracket_level && !delimiter &&
	    member (character, " \t\n;&()|<>"))
	  {
	    shell_ungetc (character);
	    goto got_token;
	  }
    
	if (!delimiter)
	  {
	    if (character == '"' || character == '`' || character == '\'')
	      {
		quoted = 1;
		delimiter = character;
		goto got_character;
	      }
	  }

	if (all_digits) all_digits = digit (character);
	if (character == '$') dollar_present = 1;

      got_character:

	token[token_index++] = character;

	if (token_index == (token_buffer_size - 1))
	  token = (char *)xrealloc (token, (token_buffer_size
					    += TOKEN_DEFAULT_GROW_SIZE));
	{
	  char *decode_prompt_string ();

	next_character:
	  if (character == '\n' && interactive && yy_input_type != st_string)
	    prompt_again ();
	}
	/* We want to remove quoted newlines (that is, a \<newline> pair)
	   unless we are within single quotes or pass_next_character is
	   set (the shell equivalent of literal-next). */
	character = shell_getc ((delimiter != '\'') && (!pass_next_character));
      }

  got_token:

    token[token_index] = '\0';
	
    if ((delimiter || dollar_paren_level || dollar_bracket_level) &&
	character == EOF)
      {
	if (dollar_paren_level && !delimiter)
	  delimiter = ')';
	else if (dollar_bracket_level && !delimiter)
	  delimiter = ']';

	report_error ("Unexpected EOF.  Looking for `%c'.", delimiter);
	return (-1);
      }

    if (all_digits)
      {
	/* Check to see what thing we should return.  If the last_read_token
	   is a `<', or a `&', or the character which ended this token is
	   a '>' or '<', then, and ONLY then, is this input token a NUMBER.
	   Otherwise, it is just a word, and should be returned as such. */

	if ((character == '<' || character == '>') ||
	    (last_read_token == LESS_AND ||
	     last_read_token == GREATER_AND))
	  {
	    yylval.number = atoi (token); /* was sscanf (token, "%d", &(yylval.number)); */
	    return (NUMBER);
	  }
      }

    /* Handle special case.  IN is recognized if the last token
       was WORD and the token before that was FOR or CASE. */
    if ((last_read_token == WORD) &&
	((token_before_that == FOR) || (token_before_that == CASE)) &&
	(STREQ (token, "in")))
      {
	if (token_before_that == CASE)
	  {
	    in_case_pattern_list = 1;
	    allow_esac_as_next++;
	  }
	TRACE("returning %s as IN", token);
	return (IN);
      }

    /* Ditto for DO in the FOR case. */
    if ((last_read_token == WORD) && (token_before_that == FOR) &&
	(STREQ (token, "do")))
      return (DO);

    /* Ditto for ESAC in the CASE case. 
       Specifically, this handles "case word in esac", which is a legal
       construct, certainly because someone will pass an empty arg to the
       case construct, and we don't want it to barf.  Of course, we should
       insist that the case construct has at least one pattern in it, but
       the designers disagree. */
    if (allow_esac_as_next)
      {
	allow_esac_as_next--;
	if (STREQ (token, "esac"))
	  {
	    in_case_pattern_list = 0;
	    return (ESAC);
	  }
      }

    /* Ditto for `{' in the FUNCTION case. */
    if (allow_open_brace)
      {
	allow_open_brace = 0;
	if (STREQ (token, "{"))
	  {
	    open_brace_awaiting_satisfaction++;
	    return ('{');
	  }
      }

    /* Check to see if it is a reserved word.  */
    if (!dollar_present && !quoted &&
	reserved_word_acceptable (last_read_token))
      {
	int i;
	for (i = 0; word_token_alist[i].word != (char *)NULL; i++)
	  if (STREQ (token, word_token_alist[i].word))
	    {
	      if (in_case_pattern_list && (word_token_alist[i].token != ESAC))
		break;

	      if (word_token_alist[i].token == ESAC)
		in_case_pattern_list = 0;

	      if (word_token_alist[i].token == '{')
		open_brace_awaiting_satisfaction++;

	      TRACE("returning %s as %d", token, word_token_alist[i].token);
	      return (word_token_alist[i].token);
	    }
      }

    /* What if we are attempting to satisfy an open-brace grouper? */
    if (open_brace_awaiting_satisfaction && strcmp (token, "}") == 0)
      {
	open_brace_awaiting_satisfaction--;
	return ('}');
      }

    the_word = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
    the_word->word = (char *)xmalloc (1 + strlen (token));
    strcpy (the_word->word, token);
    the_word->dollar_present = dollar_present;
    the_word->quoted = quoted;
    the_word->assignment = assignment (token);

    yylval.word = the_word;

    result = WORD;

    if ((last_read_token == ASSIGNMENT_WORD || command_word_acceptable (last_read_token)) &&
        the_word->assignment &&
        token[0] != '=' &&
        legal_assignment (token))
      result = ASSIGNMENT_WORD;

    TRACE("read_token: returning %s as %s", token,
    		(result == WORD) ? "WORD" :
    				   result == NAME ? "NAME" : "ASSIGNMENT_WORD");
    if (last_read_token == FUNCTION)
      allow_open_brace = 1;
  }
  return (result);
}

legal_assignment (s)
char *s;
{
  extern char *index ();
  char *t = index (s, '=');
  int result;

  *t = '\0';
  result = legal_identifier (s);
  *t = '=';
  return (result);
}

/* Return 1 if this token is a legal shell `identifier'; that is, it consists
   solely of letters, digits, and underscores, and does not begin with a
   digit. */
legal_identifier (name)
     char *name;
{
  register char *s;

  if (!name || !*name)
    return (0);

  if (digit (*name))
    return (0);

  for (s = name; s && *s; s++)
    {
      if (!isletter (*s) && !digit (*s) && (*s != '_'))
        return (0);
    }
  return (1);
}

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
reserved_word_acceptable (token)
     int token;
{
  if (member (token, "\n;()|&{") ||
      token == AND_AND ||
      token == BANG ||
      token == DO ||
      token == ELIF ||
      token == ELSE ||
      token == IF ||
      token == FI ||
      token == ESAC ||
      token == OR_OR ||
      token == SEMI_SEMI ||
      token == THEN ||
      token == UNTIL ||
      token == WHILE ||
      token == 0)
    return (1);
  else
    return (0);
}

/* Return 1 if TOKEN is a token that after being read would allow
   a reserved word to be seen, else 0. */
static int
command_word_acceptable (token)
     int token;
{
  if (member (token, "\n;()|&{") ||
      token == AND_AND ||
      token == BANG ||
      token == DO ||
      token == ELIF ||
      token == ELSE ||
      token == IF ||
      token == OR_OR ||
      token == THEN ||
      token == UNTIL ||
      token == WHILE ||
      token == 0)
    return (1);
  else
    return (0);
}

reset_readline_prompt ()
{
}

/* Add a line to the history list.
   The variable COMMAND_ORIENTED_HISTORY controls the style of history
   remembering;  when non-zero, and LINE is not the first line of a
   complete parser construct, append LINE to the last history line instead
   of adding it as a new line. */
bash_add_history (line)
     char *line;
{
}

/* Issue a prompt, or prepare to issue a prompt when the next character
   is read. */
prompt_again ()
{
  char *temp_prompt, *decode_prompt_string ();

  ps1_prompt = "posix$ ";
  ps2_prompt = "> ";

  if (!prompt_string_pointer)
    prompt_string_pointer = &ps1_prompt;

  current_prompt_string = *prompt_string_pointer;
  prompt_string_pointer = &ps2_prompt;

  fprintf (stderr, "%s", current_prompt_string);
  fflush (stderr);
}

char *
decode_prompt_string (string)
     char *string;
{
  char *result;
  result = savestring (string);

  return (result);
}

/* Report a syntax error, and restart the parser.  Call here for fatal
   errors. */
yyerror ()
{
  report_syntax_error ((char *)NULL);
  reset_parser ();
}

/* Report a syntax error with line numbers, etc.
   Call here for recoverable errors.  If you have a message to print,
   then place it in MESSAGE, otherwise pass NULL and this will figure
   out an appropriate message for you. */
report_syntax_error (message)
     char *message;
{
  if (message)
    {
      if (!interactive)
	{
	  char *name = stream_name ? stream_name : "stdin";
	  report_error ("%s:%d: `%s'", name, line_number, message);
	}
      else
	report_error ("%s", message);

      return;
    }

  if (shell_input_line && *shell_input_line)
    {
      char *error_token, *t = shell_input_line;
      register int i = shell_input_line_index;
      int token_end = 0;

      if (!t[i] && i)
	i--;

      while (i && (t[i] == ' ' || t[i] == '\t' || t[i] == '\n'))
	i--;

      if (i)
	token_end = i + 1;

      while (i && !member (t[i], " \n\t;|&"))
	i--;

      while (i != token_end && member (t[i], " \n\t"))
	i++;

      if (token_end)
	{
	  error_token = (char *)alloca (1 + (token_end - i));
	  strncpy (error_token, t + i, token_end - i);
	  error_token[token_end - i] = '\0';

	  report_error ("syntax error near `%s'", error_token);
	}
      else if ((i == 0) && (token_end == 0))	/* a 1-character token */
	{
	  error_token = (char *) alloca (2);
	  strncpy(error_token, t + i, 1);
	  error_token[1] = '\0';

	  report_error ("syntax error near `%s'", error_token);
	}

      if (!interactive)
	{
	  char *temp = savestring (shell_input_line);
	  char *name = stream_name ? stream_name : "stdin";
	  int l = strlen (temp);

	  while (l && temp[l - 1] == '\n')
	    temp[--l] = '\0';

	  report_error ("%s:%d: `%s'", name, line_number, temp);
	  free (temp);
	}
    }
  else
    report_error ("Syntax error");
}

/* ??? Needed function. ??? We have to be able to discard the constructs
   created during parsing.  In the case of error, we want to return
   allocated objects to the memory pool.  In the case of no error, we want
   to throw away the information about where the allocated objects live.
   (dispose_command () will actually free the command. */
discard_parser_constructs (error_p)
     int error_p;
{
/*   if (error_p) {
     fprintf (stderr, "*");
  } */
}
   
handle_eof_input_unit ()
{
  EOF_Reached = 1;
}

TRACE(va_alist)
va_dcl
{
  va_list args;
  char *format;
  
  va_start (args);
  fprintf(stderr, "TRACE: ");
  format = va_arg (args, char *);
  vfprintf (stderr, format, args);
  fprintf (stderr, "\n");
        
  va_end (args);
}
