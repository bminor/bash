/* I can't stand it anymore!  Please can't we just write the
   whole Unix system in lisp or something? */

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
Foundation, 59 Temple Place, Suite 330, Boston, MA 02111 USA. */

/* **************************************************************** */
/*								    */
/*		      Unwind Protection Scheme for Bash		    */
/*								    */
/* **************************************************************** */
#include "config.h"

#include "bashtypes.h"
#include "bashansi.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "command.h"
#include "general.h"
#include "unwind_prot.h"
#include "quit.h"
#include "sig.h"

/* If CLEANUP is null, then ARG contains a tag to throw back to. */
typedef struct _uwp {
  struct _uwp *next;
  Function *cleanup;
  char *arg;
} UNWIND_ELT;

/* Structure describing a saved variable and the value to restore it to.
   If a cleanup function is set to restore_variable, the `arg' pointer
   points to this. */
typedef struct {
  int *variable;
  char *desired_setting;
  int size;
} SAVED_VAR;

static void without_interrupts ();
static void unwind_frame_discard_internal ();
static void unwind_frame_run_internal ();
static void add_unwind_protect_internal ();
static void remove_unwind_protect_internal ();
static void run_unwind_protects_internal ();
static void clear_unwind_protects_internal ();
static void restore_variable ();
static void discard_saved_var ();

static UNWIND_ELT *unwind_protect_list = (UNWIND_ELT *)NULL;

extern int interrupt_immediately;

/* Run a function without interrupts.  This relies on the fact that the
   FUNCTION cannot change the value of interrupt_immediately.  (I.e., does
   not call QUIT (). */
static void
without_interrupts (function, arg1, arg2)
     VFunction *function;
     char *arg1, *arg2;
{
  int old_interrupt_immediately;

  old_interrupt_immediately = interrupt_immediately;
  interrupt_immediately = 0;

  (*function)(arg1, arg2);

  interrupt_immediately = old_interrupt_immediately;
}

/* Start the beginning of a region. */
void
begin_unwind_frame (tag)
     char *tag;
{
  add_unwind_protect ((Function *)NULL, tag);
}

/* Discard the unwind protects back to TAG. */
void
discard_unwind_frame (tag)
     char *tag;
{
  if (unwind_protect_list)
    without_interrupts (unwind_frame_discard_internal, tag, (char *)NULL);
}

/* Run the unwind protects back to TAG. */
void
run_unwind_frame (tag)
     char *tag;
{
  if (unwind_protect_list)
    without_interrupts (unwind_frame_run_internal, tag, (char *)NULL);
}

/* Add the function CLEANUP with ARG to the list of unwindable things. */
void
add_unwind_protect (cleanup, arg)
     Function *cleanup;
     char *arg;
{
  without_interrupts (add_unwind_protect_internal, (char *)cleanup, arg);
}

/* Remove the top unwind protect from the list. */
void
remove_unwind_protect ()
{
  if (unwind_protect_list)
    without_interrupts
      (remove_unwind_protect_internal, (char *)NULL, (char *)NULL);
}

/* Run the list of cleanup functions in unwind_protect_list. */
void
run_unwind_protects ()
{
  if (unwind_protect_list)
    without_interrupts
      (run_unwind_protects_internal, (char *)NULL, (char *)NULL);
}

/* Erase the unwind-protect list.  If flags is 1, free the elements. */
void
clear_unwind_protect_list (flags)
     int flags;
{
  if (unwind_protect_list)
    without_interrupts
      (clear_unwind_protects_internal, (char *)flags, (char *)NULL);
}

/* **************************************************************** */
/*								    */
/*			The Actual Functions		 	    */
/*								    */
/* **************************************************************** */

static void
add_unwind_protect_internal (cleanup, arg)
     Function *cleanup;
     char *arg;
{
  UNWIND_ELT *elt;

  elt = (UNWIND_ELT *)xmalloc (sizeof (UNWIND_ELT));
  elt->cleanup = cleanup;
  elt->arg = arg;
  elt->next = unwind_protect_list;
  unwind_protect_list = elt;
}

static void
remove_unwind_protect_internal (ignore1, ignore2)
     char *ignore1, *ignore2;
{
  UNWIND_ELT *elt;

  elt = unwind_protect_list;
  if (elt)
    {
      unwind_protect_list = unwind_protect_list->next;
      if (elt->cleanup && elt->cleanup == (Function *)restore_variable)
	discard_saved_var ((SAVED_VAR *)elt->arg);
      free (elt);
    }
}

static void
run_unwind_protects_internal (ignore1, ignore2)
     char *ignore1, *ignore2;
{
  UNWIND_ELT *t, *elt = unwind_protect_list;

  while (elt)
   {
      /* This function can be run at strange times, like when unwinding
	 the entire world of unwind protects.  Thus, we may come across
	 an element which is simply a label for a catch frame.  Don't call
	 the non-existant function. */
      if (elt->cleanup)
	(*(elt->cleanup)) (elt->arg);

      t = elt;
      elt = elt->next;
      free (t);
    }
  unwind_protect_list = elt;
}

static void
clear_unwind_protects_internal (flag, ignore)
     char *flag, *ignore;
{
  int free_elts = (int)flag;
  UNWIND_ELT *elt;

  if (free_elts != 0 && unwind_protect_list)
    {
      while (unwind_protect_list)
	remove_unwind_protect_internal ((char *)NULL, (char *)NULL);
    }
  unwind_protect_list = (UNWIND_ELT *)NULL;
}

static void
unwind_frame_discard_internal (tag, ignore)
     char *tag, *ignore;
{
  UNWIND_ELT *elt;

  while (elt = unwind_protect_list)
    {
      unwind_protect_list = unwind_protect_list->next;
      if (elt->cleanup == 0 && (STREQ (elt->arg, tag)))
	{
	  free (elt);
	  break;
	}
      else if (elt->cleanup && elt->cleanup == (Function *)restore_variable)
	{
	  discard_saved_var ((SAVED_VAR *)elt->arg);
	  free (elt);
	}
      else
	free (elt);
    }
}

static void
unwind_frame_run_internal (tag, ignore)
     char *tag, *ignore;
{
  UNWIND_ELT *elt;

  while (elt = unwind_protect_list)
    {
      unwind_protect_list = elt->next;

      /* If tag, then compare. */
      if (!elt->cleanup)
	{
	  if (STREQ (elt->arg, tag))
	    {
	      free (elt);
	      break;
	    }
	  free (elt);
	  continue;
	}
      else
	{
	  (*(elt->cleanup)) (elt->arg);
	  free (elt);
	}
    }
}

static void
discard_saved_var (sv)
     SAVED_VAR *sv;
{
  if (sv->size != sizeof (int))
    free (sv->desired_setting);
  free (sv);
}

/* Restore the value of a variable, based on the contents of SV.  If
   sv->size is greater than sizeof (int), sv->desired_setting points to
   a block of memory SIZE bytes long holding the value, rather than the
   value itself.  This block of memory is copied back into the variable. */
static void
restore_variable (sv)
     SAVED_VAR *sv;
{
  if (sv->size != sizeof (int))
    {
      FASTCOPY ((char *)sv->desired_setting, (char *)sv->variable, sv->size);
      free (sv->desired_setting);
    }
  else
    *(sv->variable) = (int)sv->desired_setting;

  free (sv);
}

/* Save the value of a variable so it will be restored when unwind-protects
   are run.  VAR is a pointer to the variable.  VALUE is the value to be
   saved.  SIZE is the size in bytes of VALUE.  If SIZE is bigger than what
   can be saved in an int, memory will be allocated and the value saved
   into that using bcopy (). */
void
unwind_protect_var (var, value, size)
     int *var;
     char *value;
     int size;
{
  SAVED_VAR *s = (SAVED_VAR *)xmalloc (sizeof (SAVED_VAR));

  s->variable = var;
  if (size != sizeof (int))
    {
      s->desired_setting = (char *)xmalloc (size);
      FASTCOPY (value, (char *)s->desired_setting, size);
    }
  else
    s->desired_setting = value;
  s->size = size;
  add_unwind_protect ((Function *)restore_variable, (char *)s);
}
