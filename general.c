/* general.c -- Stuff that is used by all files. */

/* Copyright (C) 1987, 1988, 1989, 1990, 1991, 1992
   Free Software Foundation, Inc.

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

#include "config.h"	/* includes unistd.h for us */
#include <stdio.h>
#include <ctype.h>
#include <errno.h>
#include "bashtypes.h"
#include <sys/param.h>
#if defined (_POSIX_VERSION)
#  if defined (amiga) && defined (USGr4)
#    define _POSIX_SOURCE
#  endif
#  include <signal.h>
#  if defined (amiga) && defined (USGr4)
#    undef _POSIX_SOURCE
#  endif
#endif /* _POSIX_VERSION */
#include "filecntl.h"
#include "bashansi.h"
#include "shell.h"
#include <tilde/tilde.h>

#if !defined (USG) || defined (HAVE_RESOURCE)
#  include <sys/time.h>
#endif

#include <sys/times.h>
#include "maxpath.h"

#if !defined (errno)
extern int errno;
#endif /* !errno */

/* Make the functions strchr and strrchr if they do not exist. */
#if !defined (HAVE_STRCHR)
char *
strchr (string, c)
     char *string;
     int c;
{
  register int i;

  for (i = 0; string && string[i]; i++)
    if (string[i] == c)
      return ((char *) (string + i));

  return ((char *) NULL);
}

char *
strrchr (string, c)
     char *string;
     int c;
{
  register int i;

  if (string)
    i = strlen (string) - 1;
  else
    i = -1;

  for (; string && i > -1; i--)
    if (string[i] == c)
      return ((char *) (string + i));

  return ((char *) NULL);
}
#endif /* !HAVE_STRCHR */

/* **************************************************************** */
/*								    */
/*		   Memory Allocation and Deallocation.		    */
/*								    */
/* **************************************************************** */

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
     GENPTR pointer;
     int size;
{
  char *temp;

  if (!pointer)
    temp = xmalloc (size);
  else
    temp = (char *)realloc (pointer, size);

  if (!temp)
    fatal_error ("Out of virtual memory!");

  return (temp);
}

/* Use this as the function to call when adding unwind protects so we
   don't need to know what free() returns. */
void
xfree (string)
     char *string;
{
  free (string);
}

/* **************************************************************** */
/*								    */
/*		     Integer to String Conversion		    */
/*								    */
/* **************************************************************** */

/* Number of characters that can appear in a string representation
   of an integer.  32 is larger than the string rep of 2^^31 - 1. */
#define MAX_INT_LEN 32

/* Integer to string conversion.  This conses the string; the
   caller should free it. */
char *
itos (i)
     int i;
{
  char *buf, *p, *ret;
  int negative = 0;
  unsigned int ui;

  buf = xmalloc (MAX_INT_LEN);

  if (i < 0)
    {
      negative++;
      i = -i;
    }

  ui = (unsigned int) i;

  buf[MAX_INT_LEN - 1] = '\0';
  p = &buf[MAX_INT_LEN - 2];

  do
    *p-- = (ui % 10) + '0';
  while (ui /= 10);

  if (negative)
    *p-- = '-';

  ret = savestring (p + 1);
  free (buf);
  return (ret);
}

/* Return non-zero if all of the characters in STRING are digits. */
int
all_digits (string)
     char *string;
{
  while (*string)
    {
      if (!digit (*string))
	return (0);
      else
	string++;
    }
  return (1);
}

/* atol(3) is not universal */
long
string_to_long (s)
     char *s;
{
  long ret = 0L;
  int neg = 0;

  while (s && *s && whitespace (*s))
    s++;
  if (*s == '-' || *s == '+')
    {
      neg = *s == '-';
      s++;
    }
  for ( ; s && *s && digit (*s); s++)
    ret = (ret * 10) + digit_value (*s);
  return (neg ? -ret : ret);
}

#if defined (RLIMTYPE)
RLIMTYPE
string_to_rlimtype (s)
     char *s;
{
  RLIMTYPE ret = 0;
  int neg = 0;

  while (s && *s && whitespace (*s))
    s++;
  if (*s == '-' || *s == '+')
    {
      neg = *s == '-';
      s++;
    }
  for ( ; s && *s && digit (*s); s++)
    ret = (ret * 10) + digit_value (*s);
  return (neg ? -ret : ret);
}

void
print_rlimtype (n, addnl)
     RLIMTYPE n;
     int addnl;
{
  char s[sizeof (RLIMTYPE) * 3 + 1];
  int len = sizeof (RLIMTYPE) * 3 + 1;

  if (n == 0)
    {
      printf ("0%s", addnl ? "\n" : "");
      return;
    }

  if (n < 0)
    {
      putchar ('-');
      n = -n;
    }

  s[--len] = '\0';
  for ( ; n != 0; n /= 10)
    s[--len] = n % 10 + '0';
  printf ("%s%s", s + len, addnl ? "\n" : "");
}
#endif /* RLIMTYPE */

/* Return 1 if this token is a legal shell `identifier'; that is, it consists
   solely of letters, digits, and underscores, and does not begin with a
   digit. */
int
legal_identifier (name)
     char *name;
{
  register char *s;

  if (!name || !*name || digit (*name))
    return (0);

  for (s = name; s && *s; s++)
    {
      if (!isletter (*s) && !digit (*s) && (*s != '_'))
        return (0);
    }
  return (1);
}

/* Make sure that WORD is a valid shell identifier, i.e.
   does not contain a dollar sign, nor is quoted in any way.  Nor
   does it consist of all digits.  If CHECK_WORD is non-zero,
   the word is checked to ensure that it consists of only letters,
   digits, and underscores. */
check_identifier (word, check_word)
     WORD_DESC *word;
     int check_word;
{
  if (word->dollar_present || word->quoted || all_digits (word->word))
    {
      report_error ("`%s' is not a valid identifier", word->word);
      return (0);
    }
  else if (check_word && legal_identifier (word->word) == 0)
    {
      report_error ("`%s' is not a valid identifier", word->word);
      return (0);
    }
  else
    return (1);
}

/* A function to unset no-delay mode on a file descriptor.  Used in shell.c
   to unset it on the fd passed as stdin.  Should be called on stdin if
   readline gets an EAGAIN or EWOULDBLOCK when trying to read input. */

#if !defined (O_NDELAY)
#  if defined (FNDELAY)
#    define O_NDELAY FNDELAY
#  endif
#endif /* O_NDELAY */

/* Make sure no-delay mode is not set on file descriptor FD. */
void
unset_nodelay_mode (fd)
     int fd;
{
  int flags, set = 0;

  if ((flags = fcntl (fd, F_GETFL, 0)) < 0)
    return;

#if defined (O_NONBLOCK)
  if (flags & O_NONBLOCK)
    {
      flags &= ~O_NONBLOCK;
      set++;
    }
#endif /* O_NONBLOCK */

#if defined (O_NDELAY)
  if (flags & O_NDELAY)
    {
      flags &= ~O_NDELAY;
      set++;
    }
#endif /* O_NDELAY */

  if (set)
    fcntl (fd, F_SETFL, flags);
}


/* **************************************************************** */
/*								    */
/*			Generic List Functions			    */
/*								    */
/* **************************************************************** */

/* Call FUNCTION on every member of LIST, a generic list. */
void
map_over_list (list, function)
     GENERIC_LIST *list;
     Function *function;
{
  while (list)
    {
      (*function) (list);
      list = list->next;
    }
}

/* Call FUNCTION on every string in WORDS. */
void
map_over_words (words, function)
     WORD_LIST *words;
     Function *function;
{
  while (words)
    {
      (*function)(words->word->word);
      words = words->next;
    }
}

/* Reverse the chain of structures in LIST.  Output the new head
   of the chain.  You should always assign the output value of this
   function to something, or you will lose the chain. */
GENERIC_LIST *
reverse_list (list)
     GENERIC_LIST *list;
{
  register GENERIC_LIST *next, *prev = (GENERIC_LIST *)NULL;

  while (list)
    {
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
     GENERIC_LIST *list;
{
  register int i;

  for (i = 0; list; list = list->next, i++);
  return (i);
}

/* A global variable which acts as a sentinel for an `error' list return. */
GENERIC_LIST global_error_list;

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
     char *arg;
{
  register GENERIC_LIST *prev = (GENERIC_LIST *)NULL;
  register GENERIC_LIST *temp = *list;

  while (temp)
    {
      if ((*comparer) (temp, arg))
	{
	  if (prev)
	    prev->next = temp->next;
	  else
	    *list = temp->next;
	  return (temp);
	}
      prev = temp;
      temp = temp->next;
    }
  return ((GENERIC_LIST *)&global_error_list);
}

/* Find NAME in ARRAY.  Return the index of NAME, or -1 if not present.
   ARRAY should be NULL terminated. */
int
find_name_in_list (name, array)
     char *name, **array;
{
  int i;

  for (i = 0; array[i]; i++)
    if (strcmp (name, array[i]) == 0)
      return (i);

  return (-1);
}

/* Return the length of ARRAY, a NULL terminated array of char *. */
int
array_len (array)
     char **array;
{
  register int i;
  for (i = 0; array[i]; i++);
  return (i);
}

/* Free the contents of ARRAY, a NULL terminated array of char *. */
void
free_array (array)
     char **array;
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
     char **array;
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

/* Comparison routine for use with qsort() on arrays of strings. */
int
qsort_string_compare (s1, s2)
     register char **s1, **s2;
{
  int result;

  if ((result = **s1 - **s2) == 0)
    result = strcmp (*s1, *s2);

  return (result);
}

/* Append LIST2 to LIST1.  Return the header of the list. */
GENERIC_LIST *
list_append (head, tail)
     GENERIC_LIST *head, *tail;
{
  register GENERIC_LIST *t_head = head;

  if (!t_head)
    return (tail);

  while (t_head->next)
    t_head = t_head->next;
  t_head->next = tail;
  return (head);
}

/* Some random string stuff. */

/* Remove all leading whitespace from STRING.  This includes
   newlines.  STRING should be terminated with a zero. */
void
strip_leading (string)
     char *string;
{
  char *start = string;

  while (*string && (whitespace (*string) || *string == '\n'))
    string++;

  if (string != start)
    {
      int len = strlen (string);
      FASTCOPY (string, start, len);
      start[len] = '\0';
    }
}

/* Remove all trailing whitespace from STRING.  This includes
   newlines.  If NEWLINES_ONLY is non-zero, only trailing newlines
   are removed.  STRING should be terminated with a zero. */
void
strip_trailing (string, newlines_only)
     char *string;
     int newlines_only;
{
  int len = strlen (string) - 1;

  while (len >= 0)
    {
      if ((newlines_only && string[len] == '\n') ||
          (!newlines_only && whitespace (string[len])))
        len--;
      else
        break;
    }
  string[len + 1] = '\0';
}

/* Canonicalize PATH, and return a new path.  The new path differs from PATH
   in that:
	Multple `/'s are collapsed to a single `/'.
	Leading `./'s and trailing `/.'s are removed.
	Trailing `/'s are removed.
	Non-leading `../'s and trailing `..'s are handled by removing
	portions of the path. */
char *
canonicalize_pathname (path)
     char *path;
{
  register int i, start;
  char stub_char;
  char *result;

  /* The result cannot be larger than the input PATH. */
  result = savestring (path);

  stub_char = (*path == '/') ? '/' : '.';

  /* Walk along RESULT looking for things to compact. */
  i = 0;
  while (1)
    {
      if (!result[i])
	break;

      while (result[i] && result[i] != '/')
	i++;

      start = i++;

      /* If we didn't find any slashes, then there is nothing left to do. */
      if (!result[start])
	break;

      /* Handle multiple `/'s in a row. */
      while (result[i] == '/')
	i++;

#if !defined (apollo)
      if ((start + 1) != i)
#else
      if ((start + 1) != i && (start != 0 || i != 2))
#endif /* apollo */
	{
	  strcpy (result + start + 1, result + i);
	  i = start + 1;
	}

#if 0
      /* Handle backslash-quoted `/'. */
      if (start > 0 && result[start - 1] == '\\')
	continue;
#endif

      /* Check for trailing `/'. */
      if (start && !result[i])
	{
	zero_last:
	  result[--i] = '\0';
	  break;
	}

      /* Check for `../', `./' or trailing `.' by itself. */
      if (result[i] == '.')
	{
	  /* Handle trailing `.' by itself. */
	  if (!result[i + 1])
	    goto zero_last;

	  /* Handle `./'. */
	  if (result[i + 1] == '/')
	    {
	      strcpy (result + i, result + i + 1);
	      i = (start < 0) ? 0 : start;
	      continue;
	    }

	  /* Handle `../' or trailing `..' by itself. */
	  if (result[i + 1] == '.' &&
	      (result[i + 2] == '/' || !result[i + 2]))
	    {
	      while (--start > -1 && result[start] != '/');
	      strcpy (result + start + 1, result + i + 2);
	      i = (start < 0) ? 0 : start;
	      continue;
	    }
	}
    }

  if (!*result)
    {
      *result = stub_char;
      result[1] = '\0';
    }
  return (result);
}

/* Turn STRING (a pathname) into an absolute pathname, assuming that
   DOT_PATH contains the symbolic location of `.'.  This always
   returns a new string, even if STRING was an absolute pathname to
   begin with. */
char *
make_absolute (string, dot_path)
     char *string, *dot_path;
{
  char *result;
  int result_len;
  
  if (!dot_path || *string == '/')
    result = savestring (string);
  else
    {
      if (dot_path && dot_path[0])
	{
	  result = xmalloc (2 + strlen (dot_path) + strlen (string));
	  strcpy (result, dot_path);
	  result_len = strlen (result);
	  if (result[result_len - 1] != '/')
	    {
	      result[result_len++] = '/';
	      result[result_len] = '\0';
	    }
	}
      else
	{
	  result = xmalloc (3 + strlen (string));
	  result[0] = '.'; result[1] = '/'; result[2] = '\0';
	  result_len = 2;
	}

      strcpy (result + result_len, string);
    }

  return (result);
}

/* Return 1 if STRING contains an absolute pathname, else 0. */
int
absolute_pathname (string)
     char *string;
{
  if (!string || !*string)
    return (0);

  if (*string == '/')
    return (1);

  if (*string++ == '.')
    {
      if (!*string || *string == '/')
	return (1);

      if (*string == '.' && (string[1] == '\0' || string[1] == '/'))
	return (1);
    }
  return (0);
}

/* Return 1 if STRING is an absolute program name; it is absolute if it
   contains any slashes.  This is used to decide whether or not to look
   up through $PATH. */
int
absolute_program (string)
     char *string;
{
  return ((char *)strchr (string, '/') != (char *)NULL);
}

/* Return the `basename' of the pathname in STRING (the stuff after the
   last '/').  If STRING is not a full pathname, simply return it. */
char *
base_pathname (string)
     char *string;
{
  char *p;

  if (!absolute_pathname (string))
    return (string);

  p = (char *)strrchr (string, '/');
  if (p)
    return (++p);
  else
    return (string);
}

/* Return the full pathname of FILE.  Easy.  Filenames that begin
   with a '/' are returned as themselves.  Other filenames have
   the current working directory prepended.  A new string is
   returned in either case. */
char *
full_pathname (file)
     char *file;
{
  char *disposer;

  if (*file == '~')
    file = tilde_expand (file);
  else
    file = savestring (file);

  if ((*file == '/') && absolute_pathname (file))
    return (file);

  disposer = file;

  {
    char *current_dir = xmalloc (2 + MAXPATHLEN + strlen (file));
    int dlen;
    if (getwd (current_dir) == 0)
      {
	report_error (current_dir);
	free (current_dir);
	return ((char *)NULL);
      }
    dlen = strlen (current_dir);
    current_dir[dlen++] = '/';

    /* Turn /foo/./bar into /foo/bar. */
    if (file[0] == '.' && file[1] == '/')
      file += 2;

    strcpy (current_dir + dlen, file);
    free (disposer);
    return (current_dir);
  }
}

#if !defined (HAVE_STRCASECMP)

#if !defined (to_upper)
#  define to_upper(c) (islower(c) ? toupper(c) : (c))
#endif /* to_upper */

/* Compare at most COUNT characters from string1 to string2.  Case
   doesn't matter. */
int
strnicmp (string1, string2, count)
     char *string1, *string2;
     int count;
{
  register char ch1, ch2;

  while (count)
    {
      ch1 = *string1++;
      ch2 = *string2++;
      if (to_upper(ch1) == to_upper(ch2))
	count--;
      else
	break;
    }
  return (count);
}

/* strcmp (), but caseless. */
int
stricmp (string1, string2)
     char *string1, *string2;
{
  register char ch1, ch2;

  while (*string1 && *string2)
    {
      ch1 = *string1++;
      ch2 = *string2++;
      if (to_upper(ch1) != to_upper(ch2))
	return (1);
    }
  return (*string1 - *string2);
}
#endif /* !HAVE_STRCASECMP */

/* Determine if s2 occurs in s1.  If so, return a pointer to the
   match in s1.  The compare is case insensitive. */
char *
strindex (s1, s2)
     char *s1, *s2;
{
  register int i, l = strlen (s2);
  register int len = strlen (s1);

  for (i = 0; (len - i) >= l; i++)
    if (strnicmp (s1 + i, s2, l) == 0)
      return (s1 + i);
  return ((char *)NULL);
}

/* Set the environment variables $LINES and $COLUMNS in response to
   a window size change. */
void
set_lines_and_columns (lines, cols)
     int lines, cols;
{
  char *val;

  val = itos (lines);
  bind_variable ("LINES", val);
  free (val);

  val = itos (cols);
  bind_variable ("COLUMNS", val);
  free (val);
}

/* A wrapper for bcopy that can be prototyped in general.h */
void
xbcopy (s, d, n)
     char *s, *d;
     int n;
{
  FASTCOPY (s, d, n);
}

/* Return a string corresponding to the error number E.  From
   the ANSI C spec. */
#if defined (strerror)
#  undef strerror
#endif

#if !defined (HAVE_STRERROR)
char *
strerror (e)
     int e;
{
  extern int sys_nerr;
  extern char *sys_errlist[];
  static char emsg[40];

  if (e > 0 && e < sys_nerr)
    return (sys_errlist[e]);
  else
    {
      sprintf (emsg, "Unknown error %d", e);
      return (&emsg[0]);
    }
}
#endif /* HAVE_STRERROR */

#if (defined (USG) && !defined (HAVE_TIMEVAL)) || defined (Minix)
#  define TIMEVAL_MISSING
#endif

#if !defined (TIMEVAL_MISSING) || defined (HAVE_RESOURCE)
/* Print the contents of a struct timeval * in a standard way. */
void
print_timeval (tvp)
     struct timeval *tvp;
{
  int minutes, seconds_fraction;
  long seconds;

  seconds = tvp->tv_sec;

  seconds_fraction = tvp->tv_usec % 1000000;
  seconds_fraction = (seconds_fraction * 100) / 1000000;

  minutes = seconds / 60;
  seconds %= 60;

  printf ("%0dm%0ld.%02ds",  minutes, seconds, seconds_fraction);
}
#endif /* !TIMEVAL_MISSING || HAVE_RESOURCE */

/* Print the time defined by a time_t (returned by the `times' and `time'
   system calls) in a standard way.  This is scaled in terms of HZ, which
   is what is returned by the `times' call. */

#if !defined (BrainDeath)
#  if !defined (HZ)
#    if defined (USG)
#      define HZ 100		/* From my Sys V.3.2 manual for times(2) */
#    else
#      define HZ 60		/* HZ is always 60 on BSD systems */
#    endif /* USG */
#  endif /* HZ */

void
print_time_in_hz (t)
  time_t t;
{
  int minutes, seconds_fraction;
  long seconds;

  seconds_fraction = t % HZ;
  seconds_fraction = (seconds_fraction * 100) / HZ;

  seconds = t / HZ;

  minutes = seconds / 60;
  seconds %= 60;

  printf ("%0dm%0ld.%02ds",  minutes, seconds, seconds_fraction);
}
#endif /* BrainDeath */

#if !defined (HAVE_DUP2)
/* Replacement for dup2 (), for those systems which either don't have it,
   or supply one with broken behaviour. */
int
dup2 (fd1, fd2)
     int fd1, fd2;
{
  extern int getdtablesize ();
  int saved_errno, r;

  /* If FD1 is not a valid file descriptor, then return immediately with
     an error. */
  if (fcntl (fd1, F_GETFL, 0) == -1)
    return (-1);

  if (fd2 < 0 || fd2 >= getdtablesize ())
    {
      errno = EBADF;
      return (-1);
    }

  if (fd1 == fd2)
    return (0);

  saved_errno = errno;

  (void) close (fd2);
  r = fcntl (fd1, F_DUPFD, fd2);

  if (r >= 0)
    errno = saved_errno;
  else
    if (errno == EINVAL)
      errno = EBADF;

  /* Force the new file descriptor to remain open across exec () calls. */
  SET_OPEN_ON_EXEC (fd2);
  return (r);
}
#endif /* !HAVE_DUP2 */

/*
 * Return the total number of available file descriptors.
 *
 * On some systems, like 4.2BSD and its descendents, there is a system call
 * that returns the size of the descriptor table: getdtablesize().  There are
 * lots of ways to emulate this on non-BSD systems.
 *
 * On System V.3, this can be obtained via a call to ulimit:
 *	return (ulimit(4, 0L));
 *
 * On other System V systems, NOFILE is defined in /usr/include/sys/param.h
 * (this is what we assume below), so we can simply use it:
 *	return (NOFILE);
 *
 * On POSIX systems, there are specific functions for retrieving various
 * configuration parameters:
 *	return (sysconf(_SC_OPEN_MAX));
 *
 */

#if !defined (USG) && !defined (HPUX) && !defined (HAVE_GETDTABLESIZE)
#  define HAVE_GETDTABLESIZE
#endif /* !USG && !HPUX && !HAVE_GETDTABLESIZE */

#if defined (hppa) && (defined (hpux_8) || defined (hpux_9))
#  undef HAVE_GETDTABLESIZE
#endif /* hppa && hpux_8 */

#if !defined (HAVE_GETDTABLESIZE)
int
getdtablesize ()
{
#  if defined (_POSIX_VERSION) && defined (_SC_OPEN_MAX)
  return (sysconf(_SC_OPEN_MAX));	/* Posix systems use sysconf */
#  else /* ! (_POSIX_VERSION && _SC_OPEN_MAX) */
#    if defined (USGr3)
  return (ulimit (4, 0L));	/* System V.3 systems use ulimit(4, 0L) */
#    else /* !USGr3 */
#      if defined (NOFILE)	/* Other systems use NOFILE */
  return (NOFILE);
#      else /* !NOFILE */
  return (20);			/* XXX - traditional value is 20 */
#      endif /* !NOFILE */
#    endif /* !USGr3 */
#  endif /* ! (_POSIX_VERSION && _SC_OPEN_MAX) */
}
#endif /* !HAVE_GETDTABLESIZE */

#if defined (USG)

#if !defined (HAVE_BCOPY)
bcopy (s,d,n) char *d,*s; { FASTCOPY (s, d, n); }
bzero (s,n) char *s; int n; { memset(s, '\0', n); }
#endif /* !HAVE_BCOPY */

#if !defined (HAVE_GETHOSTNAME)
#include <sys/utsname.h>
int
gethostname (name, namelen)
     char *name;
     int namelen;
{
  int i;
  struct utsname ut;

  --namelen;

  uname (&ut);
  i = strlen (ut.nodename) + 1;
  strncpy (name, ut.nodename, i < namelen ? i : namelen);
  name[namelen] = '\0';
  return (0);
}
#endif /* !HAVE_GETHOSTNAME */
#endif /* USG */

#if !defined (HAVE_GETWD)
char *
getwd (string)
     char *string;
{
  extern char *getcwd ();
  char *result;

  result = getcwd (string, MAXPATHLEN);
  if (result == NULL)
    strcpy (string, "getwd: cannot access parent directories");
  return (result);
}
#endif /* !HAVE_GETWD */

/* A slightly related function.  Get the prettiest name of this
   directory possible. */
static char tdir[MAXPATHLEN];

/* Return a pretty pathname.  If the first part of the pathname is
   the same as $HOME, then replace that with `~'.  */
char *
polite_directory_format (name)
     char *name;
{
  char *home = get_string_value ("HOME");
  int l = home ? strlen (home) : 0;

  if (l > 1 && strncmp (home, name, l) == 0 && (!name[l] || name[l] == '/'))
    {
      strcpy (tdir + 1, name + l);
      tdir[0] = '~';
      return (tdir);
    }
  else
    return (name);
}

#if defined (NO_READ_RESTART_ON_SIGNAL)
static char localbuf[128];
static int local_index = 0, local_bufused = 0;

/* Posix and USG systems do not guarantee to restart read () if it is
   interrupted by a signal.  We do the read ourselves, and restart it
   if it returns EINTR. */
int
getc_with_restart (stream)
     FILE *stream;
{
  /* Try local buffering to reduce the number of read(2) calls. */
  if (local_index == local_bufused || local_bufused == 0)
    {
      while (1)
	{
	  local_bufused = read (fileno (stream), localbuf, sizeof(localbuf));
	  if (local_bufused > 0)
	    break;
	  else if (local_bufused == 0 || errno != EINTR)
	    {
	      local_index = 0;
	      return EOF;
	    }
	}
      local_index = 0;
    }
  return (localbuf[local_index++]);
}

int
ungetc_with_restart (c, fp)
     int c;
     FILE *fp;
{
  if (local_index == 0 || local_bufused == 0 || c == EOF)
    return EOF;
  return (localbuf[--local_index] = c);
}

#endif /* NO_READ_RESTART_ON_SIGNAL */

#if defined (USG) || defined (AIX) || (defined (_POSIX_VERSION) && defined (Ultrix))
/* USG and strict POSIX systems do not have killpg ().  But we use it in
   jobs.c, nojobs.c and some of the builtins.  This can also be redefined
   as a macro if necessary. */
#if !defined (_POSIX_VERSION)
#  define pid_t int
#endif /* _POSIX_VERSION */

int
killpg (pgrp, sig)
     pid_t pgrp;
     int sig;
{
  return (kill (-pgrp, sig));
}
#endif /* USG  || AIX || (_POSIX_VERSION && Ultrix) */

/* **************************************************************** */
/*								    */
/*		    Tilde Initialization and Expansion		    */
/*								    */
/* **************************************************************** */

/* If tilde_expand hasn't been able to expand the text, perhaps it
   is a special shell expansion.  This function is installed as the
   tilde_expansion_failure_hook.  It knows how to expand ~- and ~+. */
static char *
bash_tilde_expand (text)
     char *text;
{
  char *result = (char *)NULL;

  if (!text[1])
    {
      if (*text == '+')
        result = get_string_value ("PWD");
      else if (*text == '-')
        result = get_string_value ("OLDPWD");
    }

  if (result)
    result = savestring (result);

  return (result);
}

/* Initialize the tilde expander.  In Bash, we handle `~-' and `~+', as
   well as handling special tilde prefixes; `:~" and `=~' are indications
   that we should do tilde expansion. */
void
tilde_initialize ()
{
  static int times_called = 0;

  /* Tell the tilde expander that we want a crack if it fails. */
  tilde_expansion_failure_hook = (CPFunction *)bash_tilde_expand;

  /* Tell the tilde expander about special strings which start a tilde
     expansion, and the special strings that end one.  Only do this once.
     tilde_initialize () is called from within bashline_reinitialize (). */
  if (times_called == 0)
    {
      tilde_additional_prefixes = (char **)xmalloc (3 * sizeof (char *));
      tilde_additional_prefixes[0] = "=~";
      tilde_additional_prefixes[1] = ":~";
      tilde_additional_prefixes[2] = (char *)NULL;

      tilde_additional_suffixes = (char **)xmalloc (3 * sizeof (char *));
      tilde_additional_suffixes[0] = ":";
      tilde_additional_suffixes[1] = "=~";
      tilde_additional_suffixes[2] = (char *)NULL;
    }
  times_called++;
}

#if defined (_POSIX_VERSION)

#if !defined (SA_INTERRUPT)
#  define SA_INTERRUPT 0
#endif

#if !defined (SA_RESTART)
#  define SA_RESTART 0
#endif

SigHandler *
set_signal_handler (sig, handler)
     int sig;
     SigHandler *handler;
{
  struct sigaction act, oact;

  act.sa_handler = handler;
  act.sa_flags = 0;
#if 0
  if (sig == SIGALRM)
    act.sa_flags |= SA_INTERRUPT;	/* XXX */
  else
    act.sa_flags |= SA_RESTART;		/* XXX */
#endif
  sigemptyset (&act.sa_mask);
  sigemptyset (&oact.sa_mask);
  sigaction (sig, &act, &oact);
  return (oact.sa_handler);
}
#endif /* _POSIX_VERSION */
