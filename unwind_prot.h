/* unwind_prot.h - Macros and functions for hacking unwind protection. */

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
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#if !defined (_UNWIND_PROT_H)
#define _UNWIND_PROT_H

/* Run a function without interrupts. */
extern void begin_unwind_frame ();
extern void discard_unwind_frame ();
extern void run_unwind_frame ();
extern void add_unwind_protect ();
extern void remove_unwind_protect ();
extern void run_unwind_protects ();
extern void unwind_protect_var ();
extern void clear_unwind_protect_list ();

/* Try to force correct alignment on machines where pointers and ints
   differ in size. */
typedef union {
  char *s;
  int i;
} UWP;

/* Define for people who like their code to look a certain way. */
#define end_unwind_frame()

/* How to protect an integer. */
#define unwind_protect_int(X) \
	do \
	  { \
	    UWP u; \
	    u.i = (X); \
	    unwind_protect_var (&(X), u.s, sizeof (int)); \
	  } \
	while (0)

#define unwind_protect_short(X) \
  unwind_protect_var ((int *)&(X), (char *)&(X), sizeof (short))

/* How to protect a pointer to a string. */
#define unwind_protect_string(X) \
  unwind_protect_var ((int *)&(X), \
		      ((sizeof (char *) == sizeof (int)) ? (char *) (X) : (char *) &(X)), \
		       sizeof (char *))

/* How to protect any old pointer. */
#define unwind_protect_pointer(X) unwind_protect_string (X)

/* How to protect the contents of a jmp_buf. */
#define unwind_protect_jmp_buf(X) \
  unwind_protect_var ((int *)(X), (char *)(X), sizeof (procenv_t))

#endif /* _UNWIND_PROT_H */
