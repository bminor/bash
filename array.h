/* array.h -- definitions for the interface exported by array.c that allows
   the rest of the shell to manipulate array variables. */

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
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "stdc.h"

typedef int	arrayind_t;

enum atype {array_indexed, array_assoc};

typedef struct array {
	enum atype	type;
	arrayind_t	max_index, num_elements, max_size;
	struct array_element *head;
} ARRAY;

typedef struct array_element {
	arrayind_t	ind;
	char	*value;
	struct array_element *next, *prev;
} ARRAY_ELEMENT;

char	*array_reference __P((ARRAY *, arrayind_t));

extern int	array_add_element __P((ARRAY *, arrayind_t, char *));
extern ARRAY_ELEMENT *array_delete_element __P((ARRAY *, arrayind_t));

extern ARRAY_ELEMENT *new_array_element __P((arrayind_t, char *));
extern void	destroy_array_element __P((ARRAY_ELEMENT *));

extern ARRAY	*new_array __P((void));
extern void	empty_array __P((ARRAY *));
extern void	dispose_array __P((ARRAY *));
extern ARRAY	*dup_array __P((ARRAY *));
extern ARRAY	*dup_array_subrange __P((ARRAY *, ARRAY_ELEMENT *, ARRAY_ELEMENT *));
extern ARRAY_ELEMENT *new_array_element __P((arrayind_t, char *));
extern ARRAY_ELEMENT *copy_array_element __P((ARRAY_ELEMENT *));

extern WORD_LIST *array_to_word_list __P((ARRAY *));
extern ARRAY *word_list_to_array __P((WORD_LIST *));
extern ARRAY *assign_word_list __P((ARRAY *, WORD_LIST *));

extern char **array_to_argv __P((ARRAY *));

extern char *array_to_assignment_string __P((ARRAY *));
extern char *quoted_array_assignment_string __P((ARRAY *));
extern char *array_to_string __P((ARRAY *, char *, int));
extern ARRAY *string_to_array __P((char *, char *));

extern char *array_subrange __P((ARRAY *, int, int, int));
extern char *array_pat_subst __P((ARRAY *, char *, char *, int));

extern ARRAY *array_quote __P((ARRAY *));

#define array_num_elements(a)	((a)->num_elements)
#define array_max_index(a)	((a)->max_index)
#define array_head(a)		((a)->head)
#define array_empty(a)		((a)->num_elements == 0)

#define element_value(ae)	((ae)->value)
#define element_index(ae)	((ae)->ind)
#define element_forw(ae)	((ae)->next)
#define element_back(ae)	((ae)->prev)

#define ALL_ELEMENT_SUB(c)	((c) == '@' || (c) == '*')

#endif /* _ARRAY_H_ */
