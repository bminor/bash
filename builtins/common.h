/* common.h -- extern declarations for functions defined in common.c. */

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

#if  !defined (__COMMON_H)
#  define __COMMON_H

#include "../stdc.h"

#define ISOPTION(s, c)	(s[0] == '-' && !s[2] && s[1] == c)

/* Flag values for parse_and_execute () */
#define SEVAL_NONINT	0x01
#define SEVAL_INTERACT	0x02
#define SEVAL_NOHIST	0x04

/* Functions from common.c */
extern void builtin_error __P((const char *, ...));
extern void builtin_usage __P((void));
extern void no_args __P((WORD_LIST *));
extern void bad_option __P((char *));
extern int no_options __P((WORD_LIST *));

extern char **make_builtin_argv __P((WORD_LIST *, int *));
extern void remember_args __P((WORD_LIST *, int));

extern void push_context __P((void));
extern void pop_context __P((void));
extern void push_dollar_vars __P((void));
extern void pop_dollar_vars __P((void));
extern void dispose_saved_dollar_vars __P((void));
extern int dollar_vars_changed __P((void));
extern void set_dollar_vars_unchanged __P((void));
extern void set_dollar_vars_changed __P((void));

extern int get_numeric_arg __P((WORD_LIST *, int));
extern int read_octal __P((char *));

/* Keeps track of the current working directory. */
extern char *the_current_working_directory;
extern char *get_working_directory __P((char *));
extern void set_working_directory __P((char *));

#if defined (JOB_CONTROL)
extern int get_job_spec __P((WORD_LIST *));
#endif
extern int display_signal_list __P((WORD_LIST *, int));

/* It's OK to declare a function as returning a Function * without
   providing a definition of what a `Function' is. */
extern struct builtin *builtin_address_internal __P((char *, int));
extern Function *find_shell_builtin __P((char *));
extern Function *builtin_address __P((char *));
extern Function *find_special_builtin __P((char *));
extern void initialize_shell_builtins __P((void));

extern char *single_quote __P((char *));
extern char *double_quote __P((char *));
extern char *backslash_quote __P((char *));
extern int contains_shell_metas __P((char *));

/* Functions from set.def */
extern void initialize_shell_options __P((int));
extern void list_minus_o_opts __P((int, int));
extern int set_minus_o_option __P((int, char *));
extern int minus_o_option_value __P((char *));
extern void reset_shell_options __P((void));

/* Functions from shopt.def */
extern void reset_shopt_options __P((void));

/* Functions from type.def */
extern int describe_command __P((char *, int, int));

/* Functions from setattr.def */
extern int set_or_show_attributes __P((WORD_LIST *, int, int));
extern int show_var_attributes __P((SHELL_VAR *, int, int));
extern int show_name_attributes __P((char *, int));
extern void set_var_attribute __P((char *, int, int));

/* Functions from pushd.def */
extern char *get_dirstack_from_string __P((char *));
extern char *get_dirstack_element __P((int, int));
extern void set_dirstack_element __P((int, int, char *));
extern WORD_LIST *get_directory_stack __P((void));

/* Functions from evalstring.c */
extern int parse_and_execute __P((char *, char *, int));
extern void parse_and_execute_cleanup __P((void));

/* Functions from evalfile.c */
extern int maybe_execute_file __P((char *, int));
extern int source_file __P((char *));

#endif /* !__COMMON_H */
