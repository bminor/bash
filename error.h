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

/* Get the name of the shell or shell script for an error message. */
extern char *get_name_for_error ();

/* Report an error having to do with FILENAME. */
extern void file_error ();

/* Report a programmer's error, and abort.  Pass REASON, and ARG1 ... ARG5. */
extern void programming_error ();

/* General error reporting.  Pass FORMAT and ARG1 ... ARG5. */
extern void report_error ();

/* Report an unrecoverable error and exit.  Pass FORMAT and ARG1 ... ARG5. */
extern void fatal_error ();
