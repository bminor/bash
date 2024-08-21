/* unwind_prot.c - a simple unwind-protect system for internal variables */

/* I can't stand it anymore!  Please can't we just write the
   whole Unix system in lisp or something? */

/* Copyright (C) 1987-2024 Free Software Foundation, Inc.

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
#include "sig.h"
#include "quit.h"
#include "bashintl.h"	/* for _() */
#include "error.h"	/* for internal_warning */
#include "ocache.h"

/* Structure describing a saved variable and the value to restore it to.  */
typedef struct {
  char *variable;
  int size;
  char desired_setting[1]; /* actual size is `size' */
} SAVED_VAR;

/* If HEAD.CLEANUP is null, then ARG.V contains a tag to throw back to.
   If HEAD.CLEANUP is restore_variable, then SV.V contains the saved
   variable.  Otherwise, call HEAD.CLEANUP (ARG.V) to clean up.  */
typedef union uwp {
  struct uwp_head {
    union uwp *next;
    sh_uwfunc_t *cleanup;
  } head;
  struct {
    struct uwp_head uwp_head;
    void *v;
  } arg;
  struct {
    struct uwp_head uwp_head;
    SAVED_VAR v;
  } sv;
} UNWIND_ELT;

static void unwind_frame_discard_internal (char *);
static void unwind_frame_run_internal (char *);
static void add_unwind_protect_internal (sh_uwfunc_t *, void *);
static void remove_unwind_protect_internal (void);
static void run_unwind_protects_internal (void);
static void clear_unwind_protects_internal (int);
static inline void restore_variable (void *);
static void unwind_protect_mem_internal (void *, int);

static UNWIND_ELT *unwind_protect_list = (UNWIND_ELT *)NULL;

/* Allocating from a cache of unwind-protect elements */
#define UWCACHESIZE	128

sh_obj_cache_t uwcache = {0, 0, 0};

#if 0
#define uwpalloc(elt)	(elt) = (UNWIND_ELT *)xmalloc (sizeof (UNWIND_ELT))
#define uwpfree(elt)	free(elt)
#else
#define uwpalloc(elt)	ocache_alloc (uwcache, UNWIND_ELT, elt)
#define uwpfree(elt)	ocache_free (uwcache, UNWIND_ELT, elt)
#endif

void
uwp_init (void)
{
  ocache_create (uwcache, UNWIND_ELT, UWCACHESIZE);
}

/* Start the beginning of a region. */
void
begin_unwind_frame (char *tag)
{
  add_unwind_protect (NULL, tag);
}

/* Discard the unwind protects back to TAG. */
void
discard_unwind_frame (char *tag)
{
  if (unwind_protect_list)
    unwind_frame_discard_internal (tag);
}

/* Run the unwind protects back to TAG. */
void
run_unwind_frame (char *tag)
{
  if (unwind_protect_list)
    unwind_frame_run_internal (tag);
}

/* Add the function CLEANUP with ARG to the list of unwindable things. */
void
add_unwind_protect (sh_uwfunc_t *cleanup, void *arg)
{
  add_unwind_protect_internal (cleanup, arg);
}

/* Remove the top unwind protect from the list. */
void
remove_unwind_protect (void)
{
  if (unwind_protect_list)
    remove_unwind_protect_internal ();
}

/* Run the list of cleanup functions in unwind_protect_list. */
void
run_unwind_protects (void)
{
  if (unwind_protect_list)
    run_unwind_protects_internal ();
}

/* Erase the unwind-protect list.  If flags is 1, free the elements. */
void
clear_unwind_protect_list (int flags)
{
  if (unwind_protect_list)
    clear_unwind_protects_internal (flags);
}

int
have_unwind_protects (void)
{
  return (unwind_protect_list != 0);
}

int
unwind_protect_tag_on_stack (const char *tag)
{
  UNWIND_ELT *elt;

  elt = unwind_protect_list;
  while (elt)
    {
      if (elt->head.cleanup == 0 && STREQ (elt->arg.v, tag))
	return 1;
      elt = elt->head.next;
    }
  return 0;
}

/* **************************************************************** */
/*								    */
/*			The Actual Functions		 	    */
/*								    */
/* **************************************************************** */

static void
add_unwind_protect_internal (sh_uwfunc_t *cleanup, void *arg)
{
  UNWIND_ELT *elt;

  uwpalloc (elt);
  elt->head.next = unwind_protect_list;
  elt->head.cleanup = cleanup;
  elt->arg.v = arg;
  unwind_protect_list = elt;
}

static void
remove_unwind_protect_internal (void)
{
  UNWIND_ELT *elt;

  elt = unwind_protect_list;
  if (elt)
    {
      unwind_protect_list = unwind_protect_list->head.next;
      uwpfree (elt);
    }
}

static void
run_unwind_protects_internal (void)
{
  unwind_frame_run_internal (NULL);
}

static void
clear_unwind_protects_internal (int flag)
{
  if (flag)
    {
      while (unwind_protect_list)
	remove_unwind_protect_internal ();
    }
  unwind_protect_list = (UNWIND_ELT *)NULL;
}

static void
unwind_frame_discard_internal (char *tag)
{
  UNWIND_ELT *elt;
  int found;

  found = 0;
  while (elt = unwind_protect_list)
    {
      unwind_protect_list = unwind_protect_list->head.next;
      if (elt->head.cleanup == 0 && (STREQ (elt->arg.v, tag)))
	{
	  uwpfree (elt);
	  found = 1;
	  break;
	}
      else
	uwpfree (elt);
    }

  if (found == 0)
    internal_warning ("unwind_frame_discard: %s: %s", tag, _("frame not found"));
}

/* Restore the value of a variable, based on the contents of SV.
   sv->desired_setting is a block of memory SIZE bytes long holding the
   value itself.  This block of memory is copied back into the variable. */
static inline void
restore_variable (void *arg)
{
  SAVED_VAR *sv;
  sv = arg;
  FASTCOPY (sv->desired_setting, sv->variable, sv->size);
}

static void
unwind_frame_run_internal (char *tag)
{
  UNWIND_ELT *elt;
  int found;

  found = 0;
  while (elt = unwind_protect_list)
    {
      unwind_protect_list = elt->head.next;

      /* If tag, then compare. */
      if (elt->head.cleanup == 0)
	{
	  if (tag && STREQ (elt->arg.v, tag))
	    {
	      uwpfree (elt);
	      found = 1;
	      break;
	    }
	}
      else
	{
	  if (elt->head.cleanup == restore_variable)
	    restore_variable (&elt->sv.v);
	  else
	    (*(elt->head.cleanup)) (elt->arg.v);
	}

      uwpfree (elt);
    }
  if (tag && found == 0)
    internal_warning ("unwind_frame_run: %s: %s", tag, _("frame not found"));
}

static void
unwind_protect_mem_internal (void *var, int size)
{
  size_t allocated;
  UNWIND_ELT *elt;

  allocated = size + offsetof (UNWIND_ELT, sv.v.desired_setting[0]);
  if (allocated < sizeof (UNWIND_ELT))
    allocated = sizeof (UNWIND_ELT);
  elt = (UNWIND_ELT *)xmalloc (allocated);
  elt->head.next = unwind_protect_list;
  elt->head.cleanup = restore_variable;
  elt->sv.v.variable = var;
  elt->sv.v.size = size;
  FASTCOPY (var, elt->sv.v.desired_setting, size);
  unwind_protect_list = elt;
}

/* Save the value of a variable so it will be restored when unwind-protects
   are run.  VAR is a pointer to the variable.  SIZE is the size in
   bytes of VAR.  */
void
unwind_protect_mem (char *var, int size)
{
  unwind_protect_mem_internal (var, size);
}

#if defined (DEBUG)
#include <stdio.h>

void
print_unwind_protect_tags (void)
{
  UNWIND_ELT *elt;

  elt = unwind_protect_list;
  while (elt)
    {
      if (elt->head.cleanup == 0)
        fprintf(stderr, "tag: %s\n", (char *)elt->arg.v);
      elt = elt->head.next;
    }
}
#endif
