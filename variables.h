/* variables.h -- data structures for shell variables. */

/* Copyright (C) 1987,1991 Free Software Foundation, Inc.

   This file is part of GNU Bash, the Bourne Again SHell.

   Bash is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   Bash is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
   or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
   License for more details.

   You should have received a copy of the GNU General Public License
   along with Bash; see the file COPYING.  If not, write to the Free
   Software Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

#if !defined (_VARIABLES_H_)
#define _VARIABLES_H_

#include "stdc.h"
#include "array.h"

/* Shell variables and functions are stored in hash tables. */
#include "hashlib.h"

/* Placeholder for future modifications if cross-compiling or building a
   `fat' binary, e.g. on Apple Rhapsody.  These values are used in multiple
   files, so they appear here. */
#if !defined (RHAPSODY)
#  define HOSTTYPE	CONF_HOSTTYPE
#  define OSTYPE	CONF_OSTYPE
#  define MACHTYPE	CONF_MACHTYPE
#else /* RHAPSODY */
#  if   defined(__powerpc__) || defined(__ppc__)
#    define HOSTTYPE "powerpc"
#  elif defined(__i386__)
#    define HOSTTYPE "i386"
#  else
#    define HOSTTYPE CONF_HOSTTYPE
#  endif

#  define OSTYPE CONF_OSTYPE
#  define VENDOR CONF_VENDOR

#  define MACHTYPE HOSTTYPE "-" VENDOR "-" OSTYPE
#endif /* RHAPSODY */

/* What a shell variable looks like. */

typedef struct variable *DYNAMIC_FUNC ();

typedef struct variable {
  char *name;			/* Symbol that the user types. */
  char *value;			/* Value that is returned. */
  char *exportstr;		/* String for the environment. */
  DYNAMIC_FUNC *dynamic_value;	/* Function called to return a `dynamic'
				   value for a variable, like $SECONDS
				   or $RANDOM. */
  DYNAMIC_FUNC *assign_func; 	/* Function called when this `special
				   variable' is assigned a value in
				   bind_variable. */
  int attributes;		/* export, readonly, array, invisible... */
  int context;			/* Which context this variable belongs to. */
  struct variable *prev_context; /* Value from previous context or NULL. */
} SHELL_VAR;

/* The various attributes that a given variable can have. */
#define att_exported  0x001	/* export to environment */
#define att_readonly  0x002	/* cannot change */
#define att_invisible 0x004	/* cannot see */
#define att_array     0x008	/* value is an array */
#define att_nounset   0x010	/* cannot unset */
#define att_function  0x020	/* value is a function */
#define att_integer   0x040	/* internal representation is int */
#define att_imported  0x080	/* came from environment */
#define att_local     0x100	/* variable is local to a function */
#define att_tempvar   0x200	/* variable came from the temp environment */
#define att_importstr 0x400	/* exportstr points into initial environment */

#define exported_p(var)		((((var)->attributes) & (att_exported)))
#define readonly_p(var)		((((var)->attributes) & (att_readonly)))
#define invisible_p(var)	((((var)->attributes) & (att_invisible)))
#define array_p(var)		((((var)->attributes) & (att_array)))
#define non_unsettable_p(var)	((((var)->attributes) & (att_nounset)))
#define function_p(var)		((((var)->attributes) & (att_function)))
#define integer_p(var)		((((var)->attributes) & (att_integer)))
#define imported_p(var)         ((((var)->attributes) & (att_imported)))
#define local_p(var)		((((var)->attributes) & (att_local)))
#define tempvar_p(var)		((((var)->attributes) & (att_tempvar)))

#define value_cell(var) ((var)->value)
#define function_cell(var) (COMMAND *)((var)->value)
#define array_cell(var) ((ARRAY *)(var)->value)

#define SETVARATTR(var, attr, undo) \
	((undo == 0) ? ((var)->attributes |= (attr)) \
		     : ((var)->attributes &= ~(attr)))

#define VSETATTR(var, attr)	((var)->attributes |= (attr))
#define VUNSETATTR(var, attr)	((var)->attributes &= ~(attr))

/* Macros to perform various operations on `exportstr' member of a SHELL_VAR. */
#define CLEAR_EXPORTSTR(var)	(var)->exportstr = (char *)NULL
#define COPY_EXPORTSTR(var)	((var)->exportstr) ? savestring ((var)->exportstr) : (char *)NULL
#define SET_EXPORTSTR(var, value)  (var)->exportstr = (value)
#define SAVE_EXPORTSTR(var, value) (var)->exportstr = (value) ? savestring (value) : (char *)NULL

#define FREE_EXPORTSTR(var) \
	do { \
	  if ((var)->exportstr) \
	    { \
	      if (((var)->attributes & att_importstr) == 0) \
	        free ((var)->exportstr); \
	    } \
	} while (0)

#if 0
#define CACHE_IMPORTSTR(var, value) \
	do { \
	  (var)->exportstr = value; \
	  (var)->attributes |= att_importstr; \
	} while (0)
#else
#define CACHE_IMPORTSTR(var, value) \
	do { \
	  (var)->exportstr = savestring (value); \
	} while (0)
#endif

#define INVALIDATE_EXPORTSTR(var) \
	do { \
	  if ((var)->exportstr) \
	    { \
	      if (((var)->attributes & att_importstr) == 0) \
		free ((var)->exportstr); \
	      (var)->exportstr = (char *)NULL; \
	      (var)->attributes &= ~att_importstr; \
	    } \
	} while (0)
	
/* Stuff for hacking variables. */
extern int variable_context;
extern HASH_TABLE *shell_variables, *shell_functions;
extern char *dollar_vars[];
extern char **export_env;
extern char **non_unsettable_vars;

extern void initialize_shell_variables __P((char **, int));
extern SHELL_VAR *set_if_not __P((char *, char *));
extern void set_lines_and_columns __P((int, int));

extern void set_ppid __P((void));

extern void make_funcname_visible __P((int));

extern SHELL_VAR *find_function __P((char *));
extern SHELL_VAR *find_variable __P((char *));
extern SHELL_VAR *find_variable_internal __P((char *, int));
extern SHELL_VAR *find_tempenv_variable __P((char *));
extern SHELL_VAR *copy_variable __P((SHELL_VAR *));
extern SHELL_VAR *make_local_variable __P((char *));
extern SHELL_VAR *bind_variable __P((char *, char *));
extern SHELL_VAR *bind_function __P((char *, COMMAND *));

extern SHELL_VAR **map_over __P((Function *, HASH_TABLE *));
extern SHELL_VAR **all_shell_variables __P((void));
extern SHELL_VAR **all_shell_functions __P((void));
extern SHELL_VAR **all_visible_variables __P((void));
extern SHELL_VAR **all_visible_functions __P((void));
extern SHELL_VAR **all_exported_variables __P((void));
#if defined (ARRAY_VARS)
extern SHELL_VAR **all_array_variables __P((void));
#endif

extern char **all_variables_matching_prefix __P((char *));

extern char **make_var_array __P((HASH_TABLE *));
extern char **add_or_supercede_exported_var __P((char *, int));

extern char *get_string_value __P((char *));
extern char *make_variable_value __P((SHELL_VAR *, char *));

extern SHELL_VAR *bind_variable_value __P((SHELL_VAR *, char *));
extern SHELL_VAR *bind_int_variable __P((char *, char *));

extern int assignment __P((char *));
extern int variable_in_context __P((SHELL_VAR *));
extern int assign_in_env __P((char *));
extern int unbind_variable __P((char *));
extern int makunbound __P((char *, HASH_TABLE *));
extern int kill_local_variable __P((char *));
extern void delete_all_variables __P((HASH_TABLE *));

extern void adjust_shell_level __P((int));
extern void non_unsettable __P((char *));
extern void dispose_variable __P((SHELL_VAR *));
extern void dispose_used_env_vars __P((void));
extern void dispose_function_env __P((void));
extern void dispose_builtin_env __P((void));
extern void merge_temporary_env __P((void));
extern void merge_builtin_env __P((void));
extern void kill_all_local_variables __P((void));
extern void set_var_read_only __P((char *));
extern void set_func_read_only __P((char *));
extern void set_var_auto_export __P((char *));
extern void set_func_auto_export __P((char *));
extern void sort_variables __P((SHELL_VAR **));
extern void maybe_make_export_env __P((void));
extern void update_export_env_inplace __P((char *, int, char *));
extern void put_command_name_into_env __P((char *));
extern void put_gnu_argv_flags_into_env __P((int, char *));
extern void print_var_list __P((SHELL_VAR **));
extern void print_assignment __P((SHELL_VAR *));
extern void print_var_value __P((SHELL_VAR *, int));
extern void print_var_function __P((SHELL_VAR *));

extern char *indirection_level_string __P((void));

#if defined (ARRAY_VARS)
extern SHELL_VAR *make_new_array_variable __P((char *));
extern SHELL_VAR *make_local_array_variable __P((char *));
extern SHELL_VAR *convert_var_to_array __P((SHELL_VAR *));
extern SHELL_VAR *bind_array_variable __P((char *, int, char *));
extern SHELL_VAR *assign_array_from_string  __P((char *, char *));
extern SHELL_VAR *assign_array_var_from_word_list __P((SHELL_VAR *, WORD_LIST *));
extern SHELL_VAR *assign_array_var_from_string __P((SHELL_VAR *, char *));
extern int unbind_array_element __P((SHELL_VAR *, char *));
extern int skipsubscript __P((char *, int));
extern void print_array_assignment __P((SHELL_VAR *, int));

extern void set_pipestatus_array __P((int *));
#endif

extern void set_pipestatus_from_exit __P((int));

/* The variable in NAME has just had its state changed.  Check to see if it
   is one of the special ones where something special happens. */
extern void stupidly_hack_special_variables __P((char *));

/* The `special variable' functions that get called when a particular
   variable is set. */
void sv_path (), sv_mail (), sv_ignoreeof (), sv_strict_posix ();
void sv_optind (), sv_opterr (), sv_globignore (), sv_locale ();

#if defined (READLINE)
void sv_terminal (), sv_hostfile ();
#endif

#if defined (HAVE_TZSET) && defined (PROMPT_STRING_DECODE)
void sv_tz ();
#endif

#if defined (HISTORY)
void sv_histsize (), sv_histignore (), sv_history_control ();
#  if defined (BANG_HISTORY)
void sv_histchars ();
#  endif
#endif /* HISTORY */

#endif /* !_VARIABLES_H_ */
