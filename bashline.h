/* bashline.h -- interface to the bash readline functions in bashline.c. */

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

#if !defined (_BASHLINE_H_)
#define _BASHLINE_H_

#include "stdc.h"

extern int bash_readline_initialized;

extern void posix_readline_initialize __P((int));
extern void initialize_readline __P((void));
extern void bashline_reinitialize __P((void));
extern int bash_re_edit __P((char *));

#endif /* _BASHLINE_H_ */
