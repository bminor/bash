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

#define ISOPTION(s, c)	(s[0] == '-' && !s[2] && s[1] == c)

extern void builtin_error ();
extern void bad_option ();

extern int get_numeric_arg ();

extern void remember_args ();

extern void no_args ();

extern int read_octal ();

extern char *find_hashed_filename ();
extern void remove_hashed_filename ();
extern void remember_filename ();

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

extern int parse_and_execute ();
extern void parse_and_execute_cleanup ();

extern void initialize_shell_builtins ();

/* It's OK to declare a function as returning a Function * without
   providing a definition of what a `Function' is. */
extern Function *find_shell_builtin ();
extern Function *builtin_address ();

extern char *single_quote ();
extern char *double_quote ();

#endif /* !__COMMON_H */
