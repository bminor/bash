/* hashlib.h -- the data structures used in hashing in Bash. */

/* Copyright (C) 1993 Free Software Foundation, Inc.

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

#if !defined (_HASHLIB_H_)
#define _HASHLIB_H_

#include "stdc.h"

typedef struct bucket_contents {
  struct bucket_contents *next;	/* Link to next hashed key in this bucket. */
  char *key;			/* What we look up. */
  char *data;			/* What we really want. */
  int times_found;		/* Number of times this item has been found. */
} BUCKET_CONTENTS;

typedef struct hash_table {
  BUCKET_CONTENTS **bucket_array;	/* Where the data is kept. */
  int nbuckets;			/* How many buckets does this table have. */
  int nentries;			/* How many entries does this table have. */
} HASH_TABLE;

extern int hash_string __P((char *, HASH_TABLE *));
extern HASH_TABLE *make_hash_table __P((int));
extern BUCKET_CONTENTS *find_hash_item __P((char *, HASH_TABLE *));
extern BUCKET_CONTENTS *remove_hash_item __P((char *, HASH_TABLE *));
extern BUCKET_CONTENTS *add_hash_item __P((char *, HASH_TABLE *));
extern void flush_hash_table __P((HASH_TABLE *, VFunction *));
extern void dispose_hash_table __P((HASH_TABLE *));

/* Redefine the function as a macro for speed. */
#define get_hash_bucket(bucket, table) \
	((table && (bucket < table->nbuckets)) ?  \
		table->bucket_array[bucket] : \
		(BUCKET_CONTENTS *)NULL)

/* Default number of buckets in the hash table. */
#define DEFAULT_HASH_BUCKETS 53	/* was 107 */

#define HASH_ENTRIES(ht)	(ht)->nentries

#if !defined (NULL)
#  if defined (__STDC__)
#    define NULL ((void *) 0)
#  else
#    define NULL 0x0
#  endif /* !__STDC__ */
#endif /* !NULL */

#endif /* _HASHLIB_H */
