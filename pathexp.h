/* pathexp.h -- The shell interface to the globbing library. */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

#if !defined (_PATHEXP_H_)
#define _PATHEXP_H_

#if defined (USE_POSIX_GLOB_LIBRARY)
#  define GLOB_FAILED(glist)	!(glist)
#else /* !USE_POSIX_GLOB_LIBRARY */
#  define GLOB_FAILED(glist)	(glist) == (char **)&glob_error_return
extern int noglob_dot_filenames;
extern char *glob_error_return;
#endif /* !USE_POSIX_GLOB_LIBRARY */

extern int glob_dot_filenames;

extern int unquoted_glob_pattern_p __P((char *));

/* PATHNAME can contain characters prefixed by CTLESC;; this indicates
   that the character is to be quoted.  We quote it here in the style
   that the glob library recognizes.  If CONVERT_QUOTED_NULLS is non-zero,
   we change quoted null strings (pathname[0] == CTLNUL) into empty
   strings (pathname[0] == 0).  If this is called after quote removal
   is performed, CONVERT_QUOTED_NULLS should be 0; if called when quote
   removal has not been done (for example, before attempting to match a
   pattern while executing a case statement), CONVERT_QUOTED_NULLS should
   be 1. */
extern char *quote_string_for_globbing __P((char *, int));

extern char *quote_globbing_chars __P((char *));

/* Call the glob library to do globbing on PATHNAME. */
extern char **shell_glob_filename __P((char *));

/* Filename completion ignore.  Used to the "fignore" facility of
   tcsh and GLOBIGNORE (like ksh-93 FIGNORE).

   It is passed a NULL-terminated array of (char *)'s that must be
   free()'d if they are deleted.  The first element (names[0]) is the
   least-common-denominator string of the matching patterns (i.e.
   u<TAB> produces names[0] = "und", names[1] = "under.c", names[2] =
   "undun.c", name[3] = NULL).  */

struct ign {
  char *val;
  int len, flags;
};

struct ignorevar {
  char *varname;	/* FIGNORE or GLOBIGNORE */
  struct ign *ignores;	/* Store the ignore strings here */
  int num_ignores;	/* How many are there? */
  char *last_ignoreval;	/* Last value of variable - cached for speed */
  Function *item_func;	/* Called when each item is parsed from $`varname' */
};

extern void setup_ignore_patterns __P((struct ignorevar *));

extern void setup_glob_ignore __P((char *));
extern int should_ignore_glob_matches __P((void));
extern void ignore_glob_matches __P((char **));

#endif
