/* File-name wildcard pattern matching for GNU.
   Copyright (C) 1985, 1988, 1989 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef	_GLOB_H_
#define	_GLOB_H_

#include "stdc.h"

extern int glob_pattern_p __P((char *));
extern char **glob_vector __P((char *, char *));
extern char **glob_filename __P((char *));

extern char *glob_error_return;
extern int noglob_dot_filenames;
extern int glob_ignore_case;

#endif /* _GLOB_H_ */
