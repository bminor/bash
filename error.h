/* error.h -- External declarations of functions appearing in error.c. */

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

#if !defined (_ERROR_H_)
#define _ERROR_H_

#include "stdc.h"

/* Get the name of the shell or shell script for an error message. */
extern char *get_name_for_error ();

/* Report an error having to do with FILENAME. */
extern void file_error __P((char *));

/* Report a programmer's error, and abort.  Pass REASON, and ARG1 ... ARG5. */
extern void programming_error __P((const char *, ...));

/* General error reporting.  Pass FORMAT and ARG1 ... ARG5. */
extern void report_error __P((const char *, ...));

/* Error messages for parts of the parser that don't call report_syntax_error */
extern void parser_error __P((int, const char *, ...));

/* Report an unrecoverable error and exit.  Pass FORMAT and ARG1 ... ARG5. */
extern void fatal_error __P((const char *, ...));

/* Report a system error, like BSD warn(3). */
extern void sys_error __P((const char *, ...));

/* Report an internal error. */
extern void internal_error __P((const char *, ...));

/* Report an internal warning. */
extern void internal_warning __P((const char *, ...));

/* Report an error having to do with command parsing or execution. */
extern void command_error __P((const char *, int, int, int));

extern char *command_errstr __P((int));

#endif /* !_ERROR_H_ */
