/* bashhist.h -- interface to the bash history functions in bashhist.c. */

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

#if !defined (__BASHHIST_H__)
#define __BASHHIST_H__

extern int remember_on_history;
extern int history_lines_this_session;
extern int history_lines_in_file;
extern int history_expansion;
extern int history_control;
extern int command_oriented_history;

#  if defined (BANG_HISTORY)
extern int history_expansion_inhibited;
#  endif /* BANG_HISTORY */

extern void load_history ();
extern void save_history ();
extern int maybe_save_shell_history ();
extern char *pre_process_line ();
extern int history_number ();
extern void maybe_add_history ();

#endif /* __BASHHIST_H__ */
