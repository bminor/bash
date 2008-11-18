/* arrayfunc.c -- High-level array functions used by other parts of the shell. */

/* Copyright (C) 2001-2006 Free Software Foundation, Inc.

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

#include "config.h"

#if defined (ARRAY_VARS)

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include <stdio.h>

#include "bashintl.h"

#include "shell.h"

#include "shmbutil.h"

#include "builtins/common.h"

extern char *this_command_name;
extern int last_command_exit_value;
extern int array_needs_making;

static SHELL_VAR *bind_array_var_internal __P((SHELL_VAR *, arrayind_t, char *, int));

static void quote_array_assignment_chars __P((WORD_LIST *));
static char *array_value_internal __P((char *, int, int, int *));

/* Standard error message to use when encountering an invalid array subscript */
char *bash_badsub_errmsg = N_("bad array subscript");

/* **************************************************************** */
/*								    */
/*  Functions to manipulate array variables and perform assignments */
/*								    */
/* **************************************************************** */

/* Convert a shell variable to an array variable.  The original value is
   saved as array[0]. */
SHELL_VAR *
convert_var_to_array (var)
     SHELL_VAR *var;
{
  char *oldval;
  ARRAY *array;

  oldval = value_cell (var);
  array = array_create ();
  if (oldval)
    array_insert (array, 0, oldval);

  FREE (value_cell (var));
  var_setarray (var, array);

  /* these aren't valid anymore */
  var->dynamic_value = (sh_var_value_func_t *)NULL;
  var->assign_func = (sh_var_assign_func_t *)NULL;

  INVALIDATE_EXPORTSTR (var);
  if (exported_p (var))
    array_needs_making++;

  VSETATTR (var, att_array);
  VUNSETATTR (var, att_invisible);

  return var;
}

static SHELL_VAR *
bind_array_var_internal (entry, ind, value, flags)
     SHELL_VAR *entry;
     arrayind_t ind;
     char *value;
     int flags;
{
  SHELL_VAR *dentry;
  char *newval;

  /* If we're appending, we need the old value of the array reference, so
     fake out make_variable_value with a dummy SHELL_VAR */
  if (flags & ASS_APPEND)
    {
      dentry = (SHELL_VAR *)xmalloc (sizeof (SHELL_VAR));
      dentry->name = savestring (entry->name);
      newval = array_reference (array_cell (entry), ind);
      if (newval)
	dentry->value = savestring (newval);
      else
	{
	  dentry->value = (char *)xmalloc (1);
	  dentry->value[0] = '\0';
	}
      dentry->exportstr = 0;
      dentry->attributes = entry->attributes & ~(att_array|att_exported);
      /* Leave the rest of the members uninitialized; the code doesn't look
	 at them. */
      newval = make_variable_value (dentry, value, flags);	 
      dispose_variable (dentry);
    }
  else
    newval = make_variable_value (entry, value, flags);

  if (entry->assign_func)
    (*entry->assign_func) (entry, newval, ind);
  else
    array_insert (array_cell (entry), ind, newval);
  FREE (newval);

  return (entry);
}

/* Perform an array assignment name[ind]=value.  If NAME already exists and
   is not an array, and IND is 0, perform name=value instead.  If NAME exists
   and is not an array, and IND is not 0, convert it into an array with the
   existing value as name[0].

   If NAME does not exist, just create an array variable, no matter what
   IND's value may be. */
SHELL_VAR *
bind_array_variable (name, ind, value, flags)
     char *name;
     arrayind_t ind;
     char *value;
     int flags;
{
  SHELL_VAR *entry;

  entry = var_lookup (name, shell_variables);

  if (entry == (SHELL_VAR *) 0)
    entry = make_new_array_variable (name);
  else if (readonly_p (entry) || noassign_p (entry))
    {
      if (readonly_p (entry))
	err_readonly (name);
      return (entry);
    }
  else if (array_p (entry) == 0)
    entry = convert_var_to_array (entry);

  /* ENTRY is an array variable, and ARRAY points to the value. */
  return (bind_array_var_internal (entry, ind, value, flags));
}

/* Parse NAME, a lhs of an assignment statement of the form v[s], and
   assign VALUE to that array element by calling bind_array_variable(). */
SHELL_VAR *
assign_array_element (name, value, flags)
     char *name, *value;
     int flags;
{
  char *sub, *vname;
  arrayind_t ind;
  int sublen;
  SHELL_VAR *entry;

  vname = array_variable_name (name, &sub, &sublen);

  if (vname == 0)
    return ((SHELL_VAR *)NULL);

  if ((ALL_ELEMENT_SUB (sub[0]) && sub[1] == ']') || (sublen <= 1))
    {
      free (vname);
      err_badarraysub (name);
      return ((SHELL_VAR *)NULL);
    }

  ind = array_expand_index (sub, sublen);
  if (ind < 0)
    {
      free (vname);
      err_badarraysub (name);
      return ((SHELL_VAR *)NULL);
    }

  entry = bind_array_variable (vname, ind, value, flags);

  free (vname);
  return (entry);
}

/* Find the array variable corresponding to NAME.  If there is no variable,
   create a new array variable.  If the variable exists but is not an array,
   convert it to an indexed array.  If CHECK_FLAGS is non-zero, an existing
   variable is checked for the readonly or noassign attribute in preparation
   for assignment (e.g., by the `read' builtin). */
SHELL_VAR *
find_or_make_array_variable (name, check_flags)
     char *name;
     int check_flags;
{
  SHELL_VAR *var;

  var = find_variable (name);

  if (var == 0)
    var = make_new_array_variable (name);
  else if (check_flags && (readonly_p (var) || noassign_p (var)))
    {
      if (readonly_p (var))
	err_readonly (name);
      return ((SHELL_VAR *)NULL);
    }
  else if (array_p (var) == 0)
    var = convert_var_to_array (var);

  return (var);
}
  
/* Perform a compound assignment statement for array NAME, where VALUE is
   the text between the parens:  NAME=( VALUE ) */
SHELL_VAR *
assign_array_from_string (name, value, flags)
     char *name, *value;
     int flags;
{
  SHELL_VAR *var;

  var = find_or_make_array_variable (name, 1);
  if (var == 0)
    return ((SHELL_VAR *)NULL);

  return (assign_array_var_from_string (var, value, flags));
}

/* Sequentially assign the indices of indexed array variable VAR from the
   words in LIST. */
SHELL_VAR *
assign_array_var_from_word_list (var, list, flags)
     SHELL_VAR *var;
     WORD_LIST *list;
     int flags;
{
  register arrayind_t i;
  register WORD_LIST *l;
  ARRAY *a;

  a = array_cell (var);
  i = (flags & ASS_APPEND) ? array_max_index (a) + 1 : 0;

  for (l = list; l; l = l->next, i++)
    if (var->assign_func)
      (*var->assign_func) (var, l->word->word, i);
    else
      array_insert (a, i, l->word->word);
  return var;
}

WORD_LIST *
expand_compound_array_assignment (value, flags)
     char *value;
     int flags;
{
  WORD_LIST *list, *nlist;
  char *val;
  int ni;

  /* I don't believe this condition is ever true any more. */
  if (*value == '(')	/*)*/
    {
      ni = 1;
      val = extract_array_assignment_list (value, &ni);
      if (val == 0)
	return (WORD_LIST *)NULL;
    }
  else
    val = value;

  /* Expand the value string into a list of words, performing all the
     shell expansions including pathname generation and word splitting. */
  /* First we split the string on whitespace, using the shell parser
     (ksh93 seems to do this). */
  list = parse_string_to_word_list (val, 1, "array assign");

  /* If we're using [subscript]=value, we need to quote each [ and ] to
     prevent unwanted filename expansion. */
  if (list)
    quote_array_assignment_chars (list);

  /* Now that we've split it, perform the shell expansions on each
     word in the list. */
  nlist = list ? expand_words_no_vars (list) : (WORD_LIST *)NULL;

  dispose_words (list);

  if (val != value)
    free (val);

  return nlist;
}

void
assign_compound_array_list (var, nlist, flags)
     SHELL_VAR *var;
     WORD_LIST *nlist;
     int flags;
{
  ARRAY *a;
  WORD_LIST *list;
  char *w, *val, *nval;
  int len, iflags;
  arrayind_t ind, last_ind;

  a = array_cell (var);

  /* Now that we are ready to assign values to the array, kill the existing
     value. */
  if (a && (flags & ASS_APPEND) == 0)
    array_flush (a);
  last_ind = (flags & ASS_APPEND) ? array_max_index (a) + 1 : 0;

  for (list = nlist; list; list = list->next)
    {
      iflags = flags;
      w = list->word->word;

      /* We have a word of the form [ind]=value */
      if ((list->word->flags & W_ASSIGNMENT) && w[0] == '[')
	{
	  len = skipsubscript (w, 0);

	  /* XXX - changes for `+=' */
 	  if (w[len] != ']' || (w[len+1] != '=' && (w[len+1] != '+' || w[len+2] != '=')))
	    {
	      nval = make_variable_value (var, w, flags);
	      if (var->assign_func)
		(*var->assign_func) (var, nval, last_ind);
	      else
		array_insert (a, last_ind, nval);
	      FREE (nval);
	      last_ind++;
	      continue;
	    }

	  if (len == 1)
	    {
	      err_badarraysub (w);
	      continue;
	    }

	  if (ALL_ELEMENT_SUB (w[1]) && len == 2)
	    {
	      report_error (_("%s: cannot assign to non-numeric index"), w);
	      continue;
	    }

	  ind = array_expand_index (w + 1, len);
	  if (ind < 0)
	    {
	      err_badarraysub (w);
	      continue;
	    }
	  last_ind = ind;
	  /* XXX - changes for `+=' -- just accept the syntax.  ksh93 doesn't do this */
	  if (w[len + 1] == '+' && w[len + 2] == '=')
	    {
	      iflags |= ASS_APPEND;
	      val = w + len + 3;
	    }
	  else
	    val = w + len + 2;
	}
      else		/* No [ind]=value, just a stray `=' */
	{
	  ind = last_ind;
	  val = w;
	}

      if (integer_p (var))
	this_command_name = (char *)NULL;	/* no command name for errors */
      bind_array_var_internal (var, ind, val, iflags);
      last_ind++;
    }
}

/* Perform a compound array assignment:  VAR->name=( VALUE ).  The
   VALUE has already had the parentheses stripped. */
SHELL_VAR *
assign_array_var_from_string (var, value, flags)
     SHELL_VAR *var;
     char *value;
     int flags;
{
  WORD_LIST *nlist;

  if (value == 0)
    return var;

  nlist = expand_compound_array_assignment (value, flags);
  assign_compound_array_list (var, nlist, flags);

  if (nlist)
    dispose_words (nlist);
  return (var);
}

/* For each word in a compound array assignment, if the word looks like
   [ind]=value, quote the `[' and `]' before the `=' to protect them from
   unwanted filename expansion. */
static void
quote_array_assignment_chars (list)
     WORD_LIST *list;
{
  char *s, *t, *nword;
  int saw_eq;
  WORD_LIST *l;

  for (l = list; l; l = l->next)
    {
      if (l->word == 0 || l->word->word == 0 || l->word->word[0] == '\0')
	continue;	/* should not happen, but just in case... */
      /* Don't bother if it doesn't look like [ind]=value */
      if (l->word->word[0] != '[' || xstrchr (l->word->word, '=') == 0) /* ] */
	continue;
      s = nword = (char *)xmalloc (strlen (l->word->word) * 2 + 1);
      saw_eq = 0;
      for (t = l->word->word; *t; )
	{
	  if (*t == '=')
	    saw_eq = 1;
	  if (saw_eq == 0 && (*t == '[' || *t == ']'))
	    *s++ = '\\';
	  *s++ = *t++;
	}
      *s = '\0';
      free (l->word->word);
      l->word->word = nword;
    }
}

/* This function assumes s[i] == '['; returns with s[ret] == ']' if
   an array subscript is correctly parsed. */
int
skipsubscript (s, i)
     const char *s;
     int i;
{
  int count, c;
#if defined (HANDLE_MULTIBYTE)
  mbstate_t state, state_bak;
  size_t slength, mblength;
#endif

#if defined (HANDLE_MULTIBYTE)
  memset (&state, '\0', sizeof (mbstate_t));
  slength = strlen (s + i);
#endif
  
  count = 1;
  while (count)
    {
      /* Advance one (possibly multibyte) character in S starting at I. */
#if defined (HANDLE_MULTIBYTE)
      if (MB_CUR_MAX > 1)
	{
	  state_bak = state;
	  mblength = mbrlen (s + i, slength, &state);

	  if (MB_INVALIDCH (mblength))
	    {
	      state = state_bak;
	      i++;
	      slength--;
	    }
	  else if (MB_NULLWCH (mblength))
	    return i;
	  else
	    {
	      i += mblength;
	      slength -= mblength;
	    }
	}
      else
#endif
      ++i;

      c = s[i];

      if (c == 0)
        break;
      else if (c == '[')
	count++;
      else if (c == ']')
	count--;
    }

  return i;
}

/* This function is called with SUB pointing to just after the beginning
   `[' of an array subscript and removes the array element to which SUB
   expands from array VAR.  A subscript of `*' or `@' unsets the array. */
int
unbind_array_element (var, sub)
     SHELL_VAR *var;
     char *sub;
{
  int len;
  arrayind_t ind;
  ARRAY_ELEMENT *ae;

  len = skipsubscript (sub, 0);
  if (sub[len] != ']' || len == 0)
    {
      builtin_error ("%s[%s: %s", var->name, sub, _(bash_badsub_errmsg));
      return -1;
    }
  sub[len] = '\0';

  if (ALL_ELEMENT_SUB (sub[0]) && sub[1] == 0)
    {
      unbind_variable (var->name);
      return (0);
    }
  ind = array_expand_index (sub, len+1);
  if (ind < 0)
    {
      builtin_error ("[%s]: %s", sub, _(bash_badsub_errmsg));
      return -1;
    }
  ae = array_remove (array_cell (var), ind);
  if (ae)
    array_dispose_element (ae);
  return 0;
}

/* Format and output an array assignment in compound form VAR=(VALUES),
   suitable for re-use as input. */
void
print_array_assignment (var, quoted)
     SHELL_VAR *var;
     int quoted;
{
  char *vstr;

  vstr = array_to_assign (array_cell (var), quoted);

  if (vstr == 0)
    printf ("%s=%s\n", var->name, quoted ? "'()'" : "()");
  else
    {
      printf ("%s=%s\n", var->name, vstr);
      free (vstr);
    }
}

/***********************************************************************/
/*								       */
/* Utility functions to manage arrays and their contents for expansion */
/*								       */
/***********************************************************************/

/* Return 1 if NAME is a properly-formed array reference v[sub]. */
int
valid_array_reference (name)
     char *name;
{
  char *t;
  int r, len;

  t = xstrchr (name, '[');	/* ] */
  if (t)
    {
      *t = '\0';
      r = legal_identifier (name);
      *t = '[';
      if (r == 0)
	return 0;
      /* Check for a properly-terminated non-blank subscript. */
      len = skipsubscript (t, 0);
      if (t[len] != ']' || len == 1)
	return 0;
      for (r = 1; r < len; r++)
	if (whitespace (t[r]) == 0)
	  return 1;
      return 0;
    }
  return 0;
}

/* Expand the array index beginning at S and extending LEN characters. */
arrayind_t
array_expand_index (s, len)
     char *s;
     int len;
{
  char *exp, *t;
  int expok;
  arrayind_t val;

  exp = (char *)xmalloc (len);
  strncpy (exp, s, len - 1);
  exp[len - 1] = '\0';
  t = expand_arith_string (exp, 0);
  this_command_name = (char *)NULL;
  val = evalexp (t, &expok);
  free (t);
  free (exp);
  if (expok == 0)
    {
      last_command_exit_value = EXECUTION_FAILURE;

      top_level_cleanup ();      
      jump_to_top_level (DISCARD);
    }
  return val;
}

/* Return the name of the variable specified by S without any subscript.
   If SUBP is non-null, return a pointer to the start of the subscript
   in *SUBP. If LENP is non-null, the length of the subscript is returned
   in *LENP.  This returns newly-allocated memory. */
char *
array_variable_name (s, subp, lenp)
     char *s, **subp;
     int *lenp;
{
  char *t, *ret;
  int ind, ni;

  t = xstrchr (s, '[');
  if (t == 0)
    {
      if (subp)
      	*subp = t;
      if (lenp)
	*lenp = 0;
      return ((char *)NULL);
    }
  ind = t - s;
  ni = skipsubscript (s, ind);
  if (ni <= ind + 1 || s[ni] != ']')
    {
      err_badarraysub (s);
      if (subp)
      	*subp = t;
      if (lenp)
	*lenp = 0;
      return ((char *)NULL);
    }

  *t = '\0';
  ret = savestring (s);
  *t++ = '[';		/* ] */

  if (subp)
    *subp = t;
  if (lenp)
    *lenp = ni - ind;

  return ret;
}

/* Return the variable specified by S without any subscript.  If SUBP is
   non-null, return a pointer to the start of the subscript in *SUBP.
   If LENP is non-null, the length of the subscript is returned in *LENP. */
SHELL_VAR *
array_variable_part (s, subp, lenp)
     char *s, **subp;
     int *lenp;
{
  char *t;
  SHELL_VAR *var;

  t = array_variable_name (s, subp, lenp);
  if (t == 0)
    return ((SHELL_VAR *)NULL);
  var = find_variable (t);

  free (t);
  return (var == 0 || invisible_p (var)) ? (SHELL_VAR *)0 : var;
}

/* Return a string containing the elements in the array and subscript
   described by S.  If the subscript is * or @, obeys quoting rules akin
   to the expansion of $* and $@ including double quoting.  If RTYPE
   is non-null it gets 1 if the array reference is name[@] or name[*]
   and 0 otherwise. */
static char *
array_value_internal (s, quoted, allow_all, rtype)
     char *s;
     int quoted, allow_all, *rtype;
{
  int len;
  arrayind_t ind;
  char *retval, *t, *temp;
  WORD_LIST *l;
  SHELL_VAR *var;

  var = array_variable_part (s, &t, &len);

  /* Expand the index, even if the variable doesn't exist, in case side
     effects are needed, like ${w[i++]} where w is unset. */
#if 0
  if (var == 0)
    return (char *)NULL;
#endif

  if (len == 0)
    return ((char *)NULL);	/* error message already printed */

  /* [ */
  if (ALL_ELEMENT_SUB (t[0]) && t[1] == ']')
    {
      if (rtype)
	*rtype = (t[0] == '*') ? 1 : 2;
      if (allow_all == 0)
	{
	  err_badarraysub (s);
	  return ((char *)NULL);
	}
      else if (var == 0 || value_cell (var) == 0)
	return ((char *)NULL);
      else if (array_p (var) == 0)
	l = add_string_to_list (value_cell (var), (WORD_LIST *)NULL);
      else
	{
	  l = array_to_word_list (array_cell (var));
	  if (l == (WORD_LIST *)NULL)
	    return ((char *) NULL);
	}

      if (t[0] == '*' && (quoted & (Q_HERE_DOCUMENT|Q_DOUBLE_QUOTES)))
	{
	  temp = string_list_dollar_star (l);
	  retval = quote_string (temp);
	  free (temp);
	}
      else	/* ${name[@]} or unquoted ${name[*]} */
	retval = string_list_dollar_at (l, quoted);

      dispose_words (l);
    }
  else
    {
      if (rtype)
	*rtype = 0;
      ind = array_expand_index (t, len);
      if (ind < 0)
	{
	  if (var)
	    err_badarraysub (var->name);
	  else
	    {
	      t[-1] = '\0';
	      err_badarraysub (s);
	      t[-1] = '[';	/* ] */
	    }
	  return ((char *)NULL);
	}
      if (var == 0)
	return ((char *)NULL);
      if (array_p (var) == 0)
	return (ind == 0 ? value_cell (var) : (char *)NULL);
      retval = array_reference (array_cell (var), ind);
    }

  return retval;
}

/* Return a string containing the elements described by the array and
   subscript contained in S, obeying quoting for subscripts * and @. */
char *
array_value (s, quoted, rtype)
     char *s;
     int quoted, *rtype;
{
  return (array_value_internal (s, quoted, 1, rtype));
}

/* Return the value of the array indexing expression S as a single string.
   If ALLOW_ALL is 0, do not allow `@' and `*' subscripts.  This is used
   by other parts of the shell such as the arithmetic expression evaluator
   in expr.c. */
char *
get_array_value (s, allow_all, rtype)
     char *s;
     int allow_all, *rtype;
{
  return (array_value_internal (s, 0, allow_all, rtype));
}

char *
array_keys (s, quoted)
     char *s;
     int quoted;
{
  int len;
  char *retval, *t, *temp;
  WORD_LIST *l;
  SHELL_VAR *var;

  var = array_variable_part (s, &t, &len);

  /* [ */
  if (var == 0 || ALL_ELEMENT_SUB (t[0]) == 0 || t[1] != ']')
    return (char *)NULL;

  if (array_p (var) == 0)
    l = add_string_to_list ("0", (WORD_LIST *)NULL);
  else
    {
      l = array_keys_to_word_list (array_cell (var));
      if (l == (WORD_LIST *)NULL)
        return ((char *) NULL);
    }

  if (t[0] == '*' && (quoted & (Q_HERE_DOCUMENT|Q_DOUBLE_QUOTES)))
    {
      temp = string_list_dollar_star (l);
      retval = quote_string (temp);
      free (temp);
    }
  else	/* ${!name[@]} or unquoted ${!name[*]} */
    retval = string_list_dollar_at (l, quoted);

  dispose_words (l);
  return retval;
}
#endif /* ARRAY_VARS */
