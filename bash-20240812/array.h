/* array.h -- definitions for the interface exported by array.c that allows
   the rest of the shell to manipulate array variables. */

/* Copyright (C) 1997-2022 Free Software Foundation, Inc.

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


#ifndef _ARRAY_H_
#define _ARRAY_H_

#include "stdc.h"

typedef intmax_t	arrayind_t;

typedef struct array {
	arrayind_t	max_index;
	arrayind_t	num_elements;
#ifdef ALT_ARRAY_IMPLEMENTATION
	arrayind_t	first_index;
	arrayind_t	alloc_size;
	struct array_element **elements;
#else
	struct array_element *head;
	struct array_element *lastref;
#endif
} ARRAY;

typedef struct array_element {
	arrayind_t	ind;
	char	*value;
#ifndef ALT_ARRAY_IMPLEMENTATION
	struct array_element *next, *prev;
#endif
} ARRAY_ELEMENT;

#define ARRAY_DEFAULT_SIZE	1024

typedef int sh_ae_map_func_t (ARRAY_ELEMENT *, void *);

/* Basic operations on entire arrays */
#ifdef ALT_ARRAY_IMPLEMENTATION
extern void   array_alloc (ARRAY *, arrayind_t);
extern void   array_resize (ARRAY *, arrayind_t);
extern void   array_expand (ARRAY *, arrayind_t);
extern void   array_dispose_elements (ARRAY_ELEMENT **);
#endif
extern ARRAY	*array_create (void);
extern void	array_flush (ARRAY *);
extern void	array_dispose (ARRAY *);
extern ARRAY	*array_copy (ARRAY *);
#ifndef ALT_ARRAY_IMPLEMENTATION
extern ARRAY	*array_slice (ARRAY *, ARRAY_ELEMENT *, ARRAY_ELEMENT *);
#else
extern ARRAY	*array_slice (ARRAY *, arrayind_t, arrayind_t);
#endif

extern void	array_walk (ARRAY   *, sh_ae_map_func_t *, void *);

#ifndef ALT_ARRAY_IMPLEMENTATION
extern ARRAY_ELEMENT *array_shift (ARRAY *, int, int);
#else
extern ARRAY_ELEMENT **array_shift (ARRAY *, int, int);
#endif
extern int	array_rshift (ARRAY *, int, char *);
extern ARRAY_ELEMENT *array_unshift_element (ARRAY *);
extern int	array_shift_element (ARRAY *, char *);

extern ARRAY	*array_quote (ARRAY *);
extern ARRAY	*array_quote_escapes (ARRAY *);
extern ARRAY	*array_dequote (ARRAY *);
extern ARRAY	*array_dequote_escapes (ARRAY *);
extern ARRAY	*array_remove_quoted_nulls (ARRAY *);

extern char	*array_subrange (ARRAY *, arrayind_t, arrayind_t, int, int, int);
extern char	*array_patsub (ARRAY *, char *, char *, int);
extern char	*array_modcase (ARRAY *, char *, int, int);

/* Basic operations on array elements. */
extern ARRAY_ELEMENT *array_create_element (arrayind_t, char *);
extern ARRAY_ELEMENT *array_copy_element (ARRAY_ELEMENT *);
extern void	array_dispose_element (ARRAY_ELEMENT *);

extern int	array_insert (ARRAY *, arrayind_t, char *);
extern ARRAY_ELEMENT *array_remove (ARRAY *, arrayind_t);
extern char	*array_reference (ARRAY *, arrayind_t);

/* Converting to and from arrays */
extern WORD_LIST *array_to_word_list (ARRAY *);
extern ARRAY *array_from_word_list (WORD_LIST *);
extern WORD_LIST *array_keys_to_word_list (ARRAY *);
extern WORD_LIST *array_to_kvpair_list (ARRAY *);

extern ARRAY *array_assign_list (ARRAY *, WORD_LIST *);

extern char **array_to_argv (ARRAY *, int *);
extern ARRAY *array_from_argv (ARRAY *, char **, int);

extern char *array_to_kvpair (ARRAY *, int);
extern char *array_to_assign (ARRAY *, int);
extern char *array_to_string (ARRAY *, char *, int);
extern ARRAY *array_from_string (char *, char *);

/* Flags for array_shift */
#define AS_DISPOSE	0x01

#define array_num_elements(a)	((a)->num_elements)
#define array_max_index(a)	((a)->max_index)
#ifndef ALT_ARRAY_IMPLEMENTATION
#define array_first_index(a)	((a)->head->next->ind)
#define array_head(a)		((a)->head)
#define array_alloc_size(a)	((a)->alloc_size)
#else
#define array_first_index(a)	((a)->first_index)
#define array_head(a)		((a)->elements)
#endif
#define array_empty(a)		((a)->num_elements == 0)

#define element_value(ae)	((ae)->value)
#define element_index(ae)	((ae)->ind)

#ifndef ALT_ARRAY_IMPLEMENTATION
#define element_forw(ae)	((ae)->next)
#define element_back(ae)	((ae)->prev)
#else
extern arrayind_t element_forw (ARRAY *, arrayind_t);
extern arrayind_t element_back (ARRAY *, arrayind_t);
#endif


#define set_element_value(ae, val)	((ae)->value = (val))

#ifdef ALT_ARRAY_IMPLEMENTATION
#define set_first_index(a, i)	((a)->first_index = (i))
#endif

#define set_max_index(a, i)	((a)->max_index = (i))
#define set_num_elements(a, n)	((a)->num_elements = (n))

/* Convenience */
#define array_push(a,v)	\
  do { array_rshift ((a), 1, (v)); } while (0)
#define array_pop(a) \
  do { array_shift ((a), 1, AS_DISPOSE); } while (0)

#define GET_ARRAY_FROM_VAR(n, v, a) \
  do { \
    (v) = find_variable (n); \
    (a) = ((v) && array_p ((v))) ? array_cell (v) : (ARRAY *)0; \
  } while (0)

#define ARRAY_ELEMENT_REPLACE(ae, v) \
  do { \
    free ((ae)->value); \
    (ae)->value = (v); \
  } while (0)

#ifdef ALT_ARRAY_IMPLEMENTATION
#define ARRAY_VALUE_REPLACE(a, i, v) \
   ARRAY_ELEMENT_REPLACE((a)->elements[(i)], (v))
#endif

#define ALL_ELEMENT_SUB(c)	((c) == '@' || (c) == '*')

/* In eval.c, but uses ARRAY * */
extern int execute_array_command (ARRAY *, void *);

#endif /* _ARRAY_H_ */
