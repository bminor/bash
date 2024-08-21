/* unwind_prot.h - Macros and functions for hacking unwind protection. */

/* Copyright (C) 1993-2023 Free Software Foundation, Inc.

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

#if !defined (_UNWIND_PROT_H)
#define _UNWIND_PROT_H

/* Generic function type void (*) (void *) for all unwind cleanups. A
   cleanup function does not return a value and takes a single generic
   pointer argument. This type works fine for arbitrary pointers; if a
   cleanup function needs to take an int argument, it's passed through
   a cast to intptr_t, an integer type that's safe to convert to and
   from a pointer. */
typedef void sh_uwfunc_t (void *);

extern void uwp_init (void);

/* Run a function without interrupts. */
extern void begin_unwind_frame (char *);
extern void discard_unwind_frame (char *);
extern void run_unwind_frame (char *);
extern void add_unwind_protect (sh_uwfunc_t *, void *);
extern void remove_unwind_protect (void);
extern void run_unwind_protects (void);
extern void clear_unwind_protect_list (int);
extern int have_unwind_protects (void);
extern int unwind_protect_tag_on_stack (const char *);

/* Define for people who like their code to look a certain way. */
#define end_unwind_frame()

/* How to protect a variable.  */
#define unwind_protect_var(X) unwind_protect_mem ((char *)&(X), sizeof (X))

extern void unwind_protect_mem (char *, int);

/* Backwards compatibility */
#define unwind_protect_int	unwind_protect_var
#define unwind_protect_short	unwind_protect_var
#define unwind_protect_string	unwind_protect_var
#define unwind_protect_pointer	unwind_protect_var
#define unwind_protect_jmp_buf	unwind_protect_var

#endif /* _UNWIND_PROT_H */
