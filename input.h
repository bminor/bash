/* input.h -- Structures and unions used for reading input. */
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

#if !defined (_INPUT_H)
#define _INPUT_H

#include "stdc.h"

/* Function pointers can be declared as (Function *)foo. */
#if !defined (__FUNCTION_DEF)
#  define __FUNCTION_DEF
typedef int Function ();
typedef void VFunction ();
typedef char *CPFunction ();
typedef char **CPPFunction ();
#endif /* _FUNCTION_DEF */

/* Some stream `types'. */
#define st_none   0
#define st_stream 1
#define st_string 2
#define st_stdin  3

#if defined (BUFFERED_INPUT)
#define st_bstream 4

/* Possible values for b_flag. */
#define B_EOF		0x1
#define B_ERROR		0x2
#define B_UNBUFF	0x4

/* A buffered stream.  Like a FILE *, but with our own buffering and
   synchronization.  Look in input.c for the implementation. */
typedef struct BSTREAM
{
  int	b_fd;
  char	*b_buffer;		/* The buffer that holds characters read. */
  int	b_size;			/* How big the buffer is. */
  int	b_used;			/* How much of the buffer we're using, */
  int	b_flag;			/* Flag values. */
  int	b_inputp;		/* The input pointer, index into b_buffer. */
} BUFFERED_STREAM;

extern BUFFERED_STREAM **buffers;

extern BUFFERED_STREAM *fd_to_buffered_stream ();

extern int default_buffered_input;

#endif /* BUFFERED_INPUT */

typedef union {
  FILE *file;
  char *string;
#if defined (BUFFERED_INPUT)
  int buffered_fd;
#endif
} INPUT_STREAM;

typedef struct {
  int type;
  char *name;
  INPUT_STREAM location;
  Function *getter;
  Function *ungetter;
} BASH_INPUT;

extern BASH_INPUT bash_input;

/* Functions from parse.y. */
extern void initialize_bash_input __P((void));
extern void init_yy_io __P((Function *, Function *, int, char *, INPUT_STREAM));
extern void with_input_from_stdin __P((void));
extern void with_input_from_string __P((char *, char *));
extern void with_input_from_stream __P((FILE *, char *));
extern int push_stream __P((void));
extern int pop_stream __P((void));
extern char *read_secondary_line __P((int));
extern int find_reserved_word __P((char *));
extern char *decode_prompt_string __P((char *));
extern void gather_here_documents __P((void));
extern void execute_prompt_command __P((char *));

#if defined (BUFFERED_INPUT)
/* Functions from input.c. */
extern int check_bash_input __P((int));
extern int duplicate_buffered_stream __P((int, int));
extern BUFFERED_STREAM *fd_to_buffered_stream __P((int));
extern BUFFERED_STREAM *open_buffered_stream __P((char *));
extern void free_buffered_stream __P((BUFFERED_STREAM *));
extern int close_buffered_stream __P((BUFFERED_STREAM *));
extern int close_buffered_fd __P((int));
extern int sync_buffered_stream __P((int));
extern int buffered_getchar __P((void));
extern int buffered_ungetchar __P((int));
extern void with_input_from_buffered_stream __P((int, char *));
#endif /* BUFFERED_INPUT */

#endif /* _INPUT_H */
