/* findcmd.h - functions from findcmd.c. */

/* Copyright (C) 1997 Free Software Foundation, Inc.

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

#if !defined (_REDIR_H_)
#define _REDIR_H_

#include "stdc.h"

extern void redirection_error __P((REDIRECT *, int));
extern int do_redirections __P((REDIRECT *, int, int, int));
extern char *redirection_expand __P((WORD_DESC *));
extern int stdin_redirs __P((REDIRECT *));

#endif /* _REDIR_H_ */
