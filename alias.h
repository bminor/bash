/* alias.h -- structure definitions. */

/* Copyright (C) 1987,1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 1, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#if !defined (_ALIAS_)
#define _ALIAS_

#include "hash.h"

extern char *xmalloc ();

#if !defined (whitespace)
#  define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif /* !whitespace */

#if !defined (savestring)
#  define savestring(x) (char *)strcpy (xmalloc (1 + strlen (x)), (x))
#endif /* !savestring */

#if !defined (NULL)
#  if defined (__STDC__)
#    define NULL ((void *) 0)
#  else
#    define NULL 0x0
#  endif /* !__STDC__ */
#endif /* !NULL */

typedef struct {
  char *name;
  char *value;
} ASSOC;

/* The list of known aliases. */
extern HASH_TABLE *aliases;

extern void initialize_aliases ();

/* Scan the list of aliases looking for one with NAME.  Return NULL
   if the alias doesn't exist, else a pointer to the assoc. */
extern ASSOC *find_alias ();

/* Return the value of the alias for NAME, or NULL if there is none. */
extern char *get_alias_value ();

/* Make a new alias from NAME and VALUE.  If NAME can be found,
   then replace its value. */
extern void add_alias ();

/* Remove the alias with name NAME from the alias list.  Returns
   the index of the removed alias, or -1 if the alias didn't exist. */
extern int remove_alias ();

/* Return a new line, with any aliases expanded. */
extern char *alias_expand ();

/* Return an array of all defined aliases. */
extern ASSOC **all_aliases ();

#endif /* _ALIAS_ */
