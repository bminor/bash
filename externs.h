/* externs.h -- extern function declarations which do not appear in their
   own header file. */

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

/* Make sure that this is included *after* config.h! */

#if !defined (__EXTERNS_H__)
#  define __EXTERNS_H__

#include "stdc.h"

/* Functions from expr.c. */
extern long evalexp __P((char *));

/* Functions from print_cmd.c. */
extern char *make_command_string __P((COMMAND *));
extern void print_command __P((COMMAND *));
extern void print_simple_command __P((SIMPLE_COM *));
extern char *named_function_string __P((char *, COMMAND *, int));

/* Functions from shell.c. */
extern int maybe_execute_file __P((char *, int));
extern char *indirection_level_string __P((void));
extern sighandler termination_unwind_protect __P((int));
extern sighandler sigint_sighandler __P((int));
extern void reset_terminating_signals __P((void));
extern char *shell_version_string __P((void));
extern void show_shell_version __P((void));

/* Functions from test.c. */
extern int group_member ();

/* Functions from braces.c. */
#if defined (BRACE_EXPANSION)
extern char **brace_expand __P((char *));
#endif

/* Functions from mailcheck.c */
extern int time_to_check_mail __P((void));
extern void reset_mail_timer __P((void));
extern void reset_mail_files __P((void));
extern void free_mail_files __P((void));
extern char *make_default_mailpath __P((void));
extern void remember_mail_dates __P((void));
extern void check_mail __P((void));

/* Miscellaneous functions not declared anywhere but used. */
extern char **glob_filename __P((char *));

#endif /* __EXTERNS_H__ */
