/* termcap.h - public declarations for termcap library. */

/* Copyright (C) 1991-2009,2023 Free Software Foundation, Inc.

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

#ifndef _TERMCAP_H
#define _TERMCAP_H 1

extern int tgetent (char *buffer, const char *termtype);

extern int tgetnum (const char *name);
extern int tgetflag (const char *name);
extern char *tgetstr (const char *name, char **area);

extern char PC;
extern short ospeed;
extern int tputs (const char *string, int nlines, int (*outfun) (int));

extern char *tparam (const char *ctlstring, char *buffer, int size, ...);

extern char *UP;
extern char *BC;

extern char *tgoto (const char *cstring, int hpos, int vpos);

#endif /* not _TERMCAP_H */
