/* The malloc headers and source files from the C library follow here.  */

/* Declarations for `malloc' and friends.
   Copyright 1990, 91, 92, 93, 95, 96 Free Software Foundation, Inc.
		  Written May 1989 by Mike Haertel.

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
ot, write to the Free Software Foundation, Inc.,
59 Temple Place, Suite 330, Boston, MA 02111 USA.

   The author may be reached (Email) at the address mike@ai.mit.edu,
   or (US mail) as Mike Haertel c/o Free Software Foundation.  */

/* XXX NOTES:
    1.	Augment the mstats struct so we can see how many blocks for fragments
	and how many blocks for large requests were allocated.
*/

/* CHANGES:
    1.	Reorganized the source for my benefit.
    2.	Integrated the range-checking code by default.
    3.  free(0) no longer dumps core.
    4.  Extended the statistics.
    5.  Fixed a couple of places where the stats were not kept correctly.
*/

#ifdef	HAVE_CONFIG_H
#include <config.h>
#endif

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif

#if defined (HAVE_LIMITS_H)
#  include <limits.h>
#endif

#if defined (HAVE_UNISTD_H)
#  ifdef _MINIX
#    include <sys/types.h>
#  endif
#  include <unistd.h>
#endif

#if defined (HAVE_STDDEF_H)
#  include <stddef.h>
#endif
#include <errno.h>

#if defined (RCHECK) && !defined (botch)
#  include <stdio.h>
#  define STDIO_H_INCLUDED
#endif

#include "stdc.h"

#ifndef errno
extern int errno;
#endif

/* Need an autoconf test for this. */
#if __STDC__
#  undef  genptr_t
#  define genptr_t	void *
#else
#  undef  genptr_t
#  define genptr_t	char *
#endif /* !__STDC__ */

#if !defined (HAVE_MEMSET)
#  define	memset(s, zero, n)	bzero ((s), (n))
#endif
#if !defined (HAVE_MEMCPY)
#  define	memcpy(d, s, n)		bcopy ((s), (d), (n))
#endif

/* Cope with systems lacking `memmove'.    */
#if !defined (HAVE_MEMMOVE) && !defined (memmove)
static void malloc_safe_bcopy __P ((genptr_t, genptr_t, size_t));
#  define memmove(to, from, size)	malloc_safe_bcopy ((from), (to), (size))
#endif

#ifndef	NULL
#define	NULL	0
#endif

#ifndef min
#define min(A, B) ((A) < (B) ? (A) : (B))
#endif

/* Return values for `mprobe': these are the kinds of inconsistencies that
   `mcheck' enables detection of.  */
enum mcheck_status
  {
    MCHECK_DISABLED = -1,	/* Consistency checking is not turned on.  */
    MCHECK_OK,			/* Block is fine.  */
    MCHECK_FREE,		/* Block freed twice.  */
    MCHECK_HEAD,		/* Memory before the block was clobbered.  */
    MCHECK_TAIL			/* Memory after the block was clobbered.  */
  };

/* Statistics available to the user.  */
struct mstats
  {
    size_t bytes_total; /* Total size of the heap. */
    size_t chunks_used; /* Chunks allocated by the user. */
    size_t bytes_used;	/* Byte total of user-allocated chunks. */
    size_t chunks_free; /* Chunks in the free list. */
    size_t bytes_free;	/* Byte total of chunks in the free list. */
    int nmalloc;	/* Total number of calls to malloc. */
    int nfree;		/* Total number of calls to free. */
    int nrealloc;	/* Total number of calls to realloc. */
    int nsbrk;		/* Total number of calls to sbrk. */
    size_t tsbrk;	/* Total number of bytes allocated via sbrk. */
    int negsbrk;	/* Total number of calls to sbrk with a negative arg */
    size_t tnegsbrk;	/* Total number of bytes returned to the kernel. */
  };

#ifdef RCHECK
/* Arbitrary magical numbers.  */
#define MAGICWORD	0xfedabeeb
#define MAGICFREE	0xd8675309
#define MAGICBYTE	((char) 0xd7)
#define MALLOCFLOOD	((char) 0x93)
#define FREEFLOOD	((char) 0x95)

struct hdr
  {
    size_t size;		/* Exact size requested by user.  */
    u_bits32_t magic;		/* Magic number to check header integrity.  */
  };
#endif /* RCHECK */

/* Functions exported by this library. */
/* Allocate SIZE bytes of memory.  */
extern genptr_t malloc __P ((size_t __size));

/* Re-allocate the previously allocated block
   in genptr_t, making the new block SIZE bytes long.  */
extern genptr_t realloc __P ((genptr_t __ptr, size_t __size));

/* Allocate NMEMB elements of SIZE bytes each, all initialized to 0.  */
extern genptr_t calloc __P ((size_t __nmemb, size_t __size));

/* Free a block allocated by `malloc', `realloc' or `calloc'.  */
extern void free __P ((genptr_t __ptr));

/* Allocate SIZE bytes allocated to ALIGNMENT bytes.  */
extern genptr_t memalign __P ((size_t __alignment, size_t __size));

/* Pick up the current statistics. */
extern struct mstats mstats __P ((void));

#ifdef RCHECK
extern enum mcheck_status mprobe __P((genptr_t ptr));
#endif

/* End of exported functions. */

/* The allocator divides the heap into blocks of fixed size; large
   requests receive one or more whole blocks, and small requests
   receive a fragment of a block.  Fragment sizes are powers of two,
   and all fragments of a block are the same size.  When all the
   fragments in a block have been freed, the block itself is freed.  */
#define BLOCKLOG	12
#define BLOCKSIZE	4096		/* 1 << BLOCKLOG */
#define BLOCKIFY(SIZE)	(((SIZE) + BLOCKSIZE - 1) / BLOCKSIZE)

/* Determine the amount of memory spanned by the initial heap table
   (not an absolute limit).  */
#define HEAP		4194304		/* 1 << 22 */

/* Number of contiguous free blocks allowed to build up at the end of
   memory before they will be returned to the system.  */
#define FINAL_FREE_BLOCKS	8

/* Data structure giving per-block information.  */
typedef union
  {
    /* Heap information for a busy block.  */
    struct
      {
	/* Zero for a large (multiblock) object, or positive giving the
	   logarithm to the base two of the fragment size.  */
	int type;
	union
	  {
	    struct
	      {
		size_t nfree; /* Free frags in a fragmented block.  */
		size_t first; /* First free fragment of the block.  */
	      } frag;
	    /* For a large object, in its first block, this has the number
	       of blocks in the object.  In the other blocks, this has a
	       negative number which says how far back the first block is.  */
	    ptrdiff_t size;
	  } info;
      } busy;
    /* Heap information for a free block (that may be the first of a
       free cluster).  */
    struct
      {
	size_t size;	/* Size (in blocks) of a free cluster.  */
	size_t next;	/* Index of next free cluster.  */
	size_t prev;	/* Index of previous free cluster.  */
      } free;
  } malloc_info;

/* Pointer to first block of the heap.  */
static char *_heapbase;

/* Table indexed by block number giving per-block information.  */
static malloc_info *_heapinfo;

/* Address to block number and vice versa.  */
#define BLOCK(A)	(((char *) (A) - _heapbase) / BLOCKSIZE + 1)
#define ADDRESS(B)	((genptr_t) (((B) - 1) * BLOCKSIZE + _heapbase))

/* Number of info entries.  */
static size_t heapsize;

/* Current search index for the heap table.  */
static size_t _heapindex;

/* Limit of valid info table indices.  */
static size_t _heaplimit;

/* Doubly linked lists of free fragments.  */
struct list
  {
    struct list *next;
    struct list *prev;
  };

/* Free list headers for each fragment size.  */
static struct list _fraghead[BLOCKLOG];

/* List of blocks allocated with `memalign'.  */
struct alignlist
  {
    struct alignlist *next;
    genptr_t aligned;		/* The address that memaligned returned.  */
    genptr_t exact;		/* The address that malloc returned.  */
  };

/* List of blocks allocated by memalign.  */
static struct alignlist *_aligned_blocks = NULL;

/* Internal versions of `malloc', `realloc', and `free'
   used when these functions need to call each other. */
static genptr_t imalloc __P ((size_t __size));
static genptr_t irealloc __P ((genptr_t __ptr, size_t __size));
static void ifree __P ((genptr_t __ptr));

/* Given an address in the middle of a malloc'd object,
   return the address of the beginning of the object.  */
static genptr_t malloc_find_object_address __P ((genptr_t __ptr));

/* Underlying allocation function; successive calls should
   return contiguous pieces of memory.  */
static genptr_t default_morecore __P ((ptrdiff_t __size));

/* Number of extra blocks to get each time we ask for more core.
   This reduces the frequency of calling `default_morecore'.  */
static size_t malloc_extra_blocks;

/* Nonzero if `malloc' has been called and done its initialization.  */
static int malloc_initialized;
/* Function called to initialize malloc data structures.  */
static int malloc_initialize __P ((void));

#ifdef RCHECK
static void zmemset __P((genptr_t, int, size_t));
static enum mcheck_status checkhdr __P((const struct hdr *));
static void mabort __P((enum mcheck_status));
#endif

/* Instrumentation.  */
static size_t chunks_used;
static size_t bytes_used;
static size_t chunks_free;
static size_t bytes_free;
static int nmalloc, nfree, nrealloc;
static int nsbrk;
static size_t tsbrk;
static int negsbrk;
static size_t tnegsbrk;

/* Aligned allocation.  */
static genptr_t
align (size)
     size_t size;
{
  genptr_t result;
  unsigned long int adj;

  result = default_morecore (size);
  adj = (unsigned long int) ((unsigned long int) ((char *) result -
						  (char *) NULL)) % BLOCKSIZE;
  if (adj != 0)
    {
      genptr_t new;
      adj = BLOCKSIZE - adj;
      new = default_morecore (adj);
      result = (char *) result + adj;
    }

  return result;
}

/* Get SIZE bytes, if we can get them starting at END.
   Return the address of the space we got.
   If we cannot get space at END, fail and return -1.  */
static genptr_t
get_contiguous_space (size, position)
     ptrdiff_t size;
     genptr_t position;
{
  genptr_t before;
  genptr_t after;

  before = default_morecore (0);
  /* If we can tell in advance that the break is at the wrong place,
     fail now.  */
  if (before != position)
    return 0;

  /* Allocate SIZE bytes and get the address of them.  */
  after = default_morecore (size);
  if (!after)
    return 0;

  /* It was not contiguous--reject it.  */
  if (after != position)
    {
      default_morecore (- size);
      return 0;
    }

  return after;
}

/* This is called when `_heapinfo' and `heapsize' have just
   been set to describe a new info table.  Set up the table
   to describe itself and account for it in the statistics.  */
inline static void
register_heapinfo ()
{
  size_t block, blocks;

  block = BLOCK (_heapinfo);
  blocks = BLOCKIFY (heapsize * sizeof (malloc_info));

  /* Account for the _heapinfo block itself in the statistics.  */
  bytes_used += blocks * BLOCKSIZE;
  ++chunks_used;

  /* Describe the heapinfo block itself in the heapinfo.  */
  _heapinfo[block].busy.type = 0;
  _heapinfo[block].busy.info.size = blocks;
  /* Leave back-pointers for malloc_find_address.  */
  while (--blocks > 0)
    _heapinfo[block + blocks].busy.info.size = -blocks;
}

/* Set everything up and remember that we have.  */
static int
malloc_initialize ()
{
  if (malloc_initialized)
    return 0;

  heapsize = HEAP / BLOCKSIZE;
  _heapinfo = (malloc_info *) align (heapsize * sizeof (malloc_info));
  if (_heapinfo == NULL)
    return 0;
  memset (_heapinfo, 0, heapsize * sizeof (malloc_info));
  _heapinfo[0].free.size = 0;
  _heapinfo[0].free.next = _heapinfo[0].free.prev = 0;
  _heapindex = 0;
  _heapbase = (char *) _heapinfo;
  _heaplimit = BLOCK (_heapbase + heapsize * sizeof (malloc_info));

  register_heapinfo ();

  malloc_initialized = 1;
  return 1;
}

/* Allocate INCREMENT more bytes of data space,
   and return the start of data space, or NULL on errors.
   If INCREMENT is negative, shrink data space.  */
static genptr_t
default_morecore (increment)
     ptrdiff_t increment;
{
  genptr_t result;

  nsbrk++;
  tsbrk += increment;
  if (increment < 0)
    {
      negsbrk++;
      tnegsbrk += -increment;
    }
  result = (genptr_t) sbrk (increment);
  if ((long)result == -1L)
    return NULL;
  return result;
}

static int morecore_recursing;

/* Get neatly aligned memory, initializing or
   growing the heap info table as necessary. */
static genptr_t
morecore (size)
     size_t size;
{
  genptr_t result;
  malloc_info *newinfo, *oldinfo;
  size_t newsize;

  if (morecore_recursing)
    /* Avoid recursion.  The caller will know how to handle a null return.  */
    return NULL;

  result = align (size);
  if (result == NULL)
    return NULL;

  /* Check if we need to grow the info table.  */
  if ((size_t) BLOCK ((char *) result + size) > heapsize)
    {
      /* Calculate the new _heapinfo table size.  We do not account for the
	 added blocks in the table itself, as we hope to place them in
	 existing free space, which is already covered by part of the
	 existing table.  */
      newsize = heapsize;
      do
	newsize <<= 1;
      while ((size_t) BLOCK ((char *) result + size) > newsize);

      /* We must not reuse existing core for the new info table when called
	 from realloc in the case of growing a large block, because the
	 block being grown is momentarily marked as free.  In this case
	 _heaplimit is zero so we know not to reuse space for internal
	 allocation.  */
      if (_heaplimit != 0)
	{
	  /* First try to allocate the new info table in core we already
	     have, in the usual way using realloc.  If realloc cannot
	     extend it in place or relocate it to existing sufficient core,
	     we will get called again, and the code above will notice the
	     `morecore_recursing' flag and return null.  */
	  int save = errno;	/* Don't want to clobber errno with ENOMEM.  */
	  morecore_recursing = 1;
	  newinfo = (malloc_info *) irealloc (_heapinfo, newsize * sizeof (malloc_info));
	  morecore_recursing = 0;
	  if (newinfo == NULL)
	    errno = save;
	  else
	    {
	      /* We found some space in core, and realloc has put the old
		 table's blocks on the free list.  Now zero the new part
		 of the table and install the new table location.  */
	      memset (&newinfo[heapsize], 0, (newsize - heapsize) * sizeof (malloc_info));
	      _heapinfo = newinfo;
	      heapsize = newsize;
	      goto got_heap;
	    }
	}

      /* Allocate new space for the malloc info table.  */
      while (1)
  	{
 	  newinfo = (malloc_info *) align (newsize * sizeof (malloc_info));

 	  /* Did it fail?  */
 	  if (newinfo == NULL)
 	    {
 	      default_morecore (-size);
 	      return NULL;
 	    }

 	  /* Is it big enough to record status for its own space?
 	     If so, we win.  */
 	  if ((size_t) BLOCK ((char *) newinfo + newsize * sizeof (malloc_info)) < newsize)
 	    break;

 	  /* Must try again.  First give back most of what we just got.  */
 	  default_morecore (- newsize * sizeof (malloc_info));
 	  newsize *= 2;
  	}

      /* Copy the old table to the beginning of the new,
	 and zero the rest of the new table.  */
      memcpy (newinfo, _heapinfo, heapsize * sizeof (malloc_info));
      memset (&newinfo[heapsize], 0, (newsize - heapsize) * sizeof (malloc_info));
      oldinfo = _heapinfo;
      _heapinfo = newinfo;
      heapsize = newsize;

      register_heapinfo ();

      /* Reset _heaplimit so ifree never decides
	 it can relocate or resize the info table.  */
      _heaplimit = 0;
      ifree (oldinfo);

      /* The new heap limit includes the new table just allocated.  */
      _heaplimit = BLOCK ((char *) newinfo + heapsize * sizeof (malloc_info));
      return result;
    }

 got_heap:
  _heaplimit = BLOCK ((char *) result + size);
  return result;
}

/* Allocate memory from the heap.  */
static genptr_t
imalloc (size)
     size_t size;
{
  genptr_t result;
  size_t block, blocks, lastblocks, start;
  register size_t i;
  struct list *next;

  /* ANSI C allows `malloc (0)' to either return NULL, or to return a
     valid address you can realloc and free (though not dereference).

     It turns out that some extant code (sunrpc, at least Ultrix's version)
     expects `malloc (0)' to return non-NULL and breaks otherwise.
     Be compatible.  */

#if 0
  if (size == 0)
    return NULL;
#endif

  if (size < sizeof (struct list))
    size = sizeof (struct list);

#ifdef SUNOS_LOCALTIME_BUG
  if (size < 16)
    size = 16;
#endif

  /* Determine the allocation policy based on the request size.  */
  if (size <= BLOCKSIZE / 2)
    {
      /* Small allocation to receive a fragment of a block.
	 Determine the logarithm to base two of the fragment size. */
      register size_t log = 1;
      --size;
      while ((size /= 2) != 0)
	++log;

      /* Look in the fragment lists for a
	 free fragment of the desired size. */
      next = _fraghead[log].next;
      if (next != NULL)
	{
	  /* There are free fragments of this size.
	     Pop a fragment out of the fragment list and return it.
	     Update the block's nfree and first counters. */
	  result = (genptr_t) next;
	  next->prev->next = next->next;
	  if (next->next != NULL)
	    next->next->prev = next->prev;
	  block = BLOCK (result);
	  if (--_heapinfo[block].busy.info.frag.nfree != 0)
	    _heapinfo[block].busy.info.frag.first = (unsigned long int)
	      ((unsigned long int) ((char *) next->next - (char *) NULL)
	       % BLOCKSIZE) >> log;

	  /* Update the statistics.  */
	  ++chunks_used;
	  bytes_used += 1 << log;
	  --chunks_free;
	  bytes_free -= 1 << log;
	}
      else
	{
	  /* No free fragments of the desired size, so get a new block
	     and break it into fragments, returning the first.  */
	  result = imalloc (BLOCKSIZE);
	  if (result == NULL)
	    return NULL;

	  /* Link all fragments but the first into the free list.  */
	  next = (struct list *) ((char *) result + (1 << log));
	  next->next = NULL;
	  next->prev = &_fraghead[log];
	  _fraghead[log].next = next;

	  for (i = 2; i < (size_t) (BLOCKSIZE >> log); ++i)
	    {
	      next = (struct list *) ((char *) result + (i << log));
	      next->next = _fraghead[log].next;
	      next->prev = &_fraghead[log];
	      next->prev->next = next;
	      next->next->prev = next;
	    }

	  /* Initialize the nfree and first counters for this block.  */
	  block = BLOCK (result);
	  _heapinfo[block].busy.type = log;
	  _heapinfo[block].busy.info.frag.nfree = i - 1;
	  _heapinfo[block].busy.info.frag.first = i - 1;

	  chunks_free += (BLOCKSIZE >> log) - 1;
	  bytes_free += BLOCKSIZE - (1 << log);
	  bytes_used -= BLOCKSIZE - (1 << log);
	}
    }
  else
    {
      /* Large allocation to receive one or more blocks.
	 Search the free list in a circle starting at the last place visited.
	 If we loop completely around without finding a large enough
	 space we will have to get more memory from the system.  */
      blocks = BLOCKIFY (size);
      start = block = _heapindex;
      while (_heapinfo[block].free.size < blocks)
	{
	  block = _heapinfo[block].free.next;
	  if (block == start)
	    {
	      /* Need to get more from the system.  Get a little extra.  */
	      size_t wantblocks = blocks + malloc_extra_blocks;
	      block = _heapinfo[0].free.prev;
	      lastblocks = _heapinfo[block].free.size;
	      /* Check to see if the new core will be contiguous with the
		 final free block; if so we don't need to get as much.  */
	      if (_heaplimit != 0 && block + lastblocks == _heaplimit &&
		  /* We can't do this if we will have to make the heap info
		     table bigger to accomodate the new space.  */
		  block + wantblocks <= heapsize &&
		  get_contiguous_space ((wantblocks - lastblocks) * BLOCKSIZE,
					ADDRESS (block + lastblocks)))
		{
 		  /* We got it contiguously.  Which block we are extending
		     (the `final free block' referred to above) might have
		     changed, if it got combined with a freed info table.  */
 		  block = _heapinfo[0].free.prev;
  		  _heapinfo[block].free.size += (wantblocks - lastblocks);
		  bytes_free += (wantblocks - lastblocks) * BLOCKSIZE;
 		  _heaplimit += wantblocks - lastblocks;
		  continue;
		}
	      result = morecore (wantblocks * BLOCKSIZE);
	      if (result == NULL)
		return NULL;
	      block = BLOCK (result);
	      /* Put the new block at the end of the free list.  */
	      _heapinfo[block].free.size = wantblocks;
	      _heapinfo[block].free.prev = _heapinfo[0].free.prev;
	      _heapinfo[block].free.next = 0;
	      _heapinfo[0].free.prev = block;
	      _heapinfo[_heapinfo[block].free.prev].free.next = block;
	      ++chunks_free;
	      bytes_free += wantblocks * BLOCKSIZE;
	      /* Now loop to use some of that block for this allocation.  */
	    }
	}

      /* At this point we have found a suitable free list entry.
	 Figure out how to remove what we need from the list. */
      result = ADDRESS (block);
      if (_heapinfo[block].free.size > blocks)
	{
	  /* The block we found has a bit left over,
	     so relink the tail end back into the free list. */
	  _heapinfo[block + blocks].free.size
	    = _heapinfo[block].free.size - blocks;
	  _heapinfo[block + blocks].free.next
	    = _heapinfo[block].free.next;
	  _heapinfo[block + blocks].free.prev
	    = _heapinfo[block].free.prev;
	  _heapinfo[_heapinfo[block].free.prev].free.next
	    = _heapinfo[_heapinfo[block].free.next].free.prev
	    = _heapindex = block + blocks;
	}
      else
	{
	  /* The block exactly matches our requirements,
	     so just remove it from the list. */
	  _heapinfo[_heapinfo[block].free.next].free.prev
	    = _heapinfo[block].free.prev;
	  _heapinfo[_heapinfo[block].free.prev].free.next
	    = _heapindex = _heapinfo[block].free.next;
	  --chunks_free;
	}

      _heapinfo[block].busy.type = 0;
      _heapinfo[block].busy.info.size = blocks;
      ++chunks_used;
      bytes_used += blocks * BLOCKSIZE;
      bytes_free -= blocks * BLOCKSIZE;

      /* Mark all the blocks of the object just allocated except for the
	 first with a negative number so you can find the first block by
	 adding that adjustment.  */
      while (--blocks > 0)
	_heapinfo[block + blocks].busy.info.size = -blocks;
    }

  return result;
}

genptr_t
malloc (size)
     size_t size;
{
#ifdef RCHECK
  struct hdr *hdr;
#endif

  nmalloc++;

  if (malloc_initialized == 0 && malloc_initialize () == 0)
    return NULL;

#ifdef RCHECK
  hdr = (struct hdr *) imalloc (sizeof (struct hdr) + size + 1);
  if (hdr == NULL)
    return NULL;

  hdr->size = size;
  hdr->magic = MAGICWORD;
  ((char *) &hdr[1])[size] = MAGICBYTE;
  zmemset ((genptr_t) (hdr + 1), MALLOCFLOOD, size);
  return (genptr_t) (hdr + 1);
#else
  return (imalloc (size));
#endif
}

/* Free a block of memory allocated by `malloc'. */

/* Return memory to the heap. */
static void
ifree (ptr)
     genptr_t ptr;
{
  int type;
  size_t block, blocks;
  register size_t i;
  struct list *prev, *next;
  genptr_t curbrk;
  size_t lesscore_threshold;
  register struct alignlist *l;

  if (ptr == NULL)
    return;

  /* Threshold of free space at which we will return some to the system.  */
  lesscore_threshold = FINAL_FREE_BLOCKS + 2 * malloc_extra_blocks;

  for (l = _aligned_blocks; l != NULL; l = l->next)
    if (l->aligned == ptr)
      {
	l->aligned = NULL;	/* Mark the slot in the list as free.  */
	ptr = l->exact;
	break;
      }

  block = BLOCK (ptr);

  type = _heapinfo[block].busy.type;
  switch (type)
    {
    case 0:
      /* Get as many statistics as early as we can.  */
      --chunks_used;
      bytes_used -= _heapinfo[block].busy.info.size * BLOCKSIZE;
      bytes_free += _heapinfo[block].busy.info.size * BLOCKSIZE;

      /* Find the free cluster previous to this one in the free list.
	 Start searching at the last block referenced; this may benefit
	 programs with locality of allocation.  */
      i = _heapindex;
      if (i > block)
	while (i > block)
	  i = _heapinfo[i].free.prev;
      else
	{
	  do
	    i = _heapinfo[i].free.next;
	  while (i > 0 && i < block);
	  i = _heapinfo[i].free.prev;
	}

      /* Determine how to link this block into the free list.  */
      if (block == i + _heapinfo[i].free.size)
	{
	  /* Coalesce this block with its predecessor.  */
	  _heapinfo[i].free.size += _heapinfo[block].busy.info.size;
	  block = i;
	}
      else
	{
	  /* Really link this block back into the free list.  */
	  _heapinfo[block].free.size = _heapinfo[block].busy.info.size;
	  _heapinfo[block].free.next = _heapinfo[i].free.next;
	  _heapinfo[block].free.prev = i;
	  _heapinfo[i].free.next = block;
	  _heapinfo[_heapinfo[block].free.next].free.prev = block;
	  ++chunks_free;
	}

      /* Now that the block is linked in, see if we can coalesce it
	 with its successor (by deleting its successor from the list
	 and adding in its size).  */
      if (block + _heapinfo[block].free.size == _heapinfo[block].free.next)
	{
	  _heapinfo[block].free.size
	    += _heapinfo[_heapinfo[block].free.next].free.size;
	  _heapinfo[block].free.next
	    = _heapinfo[_heapinfo[block].free.next].free.next;
	  _heapinfo[_heapinfo[block].free.next].free.prev = block;
	  --chunks_free;
	}

      /* How many trailing free blocks are there now?  */
      blocks = _heapinfo[block].free.size;

      /* Where is the current end of accessible core?  */
      curbrk = default_morecore (0);

      if (_heaplimit != 0 && curbrk == ADDRESS (_heaplimit))
	{
	  /* The end of the malloc heap is at the end of accessible core.
	     It's possible that moving _heapinfo will allow us to
	     return some space to the system.  */

 	  size_t info_block = BLOCK (_heapinfo);
 	  size_t info_blocks = _heapinfo[info_block].busy.info.size;
 	  size_t prev_block = _heapinfo[block].free.prev;
 	  size_t prev_blocks = _heapinfo[prev_block].free.size;
 	  size_t next_block = _heapinfo[block].free.next;
 	  size_t next_blocks = _heapinfo[next_block].free.size;

	  if (/* Win if this block being freed is last in core, the info table
		 is just before it, the previous free block is just before the
		 info table, and the two free blocks together form a useful
		 amount to return to the system.  */
	      (block + blocks == _heaplimit &&
	       info_block + info_blocks == block &&
	       prev_block != 0 && prev_block + prev_blocks == info_block &&
	       blocks + prev_blocks >= lesscore_threshold) ||
	      /* Nope, not the case.  We can also win if this block being
		 freed is just before the info table, and the table extends
		 to the end of core or is followed only by a free block,
		 and the total free space is worth returning to the system.  */
	      (block + blocks == info_block &&
	       ((info_block + info_blocks == _heaplimit &&
		 blocks >= lesscore_threshold) ||
		(info_block + info_blocks == next_block &&
		 next_block + next_blocks == _heaplimit &&
		 blocks + next_blocks >= lesscore_threshold)))
	      )
	    {
	      malloc_info *newinfo;
	      size_t oldlimit = _heaplimit;

	      /* Free the old info table, clearing _heaplimit to avoid
		 recursion into this code.  We don't want to return the
		 table's blocks to the system before we have copied them to
		 the new location.  */
	      _heaplimit = 0;
	      ifree (_heapinfo);
	      _heaplimit = oldlimit;

	      /* Tell malloc to search from the beginning of the heap for
		 free blocks, so it doesn't reuse the ones just freed.  */
	      _heapindex = 0;

	      /* Allocate new space for the info table and move its data.  */
	      newinfo = (malloc_info *) imalloc (info_blocks
							  * BLOCKSIZE);
	      memmove (newinfo, _heapinfo, info_blocks * BLOCKSIZE);
	      _heapinfo = newinfo;

	      /* We should now have coalesced the free block with the
		 blocks freed from the old info table.  Examine the entire
		 trailing free block to decide below whether to return some
		 to the system.  */
	      block = _heapinfo[0].free.prev;
	      blocks = _heapinfo[block].free.size;
 	    }

	  /* Now see if we can return stuff to the system.  */
	  if (block + blocks == _heaplimit && blocks >= lesscore_threshold)
	    {
	      register size_t bytes = blocks * BLOCKSIZE;
	      _heaplimit -= blocks;
	      default_morecore (-bytes);
	      _heapinfo[_heapinfo[block].free.prev].free.next
		= _heapinfo[block].free.next;
	      _heapinfo[_heapinfo[block].free.next].free.prev
		= _heapinfo[block].free.prev;
	      block = _heapinfo[block].free.prev;
	      --chunks_free;
	      bytes_free -= bytes;
	    }
	}

      /* Set the next search to begin at this block.  */
      _heapindex = block;
      break;

    default:
      /* Do some of the statistics.  */
      --chunks_used;
      bytes_used -= 1 << type;
      ++chunks_free;
      bytes_free += 1 << type;

      /* Get the address of the first free fragment in this block.  */
      prev = (struct list *) ((char *) ADDRESS (block) +
			      (_heapinfo[block].busy.info.frag.first << type));

      if (_heapinfo[block].busy.info.frag.nfree == (BLOCKSIZE >> type) - 1)
	{
	  /* If all fragments of this block are free, remove them
	     from the fragment list and free the whole block.  */
	  next = prev;
	  for (i = 1; i < (size_t) (BLOCKSIZE >> type); ++i)
	    next = next->next;
	  prev->prev->next = next;
	  if (next != NULL)
	    next->prev = prev->prev;
	  _heapinfo[block].busy.type = 0;
	  _heapinfo[block].busy.info.size = 1;

	  /* Keep the statistics accurate.  */
	  ++chunks_used;
	  bytes_used += BLOCKSIZE;
	  chunks_free -= BLOCKSIZE >> type;
	  bytes_free -= BLOCKSIZE;

	  ifree (ADDRESS (block));
	}
      else if (_heapinfo[block].busy.info.frag.nfree != 0)
	{
	  /* If some fragments of this block are free, link this
	     fragment into the fragment list after the first free
	     fragment of this block. */
	  next = (struct list *) ptr;
	  next->next = prev->next;
	  next->prev = prev;
	  prev->next = next;
	  if (next->next != NULL)
	    next->next->prev = next;
	  ++_heapinfo[block].busy.info.frag.nfree;
	}
      else
	{
	  /* No fragments of this block are free, so link this
	     fragment into the fragment list and announce that
	     it is the first free fragment of this block. */
	  prev = (struct list *) ptr;
	  _heapinfo[block].busy.info.frag.nfree = 1;
	  _heapinfo[block].busy.info.frag.first = (unsigned long int)
	    ((unsigned long int) ((char *) ptr - (char *) NULL)
	     % BLOCKSIZE >> type);
	  prev->next = _fraghead[type].next;
	  prev->prev = &_fraghead[type];
	  prev->prev->next = prev;
	  if (prev->next != NULL)
	    prev->next->prev = prev;
	}
      break;
    }
}

/* Return memory to the heap.  */
void
free (ptr)
     genptr_t ptr;
{
#ifdef RCHECK
  struct hdr *hdr;
#endif

  nfree++;

  if (ptr == 0)
    return;

#ifdef RCHECK
  hdr = ((struct hdr *) ptr) - 1;
  checkhdr (hdr);
  hdr->magic = MAGICFREE;
  zmemset (ptr, FREEFLOOD, hdr->size);
  ifree (hdr);
#else
  ifree (ptr);
#endif
}

/* Change the size of a block allocated by `malloc'. */

#ifndef HAVE_MEMMOVE
/* Snarfed directly from Emacs src/dispnew.c:
   XXX Should use system bcopy if it handles overlap.  */

/* Like bcopy except never gets confused by overlap.  */

static void
malloc_safe_bcopy (afrom, ato, size)
     genptr_t afrom;
     genptr_t ato;
     size_t size;
{
  char *from, *to;

  from = afrom;
  to = ato;
  if (size <= 0 || from == to)
    return;

  /* If the source and destination don't overlap, then bcopy can
     handle it.  If they do overlap, but the destination is lower in
     memory than the source, we'll assume bcopy can handle that.  */
  if (to < from || from + size <= to)
    bcopy (from, to, size);

  /* Otherwise, we'll copy from the end.  */
  else
    {
      register char *endf = from + size;
      register char *endt = to + size;

      /* If TO - FROM is large, then we should break the copy into
	 nonoverlapping chunks of TO - FROM bytes each.  However, if
	 TO - FROM is small, then the bcopy function call overhead
	 makes this not worth it.  The crossover point could be about
	 anywhere.  Since I don't think the obvious copy loop is too
	 bad, I'm trying to err in its favor.  */
      if (to - from < 64)
	{
	  do
	    *--endt = *--endf;
	  while (endf != from);
	}
      else
	{
	  for (;;)
	    {
	      endt -= (to - from);
	      endf -= (to - from);

	      if (endt < to)
		break;

	      bcopy (endf, endt, to - from);
	    }

	  /* If SIZE wasn't a multiple of TO - FROM, there will be a
	     little left over.  The amount left over is
	     (endt + (to - from)) - to, which is endt - from.  */
	  bcopy (from, to, endt - from);
	}
    }
}
#endif /* !HAVE_MEMMOVE */

/* Resize the given region to the new size, returning a pointer
   to the (possibly moved) region.  This is optimized for speed;
   some benchmarks seem to indicate that greater compactness is
   achieved by unconditionally allocating and copying to a
   new region.  This module has incestuous knowledge of the
   internals of both free and malloc. */
static genptr_t
irealloc (ptr, size)
     genptr_t ptr;
     size_t size;
{
  genptr_t result;
  int type;
  size_t block, blocks, oldlimit;

  if (size == 0)
    {
      ifree (ptr);
      return imalloc (0);
    }
  else if (ptr == NULL)
    return imalloc (size);

  block = BLOCK (ptr);

  type = _heapinfo[block].busy.type;
  switch (type)
    {
    case 0:
      /* Maybe reallocate a large block to a small fragment.  */
      if (size <= BLOCKSIZE / 2)
	{
	  result = imalloc (size);
	  if (result != NULL)
	    {
	      memcpy (result, ptr, size);
	      ifree (ptr);
	      return result;
	    }
	}

      /* The new size is a large allocation as well;
	 see if we can hold it in place. */
      blocks = BLOCKIFY (size);
      if (blocks < _heapinfo[block].busy.info.size)
	{
	  /* The new size is smaller; return
	     excess memory to the free list. */
	  _heapinfo[block + blocks].busy.type = 0;
	  _heapinfo[block + blocks].busy.info.size
	    = _heapinfo[block].busy.info.size - blocks;
	  _heapinfo[block].busy.info.size = blocks;
	  /* We have just created a new chunk by splitting a chunk in two.
	     Now we will free this chunk; increment the statistics counter
	     so it doesn't become wrong when ifree decrements it.  */
	  ++chunks_used;
	  ifree (ADDRESS (block + blocks));
	  result = ptr;
	}
      else if (blocks == _heapinfo[block].busy.info.size)
	/* No size change necessary.  */
	result = ptr;
      else
	{
	  /* Won't fit, so allocate a new region that will.
	     Free the old region first in case there is sufficient
	     adjacent free space to grow without moving. */
	  blocks = _heapinfo[block].busy.info.size;
	  /* Prevent free from actually returning memory to the system.  */
	  oldlimit = _heaplimit;
	  _heaplimit = 0;
	  ifree (ptr);
	  result = imalloc (size);
	  if (_heaplimit == 0)
	    _heaplimit = oldlimit;
	  if (result == NULL)
	    {
	      /* Now we're really in trouble.  We have to unfree
		 the thing we just freed.  Unfortunately it might
		 have been coalesced with its neighbors.  */
	      if (_heapindex == block)
		(void) imalloc (blocks * BLOCKSIZE);
	      else
		{
		  genptr_t previous;
		  previous  = imalloc ((block - _heapindex) * BLOCKSIZE);
		  (void) imalloc (blocks * BLOCKSIZE);
		  ifree (previous);
		}
	      return NULL;
	    }
	  if (ptr != result)
	    memmove (result, ptr, blocks * BLOCKSIZE);
	}
      break;

    default:
      /* Old size is a fragment; type is logarithm
	 to base two of the fragment size.  */
      if (size > (size_t) (1 << (type - 1)) &&
	  size <= (size_t) (1 << type))
	/* The new size is the same kind of fragment.  */
	result = ptr;
      else
	{
	  /* The new size is different; allocate a new space,
	     and copy the lesser of the new size and the old. */
	  result = imalloc (size);
	  if (result == NULL)
	    return NULL;
	  memcpy (result, ptr, min (size, (size_t) 1 << type));
	  ifree (ptr);
	}
      break;
    }

  return result;
}

genptr_t
realloc (ptr, size)
     genptr_t ptr;
     size_t size;
{
#ifdef RCHECK
  struct hdr *hdr;
  size_t osize;
#endif

  if (malloc_initialized == 0 && malloc_initialize () == 0)
    return NULL;

  nrealloc++;

#ifdef RCHECK
  hdr = ((struct hdr *) ptr) - 1;
  osize = hdr->size;

  checkhdr (hdr);
  if (size < osize)
    zmemset ((char *) ptr + size, FREEFLOOD, osize - size);
  hdr = (struct hdr *) irealloc ((genptr_t) hdr, sizeof (struct hdr) + size + 1);
  if (hdr == NULL)
    return NULL;

  hdr->size = size;
  hdr->magic = MAGICWORD;
  ((char *) &hdr[1])[size] = MAGICBYTE;
  if (size > osize)
    zmemset ((char *) (hdr + 1) + osize, MALLOCFLOOD, size - osize);
  return (genptr_t) (hdr + 1);
#else
  return (irealloc (ptr, size));
#endif
}

/* Allocate an array of NMEMB elements each SIZE bytes long.
   The entire array is initialized to zeros.  */
genptr_t
calloc (nmemb, size)
     register size_t nmemb;
     register size_t size;
{
  register genptr_t result;

  result = malloc (nmemb * size);
  if (result != NULL)
    (void) memset (result, 0, nmemb * size);

  return result;
}

/* Define the `cfree' alias for `free'.  */
void
cfree (ptr)
     genptr_t ptr;
{
  free (ptr);
}

genptr_t
memalign (alignment, size)
     size_t alignment;
     size_t size;
{
  genptr_t result;
  unsigned long int adj, lastadj;

  /* Allocate a block with enough extra space to pad the block with up to
     (ALIGNMENT - 1) bytes if necessary.  */
  result = malloc (size + alignment - 1);
  if (result == NULL)
    return NULL;

  /* Figure out how much we will need to pad this particular block
     to achieve the required alignment.  */
  adj = (unsigned long int) ((char *) result - (char *) NULL) % alignment;

  do
    {
      /* Reallocate the block with only as much excess as it needs.  */
      free (result);
      result = malloc (adj + size);
      if (result == NULL)	/* Impossible unless interrupted.  */
	return NULL;

      lastadj = adj;
      adj = (unsigned long int) ((char *) result - (char *) NULL) % alignment;
      /* It's conceivable we might have been so unlucky as to get a
	 different block with weaker alignment.  If so, this block is too
	 short to contain SIZE after alignment correction.  So we must
	 try again and get another block, slightly larger.  */
    } while (adj > lastadj);

  if (adj != 0)
    {
      /* Record this block in the list of aligned blocks, so that `free'
	 can identify the pointer it is passed, which will be in the middle
	 of an allocated block.  */

      struct alignlist *l;
      for (l = _aligned_blocks; l != NULL; l = l->next)
	if (l->aligned == NULL)
	  /* This slot is free.  Use it.  */
	  break;
      if (l == NULL)
	{
	  l = (struct alignlist *) imalloc (sizeof (struct alignlist));
	  if (l == NULL)
	    {
	      free (result);
	      return NULL;
	    }
	  l->next = _aligned_blocks;
	  _aligned_blocks = l;
	}
      l->exact = result;
      result = l->aligned = (char *) result + alignment - adj;
    }

  return result;
}

/* On some ANSI C systems, some libc functions call _malloc, _free
   and _realloc.  Make them use the GNU functions.  */

genptr_t
_malloc (size)
     size_t size;
{
  return malloc (size);
}

void
_free (ptr)
     genptr_t ptr;
{
  free (ptr);
}

genptr_t
_realloc (ptr, size)
     genptr_t ptr;
     size_t size;
{
  return realloc (ptr, size);
}

struct mstats
mstats ()
{
  struct mstats result;
	  
  result.bytes_total = (char *) default_morecore (0) - _heapbase;
  result.chunks_used = chunks_used;
  result.bytes_used = bytes_used;
  result.chunks_free = chunks_free;
  result.bytes_free = bytes_free;
  result.nmalloc = nmalloc;
  result.nrealloc = nrealloc;
  result.nfree = nfree;
  result.nsbrk = nsbrk;
  result.tsbrk = tsbrk;
  result.negsbrk = negsbrk;
  result.tnegsbrk = tnegsbrk;

  return result;
}

#ifdef RCHECK
/* Standard debugging hooks for `malloc'. */

static void
zmemset (ptr, val, size)
     genptr_t ptr;
     int val;
     size_t size;
{
  char *cp = ptr;

  while (size--)
    *cp++ = val;
}

static enum mcheck_status
checkhdr (hdr)
     const struct hdr *hdr;
{
  enum mcheck_status status;

  switch (hdr->magic)
    {
    default:
      status = MCHECK_HEAD;
      break;
    case MAGICFREE:
      status = MCHECK_FREE;
      break;
    case MAGICWORD:
      if (((char *) &hdr[1])[hdr->size] != MAGICBYTE)
	status = MCHECK_TAIL;
      else
	status = MCHECK_OK;
      break;
    }
  if (status != MCHECK_OK)
    mabort (status);
  return status;
}

#ifndef botch
botch (msg)
     char *msg;
{
  fprintf (stderr, "mcheck: %s\n", msg);
  fflush (stderr);
  abort ();
}
#endif

static void
mabort (status)
     enum mcheck_status status;
{
  const char *msg;

  switch (status)
    {
    case MCHECK_OK:
      msg = "memory is consistent, library is buggy";
      break;
    case MCHECK_HEAD:
      msg = "memory clobbered before allocated block";
      break;
    case MCHECK_TAIL:
      msg = "memory clobbered past end of allocated block";
      break;
    case MCHECK_FREE:
      msg = "block freed twice";
      break;
    default:
      msg = "bogus mcheck_status, library is buggy";
      break;
    }

  botch (msg);
}

enum mcheck_status
mprobe (ptr)
     genptr_t ptr;
{
  return checkhdr ((struct hdr *)ptr);
}

#ifndef STDIO_H_INCLUDED
#  include <stdio.h>
#endif

void
print_malloc_stats (s)
     char *s;
{
  struct mstats ms;

  ms = mstats ();
  fprintf (stderr, "Memory allocation statistics: %s\n", s ? s : "");
  fprintf (stderr, "\nTotal chunks in use: %d, total chunks free: %d\n",
	   ms.chunks_used, ms.chunks_free);
  fprintf (stderr, "Total bytes in use: %u, total bytes free: %u\n",
	   ms.bytes_used, ms.bytes_free);
  fprintf (stderr, "Total bytes (from heapbase): %d\n", ms.bytes_total);
  fprintf (stderr, "Total mallocs: %d, total frees: %d, total reallocs: %d\n",
	   ms.nmalloc, ms.nfree, ms.nrealloc);
  fprintf (stderr, "Total sbrks: %d, total bytes via sbrk: %d\n",
  	   ms.nsbrk, ms.tsbrk);
  fprintf (stderr, "Total negative sbrks: %d, total bytes returned to kernel: %d\n",
  	   ms.negsbrk, ms.tnegsbrk);
}
#endif /* RCHECK */
