/* command.h -- The structures used internally to represent commands, and
   the extern declarations of the functions used to create them. */

/* Copyright (C) 1993 Free Software Foundation, Inc.

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

#if !defined (_COMMAND_H)
#define _COMMAND_H

#include "stdc.h"

/* Instructions describing what kind of thing to do for a redirection. */
enum r_instruction {
  r_output_direction, r_input_direction, r_inputa_direction,
  r_appending_to, r_reading_until, r_duplicating_input,
  r_duplicating_output, r_deblank_reading_until, r_close_this,
  r_err_and_out, r_input_output, r_output_force,
  r_duplicating_input_word, r_duplicating_output_word
};

/* Command Types: */
enum command_type { cm_for, cm_case, cm_while, cm_if, cm_simple, cm_select,
		    cm_connection, cm_function_def, cm_until, cm_group };

/* A structure which represents a word. */
typedef struct word_desc {
  char *word;		/* Zero terminated string. */
  int dollar_present;	/* Non-zero means dollar sign present. */
  int quoted;		/* Non-zero means single, double, or back quote
			   or backslash is present. */
  int assignment;	/* Non-zero means that this word contains an
			   assignment. */
} WORD_DESC;

/* A linked list of words. */
typedef struct word_list {
  struct word_list *next;
  WORD_DESC *word;
} WORD_LIST;


/* **************************************************************** */
/*								    */
/*			Shell Command Structs			    */
/*								    */
/* **************************************************************** */

/* What a redirection descriptor looks like.  If FLAGS is IS_DESCRIPTOR,
   then we use REDIRECTEE.DEST, else we use the file specified. */

typedef union {
  long dest;			/* Place to redirect REDIRECTOR to, or ... */
  WORD_DESC *filename;		/* filename to redirect to. */
} REDIRECTEE;

typedef struct redirect {
  struct redirect *next;	/* Next element, or NULL. */
  int redirector;		/* Descriptor to be redirected. */
  int flags;			/* Flag value for `open'. */
  enum r_instruction  instruction; /* What to do with the information. */
  REDIRECTEE redirectee;	/* File descriptor or filename */
  char *here_doc_eof;		/* The word that appeared in <<foo. */
} REDIRECT;

/* An element used in parsing.  A single word or a single redirection.
   This is an ephemeral construct. */
typedef struct element {
  WORD_DESC *word;
  REDIRECT *redirect;
} ELEMENT;

/* Possible values for command->flags. */
#define CMD_WANT_SUBSHELL  0x01	/* User wants a subshell: ( command ) */
#define CMD_FORCE_SUBSHELL 0x02	/* Shell needs to force a subshell. */
#define CMD_INVERT_RETURN  0x04	/* Invert the exit value. */
#define CMD_IGNORE_RETURN  0x08	/* Ignore the exit value.  For set -e. */
#define CMD_NO_FUNCTIONS   0x10 /* Ignore functions during command lookup. */
#define CMD_INHIBIT_EXPANSION 0x20 /* Do not expand the command words. */
#define CMD_NO_FORK	   0x40	/* Don't fork; just call execve */

/* What a command looks like. */
typedef struct command {
  enum command_type type;	/* FOR CASE WHILE IF CONNECTION or SIMPLE. */
  int flags;			/* Flags controlling execution environment. */
  int line;			/* line number the command starts on */
  REDIRECT *redirects;		/* Special redirects for FOR CASE, etc. */
  union {
    struct for_com *For;
    struct case_com *Case;
    struct while_com *While;
    struct if_com *If;
    struct connection *Connection;
    struct simple_com *Simple;
    struct function_def *Function_def;
    struct group_com *Group;
#if defined (SELECT_COMMAND)
    struct select_com *Select;
#endif
  } value;
} COMMAND;

/* Structure used to represent the CONNECTION type. */
typedef struct connection {
  int ignore;			/* Unused; simplifies make_command (). */
  COMMAND *first;		/* Pointer to the first command. */
  COMMAND *second;		/* Pointer to the second command. */
  int connector;		/* What separates this command from others. */
} CONNECTION;

/* Structures used to represent the CASE command. */

/* Pattern/action structure for CASE_COM. */
typedef struct pattern_list {
  struct pattern_list *next;	/* Clause to try in case this one failed. */
  WORD_LIST *patterns;		/* Linked list of patterns to test. */
  COMMAND *action;		/* Thing to execute if a pattern matches. */
} PATTERN_LIST;

/* The CASE command. */
typedef struct case_com {
  int flags;			/* See description of CMD flags. */
  WORD_DESC *word;		/* The thing to test. */
  PATTERN_LIST *clauses;	/* The clauses to test against, or NULL. */
} CASE_COM;

/* FOR command. */
typedef struct for_com {
  int flags;		/* See description of CMD flags. */
  WORD_DESC *name;	/* The variable name to get mapped over. */
  WORD_LIST *map_list;	/* The things to map over.  This is never NULL. */
  COMMAND *action;	/* The action to execute.
			   During execution, NAME is bound to successive
			   members of MAP_LIST. */
} FOR_COM;

#if defined (SELECT_COMMAND)
/* KSH SELECT command. */
typedef struct select_com {
  int flags;		/* See description of CMD flags. */
  WORD_DESC *name;	/* The variable name to get mapped over. */
  WORD_LIST *map_list;	/* The things to map over.  This is never NULL. */
  COMMAND *action;	/* The action to execute.
			   During execution, NAME is bound to the member of
			   MAP_LIST chosen by the user. */
} SELECT_COM;
#endif /* SELECT_COMMAND */

/* IF command. */
typedef struct if_com {
  int flags;			/* See description of CMD flags. */
  COMMAND *test;		/* Thing to test. */
  COMMAND *true_case;		/* What to do if the test returned non-zero. */
  COMMAND *false_case;		/* What to do if the test returned zero. */
} IF_COM;

/* WHILE command. */
typedef struct while_com {
  int flags;			/* See description of CMD flags. */
  COMMAND *test;		/* Thing to test. */
  COMMAND *action;		/* Thing to do while test is non-zero. */
} WHILE_COM;

/* The "simple" command.  Just a collection of words and redirects. */
typedef struct simple_com {
  int flags;			/* See description of CMD flags. */
  WORD_LIST *words;		/* The program name, the arguments,
				   variable assignments, etc. */
  REDIRECT *redirects;		/* Redirections to perform. */
  int line;			/* line number the command starts on */
} SIMPLE_COM;

/* The "function_def" command.  This isn't really a command, but it is
   represented as such for now.  If the function def appears within 
   `(' `)' the parser tries to set the SUBSHELL bit of the command.  That
   means that FUNCTION_DEF has to be run through the executor.  Maybe this
   command should be defined in a subshell.  Who knows or cares. */
typedef struct function_def {
  int ignore;			/* See description of CMD flags. */
  WORD_DESC *name;		/* The name of the function. */
  COMMAND *command;		/* The parsed execution tree. */
} FUNCTION_DEF;

/* A command that is `grouped' allows pipes to take effect over
   the entire command structure. */
typedef struct group_com {
  int ignore;			/* See description of CMD flags. */
  COMMAND *command;
} GROUP_COM;

extern COMMAND *global_command;

/* Forward declarations of functions declared in copy_cmd.c. */

extern WORD_DESC *copy_word __P((WORD_DESC *));
extern WORD_LIST *copy_word_list __P((WORD_LIST *));
extern REDIRECT *copy_redirect __P((REDIRECT *));
extern REDIRECT *copy_redirects __P((REDIRECT *));
extern COMMAND *copy_command __P((COMMAND *));

#endif /* _COMMAND_H */
