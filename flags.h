/* flags.h -- a list of all the flags that the shell knows about.  You add
   a flag to this program by adding the name here, and in flags.c. */

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

#if !defined (_FLAGS_H)
#define _FLAGS_H

#include "stdc.h"

/* Welcome to the world of Un*x, where everything is slightly backwards. */
#define FLAG_ON '-'
#define FLAG_OFF '+'

#define FLAG_ERROR -1
#define FLAG_UNKNOWN (int *)0

/* The thing that we build the array of flags out of. */
struct flags_alist {
  char name;
  int *value;
};

extern struct flags_alist shell_flags[];

extern int
  mark_modified_vars, exit_immediately_on_error, disallow_filename_globbing,
  locate_commands_in_functions, place_keywords_in_env, read_but_dont_execute,
  just_one_command, unbound_vars_is_error, echo_input_at_read,
  echo_command_at_execute, lexical_scoping, no_invisible_vars, noclobber,
  hashing_disabled, forced_interactive, privileged_mode,
  asynchronous_notification, interactive_comments, no_symbolic_links;

#if defined (BANG_HISTORY)
extern int history_expansion;
#endif /* BANG_HISTORY */

#if defined (RESTRICTED_SHELL)
extern int restricted;
#endif /* RESTRICTED_SHELL */

extern int *find_flag __P((int));
extern int change_flag __P((int, int));
extern char *which_set_flags __P((void));

/* A macro for efficiency. */
#define change_flag_char(flag, on_or_off)  change_flag (flag, on_or_off)

#endif /* _FLAGS_H */
