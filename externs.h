/* externs.h -- extern function declarations which do not appear in their
   own header file. */

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
   Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

/* Make sure that this is included *after* config.h! */

#if !defined (_EXTERNS_H_)
#  define _EXTERNS_H_

#include "stdc.h"

/* Functions from expr.c. */
extern long evalexp __P((char *, int *));

/* Functions from print_cmd.c. */
extern char *make_command_string __P((COMMAND *));
extern void print_command __P((COMMAND *));
extern void print_simple_command __P((SIMPLE_COM *));
extern char *named_function_string __P((char *, COMMAND *, int));
extern void print_word_list __P((WORD_LIST *, char *));
extern void xtrace_print_word_list __P((WORD_LIST *));
#if defined (DPAREN_ARITHMETIC)
extern void xtrace_print_arith_cmd __P((WORD_LIST *));
#endif
#if defined (COND_COMMAND)
extern void xtrace_print_cond_term __P((int, int, WORD_DESC *, char *, char *));
#endif

/* Functions from shell.c. */
extern void exit_shell __P((int));
extern void disable_priv_mode __P((void));
extern void unbind_args __P((void));

#if defined (RESTRICTED_SHELL)
extern int shell_is_restricted __P((char *));
extern int maybe_make_restricted __P((char *));
#endif

extern void unset_bash_input __P((int));
extern void get_current_user_info __P((void));

/* Functions from eval.c. */
extern int reader_loop __P((void));
extern int parse_command __P((void));
extern int read_command __P((void));

/* Functions from braces.c. */
#if defined (BRACE_EXPANSION)
extern char **brace_expand __P((char *));
#endif

/* Miscellaneous functions from parse.y */
extern int yyparse __P((void));
extern void reset_parser __P((void));
extern WORD_LIST *parse_string_to_word_list __P((char *, char *));

extern int get_current_prompt_level __P((void));
extern void set_current_prompt_level __P((int));

/* Functions from version.c. */
extern char *shell_version_string __P((void));
extern void show_shell_version __P((int));

/* Declarations for functions defined in locale.c */
extern void set_default_locale __P((void));
extern void set_default_locale_vars __P((void));
extern int set_locale_var __P((char *, char *));
extern int set_lang __P((char *, char *));
extern char *get_locale_var __P((char *));
extern char *localetrans __P((char *, int, int *));

/* Declarations for functions defined in list.c. */
extern void map_over_list __P((GENERIC_LIST *, Function *));
extern void map_over_words __P((WORD_LIST *, Function *));
extern GENERIC_LIST *reverse_list ();
extern int list_length ();
extern GENERIC_LIST *list_append ();
extern GENERIC_LIST *delete_element ();

/* Declarations for functions defined in stringlib.c */
extern char *ansicstr __P((char *, int, int, int *, int *));
extern int find_name_in_array __P((char *, char **));
extern char **alloc_array __P((int));
extern int array_len __P((char **));
extern void free_array_members __P((char **));
extern void free_array __P((char **));
extern char **copy_array __P((char **));
extern int qsort_string_compare ();
extern void sort_char_array __P((char **));
extern char **word_list_to_argv __P((WORD_LIST *, int, int, int *));
extern WORD_LIST *argv_to_word_list __P((char **, int, int));

extern int find_string_in_alist __P((char *, STRING_INT_ALIST *, int));

extern char *strsub __P((char *, char *, char *, int));
extern char *strcreplace __P((char *, int, char *, int));
extern void strip_leading __P((char *));
extern void strip_trailing __P((char *, int, int));
extern char *strindex __P((char *, char *));
extern void xbcopy __P((char *, char *, int));

/* Functions from the bash library, lib/sh/libsh.a.  These should really
   go into a separate include file. */
/* Declarations for functions defined in lib/sh/getcwd.c */
#if !defined (HAVE_GETCWD)
extern char *getcwd __P((char *, size_t));
#endif

/* Declarations for functions defined in lib/sh/itos.c */
extern char *inttostr __P((int, char *, int));
extern char *itos __P((int));

/* Declarations for  functions defined in lib/sh/oslib.c */
extern long get_clk_tck __P((void));

#if !defined (strerror)
extern char *strerror __P((int));
#endif

#if !defined (HAVE_STRCASECMP)
extern int strncasecmp __P((const char *, const char *, int));
extern int strcasecmp __P((const char *, const char *));
#endif /* HAVE_STRCASECMP */

extern int dup2 __P((int, int));

#if !defined (HAVE_GETHOSTNAME)
extern int gethostname __P((char *, int));
#endif /* !HAVE_GETHOSTNAME */

#if !defined (HAVE_GETDTABLESIZE)
extern int getdtablesize __P((void));
#endif /* !HAVE_GETDTABLESIZE */

#if !defined (HAVE_SETLINEBUF)
extern int setlinebuf ();
#endif

/* declarations for functions defined in lib/sh/strtod.c */
#if !defined (HAVE_STRTOD)
extern double strtod __P((const char *, char **));
#endif

/* declarations for functions defined in lib/sh/strtol.c */
#if !defined (HAVE_STRTOL)
extern long strtol __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/strtoul.c */
#if !defined (HAVE_STRTOUL)
extern unsigned long strtoul __P((const char *, char **, int));
#endif

/* declarations for functions defined in lib/sh/zread.c */
extern int zread __P((int, char *, size_t));
extern int zread1 __P((int, char *, size_t));
extern int zreadc __P((int, char *));
extern void zreset __P((void));
extern void zsyncfd __P((int));

/* declarations for functions defined in lib/sh/zwrite.c */
extern int zwrite __P((int, unsigned char *, size_t));

/* declarations for functions defined in lib/sh/netopen.c */
extern int netopen __P((char *));

/* declarations for functions defined in lib/sh/timeval.c.  No prototypes
   so we don't have to count on having a definition of struct timeval in
   scope when this file is included. */
extern void timeval_to_secs ();
extern void print_timeval ();

/* declarations for functions defined in lib/sh/clock.c */
extern void clock_t_to_secs ();
extern void print_clock_t ();

/* declarations for functions defined in lib/sh/makepath.c */
#define MP_DOTILDE	0x01
#define MP_DOCWD	0x02
#define MP_RMDOT	0x04

extern char *sh_makepath __P((char *, char *, int));

#endif /* _EXTERNS_H_ */
