/* general.h -- defines that everybody likes to use. */

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

#if !defined (_GENERAL_H)
#define _GENERAL_H

#include "stdc.h"

/* just to make sure */
#if defined (HAVE_UNISTD_H)
#  ifdef CRAY
#    define word __word
#  endif
#  include <unistd.h>
#  ifdef CRAY
#    undef word
#  endif
#endif

#if !defined (NULL)
#  if defined (__STDC__)
#    define NULL ((void *) 0)
#  else
#    define NULL 0x0
#  endif /* !__STDC__ */
#endif /* !NULL */

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#define pointer_to_int(x) (int)((long)(x))

#if !defined (savestring)
   extern char *xmalloc ();
#  if !defined (strcpy)
   extern char *strcpy ();
#  endif
#  define savestring(x) (char *)strcpy (xmalloc (1 + strlen (x)), (x))
#endif

#ifndef whitespace
#define whitespace(c) (((c) == ' ') || ((c) == '\t'))
#endif

#ifndef digit
#define digit(c)  ((c) >= '0' && (c) <= '9')
#endif

#ifndef isletter
#define isletter(c) (((c) >= 'A' && (c) <= 'Z') || ((c) >= 'a' && (c) <= 'z'))
#endif

#ifndef digit_value
#define digit_value(c) ((c) - '0')
#endif

/* Definitions used in subst.c and by the `read' builtin for field
   splitting. */
#define spctabnl(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

#if !defined (__STDC__) && !defined (strchr)
extern char *strchr (), *strrchr ();
#endif /* !strchr */

#ifndef member
#  if defined (alpha) && defined (__GNUC__)	/* XXX */
     extern char *strchr ();
#  endif
#  define member(c, s) ((c) ? ((char *)strchr ((s), (c)) != (char *)NULL) : 0)
#endif

/* All structs which contain a `next' field should have that field
   as the first field in the struct.  This means that functions
   can be written to handle the general case for linked lists. */
typedef struct g_list {
  struct g_list *next;
} GENERIC_LIST;

/* Here is a generic structure for associating character strings
   with integers.  It is used in the parser for shell tokenization. */
typedef struct {
  char *word;
  int token;
} STRING_INT_ALIST;

/* A macro to avoid making an uneccessary function call. */
#define REVERSE_LIST(list, type) \
  ((list && list->next) ? (type)reverse_list ((GENERIC_LIST *)list) : (type)(list))

#if __GNUC__ > 1
#  define FASTCOPY(s, d, n)  __builtin_memcpy (d, s, n)
#else /* !__GNUC__ */
#  if defined (USG) && !defined (HAVE_BCOPY)
#    if defined (MEMMOVE_MISSING)
#      define FASTCOPY(s, d, n)  memcpy (d, s, n)
#    else
#      define FASTCOPY(s, d, n)  memmove (d, s, n)
#    endif /* !MEMMOVE_MISSING */
#  else
#    define FASTCOPY(s, d, n)  bcopy (s, d, n)
#  endif /* !USG || HAVE_BCOPY */
#endif /* !__GNUC__ */

/* String comparisons that possibly save a function call each. */
#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0)
#define STREQN(a, b, n) ((a)[0] == (b)[0] && strncmp(a, b, n) == 0)

/* More convenience definitions that possibly save system or libc calls. */
#define STRLEN(s) (((s) && (s)[0]) ? ((s)[1] ? ((s)[2] ? strlen(s) : 2) : 1) : 0)
#define FREE(s)  do { if (s) free (s); } while (0)
#define MEMBER(c, s) (((c) && !(s)[1] && c == s[0]) || (member(c, s)))

/* What type is a `generic' pointer?  This is used as the first argument
   to xrealloc. */
#if defined (__STDC__)
typedef void *GENPTR;
#else
typedef char *GENPTR;
#endif

/* Function pointers can be declared as (Function *)foo. */
#if !defined (__FUNCTION_DEF)
#  define __FUNCTION_DEF
typedef int Function ();
typedef void VFunction ();
typedef char *CPFunction ();
typedef char **CPPFunction ();
#endif /* _FUNCTION_DEF */

#define NOW	((time_t) time ((time_t *) 0))

/* Some defines for calling file status functions. */
#define FS_EXISTS	  0x1
#define FS_EXECABLE	  0x2
#define FS_EXEC_PREFERRED 0x4
#define FS_EXEC_ONLY	  0x8

/* Posix and USG systems do not guarantee to restart a read () that is
   interrupted by a signal. */
#if defined (USG) || defined (_POSIX_VERSION)
#  define NO_READ_RESTART_ON_SIGNAL
#endif /* USG || _POSIX_VERSION */

/* Here is a definition for set_signal_handler () which simply expands to
   a call to signal () for non-Posix systems.  The code for set_signal_handler
   in the Posix case resides in general.c. */

#if defined (VOID_SIGHANDLER)
#  define sighandler void
#else
#  define sighandler int
#endif /* !VOID_SIGHANDLER */

typedef sighandler SigHandler ();

#if !defined (_POSIX_VERSION)
#  define set_signal_handler(sig, handler) (SigHandler *)signal (sig, handler)
#else
extern SigHandler *set_signal_handler ();
#endif /* _POSIX_VERSION */

/* This function is defined in trap.c. */
extern SigHandler *set_sigint_handler __P((void));

/* Declarations for functions defined in general.c */
extern char *xmalloc __P((int));
extern char *xrealloc __P((void *, int));
extern void xfree __P((char *));
extern char *itos __P((int));
extern int all_digits __P((char *));
extern long string_to_long __P((char *));
extern int legal_identifier __P((char *));
extern int check_identifier __P((WORD_DESC *, int));
extern void unset_nodelay_mode __P((int));
extern void map_over_words __P((WORD_LIST *, Function *));

extern void map_over_list __P((GENERIC_LIST *, Function *));
extern GENERIC_LIST *reverse_list ();
extern GENERIC_LIST *delete_element ();
extern GENERIC_LIST *list_append ();
extern int list_length ();
extern int qsort_string_compare ();

extern int find_name_in_list __P((char *, char **));
extern int array_len __P((char **));
extern void free_array __P((char **));
extern char **copy_array __P((char **));
extern void strip_leading __P((char *));
extern void strip_trailing __P((char *, int));
extern char *canonicalize_pathname __P((char *));
extern char *make_absolute __P((char *, char *));
extern int absolute_pathname __P((char *));
extern int absolute_program __P((char *));
extern char *base_pathname __P((char *));
extern char *full_pathname __P((char *));
extern char *strindex __P((char *, char *));
extern void set_lines_and_columns __P((int, int));
extern void xbcopy __P((char *, char *, int));
extern char *polite_directory_format __P((char *));
extern void tilde_initialize __P((void));

#if !defined (strerror)
extern char *strerror __P((int));
#endif

#if defined (RLIMTYPE)
extern RLIMTYPE string_to_rlimtype __P((char *));
extern void print_rlimtype __P((RLIMTYPE, int));
#endif

#if !defined (HAVE_STRCASECMP)
extern int strnicmp __P((char *, char *, int));
extern int stricmp __P((char *, char *));
#else /* HAVE_STRCASECMP */
#  define stricmp strcasecmp
#  define strnicmp strncasecmp
#endif /* HAVE_STRCASECMP */

extern int dup2 __P((int, int));
extern char *getwd __P((char *));
extern int getdtablesize __P((void));

#if defined (USG) && !defined (HAVE_GETHOSTNAME)
extern int gethostname __P((char *, int));
#endif /* USG && !HAVE_GETHOSTNAME */

#endif	/* _GENERAL_H */
