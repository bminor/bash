/* quit.h -- How to handle SIGINT gracefully. */

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

#if !defined (__QUIT_H__)
#define __QUIT_H__

/* Non-zero means SIGINT has already ocurred. */
extern int interrupt_state;

extern void throw_to_top_level ();

/* Macro to call a great deal.  SIGINT just sets above variable.  When
   it is safe, put QUIT in the code, and the "interrupt" will take place. */
#define QUIT if (interrupt_state) throw_to_top_level ()

#endif /* __QUIT_H__ */
