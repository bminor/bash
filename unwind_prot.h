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
   Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#if !defined (_UNWIND_PROT_H)
#define _UNWIND_PROT_H

/* Run a function without interrupts. */
void
  begin_unwind_frame (), discard_unwind_frame (),
  run_unwind_frame (), add_unwind_protect (), remove_unwind_protect (),
  run_unwind_protects ();

/* Define for people who like their code to look a certain way. */
#define end_unwind_frame()

/* How to protect an integer. */
#define unwind_protect_int(X) unwind_protect_var (&(X), (char *)(X), sizeof (int))

/* How to protect a pointer to a string. */
#define unwind_protect_string(X) \
  unwind_protect_var ((int *)&(X), (X), sizeof (char *))

/* How to protect any old pointer. */
#define unwind_protect_pointer(X) unwind_protect_string (X)

/* How to protect the contents of a jmp_buf. */
#define unwind_protect_jmp_buf(X) \
  unwind_protect_var ((int *)(X), (char *)(X), sizeof (jmp_buf))

#endif /* _UNWIND_PROT_H */
