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

extern void builtin_error __P((const char *, ...));
extern void builtin_usage ();
extern void bad_option ();

extern char **make_builtin_argv ();

extern int get_numeric_arg ();
extern void remember_args ();
extern void no_args ();
extern int no_options ();

extern int read_octal ();

extern void push_context (), pop_context ();
extern void push_dollar_vars (), pop_dollar_vars ();
extern void dispose_saved_dollar_vars ();
extern int dollar_vars_changed ();
extern void set_dollar_vars_unchanged (), set_dollar_vars_changed ();

/* Keeps track of the current working directory. */
extern char *the_current_working_directory;
extern char *get_working_directory ();
extern void set_working_directory ();

#if defined (JOB_CONTROL)
extern int get_job_spec ();
#endif

extern int display_signal_list ();

/* It's OK to declare a function as returning a Function * without
   providing a definition of what a `Function' is. */
extern struct builtin *builtin_address_internal ();
extern Function *find_shell_builtin ();
extern Function *builtin_address ();
extern Function *find_special_builtin ();

extern void initialize_shell_builtins ();

extern char *single_quote ();
extern char *double_quote ();
extern char *backslash_quote ();
extern int contains_shell_metas ();

/* Functions from set.def */
extern void initialize_shell_options ();
extern void list_minus_o_opts ();
extern int set_minus_o_option ();
extern int minus_o_option_value ();
extern void reset_shell_options ();

/* Functions from shopt.def */
extern void reset_shopt_options ();

/* Functions from type.def */
extern int describe_command ();

/* Functions from setattr.def */
extern int set_or_show_attributes ();
extern int show_var_attributes ();
extern int show_name_attributes ();
extern void set_var_attribute ();

/* Functions from pushd.def */
extern char *get_dirstack_element ();
extern void set_dirstack_element ();
extern WORD_LIST *get_directory_stack ();

/* Functions from evalstring.c */
extern int parse_and_execute ();
extern void parse_and_execute_cleanup ();

/* Functions from evalfile.c */
extern int maybe_execute_file __P((char *, int));
extern int source_file __P((char *));

#endif /* !__COMMON_H */
