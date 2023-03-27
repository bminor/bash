/* psize-posix - display the pipe size on a Posix system. */

/* Copyright 2008,2009,2022,2023 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   Bash is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int
main(int c, char **v)
{
	char	*p;
	long	l;

	p = v[1] ? v[1] : "/";
	l = pathconf (p, _PC_PIPE_BUF);
	printf ("%ld\n", l);
	exit(0);
}
