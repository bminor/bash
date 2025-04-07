/* hashlib.h -- the data structures used in hashing in Bash. */

/* Copyright (C) 1993-2022 Free Software Foundation, Inc.

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

#if !defined (_HASHLIB_H_)
#define _HASHLIB_H_

#include "stdc.h"

#ifndef PTR_T
#  define PTR_T void *
#endif

typedef struct bucket_contents {
  struct bucket_contents *next;	/* Link to next hashed key in this bucket. */
  char *key;			/* What we look up. */
  PTR_T data;			/* What we really want. */
  unsigned int khash;		/* What key hashes to */
  int times_found;		/* Number of times this item has been found. */
} BUCKET_CONTENTS;

typedef struct hash_table {
  BUCKET_CONTENTS **bucket_array;	/* Where the data is kept. */
  int nbuckets;			/* How many buckets does this table have. */
  int nentries;			/* How many entries does this table have. */
} HASH_TABLE;

typedef int hash_wfunc (BUCKET_CONTENTS *);

/* Operations on tables as a whole */
extern HASH_TABLE *hash_create (int);
extern HASH_TABLE *hash_copy (HASH_TABLE *, sh_copy_func_t *);
extern void hash_flush (HASH_TABLE *, sh_free_func_t *);
extern void hash_dispose (HASH_TABLE *);
extern void hash_walk (HASH_TABLE *, hash_wfunc *);

/* Operations to extract information from or pieces of tables */
extern int hash_bucket (const char *, HASH_TABLE *);
extern int hash_size (HASH_TABLE *);

/* Operations on hash table entries */
extern BUCKET_CONTENTS *hash_search (const char *, HASH_TABLE *, int);
extern BUCKET_CONTENTS *hash_insert (char *, HASH_TABLE *, int);
extern BUCKET_CONTENTS *hash_remove (const char *, HASH_TABLE *, int);

/* Miscellaneous */
extern unsigned int hash_string (const char *);

/* Redefine the function as a macro for speed. */
#define hash_items(bucket, table) \
	((table && (bucket < table->nbuckets)) ?  \
		table->bucket_array[bucket] : \
		(BUCKET_CONTENTS *)NULL)

/* Default number of buckets in the hash table. */
#define DEFAULT_HASH_BUCKETS 128	/* must be power of two */

#define HASH_ENTRIES(ht)	((ht) ? (ht)->nentries : 0)

/* flags for hash_search and hash_insert */
#define HASH_NOSRCH	0x01
#define HASH_CREATE	0x02

#endif /* _HASHLIB_H */
