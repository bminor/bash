/* siglist.h -- encapsulate various definitions for sys_siglist */

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

#if !defined (_SIGLIST_H_)
#define _SIGLIST_H_

#if defined (Solaris) || defined (USGr4_2) || defined (drs6000) || defined (amiga) || defined (Minix)
#  if !defined (sys_siglist)
#    define sys_siglist _sys_siglist
#  endif /* !sys_siglist */
#endif /* Solaris || USGr4_2 || drs6000 || amiga || Minix */

#if !defined (Solaris) && !defined (Linux) && !defined (__BSD_4_4__) && \
	!defined (Minix) && !defined (NetBSD) && !defined (FreeBSD) && \
	!defined (BSD_OS)
extern char *sys_siglist[];
#endif /* !Solaris && !Linux && !__BSD_4_4__ && !Minix && !NetBSD && !FreeBSD && !BSD_OS */

#if !defined (strsignal) && !defined (Solaris) && !defined (NetBSD)
#  define strsignal(sig) (char *)sys_siglist[sig]
#endif /* !strsignal && !Solaris && !NetBSD */

#endif /* _SIGLIST_H */
