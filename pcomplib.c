/* pcomplib.c - library functions for programmable completion. */

/* Copyright (C) 1999 Free Software Foundation, Inc.

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

#include <config.h>

#if defined (PROGRAMMABLE_COMPLETION)

#include "bashansi.h"
#include <stdio.h>

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include "shell.h"
#include "pcomplete.h"

#define COMPLETE_HASH_BUCKETS	29	/* for testing */

#define STRDUP(x)	((x) ? savestring (x) : (char *)NULL)

HASH_TABLE *prog_completes = (HASH_TABLE *)NULL;

static int progcomp_initialized = 0;

COMPSPEC *
alloc_compspec ()
{
  COMPSPEC *ret;

  ret = (COMPSPEC *)xmalloc (sizeof (COMPSPEC));
  ret->refcount = 0;

  ret->actions = (unsigned long)0;
  ret->options = (unsigned long)0;

  ret->globpat = (char *)NULL;
  ret->words = (char *)NULL;
  ret->prefix = (char *)NULL;
  ret->suffix = (char *)NULL;
  ret->funcname = (char *)NULL;
  ret->command = (char *)NULL;
  ret->filterpat = (char *)NULL;

  return ret;
}

void
free_compspec (cs)
     COMPSPEC *cs;
{
  cs->refcount--;
  if (cs->refcount == 0)
    {
      FREE (cs->globpat);
      FREE (cs->words);
      FREE (cs->prefix);
      FREE (cs->suffix);
      FREE (cs->funcname);
      FREE (cs->command);
      FREE (cs->filterpat);

      free (cs);
    }
}

COMPSPEC *
copy_compspec (cs)
     COMPSPEC *cs;
{
  COMPSPEC *new;

  new = (COMPSPEC *)xmalloc (sizeof (COMPSPEC));

  new->refcount = cs->refcount;
  new->actions = cs->actions;
  new->options = cs->options;

  new->globpat = STRDUP (cs->globpat);
  new->words = STRDUP (cs->words);
  new->prefix = STRDUP (cs->prefix);
  new->suffix = STRDUP (cs->suffix);
  new->funcname = STRDUP (cs->funcname);
  new->command = STRDUP (cs->command);
  new->filterpat = STRDUP (cs->filterpat);

  return new;
}

void
initialize_progcomp ()
{
  if (progcomp_initialized == 0)
    {
      prog_completes = make_hash_table (COMPLETE_HASH_BUCKETS);
      progcomp_initialized = 1;
    }
}

int
num_progcomps ()
{
  if (progcomp_initialized == 0 || prog_completes == 0)
    return (0);
  return (prog_completes->nentries);
}

static void
free_progcomp (data)
     char *data;
{
  COMPSPEC *cs;

  cs = (COMPSPEC *)data;
  free_compspec (cs);
}
  
void
clear_progcomps ()
{
  if (prog_completes)
    flush_hash_table (prog_completes, free_progcomp);
}

int
remove_progcomp (cmd)
     char *cmd;
{
  register BUCKET_CONTENTS *item;

  if (prog_completes == 0)
    return 1;

  item = remove_hash_item (cmd, prog_completes);
  if (item)
    {
      free_progcomp (item->data);
      free (item->key);
      free (item);
      return (1);
    }
  return (0);
}

int
add_progcomp (cmd, cs)
      char *cmd;
      COMPSPEC *cs;
{
  register BUCKET_CONTENTS *item;

  if (progcomp_initialized == 0 || prog_completes == 0)
    initialize_progcomp ();

  if (cs == NULL)
    programming_error ("add_progcomp: %s: NULL COMPSPEC", cmd);

  item = add_hash_item (cmd, prog_completes);
  if (item->data)
    free_progcomp (item->data);
  else
    item->key = savestring (cmd);
  item->data = (char *)cs;
  cs->refcount++;
  return 1;
}

COMPSPEC *
find_compspec (cmd)
     const char *cmd;
{
  register BUCKET_CONTENTS *item;
  COMPSPEC *cs;

  if (prog_completes == 0)
    return ((COMPSPEC *)NULL);

  item = find_hash_item ((char *)cmd, prog_completes);	/* XXX fix const later */

  if (item == NULL)
    return ((COMPSPEC *)NULL);

  cs = (COMPSPEC *)item->data;

  return (cs);
}

void
print_all_compspecs (pfunc)
     VFunction *pfunc;
{
  BUCKET_CONTENTS *item_list;
  int bucket;
  COMPSPEC *cs;

  if (prog_completes == 0 || pfunc == 0)
    return;

  for (bucket = 0; bucket < prog_completes->nbuckets; bucket++)
    {
      item_list = get_hash_bucket (bucket, prog_completes);
      if (item_list == 0)
	continue;

      for ( ; item_list; item_list = item_list->next)
	{
	  cs = (COMPSPEC *)item_list->data;
	  (*pfunc) (item_list->key, cs);
	}
    }
}

#endif /* PROGRAMMABLE_COMPLETION */
