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

#if !defined (_GENERAL_H_)
#define _GENERAL_H_

#include "stdc.h"

#include "bashtypes.h"

#if defined (HAVE_SYS_RESOURCE_H) && defined (RLIMTYPE)
#  if defined (HAVE_SYS_TIME_H)
#    include <sys/time.h>
#  endif
#  include <sys/resource.h>
#endif

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif /* !HAVE_STRING_H */

/* Generic pointer type. */
#if defined (__STDC__)
#  define PTR_T	void *
#else
#  define PTR_T char *
#endif

/* NULL pointer type. */
#if !defined (NULL)
#  if defined (__STDC__)
#    define NULL ((void *) 0)
#  else
#    define NULL 0x0
#  endif /* !__STDC__ */
#endif /* !NULL */

#define pointer_to_int(x) (int)((long)(x))

extern char *xmalloc (), *xrealloc ();

#if defined (alpha) && defined (__GNUC__) && !defined (strchr) && !defined (__STDC__)
extern char *strchr (), *strrchr ();
#endif

#if !defined (strcpy)
extern char *strcpy ();
#endif

#if !defined (savestring)
#  define savestring(x) (char *)strcpy (xmalloc (1 + strlen (x)), (x))
#endif

#ifndef member
#  define member(c, s) ((c) ? ((char *)strchr ((s), (c)) != (char *)NULL) : 0)
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

/* Define exactly what a legal shell identifier consists of. */
#define legal_variable_starter(c) (isletter(c) || (c == '_'))
#define legal_variable_char(c)	(isletter (c) || digit (c) || c == '_')

/* Definitions used in subst.c and by the `read' builtin for field
   splitting. */
#define spctabnl(c)	((c) == ' ' || (c) == '\t' || (c) == '\n')

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
  ((list && list->next) ? (type)reverse_list ((GENERIC_LIST *)list) \
  			: (type)(list))

#if __GNUC__ > 1
#  define FASTCOPY(s, d, n)  __builtin_memcpy (d, s, n)
#else /* !__GNUC__ */
#  if !defined (HAVE_BCOPY)
#    if !defined (HAVE_MEMMOVE)
#      define FASTCOPY(s, d, n)  memcpy (d, s, n)
#    else
#      define FASTCOPY(s, d, n)  memmove (d, s, n)
#    endif /* !HAVE_MEMMOVE */
#  else /* HAVE_BCOPY */
#    define FASTCOPY(s, d, n)  bcopy (s, d, n)
#  endif /* HAVE_BCOPY */
#endif /* !__GNUC__ */

/* String comparisons that possibly save a function call each. */
#define STREQ(a, b) ((a)[0] == (b)[0] && strcmp(a, b) == 0)
#define STREQN(a, b, n) ((a)[0] == (b)[0] && strncmp(a, b, n) == 0)

/* More convenience definitions that possibly save system or libc calls. */
#define STRLEN(s) (((s) && (s)[0]) ? ((s)[1] ? ((s)[2] ? strlen(s) : 2) : 1) : 0)
#define FREE(s)  do { if (s) free (s); } while (0)
#define MEMBER(c, s) (((c) && c == (s)[0] && !(s)[1]) || (member(c, s)))

/* A fairly hairy macro to check whether an allocated string has more room,
   and to resize it using xrealloc if it does not.
   STR is the string (char *)
   CIND is the current index into the string (int)
   ROOM is the amount of additional room we need in the string (int)
   CSIZE is the currently-allocated size of STR (int)
   SINCR is how much to increment CSIZE before calling xrealloc (int) */

#define RESIZE_MALLOCED_BUFFER(str, cind, room, csize, sincr) \
  do { \
    if ((cind) + (room) >= csize) \
      { \
	while ((cind) + (room) >= csize) \
	  csize += (sincr); \
	str = xrealloc (str, csize); \
      } \
  } while (0)

/* Function pointers can be declared as (Function *)foo. */
#if !defined (_FUNCTION_DEF)
#  define _FUNCTION_DEF
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
#define FS_DIRECTORY	  0x10
#define FS_NODIRS	  0x20

/* Declarations for functions defined in xmalloc.c */
extern char *xmalloc __P((size_t));
extern char *xrealloc __P((void *, size_t));
extern void xfree __P((char *));

/* Declarations for functions defined in general.c */
extern void posix_initialize __P((int));

#if defined (RLIMTYPE)
extern RLIMTYPE string_to_rlimtype __P((char *));
extern void print_rlimtype __P((RLIMTYPE, int));
#endif

extern void timeval_to_secs ();
extern void print_timeval ();
extern void clock_t_to_secs ();
extern void print_time_in_hz ();

extern int all_digits __P((char *));
extern int legal_number __P((char *, long *));
extern int legal_identifier __P((char *));
extern int check_identifier __P((WORD_DESC *, int));

extern void unset_nodelay_mode __P((int));
extern void check_dev_tty __P((void));
extern int same_file ();	/* too many problems with prototype */
extern int move_to_high_fd __P((int, int, int));
extern int check_binary_file __P((unsigned char *, int));

extern char *canonicalize_pathname __P((char *));
extern char *make_absolute __P((char *, char *));
extern int absolute_pathname __P((char *));
extern int absolute_program __P((char *));
extern char *base_pathname __P((char *));
extern char *full_pathname __P((char *));
extern char *polite_directory_format __P((char *));

extern char *extract_colon_unit __P((char *, int *));

extern void tilde_initialize __P((void));
extern char *bash_tilde_expand __P((char *));

extern int group_member __P((gid_t));
extern char **get_group_list __P((int *));

#endif	/* _GENERAL_H_ */
