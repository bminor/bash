#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/param.h>
#include "filecntl.h"

#include "shell.h"
#include <strings.h>
char *
xmalloc (size)
     int size;
{
  register char *temp = (char *)malloc (size);

  if (!temp)
    fatal_error ("Out of virtual memory!");

  return (temp);
}

char *
xrealloc (pointer, size)
     register char *pointer;
     int size;
{
  char *temp;

  if (!pointer)
    temp = (char *)xmalloc (size);
  else
    temp = (char *)realloc (pointer, size);

  if (!temp)
    fatal_error ("Out of virtual memory!");

  return (temp);
}
/* Reverse the chain of structures in LIST.  Output the new head
   of the chain.  You should always assign the output value of this
   function to something, or you will lose the chain. */
GENERIC_LIST *
reverse_list (list)
     register GENERIC_LIST *list;
{
  register GENERIC_LIST *next, *prev = (GENERIC_LIST *)NULL;

  while (list) {
    next = list->next;
    list->next = prev;
    prev = list;
    list = next;
  }
  return (prev);
}

/* Return the number of elements in LIST, a generic list. */
int
list_length (list)
     register GENERIC_LIST *list;
{
  register int i;

  for (i = 0; list; list = list->next, i++);
  return (i);
}

/* Delete the element of LIST which satisfies the predicate function COMPARER.
   Returns the element that was deleted, so you can dispose of it, or -1 if
   the element wasn't found.  COMPARER is called with the list element and
   then ARG.  Note that LIST contains the address of a variable which points
   to the list.  You might call this function like this:

   SHELL_VAR *elt = delete_element (&variable_list, check_var_has_name, "foo");
   dispose_variable (elt);
*/
GENERIC_LIST *
delete_element (list, comparer, arg)
     GENERIC_LIST **list;
     Function *comparer;
{
  register GENERIC_LIST *prev = (GENERIC_LIST *)NULL;
  register GENERIC_LIST *temp = *list;

  while (temp) {
    if ((*comparer) (temp, arg)) {
      if (prev) prev->next = temp->next;
      else *list = temp->next;
      return (temp);
    }
    prev = temp;
    temp = temp->next;
  }
  return ((GENERIC_LIST *)-1);
}

/* Find NAME in ARRAY.  Return the index of NAME, or -1 if not present.
   ARRAY shoudl be NULL terminated. */
int
find_name_in_list (name, array)
     char *name, *array[];
{
  int i;

  for (i=0; array[i]; i++)
    if (strcmp (name, array[i]) == 0)
      return (i);

  return (-1);
}

/* Return the length of ARRAY, a NULL terminated array of char *. */
int
array_len (array)
     register char **array;
{
  register int i;
  for (i=0; array[i]; i++);
  return (i);
}

/* Free the contents of ARRAY, a NULL terminated array of char *. */
void
free_array (array)
     register char **array;
{
  register int i = 0;

  if (!array) return;

  while (array[i])
    free (array[i++]);
  free (array);
}

/* Allocate and return a new copy of ARRAY and its contents. */
char **
copy_array (array)
  register char **array;
{
  register int i;
  int len;
  char **new_array;

  len = array_len (array);
  new_array = (char **)xmalloc ((len + 1) * sizeof (char *));
  for (i = 0; array[i]; i++)
    new_array[i] = savestring (array[i]);  
  new_array[i] = (char *)NULL;
  return (new_array);
}

/* Append LIST2 to LIST1.  Return the header of the list. */
GENERIC_LIST *
list_append (head, tail)
     GENERIC_LIST *head, *tail;
{
  register GENERIC_LIST *t_head = head;

  if (!t_head)
    return (tail);

  while (t_head->next) t_head = t_head->next;
  t_head->next = tail;
  return (head);
}

#include <stdio.h>

#ifndef NULL
#define NULL 0x0
#endif

#if defined (ibm032)
/*
 * Most vanilla 4.3 (not 4.3-tahoe) sites lack vfprintf.
 * Here is the one from 4.3-tahoe (it is freely redistributable).
 *
 * Beware!  Don't trust the value returned by either of these functions; it
 * seems that pre-4.3-tahoe implementations of _doprnt () return the first
 * argument, i.e. a char *.  Besides, _doprnt () is incorrectly documented
 * in the 4.3 BSD manuals, anyway (it's wrong in SunOS 3.5 also, but they
 * have the v*printf functions (incorrectly documented (vprintf), but they
 * are present)).
 */
#include <varargs.h>

int
vfprintf (iop, fmt, ap)
     FILE *iop;
     char *fmt;
     va_list ap;
{
  int len;
  char localbuf[BUFSIZ];

  if (iop->_flag & _IONBF)
    {
      iop->_flag &= ~_IONBF;
      iop->_ptr = iop->_base = localbuf;
      len = _doprnt (fmt, ap, iop);
      (void) fflush (iop);
      iop->_flag |= _IONBF;
      iop->_base = NULL;
      iop->_bufsiz = 0;
      iop->_cnt = 0;
    }
  else
    len = _doprnt (fmt, ap, iop);
  return (ferror (iop) ? EOF : len);
}

/*
 * Ditto for vsprintf
 */
int
vsprintf (str, fmt, ap)
     char *str, *fmt;
     va_list ap;
{
  FILE f;
  int len;

  f._flag = _IOWRT+_IOSTRG;
  f._ptr = str;
  f._cnt = 32767;
  len = _doprnt (fmt, ap, &f);
  *f._ptr = 0;
  return (len);
}

#endif /* ibm032 */
