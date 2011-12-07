#include <stdio.h>
#include <limits.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h>
#include <stdarg.h>
#include "bashansi.h"
#include "shell.h"
#include "builtins.h"
#include "stdc.h"
#include "common.h"
#include "bashgetopt.h"

char *this_command_name = (char *)NULL;

void
builtin_error (const char *format, ...)
{
  va_list args;
  char *name;

  name = get_name_for_error ();
  fprintf (stderr, "%s: ", name);

  if (this_command_name && *this_command_name)
    fprintf (stderr, "%s: ", this_command_name);

  va_start (args, format);

  vfprintf (stderr, format, args);
  va_end (args);
  fprintf (stderr, "\n");
}

int
no_options(list)
WORD_LIST *list;
{
  reset_internal_getopt ();
  if (internal_getopt (list, "") != -1)
    {
      builtin_usage ();
      return (1);
    }
  return (0);
}

int
legal_number (string, result)
     char *string;
     long *result;
{
  long value;
  char *ep;

  if (result)
    *result = 0;

  value = strtol (string, &ep, 10);

  /* If *string is not '\0' but *ep is '\0' on return, the entire string
     is valid. */
  if (string && *string && *ep == '\0')
    {
      if (result)
        *result = value;
      /* The SunOS4 implementation of strtol() will happily ignore
         overflow conditions, so this cannot do overflow correctly
         on those systems. */
      return 1;
    }
    
  return (0);
}

/* Return the number of elements in LIST, a generic list. */
int
list_length (list)
     GENERIC_LIST *list;
{
  register int i;

  for (i = 0; list; list = list->next, i++);
  return (i);
}

GENERIC_LIST *
reverse_list (list)
     GENERIC_LIST *list;
{
  register GENERIC_LIST *next, *prev;

  for (prev = (GENERIC_LIST *)NULL; list; )
    {
      next = list->next;
      list->next = prev;
      prev = list;
      list = next;
    }
  return (prev);
}

WORD_DESC *
make_bare_word (string)
     char *string;
{
  WORD_DESC *temp;

  temp = (WORD_DESC *)xmalloc (sizeof (WORD_DESC));
  if (*string)
    temp->word = savestring (string);
  else
    {
      temp->word = xmalloc (1);
      temp->word[0] = '\0';
    }

  temp->flags = 0;
  return (temp);
}
WORD_LIST *
make_word_list (word, link)
     WORD_DESC *word;
     WORD_LIST *link;
{
  WORD_LIST *temp;

  temp = (WORD_LIST *)xmalloc (sizeof (WORD_LIST));
  temp->word = word;
  temp->next = link;
  return (temp);
}

void
builtin_usage()
{
  if (this_command_name && *this_command_name)
    fprintf (stderr, "%s: usage: %s args\n", this_command_name, this_command_name);
  fflush (stderr);
}

char *
xmalloc(s)
     size_t s;
{
  return (malloc (s));
}

WORD_LIST *
argv_to_word_list (array, copy, starting_index)
     char **array;
     int copy, starting_index;
{
  WORD_LIST *list;
  WORD_DESC *w;
  int i, count;

  if (array == 0 || array[0] == 0)
    return (WORD_LIST *)NULL;

  for (count = 0; array[count]; count++)
    ;

  for (i = starting_index, list = (WORD_LIST *)NULL; i < count; i++)
    {
      w = make_bare_word (copy ? "" : array[i]);
      if (copy)
        {
          free (w->word);
          w->word = array[i];
        }
      list = make_word_list (w, list);
    }
  return (REVERSE_LIST(list, WORD_LIST *));
}

/* Convert a WORD_LIST into a C-style argv.  Return the number of elements
   in the list in *IP, if IP is non-null.  A convenience function for
   loadable builtins; also used by `test'. */
char **
make_builtin_argv (list, ip)
     WORD_LIST *list;
     int *ip;
{
  char **argv;

  argv = strvec_from_word_list (list, 0, 1, ip);
  argv[0] = this_command_name;
  return argv;
}
                
