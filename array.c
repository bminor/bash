/*
 * array.c - functions to create, destroy, access, and manipulate arrays
 *	     of strings.
 *
 * Arrays are sparse doubly-linked lists.  An element's index is stored
 * with it.
 *
 * Chet Ramey
 * chet@ins.cwru.edu
 */

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

#include "config.h"

#if defined (ARRAY_VARS)

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include <stdio.h>
#include "bashansi.h"

#include "shell.h"
#include "array.h"
#include "builtins/common.h"

extern char *quote_string ();	/* XXX */

#define ADD_BEFORE(ae, new) \
	do { \
		ae->prev->next = new; \
		new->prev = ae->prev; \
		ae->prev = new; \
		new->next = ae; \
	} while(0)

/*
 * Allocate and return a new array element with index INDEX and value
 * VALUE.
 */
ARRAY_ELEMENT *
new_array_element(indx, value)
arrayind_t	indx;
char	*value;
{
	ARRAY_ELEMENT *r;

	r = (ARRAY_ELEMENT *) xmalloc(sizeof(ARRAY_ELEMENT));
	r->ind = indx;
	r->value = value ? savestring(value) : (char *)NULL;
	r->next = r->prev = (ARRAY_ELEMENT *) NULL;
	return(r);
}

void
destroy_array_element(ae)
ARRAY_ELEMENT	*ae;
{
	FREE(ae->value);
	free(ae);
}

ARRAY *
new_array()
{
	ARRAY	*r;
	ARRAY_ELEMENT	*head;

	r =(ARRAY *) xmalloc(sizeof(ARRAY));
	r->type = array_indexed;
	r->max_index = r->max_size = -1;
	r->num_elements = 0;
	head = new_array_element(-1, (char *)NULL);	/* dummy head */
	head->prev = head->next = head;
	r->head = head;
	return(r);
}

void
empty_array (a)
ARRAY	*a;
{
	register ARRAY_ELEMENT *r, *r1;

	if (a == 0)
		return;
	for (r = element_forw(a->head); r != a->head; ) {
		r1 = element_forw(r);
		destroy_array_element(r);
		r = r1;
	}
	a->head->next = a->head->prev = a->head;
	a->max_index = a->max_size = -1;
	a->num_elements = a->max_size = 0;
}

void
dispose_array(a)
ARRAY	*a;
{
	if (a == 0)
		return;
	empty_array (a);
	destroy_array_element(a->head);
	free(a);
}

ARRAY *
dup_array(a)
ARRAY	*a;
{
	ARRAY	*a1;
	ARRAY_ELEMENT	*ae, *new;

	if (!a)
		return((ARRAY *) NULL);
	a1 = new_array();
	a1->type = a->type;
	a1->max_index = a->max_index;
	a1->num_elements = a->num_elements;
	a1->max_size = a->max_size;
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae)) {
		new = new_array_element(element_index(ae), element_value(ae));
		ADD_BEFORE(a1->head, new);
	}
	return(a1);
}

#ifdef INCLUDE_UNUSED
/*
 * Make and return a new array composed of the elements in array A from
 * S to E, inclusive.
 */
ARRAY *
dup_array_subrange(array, s, e)
ARRAY		*array;
ARRAY_ELEMENT	*s, *e;
{
	ARRAY	*a;
	ARRAY_ELEMENT *p, *n;
	int	i;

	a = new_array ();
	a->type = array->type;

	for (p = s, i = 0; p != e; p = element_forw(p), i++) {
		n = new_array_element (i, element_value(p));
		ADD_BEFORE(a->head, n);
	}
	a->num_elements = a->max_index = i;
	return a;
}
#endif

#ifdef INCLUDE_UNUSED
ARRAY_ELEMENT *
copy_array_element(ae)
ARRAY_ELEMENT	*ae;
{
	return(ae ? new_array_element(element_index(ae), element_value(ae))
		  : (ARRAY_ELEMENT *) NULL);
}
#endif

/*
 * Add a new element with index I and value V to array A (a[i] = v).
 */
int
array_add_element(a, i, v)
ARRAY	*a;
arrayind_t	i;
char	*v;
{
	register ARRAY_ELEMENT *new, *ae;

	if (!a)
		return(-1);
	new = new_array_element(i, v);
	if (i > array_max_index(a)) {
		/*
		 * Hook onto the end.  This also works for an empty array.
		 * Fast path for the common case of allocating arrays
		 * sequentially.
		 */
		ADD_BEFORE(a->head, new);
		a->max_index = i;
		a->num_elements++;
		return(0);
	}
	/*
	 * Otherwise we search for the spot to insert it.
	 */
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae)) {
		if (element_index(ae) == i) {
			/*
			 * Replacing an existing element.
			 */
			destroy_array_element(new);
			free(element_value(ae));
			ae->value = savestring(v);
			return(0);
		} else if (element_index(ae) > i) {
			ADD_BEFORE(ae, new);
			a->num_elements++;
			return(0);
		}
	}
	return (-1);		/* problem */
}

/*
 * Delete the element with index I from array A and return it so the
 * caller can dispose of it.
 */
ARRAY_ELEMENT *
array_delete_element(a, i)
ARRAY	*a;
arrayind_t	i;
{
	register ARRAY_ELEMENT *ae;

	if (!a || array_empty(a))
		return((ARRAY_ELEMENT *) NULL);
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae))
		if (element_index(ae) == i) {
			ae->next->prev = ae->prev;
			ae->prev->next = ae->next;
			a->num_elements--;
			if (i == array_max_index(a))
				a->max_index = element_index(ae->prev);
			return(ae);
		}
	return((ARRAY_ELEMENT *) NULL);
}

/*
 * Return the value of a[i].
 */
char *
array_reference(a, i)
ARRAY	*a;
arrayind_t	i;
{
	register ARRAY_ELEMENT *ae;

	if (a == 0 || array_empty(a))
		return((char *) NULL);
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae))
		if (element_index(ae) == i)
			return(element_value(ae));
	return((char *) NULL);
}

#ifdef TEST_ARRAY
/*
 * Walk the array, calling FUNC once for each element, with the array
 * element as the argument.
 */
void
array_walk(a, func)
ARRAY	*a;
Function *func;
{
	register ARRAY_ELEMENT *ae;

	if (a == 0 || array_empty(a))
		return;
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae))
		(*func)(ae);
}
#endif

/*
 * Return a string that is the concatenation of all the elements in A,
 * separated by SEP.
 */
static char *
array_to_string_internal (start, end, sep, quoted)
ARRAY_ELEMENT	*start, *end;
char	*sep;
int	quoted;
{
	char	*result, *t;
	ARRAY_ELEMENT *ae;
	int	slen, rsize, rlen, reg;

	if (start == end)	/* XXX - should not happen */
		return ((char *)NULL);

	slen = strlen(sep);
	for (rsize = rlen = 0, ae = start; ae != end; ae = element_forw(ae)) {
		if (rsize == 0)
			result = xmalloc (rsize = 64);
		if (element_value(ae)) {
			t = quoted ? quote_string(element_value(ae)) : element_value(ae);
			reg = strlen(t);
			RESIZE_MALLOCED_BUFFER (result, rlen, (reg + slen + 2),
						rsize, rsize);
			strcpy(result + rlen, t);
			rlen += reg;
			if (quoted && t)
				free(t);
			/*
			 * Add a separator only after non-null elements.
			 */
			if (element_forw(ae) != end) {
				strcpy(result + rlen, sep);
				rlen += slen;
			}
		}
	}
	result[rlen] = '\0';	/* XXX */
	return(result);
}

char *
array_to_string (a, sep, quoted)
ARRAY	*a;
char	*sep;
int	quoted;
{
	if (a == 0)
		return((char *)NULL);
	if (array_empty(a))
		return(savestring(""));
	return (array_to_string_internal (element_forw(a->head), a->head, sep, quoted));
}

char *
array_to_assignment_string (a)
ARRAY	*a;
{
	char	*result, *indstr, *valstr;
	ARRAY_ELEMENT *ae;
	int	rsize, rlen, elen;

	if (a == 0 || array_empty (a))
		return((char *)NULL);

	result = xmalloc (rsize = 128);
	result[0] = '(';
	rlen = 1;

	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae)) {
		indstr = itos (element_index(ae));
		valstr = element_value (ae) ? sh_double_quote (element_value(ae))
					    : (char *)NULL;
		elen = STRLEN (indstr) + 8 + STRLEN (valstr);
		RESIZE_MALLOCED_BUFFER (result, rlen, (elen + 1), rsize, rsize);

		result[rlen++] = '[';
		strcpy (result + rlen, indstr);
		rlen += STRLEN (indstr);
		result[rlen++] = ']';
		result[rlen++] = '=';
		if (valstr) {
			strcpy (result + rlen, valstr);
			rlen += STRLEN (valstr);
		}

		if (element_forw(ae) != a->head)
		  result[rlen++] = ' ';

		FREE (indstr);
		FREE (valstr);
	}
	RESIZE_MALLOCED_BUFFER (result, rlen, 1, rsize, 8);
	result[rlen++] = ')';
	result[rlen] = '\0';
	return(result);
}

char *
quoted_array_assignment_string (a)
ARRAY	*a;
{
	char *vstr, *sv;

	sv = array_to_assignment_string (a);
	if (sv == 0)
		return ((char *)NULL);

	vstr = sh_single_quote (sv);
	free (sv);
	return (vstr);
}

#if 0
/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case sensitive. */
static char *
sindex (s1, s2)
register char *s1, *s2;
{
	register int i, l, len;

	for (i = 0, l = strlen(s2), len = strlen(s1); (len - i) >= l; i++)
		if (strncmp (s1 + i, s2, l) == 0)
			return (s1 + i);
	return ((char *)NULL);
}
#endif

#if defined (INCLUDE_UNUSED) || defined (TEST_ARRAY)
/*
 * Return an array consisting of elements in S, separated by SEP
 */
ARRAY *
string_to_array(s, sep)
char	*s, *sep;
{
	ARRAY	*a;
	WORD_LIST *w;

	if (s == 0)
		return((ARRAY *)NULL);
	w = list_string (s, sep, 0);
	if (w == 0)
		return((ARRAY *)NULL);
	a = word_list_to_array (w);
	return (a);
}
#endif

/* Convenience routines for the shell to translate to and from the form used
   by the rest of the code. */
WORD_LIST *
array_to_word_list(a)
ARRAY	*a;
{
	WORD_LIST	*list;
	ARRAY_ELEMENT	*ae;

	if (a == 0 || array_empty(a))
		return((WORD_LIST *)NULL);
	list = (WORD_LIST *)NULL;
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae))
		list = make_word_list (make_bare_word(element_value(ae)), list);
	return (REVERSE_LIST(list, WORD_LIST *));
}

char **
array_to_argv (a)
ARRAY	*a;
{
	char		**ret, *t;
	int		i;
	ARRAY_ELEMENT	*ae;

	if (a == 0 || array_empty(a))
		return ((char **)NULL);
	ret = alloc_array (array_num_elements (a) + 1);
	i = 0;
	for (ae = element_forw(a->head); ae != a->head; ae = element_forw(ae)) {
		t = element_value (ae);
		ret[i++] = t ? savestring (t) : (char *)NULL;
	}
	ret[i] = (char *)NULL;
	return (ret);
}
	
ARRAY *
assign_word_list (array, list)
ARRAY	*array;
WORD_LIST	*list;
{
	register WORD_LIST *l;
	register arrayind_t i;

	for (l = list, i = 0; l; l = l->next, i++)
		array_add_element(array, i, l->word->word);
	return array;
}

ARRAY *
word_list_to_array (list)
WORD_LIST	*list;
{
	ARRAY	*a;

	if (list == 0)
		return((ARRAY *)NULL);
	a = new_array();
	return (assign_word_list (a, list));
}

ARRAY	*
array_quote(array)
ARRAY	*array;
{
	ARRAY_ELEMENT	*a;
	char	*t;

	if (array == 0 || array->head == 0 || array_empty (array))
		return (ARRAY *)NULL;
	for (a = element_forw(array->head); a != array->head; a = element_forw(a)) {
		t = quote_string (a->value);
		FREE(a->value);
		a->value = t;
	}
	return array;
}

char *
array_subrange (a, start, end, quoted)
ARRAY	*a;
int	start, end, quoted;
{
	ARRAY_ELEMENT	*h, *p;
	int	i;

	p = array_head (a);
	if (p == 0 || array_empty (a) || start > array_num_elements (a))
		return ((char *)NULL);

	for (i = 0, p = element_forw(p); p != a->head && i < start; i++, p = element_forw(p))
		;
	if (p == a->head)
		return ((char *)NULL);
	for (h = p; p != a->head && i < end; i++, p = element_forw(p))
		;

	return (array_to_string_internal (h, p, " ", quoted));
}

char *
array_pat_subst (a, pat, rep, mflags)
ARRAY	*a;
char	*pat, *rep;
int	mflags;
{
	ARRAY		*a2;
	ARRAY_ELEMENT	*e;
	char	*t;

	if (array_head (a) == 0 || array_empty (a))
		return ((char *)NULL);

	a2 = dup_array (a);
	for (e = element_forw(a2->head); e != a2->head; e = element_forw(e)) {
		t = pat_subst(element_value(e), pat, rep, mflags);
		FREE(element_value(e));
		e->value = t;
	}

	if (mflags & MATCH_QUOTED)
		array_quote (a2);
	t = array_to_string (a2, " ", 0);
	dispose_array (a2);

	return t;
}


#if defined (TEST_ARRAY)
print_element(ae)
ARRAY_ELEMENT	*ae;
{
	printf("array[%d] = %s\n",(int)element_index(ae), element_value(ae));
}

print_array(a)
ARRAY	*a;
{
	printf("\n");
	array_walk(a, print_element);
}

main()
{
	ARRAY	*a, *new_a, *copy_of_a;
	ARRAY_ELEMENT	*ae;
	char	*s;

	a = new_array();
	array_add_element(a, 1, "one");
	array_add_element(a, 7, "seven");
	array_add_element(a, 4, "four");
	array_add_element(a, 1029, "one thousand twenty-nine");
	array_add_element(a, 12, "twelve");
	array_add_element(a, 42, "forty-two");
	print_array(a);
	s = array_to_string (a, " ", 0);
	printf("s = %s\n", s);
	copy_of_a = string_to_array(s, " ");
	printf("copy_of_a:");
	print_array(copy_of_a);
	dispose_array(copy_of_a);
	printf("\n");
	free(s);
	ae = array_delete_element(a, 4);
	destroy_array_element(ae);
	ae = array_delete_element(a, 1029);
	destroy_array_element(ae);
	array_add_element(a, 16, "sixteen");
	print_array(a);
	s = array_to_string (a, " ", 0);
	printf("s = %s\n", s);
	copy_of_a = string_to_array(s, " ");
	printf("copy_of_a:");
	print_array(copy_of_a);
	dispose_array(copy_of_a);
	printf("\n");
	free(s);
	array_add_element(a, 2, "two");
	array_add_element(a, 1029, "new one thousand twenty-nine");
	array_add_element(a, 0, "zero");
	array_add_element(a, 134, "");
	print_array(a);
	s = array_to_string (a, ":", 0);
	printf("s = %s\n", s);
	copy_of_a = string_to_array(s, ":");
	printf("copy_of_a:");
	print_array(copy_of_a);
	dispose_array(copy_of_a);
	printf("\n");
	free(s);
	new_a = copy_array(a);
	print_array(new_a);
	s = array_to_string (new_a, ":", 0);
	printf("s = %s\n", s);
	copy_of_a = string_to_array(s, ":", 0);
	printf("copy_of_a:");
	print_array(copy_of_a);
	dispose_array(copy_of_a);
	printf("\n");
	free(s);
	dispose_array(a);
	dispose_array(new_a);
}

#endif /* TEST_ARRAY */
#endif /* ARRAY_VARS */
