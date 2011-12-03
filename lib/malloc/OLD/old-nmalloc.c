/* malloc.c - dynamic memory allocation for bash. */

/*  Copyright (C) 1985, 1987, 1997 Free Software Foundation, Inc.

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2, or (at your option)
    any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111 USA.

In other words, you are welcome to use, share and improve this program.
You are forbidden to forbid anyone else to use, share and improve
what you give them.   Help stamp out software-hoarding!  */

/*
 * @(#)nmalloc.c 1 (Caltech) 2/21/82
 *
 *	U of M Modified: 20 Jun 1983 ACT: strange hacks for Emacs
 *
 *	Nov 1983, Mike@BRL, Added support for 4.1C/4.2 BSD.
 *
 * This is a very fast storage allocator.  It allocates blocks of a small 
 * number of different sizes, and keeps free lists of each size.  Blocks
 * that don't exactly fit are passed up to the next larger size.  In this 
 * implementation, the available sizes are (2^n)-4 (or -16) bytes long.
 * This is designed for use in a program that uses vast quantities of
 * memory, but bombs when it runs out.  To make it a little better, it
 * warns the user when he starts to get near the end.
 *
 * June 84, ACT: modified rcheck code to check the range given to malloc,
 * rather than the range determined by the 2-power used.
 *
 * Jan 85, RMS: calls malloc_warning to issue warning on nearly full.
 * No longer Emacs-specific; can serve as all-purpose malloc for GNU.
 * You should call malloc_init to reinitialize after loading dumped Emacs.
 * Call malloc_stats to get info on memory stats if MALLOC_STATS turned on.
 * realloc knows how to return same block given, just changing its size,
 * if the power of 2 is correct.
 */
#define MALLOC_STATS		/* for the time being */

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information will
 * go in the first int of the block, and the returned pointer will point
 * to the second.
 */

/* XXX For the time being, until an autoconf test is created (ce has one) */
#define bits32_t		int
#define u_bits32_t	unsigned int

/* Define this to have free() write 0xcf into memory as it's freed, to
   uncover callers that refer to freed memory. */
/* SCO 3.2v4 getcwd and possibly other libc routines fail with MEMSCRAMBLE */
#if !defined (NO_MEMSCRAMBLE)
#  define MEMSCRAMBLE
#endif

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

/* Determine which kind of system this is.  */
#if defined (SHELL)
#  include "bashtypes.h"
#else
#  include <sys/types.h>
#endif
#include <signal.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif

#if defined (MALLOC_STATS) || !defined (botch)
#  include <stdio.h>
#endif /* MALLOC_STATS || !botch */

/* Define getpagesize () if the system does not.  */
#ifndef HAVE_GETPAGESIZE
#  include "getpagesize.h"
#endif

#if __GNUC__ > 1
#  define FASTCOPY(s, d, n)  __builtin_memcpy (d, s, n)
#else /* !__GNUC__ */
#  if !defined (HAVE_BCOPY)
#    if !defined (HAVE_MEMMOVE)
#      define FASTCOPY(s, d, n)  memcpy (d, s, n)
#    else
#      define FASTCOPY(s, d, n)  memmove (d, s, n)
#    endif /* !HAVE_MEMMOVE */
#  else /* HAVE_BCOPY */
#    define FASTCOPY(s, d, n)  bcopy (s, d, n)
#  endif /* HAVE_BCOPY */
#endif /* !__GNUC__ */

#if !defined (NULL)
#  define NULL 0
#endif

#define NBUCKETS	30

#define ISALLOC ((char) 0xf7)	/* magic byte that implies allocation */
#define ISFREE ((char) 0x54)	/* magic byte that implies free block */
				/* this is for error checking only */
#define ISMEMALIGN ((char) 0xd6)  /* Stored before the value returned by
				     memalign, with the rest of the word
				     being the distance to the true
				     beginning of the block.  */

#if !defined (SBRK_DECLARED)
extern char *sbrk ();
#endif /* !SBRK_DECLARED */

#ifdef MALLOC_STATS
/*
 * NMALLOC[i] is the difference between the number of mallocs and frees
 * for a given block size.  TMALLOC[i] is the total number of mallocs for
 * a given block size.  NMORECORE[i] is the total number of calls to
 * morecore(i).  NMAL and NFRE are counts of the number of calls to malloc()
 * and free(), respectively.  NREALLOC is the total number of calls to
 * realloc(); NRCOPY is the number of times realloc() had to allocate new
 * memory and copy to it.  NRECURSE is a count of the number of recursive
 * calls to malloc() for the same bucket size, which can be caused by calls
 * to malloc() from a signal handler.  NSBRK is the number of calls to sbrk()
 * (whether by morecore() or for alignment); TSBRK is the total number of
 * bytes requested from the kernel with sbrk().  BYTESUSED is the total
 * number of bytes consumed by blocks currently in used; BYTESFREE is the
 * total number of bytes currently on all of the free lists.  NBSPLIT is
 * the number of times a larger block was split to satisfy a smaller request.
 * NBCOALESCE is the number of times two adjacent smaller blocks off the free
 * list were combined to satisfy a larger request.
 */
struct _malstats {
  int nmalloc[NBUCKETS];
  int tmalloc[NBUCKETS];
  int nmorecore[NBUCKETS];
  int nmal;
  int nfre;
  int nrealloc;
  int nrcopy;
  int nrecurse;
  int nsbrk;
  bits32_t tsbrk;
  bits32_t bytesused;
  bits32_t bytesfree;
  int nbsplit;
  int nbcoalesce;
};

static struct _malstats _mstats;

/* Return statistics describing allocation of blocks of size BLOCKSIZE.
   NFREE is the number of free blocks for this allocation size.  NUSED
   is the number of blocks in use.  NMAL is the number of requests for
   blocks of size BLOCKSIZE.  NMORECORE is the number of times we had
   to call MORECORE to repopulate the free list for this bucket. */
struct bucket_stats {
  u_bits32_t blocksize;
  int nfree;
  int nused;
  int nmal;
  int nmorecore;
};
#endif /* MALLOC_STATS */

/* We have a flag indicating whether memory is allocated, an index in
   nextf[], a size field, and a sentinel value to determine whether or
   not a caller wrote before the start of allocated memory; to realloc()
   memory we either copy mh_nbytes or just change mh_nbytes if there is
   enough room in the block for the new size.  Range checking is always
   done. */
struct mhead {
  char     mh_alloc;	/* ISALLOC or ISFREE */		/* 1 */
  char     mh_index;	/* index in nextf[] */		/* 1 */
/* Remainder are valid only when block is allocated */
  u_bits32_t mh_nbytes;  /* # of bytes allocated */	/* 4 */
  u_bits16_t mh_magic2;/* should be == MAGIC2 */	/* 2 */
};

/* Access free-list pointer of a block.
   It is stored at block + sizeof (char *).
   This is not a field in the mhead structure
   because we want sizeof (struct mhead)
   to describe the overhead for when the block is in use,
   and we do not want the free-list pointer to count in that.  */

#define CHAIN(a) \
  (*(struct mhead **) (sizeof (char *) + (char *) (a)))

#if defined (botch)
extern void botch ();
#else
static void
botch (s)
     char *s;
{
  fprintf (stderr, "\r\nmalloc: assertion botched: %s\r\n", s);
  (void)fflush (stderr);
  abort ();
}
#endif /* !botch */

#if !defined (__STRING)
#  if defined (__STDC__)
#    define __STRING(x) #x
#  else
#    define __STRING(x) "x"
#  endif
#endif /* !__STRING */

/* To implement range checking, we write magic values in at the beginning
   and end of each allocated block, and make sure they are undisturbed
   whenever a free or a realloc occurs. */

/* Written in each of the 4 bytes following the block's real space */
#define MAGIC1 0x55
/* Written in the 2 bytes before the block's real space */
#define MAGIC2 0x5555
#define ASSERT(p) do { if (!(p)) botch(__STRING(p)); } while (0)
#define MSLOP  4		/* 4 bytes extra for MAGIC1s */

/* Minimum and maximum bucket indices for block splitting (and to bound
   the search for a block to split). */
#define SPLIT_MIN	3
#define SPLIT_MID	9
#define SPLIT_MAX	12

/* Minimum and maximum bucket indices for block coalescing. */
#define COMBINE_MIN	6
#define COMBINE_MAX	(pagebucket - 1)

#define MIN_COMBINE_FREE	4

/* nextf[i] is free list of blocks of size 2**(i + 3)  */

static struct mhead *nextf[NBUCKETS];

/* busy[i] is nonzero while allocation of block size i is in progress.  */

static char busy[NBUCKETS];

static int pagesz;	/* system page size. */
static int pagebucket;	/* bucket for requests a page in size */

#if 0
/* Coalesce two adjacent free blocks off the free list for size NU - 1,
   as long as there are at least MIN_COMBINE_FREE free blocks and we
   can find two adjacent free blocks.  nextf[NU -1] is assumed to not
   be busy; the caller (morecore()) checks for this. */
static void
bcoalesce (nu)
     register int nu;
{
  register struct mhead *mp, *mp1, *mp2;
  register int nfree, nbuck;
  unsigned long siz;

  nbuck = nu - 1;
  if (nextf[nbuck] == 0)
    return;

  nfree = 1;
  mp1 = nextf[nbuck];
  mp = CHAIN (mp1);
  mp2 = (struct mhead *)0;
  while (CHAIN (mp))
    {
      mp2 = mp1;
      mp1 = mp;
      mp = CHAIN (mp);
      nfree++;
      /* We may not want to run all the way through the free list here;
	 if we do not, we need to check a threshold value here and break
	 if nfree exceeds it. */
    }
  if (nfree < MIN_COMBINE_FREE)
    return;
  /* OK, now we have mp1 pointing to the block we want to add to nextf[NU].
     CHAIN(mp2) must equal mp1.  Check that mp1 and mp are adjacent. */
  if (CHAIN(mp2) != mp1)
    botch ("bcoalesce: CHAIN(mp2) != mp1");
  siz = 1 << (nbuck + 3);
  if (CHAIN (mp1) != (struct mhead *)((char *)mp1 + siz))
    return;	/* not adjacent */

#ifdef MALLOC_STATS
  _mstats.nbcoalesce++;
#endif

  /* Since they are adjacent, remove them from the free list */
  CHAIN (mp2) = CHAIN (mp);

  /* And add the combined two blocks to nextf[NU]. */
  mp1->mh_alloc = ISFREE;
  mp1->mh_index = nu;
  CHAIN (mp1) = nextf[nu];
  nextf[nu] = mp1;
}
#endif

/* Split a block at index > NU (but less than SPLIT_MAX) into a set of
   blocks of the correct size, and attach them to nextf[NU].  nextf[NU]
   is assumed to be empty.  Must be called with signals blocked (e.g.,
   by morecore()). */
static void
bsplit (nu)
     register int nu;
{
  register struct mhead *mp;
  int nbuck, nblks;
  unsigned long siz;

  if (nu >= SPLIT_MID)
    {
      for (nbuck = SPLIT_MAX; nbuck > nu; nbuck--)
	{
	  if (busy[nbuck] || nextf[nbuck] == 0)
	    continue;
	  break;
	}
    }
  else
    {
      for (nbuck = nu + 1; nbuck <= SPLIT_MAX; nbuck++)
	{
	  if (busy[nbuck] || nextf[nbuck] == 0)
	    continue;
	  break;
	}
    }

  if (nbuck > SPLIT_MAX || nbuck <= nu)
    return;

  /* XXX might want to split only if nextf[nbuck] has >= 2 blocks free
     and nbuck is below some threshold. */

#ifdef MALLOC_STATS
  _mstats.nbsplit++;
#endif

  /* Figure out how many blocks we'll get. */
  siz = (1 << (nu + 3));
  nblks = (1 << (nbuck + 3)) / siz;

  /* Remove the block from the chain of larger blocks. */
  mp = nextf[nbuck];
  nextf[nbuck] = CHAIN (mp);

  /* Split the block and put it on the requested chain. */
  nextf[nu] = mp;
  while (1)
    {
      mp->mh_alloc = ISFREE;
      mp->mh_index = nu;
      if (--nblks <= 0) break;
      CHAIN (mp) = (struct mhead *)((char *)mp + siz);
      mp = (struct mhead *)((char *)mp + siz);
    }
  CHAIN (mp) = 0;
}

static void
morecore (nu)			/* ask system for more memory */
     register int nu;		/* size index to get more of  */
{
  register struct mhead *mp;
  register int nblks;
  register long siz;
  long sbrk_amt;		/* amount to get via sbrk() */

  /* Block all signals in case we are executed from a signal handler. */
#if defined (HAVE_BSD_SIGNALS)
  int oldmask;
  oldmask = sigsetmask (-1);
#else
#  if defined (HAVE_POSIX_SIGNALS)
  sigset_t set, oset;
  sigfillset (&set);
  sigemptyset (&oset);
  sigprocmask (SIG_BLOCK, &set, &oset);
#  endif /* HAVE_POSIX_SIGNALS */
#endif /* HAVE_BSD_SIGNALS */

  siz = 1 << (nu + 3);	/* size of desired block for nextf[nu] */

  if (siz < 0)
    return;		/* oops */

#ifdef MALLOC_STATS
  _mstats.nmorecore[nu]++;
#endif

  /* Try to split a larger block here, if we're within the range of sizes
     to split. */
  if (nu >= SPLIT_MIN && nu < SPLIT_MAX)
    {
      bsplit (nu);
      if (nextf[nu] != 0)
	goto morecore_done;
    }

#if 0
  /* Try to coalesce two adjacent blocks from the free list on nextf[nu - 1],
     if we can, and we're withing the range of the block coalescing limits. */
  if (nu >= COMBINE_MIN && nu < COMBINE_MAX && busy[nu - 1] == 0 && nextf[nu - 1])
    {
      bcoalesce (nu);
      if (nextf[nu] != 0)
        goto morecore_done;
    }
#endif

  /* Take at least a page, and figure out how many blocks of the requested
     size we're getting. */
  if (siz <= pagesz)
    {
      sbrk_amt = pagesz;
      nblks = sbrk_amt / siz;
    }
  else
    {
      /* We always want to request an integral multiple of the page size
	 from the kernel, so let's compute whether or not `siz' is such
	 an amount.  If it is, we can just request it.  If not, we want
	 the smallest integral multiple of pagesize that is larger than
	 `siz' and will satisfy the request. */
      sbrk_amt = siz % pagesz;
      if (sbrk_amt == 0)
	sbrk_amt = siz;
      else
	sbrk_amt = siz + pagesz - sbrk_amt;
      nblks = 1;
    }

#ifdef MALLOC_STATS
  _mstats.nsbrk++;
  _mstats.tsbrk += sbrk_amt;
#endif

  mp = (struct mhead *) sbrk (sbrk_amt);

  /* Totally out of memory. */
  if ((long)mp == -1)
    return;

  /* shouldn't happen, but just in case */
  if ((long)mp & 7)
    {
      mp = (struct mhead *) (((long)mp + 8) & ~7);
      nblks--;
    }

  /* save new header and link the nblks blocks together */
  nextf[nu] = mp;
  while (1)
    {
      mp->mh_alloc = ISFREE;
      mp->mh_index = nu;
      if (--nblks <= 0) break;
      CHAIN (mp) = (struct mhead *)((char *)mp + siz);
      mp = (struct mhead *)((char *)mp + siz);
    }
  CHAIN (mp) = 0;

morecore_done:
#if defined (HAVE_BSD_SIGNALS)
  sigsetmask (oldmask);
#else
#  if defined (HAVE_POSIX_SIGNALS)
  sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
#  endif
#endif /* HAVE_BSD_SIGNALS */
}

#if defined (MEMSCRAMBLE) || !defined (NO_CALLOC)
static char *
zmemset (s, c, n)
     char *s;
     int c;
     register int n;
{
  register char *sp;

  sp = s;
  while (--n >= 0)
    *sp++ = c;
  return (s);
}
#endif /* MEMSCRAMBLE || !NO_CALLOC */

static void
malloc_debug_dummy ()
{
  ;
}

char *
malloc (n)		/* get a block */
     size_t n;
{
  register struct mhead *p;
  register long nbytes;
  register int nunits;

  /* Get the system page size and align break pointer so everything will
     be page-aligned.  The page size must be at least 1K -- anything
     smaller is increased. */
  if (pagesz == 0)
    {
      register long sbrk_needed;

      pagesz = getpagesize ();
      if (pagesz < 1024)
        pagesz = 1024;
      /* OK, how much do we need to allocate to make things page-aligned?
         This partial page is wasted space.  Once we figure out how much
         to advance the break pointer, go ahead and do it. */
      sbrk_needed = pagesz - ((long)sbrk (0) & (pagesz - 1));	/* sbrk(0) % pagesz */
      if (sbrk_needed < 0)
        sbrk_needed += pagesz;
      /* Now allocate the wasted space. */
      if (sbrk_needed)
        {
#ifdef MALLOC_STATS
	  _mstats.nsbrk++;
	  _mstats.tsbrk += sbrk_needed;
#endif
          if ((long)sbrk (sbrk_needed) == -1)
            return (NULL);
        }
      nunits = 0;
      nbytes = 8;
      while (pagesz > nbytes)
        {
          nbytes <<= 1;
          nunits++;
        }
      pagebucket = nunits;
    }
 
  /* Figure out how many bytes are required, rounding up to the nearest
     multiple of 4, then figure out which nextf[] area to use.  Try to
     be smart about where to start searching -- if the number of bytes
     needed is greater than the page size, we can start at pagebucket. */
  nbytes = (n + sizeof *p + MSLOP + 3) & ~3;
  nunits = 0;
  if (nbytes <= (pagesz >> 1))
    {
      register unsigned int shiftr;

      shiftr = (nbytes - 1) >> 2;	/* == (nbytes - 1) / 4 */
      while (shiftr >>= 1)		/* == (nbytes - 1) / {8,16,32,...} */
	nunits++;
    }
  else
    {
      register u_bits32_t amt;

      nunits = pagebucket;
      amt = pagesz;
      while (nbytes > amt)
        {
          amt <<= 1;
          nunits++;
        }
    }

  /* In case this is reentrant use of malloc from signal handler,
     pick a block size that no other malloc level is currently
     trying to allocate.  That's the easiest harmless way not to
     interfere with the other level of execution.  */
#ifdef MALLOC_STATS
  if (busy[nunits]) _mstats.nrecurse++;
#endif
  while (busy[nunits]) nunits++;
  busy[nunits] = 1;

  /* If there are no blocks of the appropriate size, go get some */
  if (nextf[nunits] == 0)
    morecore (nunits);

  /* Get one block off the list, and set the new list head */
  if ((p = nextf[nunits]) == NULL)
    {
      busy[nunits] = 0;
      return NULL;
    }
  nextf[nunits] = CHAIN (p);
  busy[nunits] = 0;

  /* Check for free block clobbered */
  /* If not for this check, we would gobble a clobbered free chain ptr
     and bomb out on the NEXT allocate of this size block */
  if (p->mh_alloc != ISFREE || p->mh_index != nunits)
    botch ("malloc: block on free list clobbered");

  /* Fill in the info, and if range checking, set up the magic numbers */
  p->mh_alloc = ISALLOC;
  p->mh_nbytes = n;
  p->mh_magic2 = MAGIC2;
  {
    register char  *m = (char *) (p + 1) + n;

    *m++ = MAGIC1, *m++ = MAGIC1, *m++ = MAGIC1, *m = MAGIC1;
  }

#ifdef MEMSCRAMBLE
  zmemset ((char *)(p + 1), 0xdf, n);	/* scramble previous contents */
#endif
#ifdef MALLOC_STATS
  _mstats.nmalloc[nunits]++;
  _mstats.tmalloc[nunits]++;
  _mstats.nmal++;
#endif /* MALLOC_STATS */
  return (char *) (p + 1);
}

void
free (mem)
     char *mem;
{
  register struct mhead *p;
  register char *ap;
  register int nunits;

  if ((ap = mem) == 0)
    return;

  p = (struct mhead *) ap - 1;

  if (p->mh_alloc == ISMEMALIGN)
    {
      ap -= p->mh_nbytes;
      p = (struct mhead *) ap - 1;
    }

  if (p->mh_alloc != ISALLOC)
    {
      if (p->mh_alloc == ISFREE)
	botch ("free: called with already freed block argument");
      else
	botch ("free: called with unallocated block argument");
    }

  ASSERT (p->mh_magic2 == MAGIC2);
  ap += p->mh_nbytes;
  ASSERT (*ap++ == MAGIC1); ASSERT (*ap++ == MAGIC1);
  ASSERT (*ap++ == MAGIC1); ASSERT (*ap   == MAGIC1);

#ifdef MEMSCRAMBLE
  zmemset (mem, 0xcf, p->mh_nbytes);
#endif

  nunits = p->mh_index;

  ASSERT (nunits < NBUCKETS);
  p->mh_alloc = ISFREE;

  /* Protect against signal handlers calling malloc.  */
  busy[nunits] = 1;
  /* Put this block on the free list.  */
  CHAIN (p) = nextf[nunits];
  nextf[nunits] = p;
  busy[nunits] = 0;

#ifdef MALLOC_STATS
  _mstats.nmalloc[nunits]--;
  _mstats.nfre++;
#endif /* MALLOC_STATS */
}

char *
realloc (mem, n)
     char *mem;
     register size_t n;
{
  register struct mhead *p;
  register u_bits32_t tocopy;
  register unsigned int nbytes;
  register int nunits;
  register char *m;

#ifdef MALLOC_STATS
  _mstats.nrealloc++;
#endif

  if (n == 0)
    {
      free (mem);
      return (NULL);
    }
  if ((p = (struct mhead *) mem) == 0)
    return malloc (n);
  p--;
  nunits = p->mh_index;
  ASSERT (p->mh_alloc == ISALLOC);
  ASSERT (p->mh_magic2 == MAGIC2);

  m = mem + (tocopy = p->mh_nbytes);
  ASSERT (*m++ == MAGIC1); ASSERT (*m++ == MAGIC1);
  ASSERT (*m++ == MAGIC1); ASSERT (*m   == MAGIC1);

  /* See if desired size rounds to same power of 2 as actual size. */
  nbytes = (n + sizeof *p + MSLOP + 7) & ~7;

  /* If ok, use the same block, just marking its size as changed.  */
  if (nbytes > (4 << nunits) && nbytes <= (8 << nunits))
    {
      m = mem + tocopy;
      *m++ = 0;  *m++ = 0;  *m++ = 0;  *m++ = 0;
      p->mh_nbytes = n;
      m = mem + n;
      *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;
      return mem;
    }

#ifdef MALLOC_STATS
  _mstats.nrcopy++;
#endif

  if (n < tocopy)
    tocopy = n;

  if ((m = malloc (n)) == 0)
    return 0;
  FASTCOPY (mem, m, tocopy);
  free (mem);
  return m;
}

char *
memalign (alignment, size)
     unsigned int alignment;
     size_t size;
{
  register char *ptr;
  register char *aligned;
  register struct mhead *p;

  ptr = malloc (size + alignment);

  if (ptr == 0)
    return 0;
  /* If entire block has the desired alignment, just accept it.  */
  if (((int) ptr & (alignment - 1)) == 0)
    return ptr;
  /* Otherwise, get address of byte in the block that has that alignment.  */
  aligned = (char *) (((int) ptr + alignment - 1) & -alignment);

  /* Store a suitable indication of how to free the block,
     so that free can find the true beginning of it.  */
  p = (struct mhead *) aligned - 1;
  p->mh_nbytes = aligned - ptr;
  p->mh_alloc = ISMEMALIGN;
  return aligned;
}

#if !defined (HPUX)
/* This runs into trouble with getpagesize on HPUX, and Multimax machines.
   Patching out seems cleaner than the ugly fix needed.  */
#if defined (__STDC__)
void *
#else
char *
#endif
valloc (size)
     size_t size;
{
  return memalign (getpagesize (), size);
}
#endif /* !HPUX */

#ifndef NO_CALLOC
char *
calloc (n, s)
     size_t n, s;
{
  size_t total;
  char *result;

  total = n * s;
  result = malloc (total);
  if (result)
    zmemset (result, 0, total);
  return result;  
}

void
cfree (p)
     char *p;
{
  free (p);
}
#endif /* !NO_CALLOC */

#ifdef MALLOC_STATS

struct bucket_stats
malloc_bucket_stats (size)
     int size;
{
  struct bucket_stats v;
  register struct mhead *p;

  v.nfree = 0;

  if (size < 0 || size >= NBUCKETS)
    {
      v.blocksize = 0;
      v.nused = v.nmal = 0;
      return v;
    }

  v.blocksize = 1 << (size + 3);
  v.nused = _mstats.nmalloc[size];
  v.nmal = _mstats.tmalloc[size];
  v.nmorecore = _mstats.nmorecore[size];

  for (p = nextf[size]; p; p = CHAIN (p))
    v.nfree++;

  return v;
}

/* Return a copy of _MSTATS, with two additional fields filled in:
   BYTESFREE is the total number of bytes on free lists.  BYTESUSED
   is the total number of bytes in use.  These two fields are fairly
   expensive to compute, so we do it only when asked to. */
struct _malstats
malloc_stats ()
{
  struct _malstats result;
  struct bucket_stats v;
  register int i;

  result = _mstats;
  result.bytesused = result.bytesfree = 0;
  for (i = 0; i < NBUCKETS; i++)
    {
      v = malloc_bucket_stats (i);
      result.bytesfree += v.nfree * v.blocksize;
      result.bytesused += v.nused * v.blocksize;
    }
  return (result);
}

void
print_malloc_stats (s)
     char *s;
{
  register int i;
  int totused, totfree;
  struct bucket_stats v;

  fprintf (stderr, "Memory allocation statistics: %s\n\tsize\tfree\tin use\ttotal\tmorecore\n", s ? s : "");
  for (i = totused = totfree = 0; i < NBUCKETS; i++)
    {
      v = malloc_bucket_stats (i);
      fprintf (stderr, "%12lu\t%4d\t%6d\t%5d\t%8d\n", v.blocksize, v.nfree, v.nused, v.nmal, v.nmorecore);
      totfree += v.nfree * v.blocksize;
      totused += v.nused * v.blocksize;
    }
  fprintf (stderr, "\nTotal bytes in use: %d, total bytes free: %d\n",
	   totused, totfree);
  fprintf (stderr, "Total mallocs: %d, total frees: %d, total reallocs: %d (%d copies)\n",
	   _mstats.nmal, _mstats.nfre, _mstats.nrealloc, _mstats.nrcopy);
  fprintf (stderr, "Total sbrks: %d, total bytes via sbrk: %d\n",
  	   _mstats.nsbrk, _mstats.tsbrk);
  fprintf (stderr, "Total blocks split: %d, total block coalesces: %d\n",
  	   _mstats.nbsplit, _mstats.nbcoalesce);
}
#endif /* MALLOC_STATS */
