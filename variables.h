/* variables.h -- data structures for shell variables. */

/* Copyright (C) 1987-2025 Free Software Foundation, Inc.

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

#if !defined (_VARIABLES_H_)
#define _VARIABLES_H_

#include "stdc.h"
#include "array.h"
#include "assoc.h"

/* Shell variables and functions are stored in hash tables. */
#include "hashlib.h"

#include "conftypes.h"

/* A variable context. */
typedef struct var_context {
  char *name;		/* empty or NULL means global context */
  int scope;		/* 0 means global context */
  int flags;
  struct var_context *up;	/* previous function calls */
  struct var_context *down;	/* down towards global context */
  HASH_TABLE *table;		/* variables at this scope */
} VAR_CONTEXT;

/* Flags for var_context->flags */
#define VC_HASLOCAL	0x01
#define VC_HASTMPVAR	0x02
#define VC_FUNCENV	0x04	/* also function if name != NULL */
#define VC_BLTNENV	0x08	/* builtin_env */
#define VC_TEMPENV	0x10	/* temporary_env */
#define VC_SPECTEMPENV	0x20	/* temporary environment preceding a posix special builtin */

#define VC_TEMPFLAGS	(VC_FUNCENV|VC_BLTNENV|VC_TEMPENV)

/* Accessing macros */
#define vc_isfuncenv(vc)	(((vc)->flags & VC_FUNCENV) != 0)
#define vc_isbltnenv(vc)	(((vc)->flags & VC_BLTNENV) != 0)
#define vc_istempenv(vc)	(((vc)->flags & (VC_TEMPFLAGS)) == VC_TEMPENV)

#define vc_istempscope(vc)	(((vc)->flags & (VC_TEMPENV|VC_BLTNENV)) != 0)

#define vc_haslocals(vc)	(((vc)->flags & VC_HASLOCAL) != 0)
#define vc_hastmpvars(vc)	(((vc)->flags & VC_HASTMPVAR) != 0)

/* What a shell variable looks like. */

typedef struct variable *sh_var_value_func_t (struct variable *);
typedef struct variable *sh_var_assign_func_t (struct variable *, char *, arrayind_t, char *);

/* For the future */
union _value {
  char *s;			/* string value */
  intmax_t i;			/* int value */
  COMMAND *f;			/* function */
  ARRAY *a;			/* array */
  HASH_TABLE *h;		/* associative array */
  double d;			/* floating point number */
#if defined (HAVE_LONG_DOUBLE)
  long double ld;		/* long double */
#endif
  struct variable *v;		/* possible indirect variable use */
  void *opaque;			/* opaque data for future use */
};

typedef struct variable {
  char *name;			/* Symbol that the user types. */
  char *value;			/* Value that is returned. */
  char *exportstr;		/* String for the environment. */
  sh_var_value_func_t *dynamic_value;	/* Function called to return a `dynamic'
				   value for a variable, like $SECONDS
				   or $RANDOM. */
  sh_var_assign_func_t *assign_func; /* Function called when this `special
				   variable' is assigned a value in
				   bind_variable. */
  int attributes;		/* export, readonly, array, invisible... */
  int context;			/* Which context this variable belongs to. */
} SHELL_VAR;

typedef struct _vlist {
  SHELL_VAR **list;
  size_t list_size;	/* allocated size */
  size_t list_len;	/* current number of entries */
} VARLIST;

/* The various attributes that a given variable can have. */
/* First, the user-visible attributes */
#define att_exported	0x0000001	/* export to environment */
#define att_readonly	0x0000002	/* cannot change */
#define att_array	0x0000004	/* value is an array */
#define att_function	0x0000008	/* value is a function */
#define att_integer	0x0000010	/* internal representation is int */
#define att_local	0x0000020	/* variable is local to a function */
#define att_assoc	0x0000040	/* variable is an associative array */
#define att_trace	0x0000080	/* function is traced with DEBUG trap */
#define att_uppercase	0x0000100	/* word converted to uppercase on assignment */
#define att_lowercase	0x0000200	/* word converted to lowercase on assignment */
#define att_capcase	0x0000400	/* word capitalized on assignment */
#define att_nameref	0x0000800	/* word is a name reference */

#define attmask_user	0x0000fff

#define user_attrs	(att_exported|att_readonly|att_integer|att_local|att_trace|att_uppercase|att_lowercase|att_capcase|att_nameref)

/* These define attributes you can set on readonly variables using declare.
   You're allowed to set the readonly attribute on a readonly variable.
   declare checks whether it gets +r explicitly, before testing these.
   att_nameref is in there because declare performs its own validation due
   to some ksh93 quirks. */
#define valid_readonly_attrs	(att_exported|att_local|att_nameref|att_trace|att_readonly)
#define invalid_readonly_attrs	(~valid_readonly_attrs & attmask_user)

/* Internal attributes used for bookkeeping */
#define att_invisible	0x0001000	/* cannot see */
#define att_nounset	0x0002000	/* cannot unset */
#define att_noassign	0x0004000	/* assignment not allowed */
#define att_imported	0x0008000	/* came from environment */
#define att_special	0x0010000	/* requires special handling */
#define att_nofree	0x0020000	/* do not free value on unset */
#define att_regenerate	0x0040000	/* regenerate when exported */

#define	attmask_int	0x00ff000

/* Internal attributes used for variable scoping. */
#define att_tempvar	0x0100000	/* variable came from the temp environment */
#define att_propagate	0x0200000	/* propagate to previous scope */

#define attmask_scope	0x0f00000

#define exported_p(var)		((((var)->attributes) & (att_exported)))
#define readonly_p(var)		((((var)->attributes) & (att_readonly)))
#define array_p(var)		((((var)->attributes) & (att_array)))
#define function_p(var)		((((var)->attributes) & (att_function)))
#define integer_p(var)		((((var)->attributes) & (att_integer)))
#define local_p(var)		((((var)->attributes) & (att_local)))
#define assoc_p(var)		((((var)->attributes) & (att_assoc)))
#define trace_p(var)		((((var)->attributes) & (att_trace)))
#define uppercase_p(var)	((((var)->attributes) & (att_uppercase)))
#define lowercase_p(var)	((((var)->attributes) & (att_lowercase)))
#define capcase_p(var)		((((var)->attributes) & (att_capcase)))
#define nameref_p(var)		((((var)->attributes) & (att_nameref)))

#define invisible_p(var)	((((var)->attributes) & (att_invisible)))
#define non_unsettable_p(var)	((((var)->attributes) & (att_nounset)))
#define noassign_p(var)		((((var)->attributes) & (att_noassign)))
#define imported_p(var)		((((var)->attributes) & (att_imported)))
#define specialvar_p(var)	((((var)->attributes) & (att_special)))
#define nofree_p(var)		((((var)->attributes) & (att_nofree)))
#define regen_p(var)		((((var)->attributes) & (att_regenerate)))

#define tempvar_p(var)		((((var)->attributes) & (att_tempvar)))
#define propagate_p(var)	((((var)->attributes) & (att_propagate)))

/* Variable names: lvalues */
#define name_cell(var)		((var)->name)

/* Accessing variable values: rvalues */
#define value_cell(var)		((var)->value)
#define function_cell(var)	(COMMAND *)((var)->value)
#define array_cell(var)		(ARRAY *)((var)->value)
#define assoc_cell(var)		(HASH_TABLE *)((var)->value)
#define nameref_cell(var)	((var)->value)		/* so it can change later */

#define NAMEREF_MAX	8	/* only 8 levels of nameref indirection */

#define var_isset(var)		((var)->value != 0)
#define var_isunset(var)	((var)->value == 0)
#define var_isnull(var)		((var)->value && *(var)->value == 0)

/* Assigning variable values: lvalues */
#define var_setvalue(var, str)	((var)->value = (str))
#define var_setfunc(var, func)	((var)->value = (char *)(func))
#define var_setarray(var, arr)	((var)->value = (char *)(arr))
#define var_setassoc(var, arr)	((var)->value = (char *)(arr))
#define var_setref(var, str)	((var)->value = (str))

/* Make VAR be auto-exported. */
#define set_auto_export(var) \
  do { (var)->attributes |= att_exported; array_needs_making = 1; } while (0)

#define SETVARATTR(var, attr, undo) \
	((undo == 0) ? ((var)->attributes |= (attr)) \
		     : ((var)->attributes &= ~(attr)))

#define VSETATTR(var, attr)	((var)->attributes |= (attr))
#define VUNSETATTR(var, attr)	((var)->attributes &= ~(attr))

#define VGETFLAGS(var)		((var)->attributes)

#define VSETFLAGS(var, flags)	((var)->attributes = (flags))
#define VCLRFLAGS(var)		((var)->attributes = 0)

/* Macros to perform various operations on `exportstr' member of a SHELL_VAR. */
#define CLEAR_EXPORTSTR(var)	(var)->exportstr = (char *)NULL
#define COPY_EXPORTSTR(var)	((var)->exportstr) ? savestring ((var)->exportstr) : (char *)NULL
#define SET_EXPORTSTR(var, value)  (var)->exportstr = (value)
#define SAVE_EXPORTSTR(var, value) (var)->exportstr = (value) ? savestring (value) : (char *)NULL

#define FREE_EXPORTSTR(var) \
	do { if ((var)->exportstr) free ((var)->exportstr); } while (0)

#define CACHE_IMPORTSTR(var, value) \
	(var)->exportstr = savestring (value)

#define INVALIDATE_EXPORTSTR(var) \
	do { \
	  if ((var)->exportstr) \
	    { \
	      free ((var)->exportstr); \
	      (var)->exportstr = (char *)NULL; \
	    } \
	} while (0)

#define ifsname(s)	((s)[0] == 'I' && (s)[1] == 'F' && (s)[2] == 'S' && (s)[3] == '\0')

/* Flag values for make_local_variable and its array counterparts */
#define MKLOC_ASSOCOK		0x01
#define MKLOC_ARRAYOK		0x02
#define MKLOC_INHERIT		0x04

/* Special value for nameref with invalid value for creation or assignment */
extern SHELL_VAR nameref_invalid_value;
#define INVALID_NAMEREF_VALUE	(void *)&nameref_invalid_value

/* Assignment statements */
#define ASSIGN_DISALLOWED(v, f) \
  ((readonly_p (v) && (f&ASS_FORCE) == 0) || noassign_p (v))
	
/* Stuff for hacking variables. */
typedef int sh_var_map_func_t (SHELL_VAR *);

/* Where we keep the variables and functions */
extern VAR_CONTEXT *global_variables;
extern VAR_CONTEXT *shell_variables;

extern HASH_TABLE *shell_functions;
extern HASH_TABLE *temporary_env;

extern int variable_context;
extern char *dollar_vars[];
extern char **export_env;

extern int tempenv_assign_error;
extern int array_needs_making;
extern int shell_level;

/* XXX */
extern WORD_LIST *rest_of_args;
extern int posparam_count;
extern pid_t dollar_dollar_pid;

extern int localvar_inherit;		/* declared in variables.c */

extern void initialize_shell_variables (char **, int);

extern int validate_inherited_value (SHELL_VAR *, int);

extern SHELL_VAR *set_if_not (const char *, const char *);

extern void sh_set_lines_and_columns (int, int);
extern void set_pwd (void);
extern void set_ppid (void);
extern void make_funcname_visible (int);

extern SHELL_VAR *var_lookup (const char *, VAR_CONTEXT *);

extern SHELL_VAR *find_function (const char *);
extern FUNCTION_DEF *find_function_def (const char *);
extern SHELL_VAR *find_variable (const char *);
extern SHELL_VAR *find_variable_noref (const char *);
extern SHELL_VAR *find_variable_last_nameref (const char *, int);
extern SHELL_VAR *find_global_variable_last_nameref (const char *, int);
extern SHELL_VAR *find_variable_nameref (SHELL_VAR *);
extern SHELL_VAR *find_variable_nameref_for_create (const char *, int);
extern SHELL_VAR *find_variable_nameref_for_assignment (const char *, int);
/*extern SHELL_VAR *find_variable_internal (const char *, int);*/
extern SHELL_VAR *find_variable_tempenv (const char *);
extern SHELL_VAR *find_variable_notempenv (const char *);
extern SHELL_VAR *find_global_variable (const char *);
extern SHELL_VAR *find_global_variable_noref (const char *);
extern SHELL_VAR *find_shell_variable (const char *);
extern SHELL_VAR *find_tempenv_variable (const char *);
extern SHELL_VAR *find_variable_no_invisible (const char *);
extern SHELL_VAR *find_variable_for_assignment (const char *);
extern char *nameref_transform_name (const char *, int);
extern SHELL_VAR *copy_variable (SHELL_VAR *);
extern SHELL_VAR *make_local_variable (const char *, int);
extern SHELL_VAR *bind_variable (const char *, const char *, int);
extern SHELL_VAR *bind_global_variable (const char *, const char *, int);
extern SHELL_VAR *bind_function (const char *, COMMAND *);

extern void bind_function_def (const char *, FUNCTION_DEF *, int);

extern SHELL_VAR **map_over (sh_var_map_func_t *, VAR_CONTEXT *);
SHELL_VAR **map_over_funcs (sh_var_map_func_t *);
     
extern SHELL_VAR **all_shell_variables (void);
extern SHELL_VAR **all_shell_functions (void);
extern SHELL_VAR **all_visible_variables (void);
extern SHELL_VAR **all_visible_functions (void);
extern SHELL_VAR **all_exported_variables (void);
extern SHELL_VAR **local_exported_variables (void);
extern SHELL_VAR **all_local_variables (int);
#if defined (ARRAY_VARS)
extern SHELL_VAR **all_array_variables (void);
#endif
extern char **all_variables_matching_prefix (const char *);

extern char **make_var_array (HASH_TABLE *);
extern char **add_or_supercede_exported_var (char *, int);

extern char *get_variable_value (SHELL_VAR *);
extern char *get_string_value (const char *);
extern char *sh_get_env_value (const char *);
extern char *make_variable_value (SHELL_VAR *, const char *, int);

extern SHELL_VAR *bind_variable_value (SHELL_VAR *, char *, int);
extern SHELL_VAR *bind_int_variable (const char *, const char *, int);
extern SHELL_VAR *bind_var_to_int (const char *, intmax_t, int);

extern int assign_in_env (const WORD_DESC *, int);

extern int posix_unbind_tempvar (const char *);
extern int unbind_variable (const char *);
extern int check_unbind_variable (const char *);
extern int unbind_nameref (const char *);
extern int unbind_variable_noref (const char *);
extern int unbind_global_variable (const char *);
extern int unbind_global_variable_noref (const char *);
extern int unbind_func (const char *);
extern int unbind_function_def (const char *);
extern int delete_var (const char *, VAR_CONTEXT *);
extern int makunbound (const char *, VAR_CONTEXT *);
extern int kill_local_variable (const char *);

extern void delete_all_variables (HASH_TABLE *);
extern void delete_all_contexts (VAR_CONTEXT *);
extern void reset_local_contexts (void);

extern VAR_CONTEXT *new_var_context (char *, int);
extern void dispose_var_context (VAR_CONTEXT *);
extern VAR_CONTEXT *push_var_context (char *, int, HASH_TABLE *);
extern void pop_var_context (void);
extern VAR_CONTEXT *push_scope (int, HASH_TABLE *);
extern void pop_scope (void *);		/* XXX uw_ */

extern void clear_dollar_vars (void);

extern void push_context (char *, int, HASH_TABLE *);
extern void pop_context (void *);	/* XXX uw_ */
extern void push_dollar_vars (void);
extern void pop_dollar_vars (void);
extern void dispose_saved_dollar_vars (void);

extern void init_bash_argv (void);
extern void save_bash_argv (void);
extern void push_args (WORD_LIST *);
extern void pop_args (void);
extern void uw_pop_args (void *);

#if defined (ARRAY_VARS)
extern void push_source (ARRAY *, char *);
#endif

extern void adjust_shell_level (int);
extern void non_unsettable (char *);
extern void dispose_variable (SHELL_VAR *);
extern void dispose_used_env_vars (void);
extern void dispose_function_env (void);
extern void dispose_builtin_env (void);
extern void merge_temporary_env (void);
extern void merge_function_temporary_env (void);
extern void flush_temporary_env (void);
extern HASH_TABLE *copy_temporary_env (void);
extern void merge_builtin_env (void);
extern void kill_all_local_variables (void);

extern HASH_TABLE *copy_vartab (HASH_TABLE *);

extern void set_var_read_only (char *);
extern void set_func_read_only (const char *);
extern void set_var_auto_export (char *);
extern void set_func_auto_export (const char *);

extern void sort_variables (SHELL_VAR **);

extern int chkexport (char *);
extern void maybe_make_export_env (void);
extern void update_export_env_inplace (char *, int, char *);
extern void put_command_name_into_env (char *);
extern void put_gnu_argv_flags_into_env (intmax_t, char *);

extern void print_var_list (SHELL_VAR **);
extern void print_func_list (SHELL_VAR **);
extern void print_assignment (SHELL_VAR *);
extern void print_var_value (SHELL_VAR *, int);
extern void print_var_function (SHELL_VAR *);

#if defined (ARRAY_VARS)
extern SHELL_VAR *make_new_array_variable (const char *);
extern SHELL_VAR *make_local_array_variable (const char *, int);

extern SHELL_VAR *make_new_assoc_variable (const char *);
extern SHELL_VAR *make_local_assoc_variable (const char *, int);

extern void set_pipestatus_array (int *, int);
extern ARRAY *save_pipestatus_array (void);
extern void restore_pipestatus_array (ARRAY *);
#endif

extern void set_pipestatus_from_exit (int);

/* The variable in NAME has just had its state changed.  Check to see if it
   is one of the special ones where something special happens. */
extern void stupidly_hack_special_variables (const char *);

/* Reinitialize some special variables that have external effects upon unset
   when the shell reinitializes itself. */
extern void reinit_special_variables (void);

extern int get_random_number (void);

/* The `special variable' functions that get called when a particular
   variable is set. */
extern void sv_ifs (const char *);
extern void sv_path (const char *);
extern void sv_mail (const char *);
extern void sv_funcnest (const char *);
extern void sv_execignore (const char *);
extern void sv_globignore (const char *);
extern void sv_ignoreeof (const char *);
extern void sv_strict_posix (const char *);
extern void sv_optind (const char *);
extern void sv_opterr (const char *);
extern void sv_locale (const char *);
extern void sv_xtracefd (const char *);
extern void sv_shcompat (const char *);
extern void sv_globsort (const char *);

#if defined (READLINE)
extern void sv_comp_wordbreaks (const char *);
extern void sv_terminal (const char *);
extern void sv_hostfile (const char *);
extern void sv_winsize (const char *);
#endif

#if defined (__CYGWIN__)
extern void sv_home (const char *);
#endif

#if defined (HISTORY)
extern void sv_histsize (const char *);
extern void sv_histignore (const char *);
extern void sv_history_control (const char *);
#  if defined (BANG_HISTORY)
extern void sv_histchars (const char *);
#  endif
extern void sv_histtimefmt (const char *);
#endif /* HISTORY */

#if defined (HAVE_TZSET)
extern void sv_tz (const char *);
#endif

#if defined (JOB_CONTROL)
extern void sv_childmax (const char *);
#endif

#endif /* !_VARIABLES_H_ */
