/* Hash.c -- Where hashing for bash is done. */

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

/* There appears to be library functions for this stuff, but it seems like
   a lot of overhead, so I just implemented this hashing stuff on my own. */

#if defined (HAVE_STRING_H)
#  include <string.h>
#else /* !HAVE_STRING_H */
#  include <strings.h>
#endif /* !HAVE_STRING_H */

#if defined (HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  include "ansi_stdlib.h"
#endif /* HAVE_STDLIB_H */

#include "shell.h"
#include "hash.h"

HASH_TABLE *hashed_filenames;

#define FILENAME_HASH_BUCKETS 107

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
  HASH_TABLE *new_table = (HASH_TABLE *)xmalloc (sizeof (HASH_TABLE));

  if (buckets == 0)
    buckets = DEFAULT_HASH_BUCKETS;

  new_table->bucket_array =
    (BUCKET_CONTENTS **)xmalloc (buckets * sizeof (BUCKET_CONTENTS *));
  new_table->nbuckets = buckets;
  new_table->nentries = 0;
  initialize_hash_table (new_table);
  return (new_table);
}

#if 0
/* UNUSED */
/* Create the hash table for filenames that we use in the shell. */
initialize_hashed_filenames ()
{
  hashed_filenames = make_hash_table (FILENAME_HASH_BUCKETS);
}
#endif

/* Return the location of the bucket which should contain the data
   for STRING.  TABLE is a pointer to a HASH_TABLE. */

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

  if (!table)
    return (BUCKET_CONTENTS *)NULL;

  which_bucket = hash_string (string, table);

  list = table->bucket_array[which_bucket];

  while (list)
    {
      if (STREQ (list->key, string))
	{
	  list->times_found++;
	  return (list);
	}
      else list = list->next;
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

  if (!table)
    return (BUCKET_CONTENTS *)NULL;

  the_bucket = hash_string (string, table);
  prev = (BUCKET_CONTENTS *)NULL;
  temp = table->bucket_array[the_bucket];

  while (temp)
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
      temp = temp->next;
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

  if (!table)
    table = make_hash_table (0);

  if ((item = find_hash_item (string, table)) == 0)
    {
      int bucket = hash_string (string, table);
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
      fprintf (stderr, "Out of memory!");
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
  
  printf ("You have entered %d (%d) items.  The distribution is:\n",
	  table->nentries, count);

  /* Print out a count of how many strings hashed to each bucket, so we can
     see how even the distribution is. */
  for (count = 0; count < table->nbuckets; count++)
    {
      int bcount;
      register BUCKET_CONTENTS *list = get_hash_bucket (count, table);
    
      printf ("slot %3d: ", count);
      bcount = 0;

      for (bcount = 0; list; list = list->next)
        bcount++;

      printf ("%d\n", bcount);
    }
    exit (0);
}

#endif /* TEST_HASHING */

/*
 * Local variables:
 * compile-command: "gcc -g -DTEST_HASHING -o hash hash.c"
 * end:
 */
