/* hashlib.c -- functions to manage and access hash tables for bash. */

/* Copyright (C) 1987, 1989, 1991 Free Software Foundation, Inc.

This file is part of GNU Bash, the Bourne Again SHell.

Bash is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 1, or (at your option) any later
version.

Bash is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with Bash; see the file COPYING.  If not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include "config.h"

#include "bashansi.h"

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#include <stdio.h>

#include "shell.h"
#include "hashlib.h"

/* Zero the buckets in TABLE. */
static void
initialize_hash_table (table)
     HASH_TABLE *table;
{
  register int i;
  for (i = 0; i < table->nbuckets; i++)
    table->bucket_array[i] = (BUCKET_CONTENTS *)NULL;
}

/* Make a new hash table with BUCKETS number of buckets.  Initialize
   each slot in the table to NULL. */
HASH_TABLE *
make_hash_table (buckets)
     int buckets;
{
  HASH_TABLE *new_table;

  new_table = (HASH_TABLE *)xmalloc (sizeof (HASH_TABLE));
  if (buckets == 0)
    buckets = DEFAULT_HASH_BUCKETS;

  new_table->bucket_array =
    (BUCKET_CONTENTS **)xmalloc (buckets * sizeof (BUCKET_CONTENTS *));
  new_table->nbuckets = buckets;
  new_table->nentries = 0;
  initialize_hash_table (new_table);
  return (new_table);
}

/* Return the location of the bucket which should contain the data
   for STRING.  TABLE is a pointer to a HASH_TABLE. */

/* A possibly better distribution may be obtained by initializing i to
   ~0UL and using i = (i * 33) + *string++ as the step */

#define ALL_ONES (~((unsigned long) 0))
#define BITS(h, n) ((unsigned long)(h) & ~(ALL_ONES << (n)))

int
hash_string (string, table)
     char *string;
     HASH_TABLE *table;
{
  register unsigned int i = 0;

  while (*string)
    i = (i << 2) + *string++;

  return (BITS (i, 31) % table->nbuckets);
}

/* Return a pointer to the hashed item, or NULL if the item
   can't be found. */
BUCKET_CONTENTS *
find_hash_item (string, table)
     char *string;
     HASH_TABLE *table;
{
  BUCKET_CONTENTS *list;
  int which_bucket;

  if (table == 0)
    return (BUCKET_CONTENTS *)NULL;

  which_bucket = hash_string (string, table);

  for (list = table->bucket_array[which_bucket]; list; list = list->next)
    {
      if (STREQ (list->key, string))
	{
	  list->times_found++;
	  return (list);
	}
    }
  return (BUCKET_CONTENTS *)NULL;
}

/* Remove the item specified by STRING from the hash table TABLE.
   The item removed is returned, so you can free its contents.  If
   the item isn't in this table NULL is returned. */
BUCKET_CONTENTS *
remove_hash_item (string, table)
     char *string;
     HASH_TABLE *table;
{
  int the_bucket;
  BUCKET_CONTENTS *prev, *temp;

  if (table == 0)
    return (BUCKET_CONTENTS *)NULL;

  the_bucket = hash_string (string, table);
  prev = (BUCKET_CONTENTS *)NULL;
  for (temp = table->bucket_array[the_bucket]; temp; temp = temp->next)
    {
      if (STREQ (temp->key, string))
	{
	  if (prev)
	    prev->next = temp->next;
	  else
	    table->bucket_array[the_bucket] = temp->next;

	  table->nentries--;
	  return (temp);
	}
      prev = temp;
    }
  return ((BUCKET_CONTENTS *) NULL);
}

/* Create an entry for STRING, in TABLE.  If the entry already
   exists, then return it. */
BUCKET_CONTENTS *
add_hash_item (string, table)
     char *string;
     HASH_TABLE *table;
{
  BUCKET_CONTENTS *item;
  int bucket;

  if (table == 0)
    table = make_hash_table (0);

  if ((item = find_hash_item (string, table)) == 0)
    {
      bucket = hash_string (string, table);
      item = table->bucket_array[bucket];

      while (item && item->next)
	item = item->next;

      if (item)
	{
	  item->next = (BUCKET_CONTENTS *)xmalloc (sizeof (BUCKET_CONTENTS));
	  item = item->next;
	}
      else
	{
	  table->bucket_array[bucket] =
	    (BUCKET_CONTENTS *)xmalloc (sizeof (BUCKET_CONTENTS));
	  item = table->bucket_array[bucket];
	}

      item->data = (char *)NULL;
      item->next = (BUCKET_CONTENTS *)NULL;
      item->key = string;
      table->nentries++;
      item->times_found = 0;
    }

  return (item);
}

/* Remove and discard all entries in TABLE.  If FREE_DATA is non-null, it
   is a function to call to dispose of a hash item's data.  Otherwise,
   free() is called. */
void
flush_hash_table (table, free_data)
     HASH_TABLE *table;
     VFunction *free_data;
{
  int i;
  register BUCKET_CONTENTS *bucket, *item;

  if (table == 0)
    return;

  for (i = 0; i < table->nbuckets; i++)
    {
      bucket = table->bucket_array[i];

      while (bucket)
	{
	  item = bucket;
	  bucket = bucket->next;

	  if (free_data)
	    (*free_data) (item->data);
	  else
	    free (item->data);
	  free (item->key);
	  free (item);
	}
      table->bucket_array[i] = (BUCKET_CONTENTS *)NULL;
    }
}

/* Free the hash table pointed to by TABLE. */
void
dispose_hash_table (table)
     HASH_TABLE *table;
{
  free (table->bucket_array);
  free (table);
}

/* No longer necessary; everything uses the macro */
#if 0
/* Return the bucket_contents list of bucket BUCKET in TABLE.  If
   TABLE doesn't have BUCKET buckets, return NULL. */
#undef get_hash_bucket
BUCKET_CONTENTS *
get_hash_bucket (bucket, table)
     int bucket;
     HASH_TABLE *table;
{
  if (table && bucket < table->nbuckets)
    return (table->bucket_array[bucket]);
  else
    return (BUCKET_CONTENTS *)NULL;
}
#endif

#ifdef DEBUG
void
print_table_stats (table, name)
     HASH_TABLE *table;
     char *name;
{
  register int slot, bcount;
  register BUCKET_CONTENTS *bc;

  if (name == 0)
    name = "unknown hash table";

  fprintf (stderr, "%s: %d buckets; %d items\n", name, table->nbuckets, table->nentries);

  /* Print out a count of how many strings hashed to each bucket, so we can
     see how even the distribution is. */
  for (slot = 0; slot < table->nbuckets; slot++)
    {
      bc = get_hash_bucket (slot, table);

      fprintf (stderr, "\tslot %3d: ", slot);
      for (bcount = 0; bc; bc = bc->next)
        bcount++;

      fprintf (stderr, "%d\n", bcount);
    }
}
#endif

#ifdef TEST_HASHING

#undef NULL
#include <stdio.h>

HASH_TABLE *table;
#define NBUCKETS 107

char *
xmalloc (bytes)
     int bytes;
{
  char *result = (char *)malloc (bytes);
  if (!result)
    {
      fprintf (stderr, "hash: out of virtual memory\n");
      abort ();
    }
  return (result);
}

main ()
{
  char string[256];
  int count = 0;
  BUCKET_CONTENTS *tt;

  table = make_hash_table (NBUCKETS);

  for (;;)
    {
      char *temp_string;
      if (fgets (string, sizeof (string), stdin) == 0)
        break;
      if (!*string)
        break;
      temp_string = savestring (string);
      tt = add_hash_item (temp_string, table);
      if (tt->times_found)
	{
	  fprintf (stderr, "You have already added item `%s'\n", string);
	  free (temp_string);
	}
      else
	{
	  count++;
	}
    }

  print_table_stats (table, "hash test");
  exit (0);
}

#endif /* TEST_HASHING */
