/* hashcmd.c - functions for managing a hash table mapping command names to
	       full pathnames. */

/* Copyright (C) 1997 Free Software Foundation, Inc.

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

#include <config.h>

#include "bashtypes.h"
#include "posixstat.h"

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include "bashansi.h"

#include "shell.h"
#include "findcmd.h"
#include "hashcmd.h"

extern int hashing_enabled;

static int hashing_initialized = 0;

HASH_TABLE *hashed_filenames;

void
initialize_filename_hashing ()
{
  if (hashing_initialized == 0)
    {
      hashed_filenames = make_hash_table (FILENAME_HASH_BUCKETS);
      hashing_initialized = 1;
    }
}

static void
free_filename_data (data)
     char *data;
{
  free (((PATH_DATA *)data)->path);
  free (data);
}

void
flush_hashed_filenames ()
{
  if (hashed_filenames)
    flush_hash_table (hashed_filenames, free_filename_data);
}

/* Remove FILENAME from the table of hashed commands. */
void
remove_hashed_filename (filename)
     char *filename;
{
  register BUCKET_CONTENTS *item;

  if (hashing_enabled == 0 || hashed_filenames == 0)
    return;

  item = remove_hash_item (filename, hashed_filenames);
  if (item)
    {
      if (item->data)
	free_filename_data (item->data);
      free (item->key);
      free (item);
    }
}

/* Place FILENAME (key) and FULL_PATHNAME (data->path) into the
   hash table.  CHECK_DOT if non-null is for future calls to
   find_hashed_filename (); it means that this file was found
   in a directory in $PATH that is not an absolute pathname.
   FOUND is the initial value for times_found. */
void
remember_filename (filename, full_pathname, check_dot, found)
     char *filename, *full_pathname;
     int check_dot, found;
{
  register BUCKET_CONTENTS *item;

  if (hashing_enabled == 0)
    return;

  if (hashed_filenames == 0 || hashing_initialized == 0)
    initialize_filename_hashing ();

  item = add_hash_item (filename, hashed_filenames);
  if (item->data)
    free (pathdata(item)->path);
  else
    {
      item->key = savestring (filename);
      item->data = xmalloc (sizeof (PATH_DATA));
    }
  pathdata(item)->path = savestring (full_pathname);
  pathdata(item)->flags = 0;
  if (check_dot)
    pathdata(item)->flags |= HASH_CHKDOT;
  if (*full_pathname != '/')
    pathdata(item)->flags |= HASH_RELPATH;
  item->times_found = found;
}

/* Return the full pathname that FILENAME hashes to.  If FILENAME
   is hashed, but (data->flags & HASH_CHKDOT) is non-zero, check
   ./FILENAME and return that if it is executable.  This always
   returns a newly-allocated string; the caller is responsible
   for freeing it. */
char *
find_hashed_filename (filename)
     char *filename;
{
  register BUCKET_CONTENTS *item;
  char *path, *dotted_filename, *tail;
  int same;

  if (hashing_enabled == 0 || hashed_filenames == 0)
    return ((char *)NULL);

  item = find_hash_item (filename, hashed_filenames);

  if (item == NULL)
    return ((char *)NULL);

  /* If this filename is hashed, but `.' comes before it in the path,
     see if ./filename is executable.  If the hashed value is not an
     absolute pathname, see if ./`hashed-value' exists. */
  path = pathdata(item)->path;
  if (pathdata(item)->flags & (HASH_CHKDOT|HASH_RELPATH))
    {
      tail = (pathdata(item)->flags & HASH_RELPATH) ? path : filename;
      /* If the pathname does not start with a `./', add a `./' to it. */
      if (tail[0] != '.' || tail[1] != '/')
	{
	  dotted_filename = xmalloc (3 + strlen (tail));
	  dotted_filename[0] = '.'; dotted_filename[1] = '/';
	  strcpy (dotted_filename + 2, tail);
	}
      else
	dotted_filename = savestring (tail);

      if (executable_file (dotted_filename))
	return (dotted_filename);

      free (dotted_filename);

#if 0
      if (pathdata(item)->flags & HASH_RELPATH)
	return ((char *)NULL);
#endif

      /* Watch out.  If this file was hashed to "./filename", and
	 "./filename" is not executable, then return NULL. */

      /* Since we already know "./filename" is not executable, what
	 we're really interested in is whether or not the `path'
	 portion of the hashed filename is equivalent to the current
	 directory, but only if it starts with a `.'.  (This catches
	 ./. and so on.)  same_file () tests general Unix file
	 equivalence -- same device and inode. */
      if (*path == '.')
	{
	  same = 0;
	  tail = (char *)strrchr (path, '/');

	  if (tail)
	    {
	      *tail = '\0';
	      same = same_file (".", path, (struct stat *)NULL, (struct stat *)NULL);
	      *tail = '/';
	    }

	  return same ? (char *)NULL : savestring (path);
	}
    }

  return (savestring (path));
}
