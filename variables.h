/* variables.h -- data structures for shell variables. */

#if !defined (_VARIABLES_H_)
#define _VARIABLES_H_

#include "stdc.h"
#include "array.h"

/* Shell variables and functions are stored in hash tables. */
#include "hashlib.h"

/* What a shell variable looks like. */

typedef struct variable *DYNAMIC_FUNC ();

typedef struct variable {
  char *name;			/* Symbol that the user types. */
  char *value;			/* Value that is returned. */
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

/* The various attributes that a given variable can have.
   We only reserve one byte of the INT. */
#define att_exported  0x01	/* export to environment */
#define att_readonly  0x02	/* cannot change */
#define att_invisible 0x04	/* cannot see */
#define att_array     0x08	/* value is an array */
#define att_nounset   0x10	/* cannot unset */
#define att_function  0x20	/* value is a function */
#define att_integer   0x40	/* internal representation is int */
#define att_imported  0x80	/* came from environment */
#define att_local     0x100	/* variable is local to a function */

#define exported_p(var)		((((var)->attributes) & (att_exported)))
#define readonly_p(var)		((((var)->attributes) & (att_readonly)))
#define invisible_p(var)	((((var)->attributes) & (att_invisible)))
#define array_p(var)		((((var)->attributes) & (att_array)))
#define non_unsettable_p(var)	((((var)->attributes) & (att_nounset)))
#define function_p(var)		((((var)->attributes) & (att_function)))
#define integer_p(var)		((((var)->attributes) & (att_integer)))
#define imported_p(var)         ((((var)->attributes) & (att_imported)))
#define local_p(var)		((((var)->attributes) & (att_local)))

#define value_cell(var) ((var)->value)
#define function_cell(var) (COMMAND *)((var)->value)
#define array_cell(var) ((ARRAY *)(var)->value)

#define SETVARATTR(var, attr, undo) \
	((undo == 0) ? ((var)->attributes |= (attribute)) \
		     : ((var)->attributes &= ~(attribute)))

/* Stuff for hacking variables. */
extern int variable_context;
extern HASH_TABLE *shell_variables, *shell_functions;
extern char *dollar_vars[];
extern char **export_env;
extern char **non_unsettable_vars;

extern void initialize_shell_variables __P((char **, int));
extern SHELL_VAR *set_if_not __P((char *, char *));
extern void set_lines_and_columns __P((int, int));

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

extern char **make_var_array __P((HASH_TABLE *));
extern char **add_or_supercede_exported_var __P((char *, int));

extern char *get_string_value __P((char *));
extern char *make_variable_value __P((SHELL_VAR *, char *));

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
#endif

#endif /* !_VARIABLES_H_ */
