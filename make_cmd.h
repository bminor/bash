/* make_cmd.h -- Declarations of functions found in make_cmd.c */

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

#if !defined (_MAKE_CMD_H_)
#define _MAKE_CMD_H_

#include "stdc.h"

extern WORD_LIST *make_word_list __P((WORD_DESC *, WORD_LIST *));
extern WORD_LIST *add_string_to_list __P((char *, WORD_LIST *));

extern WORD_DESC *make_bare_word __P((char *));
extern WORD_DESC *make_word __P((char *));
extern WORD_DESC *make_word_from_token __P((int));

extern COMMAND *make_command __P((enum command_type, SIMPLE_COM *));
extern COMMAND *command_connect __P((COMMAND *, COMMAND *, int));
extern COMMAND *make_for_command __P((WORD_DESC *, WORD_LIST *, COMMAND *));
extern COMMAND *make_group_command __P((COMMAND *));
extern COMMAND *make_case_command __P((WORD_DESC *, PATTERN_LIST *));
extern PATTERN_LIST *make_pattern_list __P((WORD_LIST *, COMMAND *));
extern COMMAND *make_if_command __P((COMMAND *, COMMAND *, COMMAND *));
extern COMMAND *make_while_command __P((COMMAND *, COMMAND *));
extern COMMAND *make_until_command __P((COMMAND *, COMMAND *));
extern COMMAND *make_bare_simple_command __P((void));
extern COMMAND *make_simple_command __P((ELEMENT, COMMAND *));
extern void make_here_document __P((REDIRECT *));
extern REDIRECT *make_redirection __P((int, enum r_instruction, REDIRECTEE));
extern COMMAND *make_function_def __P((WORD_DESC *, COMMAND *, int, int));
extern COMMAND *clean_simple_command __P((COMMAND *));

extern COMMAND *make_select_command __P((WORD_DESC *, WORD_LIST *, COMMAND *));

extern COMMAND *connect_async_list __P((COMMAND *, COMMAND *, int));

#endif /* !_MAKE_CMD_H */
