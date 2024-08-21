/* machines.h --
   Included file in the makefile that gets run through Cpp.  This file
   tells which machines have what features based on the unique machine
   identifier present in Cpp. */

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

/* **************************************************************** */
/*                                                                  */
/*                Global Assumptions (true for most systems).       */
/*                                                                  */
/* **************************************************************** */

/* We make some global assumptions here.  This can be #undef'ed in
   various machine specific entries. */

/* If this file is being processed with Gcc, then the user has Gcc. */
#if defined (__GNUC__) && !defined (NeXT) && !defined (__FreeBSD__)
#  if !defined (HAVE_GCC)
#    define HAVE_GCC
#  endif /* HAVE_GCC */
#endif /* __GNUC__ && !NeXT && !__FreeBSD__ **/

/* Assume that all machines have the getwd () system call.  We unset it
   for USG systems. */
#define HAVE_GETWD

/* Assume that all systems have a working getcwd () call.  We unset it for
   ISC systems. */
#define HAVE_GETCWD

/* Most (but not all) systems have a good, working version of dup2 ().
   For systems that don't have the call (HP/UX), and for systems
   that don't set the open-on-exec flag for the dup'ed file descriptors,
   (Sequents running Dynix, Ultrix), #undef HAVE_DUP2 in the machine
   description. */
#define HAVE_DUP2

/* Every machine that has Gcc has alloca as a builtin in Gcc.  If you are
   compiling Bash without Gcc, then you must have alloca in a library,
   in your C compiler, or be able to assemble or compile the alloca source
   that we ship with Bash. */
#define HAVE_ALLOCA

/* We like most machines to use the GNU Malloc routines supplied in the
   source code because they provide high quality error checking.  On
   some machines, our malloc () cannot be used (because of library
   conflicts, for example), and for those, you should specifically
   #undef USE_GNU_MALLOC in the machine description. */
#define USE_GNU_MALLOC

/* This causes the Gnu malloc library (from glibc) to be used. */
/* #define USE_GNU_MALLOC_LIBRARY */

/* Assume that every operating system supplies strchr () and strrchr ()
   in a standard library until proven otherwise. */
#define HAVE_STRCHR

/* Hardware-dependent CFLAGS. */
#define MACHINE_CFLAGS

/* **************************************************************** */
/*								    */
/*			Sun Microsystems Machines	      	    */
/*								    */
/* **************************************************************** */

/* NetBSD running on a sparc. */
#if defined (sparc) && defined (__NetBSD__)
#  define M_MACHINE "sun4"
#  define M_OS "NetBSD"
#  define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY \
			-DRLIMTYPE=quad_t
#  define SYSDEP_LDFLAGS -static
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  define HAVE_DIRENT
#  define HAVE_STRCASECMP
#endif /* sparc && __NetBSD__ */

#if defined (sun) && !defined (M_MACHINE)
/* We aren't currently using GNU Malloc on Suns because of a bug in Sun's
   YP which bites us when Sun free ()'s an already free ()'ed address.
   When Sun fixes their YP, we can start using our winning malloc again. */
/* #undef USE_GNU_MALLOC */

/* Most Sun systems have signal handler functions that are void. */
#  define VOID_SIGHANDLER

/* Most Sun systems have the following. */
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS

/* Check for SunOS4 or greater. */
#  if defined (SunOS5)
#    define M_OS "SunOS5"
#    define SYSDEP_CFLAGS -DUSGr4 -DUSG -DSolaris -DOPENDIR_NOT_ROBUST \
			  -DSBRK_DECLARED -DINT_GROUPS_ARRAY
#    define EXTRA_LIB_SEARCH_PATH /usr/ccs/lib
#    if !defined (HAVE_GCC)
#      define REQUIRED_LIBRARIES -ldl
#      define SYSDEP_LDFLAGS -Bdynamic
#    endif /* !HAVE_GCC */
#    define HAVE_STRERROR
#    undef HAVE_GETWD
#    undef HAVE_SETLINEBUF
#  endif /* SunOS5 */

#  if defined (SunOS4)
#    define M_OS "SunOS4"
#    define SYSDEP_CFLAGS -DHAVE_BSD_PGRP -DOPENDIR_NOT_ROBUST -DTERMIOS_LDISC \
			  -DINT_GROUPS_ARRAY
#    define HAVE_DIRENT
#    define HAVE_DLOPEN
#    define HAVE_DLSYM
#    define HAVE_DLCLOSE
#    define REQUIRED_LIBRARIES -ldl
#    if !defined (HAVE_GCC)
#      define SYSDEP_LDFLAGS -Bdynamic
#    endif
#  endif /* SunOS4 */

#  if !defined (SunOS4) && !defined (SunOS5)
#    define M_OS "SunOS3"
#    if !defined (sparc) && !defined (__sparc__)
#      undef VOID_SIGHANDLER
#    endif /* !sparc */
#  endif /* !SunOS4 && !SunOS5 */

#  if defined (mc68010)
#    define sun2
#    define M_MACHINE "sun2"
#  endif
#  if defined (mc68020)
#    define sun3
#    define M_MACHINE "sun3"
#  endif
#  if defined (sparc) || defined (__sparc__)
#    define sun4
#    define M_MACHINE "sparc"
#  endif
#  if defined (i386)
#    define done386
#    if !defined (SunOS5)
#      define Sun386i
#      define M_MACHINE "Sun386i"
#    else
#      define M_MACHINE "i386"
#    endif
#  endif /* i386 */

#endif /* sun && !M_MACHINE */

/* **************************************************************** */
/*								    */
/*			DEC Machines (vax, decstations)   	    */
/*								    */
/* **************************************************************** */

/* ************************ */
/*                          */
/*     Alpha with OSF/1     */
/*                          */
/* ************************ */
#if defined (__alpha) || defined (alpha)
#  define M_MACHINE "alpha"
#  define M_OS "OSF1"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define HAVE_STRERROR
#  define HAVE_GETGROUPS
#  define VOID_SIGHANDLER
#  define HAVE_DLOPEN
#  define HAVE_DLSYM
#  define HAVE_DLCLOSE
#  define USE_TERMCAP_EMULATION
#  if !defined (__GNUC__)
#    define SYSDEP_CFLAGS -DNLS -D_BSD
#  endif /* !__GNUC__ */
#  undef HAVE_ALLOCA
#  undef USE_GNU_MALLOC
#endif /* __alpha || alpha */

/* ************************ */
/*			    */
/*	    Ultrix	    */
/*			    */
/* ************************ */
#if defined (ultrix)
#  if defined (MIPSEL)
#    undef HAVE_ALLOCA_H
#    define M_MACHINE "MIPSEL"
#  else /* !MIPSEL */
#    define M_MACHINE "vax"
#  endif /* !MIPSEL */
#  define SYSDEP_CFLAGS -DHAVE_BSD_PGRP -DTERMIOS_MISSING -DTERMIOS_LDISC \
			-DINT_GROUPS_ARRAY
#  define M_OS "Ultrix"
#  define HAVE_DIRENT
#  define VOID_SIGHANDLER
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS
#  undef HAVE_DUP2
#endif /* ultrix */

/* ************************ */
/*			    */
/*	VAX 4.3 BSD	    */
/*			    */
/* ************************ */
#if defined (vax) && !defined (ultrix)
#  define M_MACHINE "vax"
#  define M_OS "Bsd"
#  define HAVE_SETLINEBUF
#  define HAVE_SYS_SIGLIST
#  define HAVE_GETGROUPS
#  define USE_VFPRINTF_EMULATION
#endif /* vax && !ultrix */

/* ************************ */
/*			    */
/*	Tahoe 4.3 BSD	    */
/*			    */
/* ************************ */
#if defined (tahoe)
#  define M_MACHINE "tahoe"
#  define M_OS "Bsd"
#  define HAVE_SETLINEBUF
#  define HAVE_SYS_SIGLIST
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#endif /* tahoe */

/* **************************************************************** */
/*								    */
/*		Machines with MIPSco processors			    */
/*								    */
/* **************************************************************** */

/* **************************************** */
/*					    */
/*		SGI Iris/IRIX	    	    */
/*					    */
/* **************************************** */
#if defined (sgi)
#  if defined (Irix3)
#    define M_OS "Irix3"
#    if !defined (HAVE_GCC)
#      undef MACHINE_CFLAGS
#      define MACHINE_CFLAGS -real_frameptr -Wf,-XNl3072
#    endif
#    undef HAVE_ALLOCA
#  endif /* Irix3 */
#  if defined (Irix4)
#    define M_OS "Irix4"
#    if !defined (HAVE_GCC)
#      undef MACHINE_CFLAGS
#      define MACHINE_CFLAGS -Wf,-XNl3072
#    endif
#  endif /* Irix4 */
#  if defined (Irix5)
#    define M_OS "Irix5"
#    if !defined (HAVE_GCC)
#      undef MACHINE_CFLAGS
#      define MACHINE_CFLAGS -Wf,-XNl3072
#    endif
#  endif /* Irix5 */
#  if defined (Irix6)
#    define M_OS "Irix6"
#    if !defined (HAVE_GCC)
#      undef MACHINE_CFLAGS
#      define MACHINE_CFLAGS -mips2
#    endif /* !HAVE_GCC */
#  endif /* Irix6 */
#  define M_MACHINE "sgi"
#  define HAVE_GETGROUPS
#  define VOID_SIGHANDLER
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  if !defined (Irix6)
#    define REQUIRED_LIBRARIES -lsun
#  endif /* !Irix6 */
   /* SGI cc uses ansi c features *without* defining __STDC__ */
#  if defined (__EXTENSIONS__) && !defined (__STDC__)
#    define ANSIC -D__STDC__
#  else
#    define ANSIC
#  endif /* !__EXTENSIONS__ || __STDC__ */
#  if defined (Irix5) || defined (Irix6)
#    define SGI_CFLAGS -DUSG -DPGRP_PIPE -DHAVE_BCOPY -DHAVE_GETPW_DECLS \
		       -DHAVE_SOCKETS -DSBRK_DECLARED
#  else
#    define SGI_CFLAGS -DUSG -DPGRP_PIPE -DHAVE_BCOPY -DHAVE_GETPW_DECLS \
		       -DHAVE_SOCKETS
#  endif /* !Irix5 */
#  define SYSDEP_CFLAGS SGI_CFLAGS MACHINE_CFLAGS ANSIC
#endif  /* sgi */

/* ************************ */
/*			    */
/* 	  NEC EWS 4800	    */
/*			    */
/* ************************ */
#if defined (nec_ews)
#  if defined (SYSTYPE_SYSV) || defined (USGr4)
#    define M_MACHINE "ews4800"
#    define M_OS "USG"
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
#    define HAVE_STRERROR
#    define HAVE_DUP2
#    undef HAVE_GETWD
#    undef HAVE_RESOURCE /* ? */
     /* Alloca requires either Gcc or cc with -lucb. */
#    if !defined (HAVE_GCC)
#      define EXTRA_LIB_SEARCH_PATH /usr/ucblib
#      define REQUIRED_LIBRARIES -lc -lucb
#    endif /* !HAVE_GCC */
#    if defined (MIPSEB)
#      if !defined (HAVE_GCC)
#        undef MACHINE_CFLAGS
#        define MACHINE_CFLAGS -Wf,-XNl3072
#      endif
#      define SYSDEP_CFLAGS MACHINE_CFLAGS -DUSGr4 -DUSGr3 -D_POSIX_JOB_CONTROL
#    else /* !MIPSEB */
#      define SYSDEP_CFLAGS -DUSGr4
#    endif /* MIPSEB */
#  else /* !SYSTYPE_SYSV && !USGr4 */
#    define M_OS "Bsd"
#  endif /* !SYSTYPE_SYSV && !USGr4 */
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* nec_ews */

/* ************************ */
/*			    */
/*  Generic MIPS SVR4, 4.2  */
/*			    */
/* ************************ */
#if defined (MIPSEB) && defined (USGr4)
#  define M_MACHINE "MIPSEB"
#  define M_OS "USG"
#  if defined (sony) && !defined (HAVE_GCC)
#    undef MACHINE_CFLAGS
#    define MACHINE_CFLAGS -Wf,-XNl3072
#  endif
/* XXX - os/svr4.h -- for the future -- XXX */
#  undef HAVE_GETWD
#  define HAVE_DIRENT
#  define HAVE_STRERROR
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
/* alloca */
#  if !defined (HAVE_GCC)
#    define EXTRA_LIB_SEARCH_PATH /usr/ucblib
#    define REQUIRED_LIBRARIES -lc -lucb
#  endif /* !HAVE_GCC */
#  if defined (USGr4_2)
#    define SYSDEP_CFLAGS MACHINE_CFLAGS -DUSGr4 -DUSGr4_2
#  else
#    define SYSDEP_CFLAGS MACHINE_CFLAGS -DUSGr4
#  endif /* !USGr4_2 */
#endif

/* ************************ */
/*			    */
/*	    Sony	    */
/*			    */
/* ************************ */
#if defined (sony) && !defined (M_MACHINE)
#  if defined (MIPSEB)
#    define M_MACHINE "MIPSEB"
#  else /* !MIPSEB */
#    define M_MACHINE "sony"
#  endif /* !MIPSEB */

#  if defined (SYSTYPE_SYSV) || defined (USGr4)
#    define M_OS "USG"
#    undef HAVE_GETWD
#    define HAVE_DIRENT
#    define HAVE_STRERROR
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
     /* Alloca requires either Gcc or cc with -lucb. */
#    if !defined (HAVE_GCC)
#      define EXTRA_LIB_SEARCH_PATH /usr/ucblib
#      define REQUIRED_LIBRARIES -lc -lucb
#    endif /* !HAVE_GCC */
#    if defined (MIPSEB)
#      if !defined (HAVE_GCC)
#        undef MACHINE_CFLAGS
#        define MACHINE_CFLAGS -Wf,-XNl3072
#      endif
#      define SYSDEP_CFLAGS MACHINE_CFLAGS -DUSGr4
#    else /* !MIPSEB */
#      define SYSDEP_CFLAGS -DUSGr4
#    endif /* !MIPSEB */
#  else /* !SYSTYPE_SYSV && !USGr4 */
#    define M_OS "Bsd"
#    define SYSDEP_CFLAGS -DHAVE_UID_T
#  endif /* !SYSTYPE_SYSV && !USGr4 */
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* sony */

/* ******************************* */
/*			           */
/*  Ardent Titan OS v2.2 and later */
/*			           */
/* ******************************* */
#if defined (ardent)
#  define M_MACHINE "Ardent Titan"
#  define M_OS "Bsd"
#  if defined (titan)
#    undef HAVE_GETGROUPS
#  else
#    define HAVE_GETGROUPS
#  endif /* !titan */
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define SYSDEP_CFLAGS -43 -w
#  define SYSDEP_LDFLAGS -43
#  undef HAVE_ALLOCA
#  undef USE_GNU_MALLOC
#  undef HAVE_VPRINTF
#  undef HAVE_DIRENT_H
#endif /* ardent */

/* ************************ */
/*			    */
/*	  Stardent	    */
/*			    */
/* ************************ */
#if defined (stardent) && !defined (M_MACHINE)
#  define M_MACHINE "Stardent"
#  define M_OS "USG"
#  define HAVE_SYS_SIGLIST
#  define USE_TERMCAP_EMULATION
#  define VOID_SIGHANDLER
#  undef HAVE_GETWD
#  undef HAVE_ALLOCA
#endif /* stardent */

/* ******************************** */
/*				    */
/*	   MIPS RISC/os		    */
/*				    */
/* ******************************** */

/* Notes on compiling with "make":

   * Place /bsd43/bin in your PATH before /bin.
   * Use `$(CC) -E' instead of `/lib/cpp' in Makefile.
*/
#if defined (mips) && (!defined (M_MACHINE) || defined (RiscOS))

#  if defined (MIPSEB)
#    define M_MACHINE "MIPSEB"
#  else /* !MIPSEB */
#    if defined (MIPSEL)
#      define M_MACHINE "MIPSEL"
#    else /* !MIPSEL */
#      define M_MACHINE "mips"
#    endif /* !MIPSEL */
#  endif /* !MIPSEB */

#  define M_OS "Bsd"

   /* Special things for machines from MIPS Co. */
#  define MIPS_CFLAGS -DOPENDIR_NOT_ROBUST -DPGRP_PIPE

#  if !defined (HAVE_GCC)
#    undef MACHINE_CFLAGS
#    define MACHINE_CFLAGS -Wf,-XNl3072 -systype bsd43
#  endif /* !HAVE_GCC */
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS
#  undef HAVE_UNISTD_H
#  if !defined (HAVE_RESOURCE)
#    define HAVE_RESOURCE
#  endif /* !HAVE_RESOURCE */
#  if defined (bsd4_4)
#    define VOID_SIGHANDLER
#    define HAVE_UNISTD_H	/* ??? atsuo@harl.hitachi.co.jp */
#    define SYSDEP_CFLAGS MACHINE_CFLAGS MIPS_CFLAGS -D__BSD_4_4__
#  else
     /* /usr/include/sys/wait.h appears not to work correctly, so why use it? */
#    undef HAVE_WAIT_H
#    define SYSDEP_CFLAGS MACHINE_CFLAGS MIPS_CFLAGS
#  endif
#endif /* mips */

/* ************************ */
/*			    */
/*	  Pyramid	    */
/*			    */
/* ************************ */
#if defined (pyr)
#  define M_MACHINE "Pyramid"
#  define M_OS "Bsd"
#  if !defined (HAVE_GCC)
#    undef HAVE_ALLOCA
#  endif /* HAVE_GCC */
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* pyr */

/* ************************ */
/*			    */
/*	    IBMRT	    */
/*			    */
/* ************************ */
#if defined (ibm032)
#  define M_MACHINE "IBMRT"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define USE_VFPRINTF_EMULATION
   /* Alloca requires either gcc or hc or pcc with -ma in SYSDEP_CFLAGS. */
#  if !defined (HAVE_GCC)
#    define SYSDEP_CFLAGS -ma -U__STDC__
#  endif /* !HAVE_GCC */
#  define HAVE_GETGROUPS
/* #define USE_GNU_TERMCAP */
#endif /* ibm032 */

/* **************************************************************** */
/*								    */
/*	  All Intel 386 Processor Machines are Defined Here!	    */
/*								    */
/* **************************************************************** */

#if defined (i386)

/* Sequent Symmetry running Dynix/ptx 2.x */
#  if !defined (done386) && defined (_SEQUENT_)
#    define done386
#    define M_MACHINE "Symmetry"
#    define M_OS "Dynix"
#    define DYNIX_CFLAGS -DHAVE_GETDTABLESIZE -DHAVE_SETDTABLESIZE \
			 -DHAVE_GETPW_DECLS -DHAVE_SOCKETS
#    define SYSDEP_CFLAGS -DUSG -DUSGr3 DYNIX_CFLAGS
#    define HAVE_DIRENT
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
/* Might need to add -lsocket -linet -lnsl to the list of libraries. */
#    define REQUIRED_LIBRARIES -lPW -lseq
#    undef HAVE_GETWD
#    undef HAVE_RESOURCE
#    undef HAVE_ALLOCA
#  endif /* _SEQUENT_ */

/* Sequent Symmetry running Dynix (4.2 BSD) */
#  if !defined (done386) && defined (sequent)
#    define done386
#    define M_MACHINE "Symmetry"
#    define M_OS "Bsd"
#    define SYSDEP_CFLAGS -DCPCC -DHAVE_SETDTABLESIZE
#    define HAVE_SETLINEBUF
#    define HAVE_SYS_SIGLIST
#    define HAVE_GETGROUPS
#    define LD_HAS_NO_DASH_L
#    undef HAVE_DUP2
#  endif /* Sequent 386 */

/* NeXT 3.x on i386 */
#  if !defined (done386) && defined (NeXT)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "NeXTstep"
#    define HAVE_VPRINTF
#    define HAVE_SYS_SIGLIST
#    define HAVE_GETGROUPS
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    if !defined (HAVE_RESOURCE)
#      define HAVE_RESOURCE
#    endif
#    define HAVE_STRCASECMP
#    define GCC_STANDARD
#    undef HAVE_GETWD
#    undef HAVE_GETCWD
#    undef USE_GNU_MALLOC
#    undef HAVE_DIRENT_H
#    define SYSDEP_CFLAGS -DNeXT -DMKFIFO_MISSING -DRLOGIN_PGRP_BUG
#  endif

/* Generic 386 clone running Mach (4.3 BSD-compatible). */
#  if !defined (done386) && defined (MACH)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "Bsd"
#    define HAVE_SETLINEBUF
#    define HAVE_SYS_SIGLIST
#    define HAVE_GETGROUPS
#  endif /* i386 && MACH */

/* AIX PS/2 1.[23] for the [34]86. */
#  if !defined (done386) && defined (aixpc)
#    define done386
#    define M_MACHINE "aixpc"
#    define M_OS "AIX"
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
#    if defined (AIX_13)	/* AIX PS/2 1.3 */
#      define SYSDEP_CFLAGS -DTERMIOS_LDISC
#      define REQUIRED_LIBRARIES -lc_s
#    else
#      define SYSDEP_CFLAGS -D_BSD -DTERMIOS_LDISC
#      define REQUIRED_LIBRARIES -lbsd -lc_s
#    endif /* !AIX_13 */
#    define HAVE_GETGROUPS
#    if !defined (HAVE_GCC)
#      undef HAVE_ALLOCA
#      undef HAVE_ALLOCA_H
#    endif /* !HAVE_GCC */
#    define USE_TERMCAP_EMULATION
#  endif /* AIXPC i386 */

/* System V Release 4 on the 386 */
#  if !defined (done386) && defined (USGr4)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "USG"
#    define HAVE_DIRENT
#    define HAVE_SYS_SIGLIST
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
     /* Alloca requires either Gcc or cc with -lucb. */
#    if !defined (HAVE_GCC)
#      define EXTRA_LIB_SEARCH_PATH /usr/ucblib
#      define REQUIRED_LIBRARIES -lc -lucb
#    endif /* !HAVE_GCC */
#    define HAVE_GETGROUPS
#    if defined (USGr4_2)
#      define SYSDEP_CFLAGS -DUSGr4 -DUSGr4_2 -DSBRK_DECLARED
#    else
#      define SYSDEP_CFLAGS -DUSGr4
#    endif /* ! USGr4_2 */
#    undef HAVE_GETWD
#  endif /* System V Release 4 on i386 */

/* 386 box running Interactive Unix 2.2 or greater. */
#  if !defined (done386) && defined (isc386)
#    define done386
#    define M_MACHINE "isc386"
#    define M_OS "USG"
#    define HAVE_DIRENT
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
#    define HAVE_GETGROUPS
#    define USE_TERMCAP_EMULATION
#    if defined (HAVE_GCC)
#      define SYSDEP_LDFLAGS -posix
#      define ISC_POSIX
#    else
#      define REQUIRED_LIBRARIES -lPW
#      define SYSDEP_LDFLAGS -Xp
#      define ISC_POSIX -Xp
#    endif
#    define ISC_SYSDEPS -DUSGr3 -DPGRP_PIPE -DHAVE_GETPW_DECLS -D_POSIX_SOURCE -DOPENDIR_NOT_ROBUST -DMEMMOVE_MISSING
#    if defined (__STDC__)
#      if defined (HAVE_GCC)
#        define ISC_EXTRA -DO_NDELAY=O_NONBLOCK
#      else
#        define ISC_EXTRA -Dmode_t="unsigned short" -DO_NDELAY=O_NONBLOCK
#      endif /* HAVE_GCC */
#    else
#      define ISC_EXTRA
#    endif /* __STDC__ */
#    define SYSDEP_CFLAGS ISC_SYSDEPS ISC_POSIX ISC_EXTRA
#    undef HAVE_GETWD
#    if !defined (ISC_4)
#      undef HAVE_GETCWD
#    else /* ISC_4 */
#      undef HAVE_RESOURCE
#    endif /* ISC_4 */
#  endif /* isc386 */

/* Xenix386 machine (with help from Ronald Khoo <ronald@robobar.co.uk>). */
#  if !defined (done386) && defined (Xenix386)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "Xenix"
#    define XENIX_CFLAGS -DUSG -DUSGr3 -DMEMMOVE_MISSING

#    if defined (XENIX_22)
#      define XENIX_EXTRA -DSETVBUF_REVERSED
#      define REQUIRED_LIBRARIES -lx
#    else /* !XENIX_22 */
#      define HAVE_DIRENT
#      if defined (XENIX_23)
#        define XENIX_EXTRA -DLD_HAS_NO_DASH_L
#        define REQUIRED_LIBRARIES -ldir
#      else /* !XENIX_23 */
#        define XENIX_EXTRA -xenix
#        define SYSDEP_LDFLAGS -xenix
#        define REQUIRED_LIBRARIES -ldir -l2.3
#      endif /* !XENIX_23 */
#    endif /* !XENIX_22 */

#    define SYSDEP_CFLAGS XENIX_CFLAGS XENIX_EXTRA
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
#    define ALLOCA_ASM x386-alloca.s
#    define ALLOCA_OBJ x386-alloca.o
#    undef HAVE_ALLOCA
#    undef HAVE_GETWD
#    undef HAVE_RESOURCE
#  endif /* Xenix386 */

/* SCO UNIX 3.2 chip@count.tct.com (Chip Salzenberg) */
#  if !defined (done386) && defined (M_UNIX)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "SCO"
#    define SCO_CFLAGS -DUSG -DUSGr3 -DNO_DEV_TTY_JOB_CONTROL -DPGRP_PIPE
#    if defined (SCOv4)
#      define SYSDEP_CFLAGS SCO_CFLAGS -DWAITPID_BROKEN
#    else /* !SCOv4 */
#      define SYSDEP_CFLAGS SCO_CFLAGS -DMUST_UNBLOCK_CHLD -DOPENDIR_NOT_ROBUST
#    endif /* !SCOv4 */
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
#    define HAVE_GETGROUPS
#    undef HAVE_GETWD
#    undef HAVE_RESOURCE
/* advice from wbader@cess.lehigh.edu and Eduard.Vopicka@vse.cz */
#    if !defined (HAVE_GCC)
#      define REQUIRED_LIBRARIES -lc_s -lc -lPW
#    else
#      define REQUIRED_LIBRARIES -lc_s -lc
#    endif /* !HAVE_GCC */
#  endif /* SCO Unix on 386 boxes. */

#  if !defined (done386) && defined (__OSF1__)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "OSF1"
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_VPRINTF
#    define HAVE_STRERROR
#    define HAVE_GETGROUPS
#    define VOID_SIGHANDLER
#    define HAVE_BCOPY
#    define USE_TERMCAP_EMULATION
#    define SYSDEP_CFLAGS -D_BSD
#    define REQUIRED_LIBRARIES -lbsd
#  endif /* OSF/1 */

/* BSDI BSD/OS running on a 386 or 486. */
#  if !defined (done386) && defined (__bsdi__)
#    define done386
#    define M_MACHINE "i386"
#    if defined (BSDI2)
#      define M_OS "BSD_OS"
#      define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DRLIMTYPE=quad_t
#    else
#      define M_OS "BSD386"
#      define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY
#    endif
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_GETGROUPS
#    define HAVE_VPRINTF
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    define HAVE_DIRENT
#    define HAVE_STRCASECMP
#  endif /* !done386 && bsdi */

/* NetBSD running on a 386 or 486. */
#  if !defined (done386) && defined (__NetBSD__)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "NetBSD"
#    define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY \
			  -DRLIMTYPE=quad_t
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_GETGROUPS
#    define HAVE_VPRINTF
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    define HAVE_DIRENT
#    define HAVE_STRCASECMP
#  endif /* !done386 && __NetBSD__ */

/* FreeBSD running on a 386 or 486. */
#  if !defined (done386) && defined (__FreeBSD__)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "FreeBSD"
#    if __FreeBSD__ > 1
#      define SYSDEP_CFLAGS -D__BSD_4_4__ -DRLIMTYPE=quad_t
#    else
#      define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY
#    endif
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_GETGROUPS
#    define HAVE_VPRINTF
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    define HAVE_DIRENT
#    define HAVE_STRCASECMP
#    define GCC_STANDARD
#  endif /* !done386 && __FreeBSD__ */

/* Jolitz 386BSD running on a 386 or 486. */
#  if !defined (done386) && defined (__386BSD__)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "_386BSD"
#    define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_GETGROUPS
#    define HAVE_VPRINTF
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    define HAVE_DIRENT
#    define HAVE_STRCASECMP
#  endif /* !done386 && __386BSD__ */

#  if !defined (done386) && (defined (__linux__) || defined (linux))
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "Linux"
#    define SYSDEP_CFLAGS -DHAVE_GETDTABLESIZE -DHAVE_BCOPY \
			  -DHAVE_GETPW_DECLS -DHAVE_GETHOSTNAME
#    define REQUIRED_LIBRARIES
#    define HAVE_GETGROUPS
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    define HAVE_SYS_SIGLIST
#    define HAVE_VPRINTF
#    define HAVE_VARARGS_H
#    define SEARCH_LIB_NEEDS_SPACE
#    if defined (__GNUC__)
#      define HAVE_FIXED_INCLUDES
#    endif /* __GNUC__ */
#    undef USE_GNU_MALLOC
#    undef HAVE_SETLINEBUF
#    undef HAVE_GETWD
#  endif  /* !done386 && __linux__ */

/* QNX 4.2 with GCC pt@flard.ocunix.on.ca (Paul Trunley) */
#  if !defined (done386) && defined (qnx)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "QNX"
#    define SYSDEP_CFLAGS -D_POSIX_SOURCE -O2 -DUSG
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
#    define HAVE_GCC
#    define HAVE_FIXED_INCLUDES
#    define HAVE_STRERROR
#    define HAVE_GETGROUPS
#    undef USE_GNU_MALLOC
#  endif /* QNX 4.2 with GCC */

/* Lynx 2.1.0 (Mike Brennen <mbrennen@maverick.intecom.com>) */
#  if !defined (done386) && (defined (__Lynx__) || defined (Lynx))
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "Lynx"
#    define SYSDEP_CFLAGS -DHAVE_GETDTABLESIZE -DHAVE_BCOPY
#    define REQUIRED_LIBRARIES -lc_p
#    define HAVE_GETGROUPS
#    define VOID_SIGHANDLER
#    define HAVE_SYS_SIGLIST
#    define HAVE_VPRINTF
#    define HAVE_VARARGS_H
#    if defined (__GNUC__)
#      define HAVE_FIXED_INCLUDES
#    endif /* __GNUC__ */
/* Actually, Lynx does have unistd.h, but it defines _POSIX_VERSION,
   and doesn't supply a fully compatible job control package.  We just
   pretend that it doesn't have it. */
#  undef HAVE_UNISTD_H
/* Lynx's wait structure reverses w_Stopval and w_Stopsig - don't use it */
#  undef HAVE_WAIT_H
#  undef HAVE_DIRENT_H
#  endif  /* !done386 && __Lynx__ */

/* Assume a generic 386 running Sys V Release 3. */
#  if !defined (done386)
#    define done386
#    define M_MACHINE "i386"
#    define M_OS "USG"
#    define SYSDEP_CFLAGS -DUSGr3
#    define HAVE_VPRINTF
#    define VOID_SIGHANDLER
     /* Alloca requires either Gcc or cc with libPW.a */
#    if !defined (HAVE_GCC)
#      define REQUIRED_LIBRARIES -lPW
#    endif /* !HAVE_GCC */
#    undef HAVE_GETWD
#  endif /* Generic i386 Box running Sys V release 3. */
#endif /* All i386 Machines with an `i386' define in cpp. */

/* **************************************************************** */
/*			                                            */
/*	                 Alliant FX/800                             */
/*			                                            */
/* **************************************************************** */
/* Original descs flushed.  FX/2800 machine desc 1.13 bfox@ai.mit.edu.
   Do NOT use -O with the stock compilers.  If you must optimize, use
   -uniproc with fxc, and avoid using scc. */
#if defined (alliant)
#  define M_MACHINE "alliant"
#  define M_OS "Concentrix"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#  define HAVE_RESOURCE
#  define VOID_SIGHANDLER
#  define HAVE_STRERROR
#  define USE_GNU_MALLOC
#  define LD_HAS_NO_DASH_L
#  define SYSDEP_CFLAGS -DTERMIOS_MISSING -DMKFIFO_MISSING \
	-DHAVE_BSD_PGRP -DRLOGIN_PGRP_BUG -w
   /* Actually, Alliant does have unistd.h, but it defines _POSIX_VERSION,
      and doesn't supply a fully compatible job control package.  We just
      pretend that it doesn't have it. */
#  undef HAVE_UNISTD_H
#  undef HAVE_ALLOCA
#endif /* alliant */

/* **************************************************************** */
/*                                                                  */
/*            Motorola Delta series running System V R3V6/7         */
/*                                                                  */
/* **************************************************************** */
/* Contributed by Robert L. McMillin (rlm@ms_aspen.hac.com).  */

#if defined (m68k) && defined (sysV68)
#  define M_MACHINE "Delta"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -DUSGr3
#  define VOID_SIGHANDLER
#  define HAVE_VPRINTF
#  define REQUIRED_LIBRARIES -lm881
#  undef HAVE_GETWD
#  undef HAVE_RESOURCE
#  undef HAVE_DUP2
#  undef HAVE_ALLOCA
#endif /* Delta series */

/* **************************************************************** */
/*                                                                  */
/*            Motorola m68k system running Linux                    */
/*                                                                  */
/* **************************************************************** */

#if defined (mc68020) && (defined (__linux__) || defined (linux))
#    define M_MACHINE "m68k"
#    define M_OS "Linux"
#    define SYSDEP_CFLAGS -DUSG -DUSGr3 -DHAVE_GETDTABLESIZE -DHAVE_BCOPY \
                        -DHAVE_GETPW_DECLS -DHAVE_GETHOSTNAME
#    define REQUIRED_LIBRARIES
#    define HAVE_GETGROUPS
#    define HAVE_STRERROR
#    define VOID_SIGHANDLER
#    define HAVE_SYS_SIGLIST
#    define HAVE_VPRINTF
#    define HAVE_VARARGS_H
#    define SEARCH_LIB_NEEDS_SPACE
#    if defined (__GNUC__)
#      define HAVE_FIXED_INCLUDES
#    endif /* __GNUC__ */
#    undef USE_GNU_MALLOC
#    undef HAVE_SETLINEBUF
#    undef HAVE_GETWD
#  endif  /* m68k && __linux__ */

/* **************************************************************** */
/*								    */
/*		      Gould 9000 - UTX/32 R2.1A			    */
/*								    */
/* **************************************************************** */
#if defined (gould)		/* Maybe should be GOULD_PN ? */
#  define M_MACHINE "gould"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* gould */

/* ************************ */
/*			    */
/*	    NeXT	    */
/*			    */
/* ************************ */
#if defined (NeXT) && !defined (M_MACHINE)
#  define M_MACHINE "NeXT"
#  define M_OS "NeXTstep"
#  define HAVE_VPRINTF
#  define HAVE_SYS_SIGLIST
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  if !defined (HAVE_RESOURCE)
#    define HAVE_RESOURCE
#  endif
#  define HAVE_STRCASECMP
#  define GCC_STANDARD
#  undef HAVE_GETWD
#  undef HAVE_GETCWD
#  undef HAVE_DIRENT_H
#  define SYSDEP_CFLAGS -DMKFIFO_MISSING -DRLOGIN_PGRP_BUG
#  undef USE_GNU_MALLOC
#endif /* NeXT */

/* ********************** */
/*                        */
/*       m68k NetBSD      */
/*                        */
/* ********************** */
#if defined (m68k) && defined (__NetBSD__)
#  include <machine/param.h>
#  define M_MACHINE MACHINE
#  define M_OS "NetBSD"
/* os/netbsd.h */
#  define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY \
			-DRLIMTYPE=quad_t
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  define HAVE_DIRENT
#  define HAVE_STRCASECMP
#endif /* m68k && __NetBSD__ */

/* ************************ */
/*			    */
/*	hp9000 4.4 BSD	    */
/*			    */
/* ************************ */
#if defined (hp9000) && defined (__BSD_4_4__)
#  define M_MACHINE "hp9000"
#  define M_OS "BSD_4_4"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_STRCASECMP
#  define SYSDEP_CFLAGS -DHAVE_GETDTABLESIZE -DHAVE_BCOPY -DHAVE_RESOURCE
#  undef HAVE_ALLOCA
#endif /* hp9000 && __BSD_4_4__ */

/* ************************ */
/*			    */
/*	hp9000 4.3 BSD	    */
/*			    */
/* ************************ */
#if defined (hp9000) && !defined (hpux) && !defined (M_MACHINE)
#  define M_MACHINE "hp9000"
#  define M_OS "Bsd"
#  undef HAVE_ALLOCA
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define USE_VFPRINTF_EMULATION
#endif /* hp9000 && !hpux */

/* ************************ */
/*                          */
/*          hpux            */
/*                          */
/* ************************ */
#if defined (hpux)

/* HPUX comes in several different flavors, from pre-release 6.2 (basically
   straight USG), to Posix compliant 9.0. */

   /* HP machines come in several processor types.
      They are distinguished here. */
#  if defined (hp9000s200) && !defined (hp9000s300)
#    define M_MACHINE "hp9000s200"
#  endif /* hp9000s200 */
#  if defined (hp9000s300) && !defined (M_MACHINE)
#    define M_MACHINE "hp9000s300"
#  endif /* hp9000s300 */
#  if defined (hp9000s500) && !defined (M_MACHINE)
#    define M_MACHINE "hp9000s500"
#  endif /* hp9000s500 */
#  if defined (hp9000s700) && !defined (M_MACHINE)
#    define M_MACHINE "hp9000s700"
#  endif /* hp9000s700 */
#  if defined (hp9000s800) && !defined (M_MACHINE)
#    define M_MACHINE "hp9000s800"
#  endif /* hp9000s800 */
#  if defined (hppa) && !defined (M_MACHINE)
#    define M_MACHINE "hppa"
#  endif /* hppa */

/* Define the OS as the particular type that we are using. */
/* This is for HP-UX systems earlier than HP-UX 6.2 -- no job control. */
#  if defined (HPUX_USG)
#    define M_OS "USG"
#    define HPUX_CFLAGS -Dhpux
#    define REQUIRED_LIBRARIES -lPW -lBSD
#    undef HAVE_WAIT_H
#    define HPUX_EXTRA
#  else /* !HPUX_USG */

/* All of the other operating systems need HPUX to be defined. */
#    define HPUX_EXTRA -DHPUX -Dhpux -DHAVE_GETHOSTNAME -DUSG

     /* HPUX 6.2 .. 6.5 require -lBSD for getwd (), and -lPW for alloca (). */
#    if defined (HPUX_6)
#      define M_OS "hpux_6"
#      define REQUIRED_LIBRARIES -lPW -lBSD
#      undef HAVE_ALLOCA
#      undef HAVE_WAIT_H
#    endif /* HPUX_6 */

     /* On HP-UX 7.x, we do not link with -lBSD, so we don't have getwd (). */
#    if defined (HPUX_7)
#      define M_OS "hpux_7"
#      define REQUIRED_LIBRARIES -lPW
#      define HPUX_CFLAGS -DHAVE_SOCKETS
#      undef HAVE_GETWD
#      undef USE_GNU_MALLOC
#    endif /* HPUX_7 */

     /* HP-UX 8.x systems do not have a working alloca () on all platforms.
	This can cause us problems, especially when globbing.  HP has the
	same YP bug as Sun, so we #undef USE_GNU_MALLOC. */
#    if defined (HPUX_8)
#      define M_OS "hpux_8"
#      if !defined (__GNUC__)
#        undef HAVE_ALLOCA
#	 define HPUX_ANSI +O3 -Aa -D_HPUX_SOURCE
#      else
#	 define HPUX_ANSI
#      endif
#      undef HAVE_GETWD
#      undef USE_GNU_MALLOC
#      define HPUX_CFLAGS -DSBRK_DECLARED -DHAVE_SOCKETS HPUX_ANSI
#    endif /* HPUX_8 */

     /* HP-UX 9.0 reportedly fixes the alloca problems present in the 8.0
        release.  If so, -lPW is required to include it. */
#    if defined (HPUX_9)
#      define M_OS "hpux_9"
#      if !defined (__GNUC__)
#        undef HAVE_ALLOCA
#	 define HPUX_ANSI +O3 -Ae
#      else
#	 define HPUX_ANSI
#      endif
#      undef HAVE_GETWD
#      undef USE_GNU_MALLOC
#      undef HAVE_RESOURCE
#      define HPUX_CFLAGS -DSBRK_DECLARED -DHAVE_SOCKETS -DHAVE_GETHOSTNAME HPUX_ANSI
#    endif /* HPUX_9 */

#  endif /* !HPUX_USG */

   /* All of the HPUX systems that we have tested have the following. */
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define USE_TERMCAP_EMULATION
#  define SEARCH_LIB_NEEDS_SPACE

#  if defined (HPUX_CFLAGS)
#    define SYSDEP_CFLAGS HPUX_CFLAGS HPUX_EXTRA
#  else /* !HPUX_CFLAGS */
#    define SYSDEP_CFLAGS HPUX_EXTRA
#  endif /* !HPUX_CFLAGS */

#endif /* hpux */

/* ************************ */
/*                          */
/*        MIPS OSF/1        */
/*                          */
/* ************************ */
#  if defined (MIPSEL) && defined (__OSF1__)
#    define M_MACHINE "mips"
#    define M_OS "OSF1"
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_VPRINTF
#    define HAVE_STRERROR
#    define HAVE_GETGROUPS
#    define VOID_SIGHANDLER
#    define HAVE_BCOPY
#    define USE_TERMCAP_EMULATION
#    define SYSDEP_CFLAGS -D_BSD
#    define REQUIRED_LIBRARIES -lbsd
#  endif /* MIPSEL && __OSF1__ */

/* ************************ */
/*                          */
/*        HP OSF/1          */
/*                          */
/* ************************ */
#if defined (__hp_osf)
#  define M_MACHINE "HPOSF1"
#  define M_OS "OSF1"
#  define SYSDEP_CFLAGS -q no_sl_enable
#  define SYSDEP_LDFLAGS -q lang_level:classic
#  define REQUIRED_LIBRARIES -lPW
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  undef HAVE_ALLOCA
#endif /* __hp_osf */

/* ************************ */
/*                          */
/*        KSR1 OSF/1        */
/*                          */
/* ************************ */
#if defined (__ksr1__)
#  define M_MACHINE "KSR1"
#  define M_OS "OSF1"
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define SYSDEP_CFLAGS -DHAVE_GETDTABLESIZE -DHAVE_BCOPY -DHAVE_UID_T
#  undef HAVE_ALLOCA
#  undef USE_GNU_MALLOC
#endif /* ksr1 */

/* ************************ */
/*                          */
/*   Intel Paragon - OSF/1  */
/*                          */
/* ************************ */
#if defined (__i860) && defined (__PARAGON__)
#  define M_MACHINE "Paragon"
#  define M_OS "OSF1"
#  define HAVE_GETGROUPS
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_STRERROR
#  define HAVE_SYS_SIGLIST
#endif /* __i860 && __PARAGON__ */

/* ************************ */
/*                          */
/* IBM AIX/ESA (OSF/1)      */
/*                          */
/* ************************ */
#if defined(AIXESA) || (defined(__ibmesa) && defined(_AIX))
#  define M_MACHINE "IBMESA"
#  define M_OS "OSF1"
#  define HAVE_GETGROUPS
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_STRERROR
#  define HAVE_SYS_SIGLIST
#  define HAVE_ALLOCA_H		/* hack for AIX/ESA, which has malloc.h */
#  undef USE_GNU_MALLOC
#endif /* AIXESA || (__ibmesa && _AIX) */

/* ************************ */
/*                          */
/*   Intel i860  -- SVR4    */
/*                          */
/* ************************ */
#if defined (__i860) && defined (USGr4) && !defined (M_MACHINE)
#  define M_MACHINE "i860"
#  define M_OS "USG"
#  define HAVE_DIRENT
#  define HAVE_SYS_SIGLIST
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  if !defined (HAVE_GCC) && !defined (HAVE_ALLOCA_H)
#    undef HAVE_ALLOCA
#  endif /* !HAVE_GCC && !HAVE_ALLOCA_H */
#  if defined (USGr4_2)
#    define SYSDEP_CFLAGS -DUSGr4 -DUSGr4_2
#  else
#    define SYSDEP_CFLAGS -DUSGr4
#  endif /* ! USGr4_2 */
#  undef HAVE_GETWD
#endif /* __i860 && USGr4 */

/* ************************ */
/*			    */
/*	    Xenix286	    */
/*			    */
/* ************************ */
#if defined (Xenix286)
#  define M_MACHINE "i286"
#  define M_OS "Xenix"

#  define XENIX_CFLAGS -DUSG -DUSGr3 -DMEMMOVE_MISSING

#  if defined (XENIX_22)
#    define XENIX_EXTRA -DSETVBUF_REVERSED
#    define REQUIRED_LIBRARIES -lx
#  else /* !XENIX_22 */
#    define HAVE_DIRENT
#    if defined (XENIX_23)
#      define XENIX_EXTRA -DLD_HAS_NO_DASH_L
#      define REQUIRED_LIBRARIES -ldir
#    else /* !XENIX_23 */
#      define XENIX_EXTRA -xenix
#      define SYSDEP_LDFLAGS -xenix
#      define REQUIRED_LIBRARIES -ldir -l2.3
#    endif /* !XENIX_23 */
#  endif /* !XENIX_22 */

#  define SYSDEP_CFLAGS XENIX_CFLAGS XENIX_EXTRA
#  undef HAVE_ALLOCA
#  undef HAVE_GETWD
#  undef HAVE_RESOURCE
#endif /* Xenix286 */

/* ************************ */
/*			    */
/*	    convex	    */
/*			    */
/* ************************ */
#if defined (convex)
#  define M_MACHINE "convex"
#  define M_OS "Bsd"
#  undef HAVE_ALLOCA
#  define HAVE_SETLINEBUF
#  define HAVE_SYS_SIGLIST
#  define HAVE_GETGROUPS
#endif /* convex */

/* ************************ */
/*                          */
/*          AIX/RT          */
/*                          */
/* ************************ */
#if defined (aix) && !defined (aixpc)
#  define M_MACHINE "AIXRT"
#  define M_OS "USG"
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define HAVE_SYS_SIGLIST
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  define USE_TERMCAP_EMULATION
#  if !defined (HAVE_GCC)
#    undef MACHINE_CFLAGS
#    define MACHINE_CFLAGS -a
#  endif /* !HAVE_GCC */
#  define SYSDEP_CFLAGS MACHINE_CFLAGS -DNLS -DUSGr3 -DHAVE_BCOPY
#  undef USE_GNU_MALLOC
#  undef HAVE_ALLOCA
#  undef HAVE_RESOURCE
#endif /* aix && !aixpc */

/* **************************************** */
/*					    */
/*		IBM RISC 6000		    */
/*					    */
/* **************************************** */
#if defined (RISC6000) || defined (_IBMR2)
#  define M_MACHINE "RISC6000"
#  define M_OS "AIX"
#  define HAVE_DIRENT
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define USE_TERMCAP_EMULATION
#  define HAVE_GETGROUPS
#  define SYSDEP_CFLAGS -DNLS -DUSGr3 -DHAVE_BCOPY
#  undef HAVE_ALLOCA
#  undef HAVE_GETWD
#  undef USE_GNU_MALLOC
#endif /* RISC6000 */

/* **************************************** */
/*					    */
/*	u370 IBM AIX/370		    */
/*					    */
/* **************************************** */
#if defined (u370)
#  if defined (_AIX370)
#    define M_MACHINE "AIX370"
#    define M_OS "Bsd"
#    define REQUIRED_LIBRARIES -lbsd
#    define HAVE_SETLINEBUF
#    define HAVE_VPRINTF
#    define SYSDEP_CFLAGS -D_BSD
#    define HAVE_GETGROUPS
#    define USE_TERMCAP_EMULATION
#    undef USE_GNU_MALLOC
#  endif /* _AIX370 */
#  if defined (USGr4) /* System V Release 4 on 370 series architecture. */
#    define M_MACHINE "uxp"
#    define M_OS "USG"
#    define HAVE_DIRENT
#    define HAVE_SYS_SIGLIST
#    define HAVE_VPRINTF
#    define USE_GNU_MALLOC
#    define VOID_SIGHANDLER
#    if !defined (HAVE_GCC)
#      undef HAVE_ALLOCA
#      define EXTRA_LIB_SEARCH_PATH /usr/ucblib
#      define REQUIRED_LIBRARIES -lc -lucb
#    endif /* !HAVE_GCC */
#    define HAVE_GETGROUPS
#    define HAVE_RESOURCE
#    define SYSDEP_CFLAGS -DUSGr4 -DSBRK_DECLARED
#    endif /* USGr4 */
#endif /* u370 */

/* ************************ */
/*			    */
/*	    ATT 3B	    */
/*			    */
/* ************************ */
#if defined (att3b) || defined (u3b2)
#  if defined (att3b)
#    define M_MACHINE "att3b"
#    define HAVE_SYS_SIGLIST
#  else /* !att3b */
#    define M_MACHINE "u3b2"
#  endif /* !att3b */
#  define M_OS "USG"
#  undef HAVE_GETWD
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
   /* For an AT&T Unix before V.3 take out the -DUSGr3 and the HAVE_DIRENT. */
#  define SYSDEP_CFLAGS -DUSGr3
#  define HAVE_DIRENT
   /* Alloca requires either Gcc or cc with libPW.a. */
#  if !defined (HAVE_GCC)
#    define REQUIRED_LIBRARIES -lPW
#  endif /* !HAVE_GCC */
#endif /* att3b */

/* ************************ */
/*			    */
/*	    ATT 386	    */
/*			    */
/* ************************ */
#if defined (att386)
#  define M_MACHINE "att386"
#  define M_OS "USG"
#  undef HAVE_GETWD
   /* Alloca requires either Gcc or cc with libPW.a. */
#  if !defined (HAVE_GCC)
#    define REQUIRED_LIBRARIES -lPW
#  endif /* HAVE_GCC */
#  define HAVE_SYS_SIGLIST
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
   /* For an AT&T Unix before V.3 take out the -DUSGr3 and the HAVE_DIRENT. */
#  define SYSDEP_CFLAGS -DUSGr3
#  define HAVE_DIRENT
#endif /* att386 */

/* ************************ */
/*			    */
/*	 ATT UNIX PC	    */
/*			    */
/* ************************ */
#if defined (unixpc)
#  define M_MACHINE "unixpc"
#  define M_OS "USG"
#  define HAVE_VPRINTF
#  define HAVE_DIRENT
#  if defined (HAVE_GCC)
#    define REQUIRED_LIBRARIES -ldirent -shlib
#  else /* !HAVE_GCC */
#    define REQUIRED_LIBRARIES -ldirent
#  endif /* !HAVE_GCC */
#  undef HAVE_GETWD
#  undef HAVE_DUP2
#  undef VOID_SIGHANDLER
#  undef HAVE_WAIT_H
#endif /* unixpc */

/* ************************ */
/*			    */
/*	    Encore	    */
/*			    */
/* ************************ */
#if defined (MULTIMAX)
#  if defined (n16)
#    define M_MACHINE "Multimax32k"
#  else
#    define M_MACHINE "Multimax"
#  endif /* n16 */
#  if defined (UMAXV)
#    define M_OS "USG"
#    define REQUIRED_LIBRARIES -lPW
#    define SYSDEP_CFLAGS -DUSGr3
#    define HAVE_DIRENT
#    define HAVE_VPRINTF
#    define USE_TERMCAP_EMULATION
#    define VOID_SIGHANDLER
#  else
#    if defined (CMU)
#      define M_OS "Mach"
#    else
#      define M_OS "Bsd"
#    endif /* CMU */
#    define HAVE_SYS_SIGLIST
#    define HAVE_STRERROR
#    define HAVE_SETLINEBUF
#  endif /* UMAXV */
#  define HAVE_GETGROUPS
#endif  /* MULTIMAX */

/* ******************************************** */
/*						*/
/*   Encore Series 91 (88K BCS w Job Control)	*/
/*						*/
/* ******************************************** */
#if defined (__m88k) && defined (__UMAXV__)
#  define M_MACHINE "Gemini"
#  define M_OS "USG"
#  define REQUIRED_LIBRARIES -lPW
#  define USE_TERMCAP_EMULATION
#  define HAVE_DIRENT
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define SYSDEP_CFLAGS -q ext=pcc -D_POSIX_JOB_CONTROL -D_POSIX_VERSION \
			-Dmalloc=_malloc -Dfree=_free -Drealloc=_realloc
#endif  /* m88k && __UMAXV__ */

/* ******************************************** */
/*						*/
/*    System V Release 4 on the ICL DRS6000     */
/*						*/
/* ******************************************** */
#if defined (drs6000)
#  define M_MACHINE "drs6000"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -Xa -DUSGr4
#  define SEARCH_LIB_NEEDS_SPACE
#  define HAVE_DIRENT
#  define HAVE_SYS_SIGLIST
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  define USE_GNU_TERMCAP
#  if !defined (__GNUC__)
#    undef HAVE_ALLOCA
#  endif
#  undef HAVE_ALLOCA_H
#  undef USE_GNU_MALLOC
#endif /* drs6000 */

/* ******************************************** */
/*						*/
/*   System V Release 4 on the Sparc (generic)  */
/*						*/
/* ******************************************** */
#if defined (sparc) && defined (__svr4__) && !defined (M_MACHINE)
#  define M_MACHINE "sparc"
#  define M_OS "SVR4"
#  define SYSDEP_CFLAGS -DUSG -DUSGr4 -DHAVE_UID_T
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  define USE_GNU_TERMCAP
#  if !defined (__GNUC__)
#    undef HAVE_ALLOCA
#  endif
#  undef HAVE_BCOPY
#  undef HAVE_GETWD
#  undef USE_GNU_MALLOC
#endif /* sparc && __svr4__ */

/* ******************* */
/*		       */
/*   Commodore Amiga   */
/*		       */
/* ******************* */
#if defined (amiga) && defined (__NetBSD__)
#  define M_MACHINE "amiga"
#  define M_OS "NetBSD"
#  define SYSDEP_CFLAGS -DOPENDIR_NOT_ROBUST -DINT_GROUPS_ARRAY \
			-DRLIMTYPE=quad_t
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  define HAVE_DIRENT
#  define HAVE_STRCASECMP
#endif /* amiga && __NetBSD__ */

#if defined (amiga) && !defined (M_MACHINE)
#  define M_MACHINE "amiga"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -DUSGr4
#  if !defined (HAVE_GCC)
#    define EXTRA_LIB_SEARCH_PATH /usr/ucblib
#    define REQUIRED_LIBRARIES -lc -lucb
#  endif /* !HAVE_GCC */
#  define HAVE_DIRENT
#  define HAVE_SYS_SIGLIST
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  undef HAVE_GETWD
#  undef USE_GNU_MALLOC
#endif /* System V Release 4 on amiga */

/* ************************ */
/*			    */
/*	    clipper	    */
/*			    */
/* ************************ */
/* This is for the Orion 1/05 (A BSD 4.2 box based on a Clipper processor) */
#if defined (clipper) && !defined (M_MACHINE)
#  define M_MACHINE "clipper"
#  define M_OS "Bsd"
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif  /* clipper */

/* ******************************** */
/*				    */
/*    Integrated Solutions 68020?   */
/*				    */
/* ******************************** */
#if defined (is68k)
#  define M_MACHINE "is68k"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define USE_VFPRINTF_EMULATION
#  undef HAVE_ALLOCA
#endif /* is68k */

/* ******************************** */
/*				    */
/*	   Omron Luna/Mach 2.5	    */
/*				    */
/* ******************************** */
#if defined (luna88k)
#  define M_MACHINE "Luna88k"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define USE_GNU_MALLOC
#  define HAVE_SETLINEBUF
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS
#  define HAVE_VPRINTF
#endif /* luna88k */

/* ************************ */
/*			    */
/*   BBN Butterfly GP1000   */
/*   Mach 1000 v2.5	    */
/*			    */
/* ************************ */
#if defined (butterfly) && defined (BFLY1)
#define M_MACHINE "BBN Butterfly"
#define M_OS "Mach 1000"
#define HAVE_SETLINEBUF
#define HAVE_SYS_SIGLIST
#define HAVE_GETGROUPS
#define HAVE_VPRINTF
#  ifdef BUILDING_MAKEFILE
MAKE = make
#  endif /* BUILDING_MAKEFILE */
#endif /* butterfly */

/* **************************************** */
/*					    */
/*	    Apollo/SR10.2/BSD4.3	    */
/*					    */
/* **************************************** */
/* This is for the Apollo DN3500 running SR10.2 BSD4.3 */
#if defined (apollo)
#  define M_MACHINE "apollo"
#  define M_OS "Bsd"
#  define SYSDEP_CFLAGS -D_POSIX_VERSION -D_INCLUDE_BSD_SOURCE \
			-D_INCLUDE_POSIX_SOURCE -DTERMIOS_MISSING \
			-DHAVE_BSD_PGRP -Dpid_t=int
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* apollo */

/* ************************ */
/*			    */
/*	DG AViiON	    */
/*			    */
/* ************************ */
/* This is for the DG AViiON box (runs DG/UX with both AT&T & BSD features.) */
/* DG/UX comes standard with Gcc. */
#if defined (__DGUX__) || defined (DGUX)
#  define M_OS "DGUX"
#  if !defined (_M88KBCS_TARGET)
#    define M_MACHINE "AViiON"
#    define REQUIRED_LIBRARIES -ldgc
#  else /* _M88KBCS_TARGET */
#    define M_MACHINE "m88kBCS_AV"
#    undef MACHINE_CFLAGS
#    define MACHINE_CFLAGS -D_M88K_SOURCE
#    undef HAVE_RESOURCE
#  endif /* _M88KBCS_TARGET */
#  define SYSDEP_CFLAGS MACHINE_CFLAGS -D_DGUX_SOURCE -DPGRP_PIPE -DUSG
#  define HAVE_GCC
#  define HAVE_FIXED_INCLUDES
#  define HAVE_STRERROR
#  define HAVE_GETGROUPS
#  define VOID_SIGHANDLER
#  undef HAVE_GETWD
#  undef USE_GNU_MALLOC

/* If you want to build bash for M88K BCS compliance on a DG/UX 5.4
   or above system, do the following:
     - If you have built in this directory before run "make clean" to
       endure the Bash directory is clean.
     - Run "eval `sde-target m88kbcs`" to set the software development
       environment to build BCS objects.
     - Run "make".
     - Do "eval `sde-target default`" to reset the SDE. */
#endif /* __DGUX__ */

/* ************************ */
/*			    */
/*    Harris Night Hawk	    */
/*			    */
/* ************************ */
/* This is for the Harris Night Hawk family. */
#if defined (_CX_UX)
#  if defined (_M88K)
#    define M_MACHINE "nh4000"
#  else /* !_M88K */
#    if defined (hcx)
#      define M_MACHINE "nh2000"
#    else /* !hcx */
#      if defined (gcx)
#        define M_MACHINE "nh3000"
#      endif /* gcx */
#    endif /* !hcx */
#  endif /* !_M88K */
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -g -Xa -v -Dgetwd=bash_getwd -D_POSIX_SOURCE \
			-D_POSIX_JOB_CONTROL
#  define USE_TERMCAP_EMULATION
#  define HAVE_VPRINTF
#  define HAVE_GETGROUPS
#  define VOID_SIGHANDLER
#  undef USE_GNU_MALLOC
#  undef HAVE_GETWD
#endif /* _CX_UX */

/* **************************************** */
/*					    */
/*	    	Tektronix	    	    */
/*					    */
/* **************************************** */
/* These are unproven as yet. */
#if defined (Tek4132)
#  define M_MACHINE "Tek4132"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* Tek4132 */

#if defined (Tek4300)
#  define M_MACHINE "Tek4300"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#endif /* Tek4300 */

/* ************************ */
/*                          */
/*     Tektronix XD88       */
/*                          */
/* ************************ */
#if defined (m88k) && defined (XD88)
#  define M_MACHINE "XD88"
#  define M_OS "USG"
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define HAVE_GETCWD
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  undef HAVE_GETWD
#  undef HAVE_ALLOCA
#endif /* m88k && XD88 */

/* ************************ */
/*                          */
/*     Motorola M88100      */
/*                          */
/* ************************ */
#if defined (m88k) && (defined (M88100) || defined (USGr4))
#  define M_MACHINE "M88100"
#  define M_OS "USG"
#  if defined (USGr4)
#    define SYSDEP_CFLAGS -DUSGr4 -D_POSIX_JOB_CONTROL
#  else
#    define SYSDEP_CFLAGS -D_POSIX_JOB_CONTROL
#  endif
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define HAVE_GETGROUPS
#  undef HAVE_GETWD
#  if !defined (USGr4)
#    undef HAVE_GETCWD
#  endif
#  undef HAVE_ALLOCA
#endif /* m88k && M88100 */

/* ************************ */
/*			    */
/*     Sequent Balances     */
/*       (Dynix 3.x)	    */
/* ************************ */
#if defined (sequent) && !defined (M_MACHINE)
#  define M_MACHINE "Sequent"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_SETLINEBUF
#  define HAVE_GETGROUPS
#  define LD_HAS_NO_DASH_L
#  undef HAVE_DUP2
#endif /* sequent */

/* ****************************************** */
/*					      */
/*    NCR Tower 32, System V Release 3	      */
/*					      */
/* ****************************************** */
#if defined (tower32)
#  define M_MACHINE "tower32"
#  define M_OS "USG"
#  if !defined (HAVE_GCC)
#    define REQUIRED_LIBRARIES -lPW
     /* Disable stack/frame-pointer optimization, incompatible with alloca */
#    undef MACHINE_CFLAGS
#    define MACHINE_CFLAGS -W2,-aat
#  endif /* !HAVE_GCC */
#  define SYSDEP_CFLAGS -DUSGr3 MACHINE_CFLAGS
#  define HAVE_VPRINTF
#  define USE_TERMCAP_EMULATION
#  define VOID_SIGHANDLER
#  undef HAVE_GETWD
#endif /* tower32 */

/* ************************ */
/*			    */
/*	Concurrent	    */
/*			    */
/* ************************ */
#if defined (concurrent)
#  define M_MACHINE "Concurrent"
#  if defined (USE_BSD_UNIVERSE)
     /* Use the BSD universe (`universe ucb') */
#    define M_OS "Bsd"
#    define HAVE_SYS_SIGLIST
#    define HAVE_SETLINEBUF
#    define HAVE_GETGROUPS
#  else /* !USE_BSD_UNIVERSE */
   /* Concurrent 7000 with RTU 6.1A using the ATT universe (`universe att') */
#    define M_OS "USG"
#    define SYSDEP_CFLAGS -DHAVE_BCOPY -DHAVE_UID_T -DHAVE_GETDTABLESIZE -Dmc7000
#    define REQUIRED_LIBRARIES -ljobs
#    define HAVE_VPRINTF
#    define HAVE_GETGROUPS
#    define HAVE_DUP2
#    define HAVE_DIRENT
#    define HAVE_SYS_SIGLIST
#  endif /* !USE_BSD_UNIVERSE */
#endif /* concurrent */

/* **************************************************************** */
/*                                                                  */
/*             Honeywell Bull X20 (lele@idea.sublink.org)	    */
/*                                                                  */
/* **************************************************************** */
#if defined (hbullx20)
#  define M_MACHINE "Honeywell"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -DUSG
   /* Bull x20 needs -lposix for struct dirent. */
#  define REQUIRED_LIBRARIES -lPW -lposix
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define USE_TERMCAP_EMULATION
#  undef HAVE_GETWD
#endif  /* hbullx20 */

/* **************************************************************** */
/*                                                                  */
/*             Bull DPX2 (F.Pierresteguy@frcl.bull.fr)              */
/*                                                                  */
/* **************************************************************** */
#if defined (DPX2)
#  define M_MACHINE "DPX2"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -DUSG -DUSGr3 -DHAVE_BCOPY -DHAVE_UID_T -D_POSIX_JOB_CONTROL
/* #  define REQUIRED_LIBRARIES -lPW -lposix */
#  define HAVE_DIRENT
#  define VOID_SIGHANDLER
#  define HAVE_POSIX_SIGNALS
#  define HAVE_VPRINTF
#  define HAVE_SETVBUF
#  define HAVE_STRERROR
#  define HAVE_GETGROUPS
#  define HAVE_GETDTABLESIZE
#  define USE_TERMCAP_EMULATION
#endif  /* DPX2 */

/* ************************ */
/*			    */
/*	    CRAY	    */
/*			    */
/* ************************ */
#if defined (cray)
#  include <sys/param.h>
#  if defined (Cray1) || defined (Cray2)
#    define M_MACHINE "Cray"
#    define CRAY_STACK
#  endif
#  if defined (CrayXMP) && !defined (M_MACHINE)
#    define M_MACHINE "CrayXMP"
#    define CRAY_STACK -DCRAY_STACKSEG_END=getb67
#  endif
#  if defined (CrayYMP) && !defined (M_MACHINE)
#    define M_MACHINE "CrayYMP"
#    if RELEASE_LEVEL >= 7000
#      define CRAY_STACK -DCRAY_STACKSEG_END=_getb67
#    else
#      define CRAY_STACK -DCRAY_STACKSEG_END=getb67
#    endif /* RELEASE_LEVEL < 7000 */
#  endif
#  if !defined (M_MACHINE)
#    define M_MACHINE "Cray"
#    define CRAY_STACK
#  endif
#  define M_OS "Unicos"
#  define SYSDEP_CFLAGS -DUSG -DPGRP_PIPE -DOPENDIR_NOT_ROBUST \
			-DHAVE_BCOPY CRAY_STACK
#  define HAVE_VPRINTF
#  define HAVE_MULTIPLE_GROUPS
#  define VOID_SIGHANDLER
#  define USE_TERMCAP_EMULATION
#  undef HAVE_ALLOCA
#  undef HAVE_RESOURCE
#  undef USE_GNU_MALLOC
#endif /* cray */

/* ************************ */
/*			    */
/*	MagicStation	    */
/*			    */
/* ************************ */
#if defined (MagicStation)
#  define M_MACHINE "MagicStation"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -DUSGr4
#  define HAVE_DIRENT
#  define HAVE_GETGROUPS
#  define HAVE_STRERROR
#  define VOID_SIGHANDLER
#  undef HAVE_ALLOCA
#  undef HAVE_GETWD
#endif /* MagicStation */

/* ************************ */
/*			    */
/*	   Plexus	    */
/*			    */
/* ************************ */
#if defined (plexus)
#  define M_MACHINE "plexus"
#  define M_OS "USG"
#  define REQUIRED_LIBRARIES -lndir
#  define USE_TERMCAP_EMULATION
#  undef HAVE_DUP2
#  undef HAVE_GETWD
#  define HAVE_VPRINTF
#  undef HAVE_ALLOCA		/* -lPW doesn't work w/bash-cc? */
#endif /* plexus */

/* ************************ */
/*			    */   
/*     Siemens MX500        */
/*      (SINIX 5.2x)	    */
/* ************************ */
#if defined (sinix)
#define M_MACHINE "Siemens MX500"
#define M_OS "SINIX V5.2x"
#define USG
#define HAVE_GETCWD
#define VOID_SIGHANDLER
#define HAVE_STRERROR
#define HAVE_GETGROUPS
#define HAVE_VPRINTF
#define HAVE_POSIX_SIGNALS
#define HAVE_RESOURCE
#define USE_GNU_MALLOC
#define SYSDEP_CFLAGS -DUSGr3 -DUSG
#define REQUIRED_LIBRARIES syscalls.o
#undef HAVE_ALLOCA
#undef HAVE_GETWD
#endif /* sinix */

/* ************************ */
/*			    */
/*  Symmetric 375 (4.2 BSD) */
/*			    */
/* ************************ */
#if defined (scs) && !defined (M_MACHINE)
#  define M_MACHINE "Symmetric_375"
#  define M_OS "Bsd"
#  define HAVE_SYS_SIGLIST
#  define HAVE_GETGROUPS
#  define HAVE_SETLINEBUF
#  define USE_VFPRINTF_EMULATION
#  define USE_GNU_MALLOC
#  undef HAVE_STRCHR
#endif /* scs */

/* ************************ */
/*			    */
/*    Tandem running SVR3   */
/*			    */
/* ************************ */
#if defined (tandem) && !defined (M_MACHINE)
#  define M_MACHINE "tandem"
#  define M_OS "USG"
#  define SYSDEP_CFLAGS -DUSGr3
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
   /* Alloca requires either Gcc or cc with libPW.a */
#  if !defined (HAVE_GCC)
#    define REQUIRED_LIBRARIES -lPW
#  endif /* !HAVE_GCC */
#  undef HAVE_GETWD
#endif /* Tandem running SVR3 */

/* ****************** */
/*                    */
/*     Amdahl UTS     */
/*                    */
/* ****************** */

#if defined (UTS)
#  define M_MACHINE "uts"
#  define M_OS "systemV"
#  define SYSDEP_CFLAGS -DUSG -DMEMMOVE_MISSING
#  define REQUIRED_LIBRARIES
#  undef HAVE_SYS_SIGLIST
#  undef HAVE_GETWD
#  undef HAVE_ALLOCA
#  define HAVE_VPRINTF
#  define HAVE_DIRENT
#  undef HAVE_RESOURCE
#endif  /* UTS */

/* ************************ */
/*			    */
/*    PCS Cadmus System	    */
/*			    */
/* ************************ */
#if defined (cadmus) && !defined (M_MACHINE)
#  define M_MACHINE "cadmus"
#  define M_OS "BrainDeath"
#  define SYSDEP_CFLAGS -DUSG
#  define HAVE_DIRENT
#  define HAVE_VPRINTF
#  define VOID_SIGHANDLER
#  define USE_TERMCAP_EMULATION
#  undef HAVE_GETWD
#  undef HAVE_ALLOCA
#  undef HAVE_WAIT_H
#endif /* cadmus */

/* **************************************************************** */
/*								    */
/*			Generic Entry   			    */
/*								    */
/* **************************************************************** */

/* Use this entry for your machine if it isn't represented here.  It
   is loosely based on a Vax running 4.3 BSD. */

#if !defined (M_MACHINE)
#  define UNKNOWN_MACHINE
#endif

#if defined (UNKNOWN_MACHINE)
#  define M_MACHINE "UNKNOWN_MACHINE"
#  define M_OS "UNKNOWN_OS"

/* Required libraries for building on this system. */
#  define REQUIRED_LIBRARIES

/* Define HAVE_SYS_SIGLIST if your system has sys_siglist[]. */
#  define HAVE_SYS_SIGLIST

/* Undef HAVE_GETWD if your C library does not provide a working version
   of getwd(). */
/* #  undef HAVE_GETWD */

/* Undef HAVE_GETCWD if your C library does not provide a working version
   of getcwd(). */
/* #  undef HAVE_GETCWD */

/* Undef HAVE_ALLOCA if you are not using Gcc, and neither your library
   nor compiler has a version of alloca ().  In that case, we will use
   our version of alloca () in alloca.c */
/* #  undef HAVE_ALLOCA */

/* Undef USE_GNU_MALLOC if there appear to be library conflicts, or if you
   especially desire to use your OS's version of malloc () and friends.  We
   reccommend against this because GNU Malloc has debugging code built in. */
/* #  undef USE_GNU_MALLOC */

/* Define USE_GNU_TERMCAP if you want to use the GNU termcap library
   instead of your system termcap library. */
/* #  define USE_GNU_TERMCAP */

/* Define HAVE_SETLINEBUF if your machine has the setlinebuf ()
   stream library call.  Otherwise, setvbuf () will be used.  If
   neither of them work, you can edit in your own buffer control
   based upon your machines capabilities. */
#  define HAVE_SETLINEBUF

/* Define HAVE_VPRINTF if your machines has the vfprintf () library
   call.  Otherwise, printf will be used.  */
#  define HAVE_VPRINTF

/* Define USE_VFPRINTF_EMULATION if you want to use the BSD-compatible
   vfprintf() emulation in vprint.c. */
/* #  define USE_VFPRINTF_EMULATION */

/* Define HAVE_GETGROUPS if your OS allows you to be in multiple
   groups simultaneously by supporting the `getgroups' system call. */
#  define HAVE_GETGROUPS

/* Define SYSDEP_CFLAGS to be the flags to cc that make your compiler
   work.  For example, `-ma' on the RT makes alloca () work. */
/* This is a summary of the semi-machine-independent definitions that
   can go into SYSDEP_CFLAGS:

	AFS	-	The Andrew File System is being used
	AFS_CREATE_BUG - AFS has a bug with file creation if O_CREAT is
			 specified
	BROKEN_SIGSUSPEND - sigsuspend(2) does not work to wake up processes
			    on SIGCHLD
	HAVE_BSD_PGRP -	getpgrp(2) takes a pid argument, a la 4.3 BSD
	HAVE_BCOPY -	bcopy(3) exists and works as in BSD
	HAVE_GETDTABLESIZE - getdtablesize(2) exists and works correctly
	HAVE_GETHOSTNAME - gethostname(2) or gethostname(3) is present and
			   works as in BSD
	HAVE_GETPW_DECLS - USG machines with the getpw* functions defined in
			   <pwd.h> that cannot handle redefinitions in the
			   bash source
	HAVE_RESOURCE -	<sys/resource.h> and [gs]rlimit exist and work
	HAVE_SETDTABLESIZE - setdtablesize(2) exists and works correctly
	HAVE_SOCKETS - this system has BSD sockets added to a System V base
	HAVE_UID_T -	Definitions for uid_t and gid_t are in <sys/types.h>
	INT_GROUPS_ARRAY - the second argument to getgroups(3) is an array
			   of integers
	MEMMOVE_MISSING - the system does not have memmove(3)
	MKFIFO_MISSING - named pipes do not work or mkfifo(3) is missing
	NO_DEV_TTY_JOB_CONTROL - system can't do job control on /dev/tty
	SBRK_DECLARED - don't declare sbrk as extern char *sbrk() in
		       lib/malloc/malloc.c
	OPENDIR_NOT_ROBUST - opendir(3) allows you to open non-directory files
	PGRP_PIPE -	Requires parent-child synchronization via pipes to
			make job control work right
	SETVBUF_REVERSED - brain-damaged implementation of setvbuf that
				has args 2 and 3 reversed from the SVID and
				ANSI standard
	RLOGIN_PGRP_BUG - processes started by rlogind have a process group
			  of 0
     	TERMIOS_LDISC - system has a c_line line discipline member in struct
			termios
	TERMIOS_MISSING - the termios(3) functions are not present or don't
			  work, even though _POSIX_VERSION is defined
	USG	-	The machine is running some sort of System V Unix
	USGr3	-	The machine is running SVR3.x
	USGr4	-	The machine is running SVR4
	USGr4_2 -	The machine is running SVR4.2

	Some other machine-dependent options:
	CPCC	-	Dynix
	NLS	-	Alpha, AIX
	_BSD	- 	Alpha
	_DGUX_SOURCE -	DG/UX
	_INCLUDE_BSD_SOURCE - Apollo
	_INCLUDE_POSIX_SOURCE - Apollo

	Posix.1 options that are normally set in <unistd.h>:
	_POSIX_VERSION -	Encore, Apollo
	_POSIX_SOURCE -	ISC, QNX, Harris
	_POSIX_JOB_CONTROL - NEC EWS, Encore, Harris, Motorola, Bull
*/
#  define SYSDEP_CFLAGS

/* Define HAVE_STRERROR if your system supplies a definition for strerror ()
   in the C library, or a macro in a header file. */
/* #  define HAVE_STRERROR */

/* Define HAVE_STRCASECMP if your system supplies definitions for the
   casel-insensitive string comparison functions strcasecmp and strncasemp
   in the C library or one of the system header files. */
/* #  define HAVE_STRCASECMP */

/* Define HAVE_DIRENT if you have the dirent library and a definition of
   struct dirent.  If not, the BSD directory reading library and struct
   direct are assumed. */
/* #  define HAVE_DIRENT */

/* If your system does not supply /usr/lib/libtermcap.a, but includes
   the termcap routines as a part of the curses library, then define
   this.  This is the case on some System V machines. */
/* #  define USE_TERMCAP_EMULATION */

/* Define VOID_SIGHANDLER if your system's signal () returns a pointer to
   a function returning void. */
/* #  define VOID_SIGHANDLER */

/* Define EXTRA_LIB_SEARCH_PATH if your required libraries (or standard)
   ones for that matter) are not normally in the ld search path.  For
   example, some machines require /usr/ucblib in the ld search path so
   that they can use -lucb. */
/* #  define EXTRA_LIB_SEARCH_PATH /usr/ucblib */

/* Define SEARCH_LIB_NEEDS_SPACE if your native ld requires a space after
   the -L argument, which gives the name of an alternate directory to search
   for libraries specified with -llib.  For example, the HPUX ld requires
   this:
   	-L lib/readline -lreadline
   instead of:
   	-Llib/readline -lreadline
 */
/* #  define SEARCH_LIB_NEEDS_SPACE */

/* Define LD_HAS_NO_DASH_L if your ld can't grok the -L flag in any way, or
   if it cannot grok the -l<lib> flag, or both. */
/* #  define LD_HAS_NO_DASH_L */

/* Define GCC_STANDARD if the standard `cc' is gcc and you don't want
   to use the compiler named `gcc' for some reason. */
/* #  define GCC_STANDARD */
   
#  if defined (LD_HAS_NO_DASH_L)
#   undef SEARCH_LIB_NEEDS_SPACE
#  endif /* LD_HAS_NO_DASH_L */

#endif  /* UNKNOWN_MACHINE */
