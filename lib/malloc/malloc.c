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

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information will
 * go in the first int of the block, and the returned pointer will point
 * to the second.
 */

/* Define this to have free() write 0xcf into memory as it's freed, to
   uncover callers that refer to freed memory. */
/* SCO 3.2v4 getcwd and possibly other libc routines fail with MEMSCRAMBLE */
#if !defined (NO_MEMSCRAMBLE)
#  define MEMSCRAMBLE
#endif

#if defined (HAVE_CONFIG_H)
#  include <config.h>
#endif /* HAVE_CONFIG_H */

#if defined (SHELL)
#  include "bashtypes.h"
#  include "stdc.h"
#else
#  include <sys/types.h>

#  ifndef __P
#    if defined (__STDC__) || defined (__GNUC__) || defined (__cplusplus)
#      define __P(protos) protos
#    else
#      define __P(protos) ()
#    endif
#  endif

#endif

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

/* Determine which kind of system this is.  */
#include <signal.h>

#if defined (HAVE_STRING_H)
#  include <string.h>
#else
#  include <strings.h>
#endif

#include <stdio.h>

/* Define getpagesize () if the system does not.  */
#ifndef HAVE_GETPAGESIZE
#  include "getpagesize.h"
#endif

#include "imalloc.h"
#ifdef MALLOC_STATS
#  include "mstats.h"
#endif
#ifdef MALLOC_REGISTER
#  include "table.h"
#endif

/* System-specific omissions. */
#ifdef HPUX
#  define NO_VALLOC
#endif

#define NBUCKETS	30

#define ISALLOC ((char) 0xf7)	/* magic byte that implies allocation */
#define ISFREE ((char) 0x54)	/* magic byte that implies free block */
				/* this is for error checking only */
#define ISMEMALIGN ((char) 0xd6)  /* Stored before the value returned by
				     memalign, with the rest of the word
				     being the distance to the true
				     beginning of the block.  */


/* We have a flag indicating whether memory is allocated, an index in
   nextf[], a size field, and a sentinel value to determine whether or
   not a caller wrote before the start of allocated memory; to realloc()
   memory we either copy mh_nbytes or just change mh_nbytes if there is
   enough room in the block for the new size.  Range checking is always
   done. */
union mhead {
  bits64_t mh_align;						/* 8 */
  struct {
    char mi_alloc; 		/* ISALLOC or ISFREE */		/* 1 */
    char mi_index;		/* index in nextf[] */		/* 1 */
    /* Remainder are valid only when block is allocated */
    u_bits16_t mi_magic2;	/* should be == MAGIC2 */	/* 2 */
    u_bits32_t mi_nbytes;	/* # of bytes allocated */	/* 4 */
  } minfo;
};
#define mh_alloc	minfo.mi_alloc
#define mh_index	minfo.mi_index
#define mh_nbytes	minfo.mi_nbytes
#define mh_magic2	minfo.mi_magic2

/* Access free-list pointer of a block.
   It is stored at block + sizeof (char *).
   This is not a field in the minfo structure member of union mhead
   because we want sizeof (union mhead)
   to describe the overhead for when the block is in use,
   and we do not want the free-list pointer to count in that.  */

#define CHAIN(a) \
  (*(union mhead **) (sizeof (char *) + (char *) (a)))

/* To implement range checking, we write magic values in at the beginning
   and end of each allocated block, and make sure they are undisturbed
   whenever a free or a realloc occurs. */

/* Written in each of the 4 bytes following the block's real space */
#define MAGIC1 0x55
/* Written in the 2 bytes before the block's real space (-4 bytes) */
#define MAGIC2 0x5555
#define MSLOP  4		/* 4 bytes extra for MAGIC1s */

/* How many bytes are actually allocated for a request of size N --
   rounded up to nearest multiple of 8 after accounting for malloc
   overhead. */
#define ALLOCATED_BYTES(n)  (((n) + sizeof (union mhead) + MSLOP + 7) & ~7)

#define ASSERT(p) \
  do \
    { \
      if (!(p)) xbotch((PTR_T)0, ERR_ASSERT_FAILED, __STRING(p), file, line); \
    } \
  while (0)

/* Minimum and maximum bucket indices for block splitting (and to bound
   the search for a block to split). */
#define SPLIT_MIN	3
#define SPLIT_MID	11	/* XXX - was 9 */
#define SPLIT_MAX	14	/* XXX - was 12 */

/* Minimum and maximum bucket indices for block coalescing. */
#define COMBINE_MIN	6
#define COMBINE_MAX	(pagebucket - 1)

#define MIN_COMBINE_FREE	4

/* Flags for the internal functions. */
#define MALLOC_WRAPPER	0x01	/* wrapper function */
#define MALLOC_INTERNAL	0x02	/* internal function calling another */
#define MALLOC_NOTRACE	0x04	/* don't trace this allocation or free */
#define MALLOC_NOREG	0x08	/* don't register this allocation or free */

/* Future use. */
#define ERR_DUPFREE		0x01
#define ERR_UNALLOC		0x02
#define ERR_UNDERFLOW		0x04	
#define ERR_ASSERT_FAILED	0x08

/* Evaluates to true if NB is appropriate for bucket NU.  NB is adjusted
   appropriately by the caller to account for malloc overhead. */
#define IN_BUCKET(nb, nu) \
  ((nb) > (4 << (nu)) && ((nb) <= (8 << (nu))))

/* nextf[i] is free list of blocks of size 2**(i + 3)  */

static union mhead *nextf[NBUCKETS];

/* busy[i] is nonzero while allocation of block size i is in progress.  */

static char busy[NBUCKETS];

static int pagesz;	/* system page size. */
static int pagebucket;	/* bucket for requests a page in size */
static int maxbuck;	/* highest bucket receiving allocation request. */

/* Declarations for internal functions */
static PTR_T internal_malloc __P((size_t, const char *, int, int));
static PTR_T internal_realloc __P((PTR_T, size_t, const char *, int, int));
static void internal_free __P((PTR_T, const char *, int, int));
static PTR_T internal_memalign __P((unsigned int, size_t, const char *, int, int));
#ifndef NO_CALLOC
static PTR_T internal_calloc __P((size_t, size_t, const char *, int, int));
static void internal_cfree __P((PTR_T, const char *, int, int));
#endif
#ifndef NO_VALLOC
static PTR_T internal_valloc __P((size_t, const char *, int, int));
#endif

#if defined (botch)
extern void botch ();
#else
static void botch __P((const char *, const char *, int));
#endif
static void xbotch __P((PTR_T, int, const char *, const char *, int));

#ifdef MALLOC_STATS
extern struct _malstats _mstats;
#endif /* MALLOC_STATS */

#if !HAVE_DECL_SBRK
extern char *sbrk ();
#endif /* !HAVE_DECL_SBRK */

#ifdef SHELL
extern int interrupt_immediately;
extern int signal_is_trapped __P((int));
#endif

/* Debugging variables available to applications. */
int malloc_flags = 0;	/* future use */
int malloc_trace = 0;	/* trace allocations and frees to stderr */
int malloc_register = 0;	/* future use */

#if !defined (botch)
static void
botch (s, file, line)
{
  fprintf (stderr, "malloc: failed assertion: %s\n", s);
  (void)fflush (stderr);
  abort ();
}
#endif

/* print the file and line number that caused the assertion failure and
   call botch() to do whatever the application wants with the information */
static void
xbotch (mem, e, s, file, line)
     PTR_T mem;
     int e;
     const char *s;
     const char *file;
     int line;
{
  fprintf (stderr, "\r\nmalloc: %s:%d: assertion botched\r\n",
			file ? file : "unknown", line);
#ifdef MALLOC_REGISTER
  if (mem != NULL && malloc_register)
    mregister_describe_mem (mem, stderr);
#endif
  (void)fflush (stderr);
  botch(s, file, line);
}

#if 0
/* Coalesce two adjacent free blocks off the free list for size NU - 1,
   as long as there are at least MIN_COMBINE_FREE free blocks and we
   can find two adjacent free blocks.  nextf[NU -1] is assumed to not
   be busy; the caller (morecore()) checks for this. */
static void
bcoalesce (nu)
     register int nu;
{
  register union mhead *mp, *mp1, *mp2;
  register int nfree, nbuck;
  unsigned long siz;

  nbuck = nu - 1;
  if (nextf[nbuck] == 0)
    return;

  nfree = 1;
  mp1 = nextf[nbuck];
  mp = CHAIN (mp1);
  mp2 = (union mhead *)0;
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
    xbotch ((PTR_T)0, 0, "bcoalesce: CHAIN(mp2) != mp1", (char *)NULL, 0);
  siz = 1 << (nbuck + 3);
  if (CHAIN (mp1) != (union mhead *)((char *)mp1 + siz))
    return;	/* not adjacent */

#ifdef MALLOC_STATS
  _mstats.tbcoalesce++;
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
  register union mhead *mp;
  int nbuck, nblks, split_max;
  unsigned long siz;

  split_max = (maxbuck > SPLIT_MAX) ? maxbuck : SPLIT_MAX;

  if (nu >= SPLIT_MID)
    {
      for (nbuck = split_max; nbuck > nu; nbuck--)
	{
	  if (busy[nbuck] || nextf[nbuck] == 0)
	    continue;
	  break;
	}
    }
  else
    {
      for (nbuck = nu + 1; nbuck <= split_max; nbuck++)
	{
	  if (busy[nbuck] || nextf[nbuck] == 0)
	    continue;
	  break;
	}
    }

  if (nbuck > split_max || nbuck <= nu)
    return;

  /* XXX might want to split only if nextf[nbuck] has >= 2 blocks free
     and nbuck is below some threshold. */

#ifdef MALLOC_STATS
  _mstats.tbsplit++;
  _mstats.nsplit[nbuck]++;
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
      CHAIN (mp) = (union mhead *)((char *)mp + siz);
      mp = (union mhead *)((char *)mp + siz);
    }
  CHAIN (mp) = 0;
}

static void
block_signals (setp, osetp)
     sigset_t *setp, *osetp;
{
#ifdef HAVE_POSIX_SIGNALS
  sigfillset (setp);
  sigemptyset (osetp);
  sigprocmask (SIG_BLOCK, setp, osetp);
#else
#  if defined (HAVE_BSD_SIGNALS)
  *osetp = sigsetmask (-1);
#  endif
#endif
}

static void
unblock_signals (setp, osetp)
     sigset_t *setp, *osetp;
{
#ifdef HAVE_POSIX_SIGNALS
  sigprocmask (SIG_SETMASK, osetp, (sigset_t *)NULL);
#else
#  if defined (HAVE_BSD_SIGNALS)
  sigsetmask (*osetp);
#  endif
#endif
}
  
static void
morecore (nu)			/* ask system for more memory */
     register int nu;		/* size index to get more of  */
{
  register union mhead *mp;
  register int nblks;
  register long siz;
  long sbrk_amt;		/* amount to get via sbrk() */
  sigset_t set, oset;
  int blocked_sigs;

  /* Block all signals in case we are executed from a signal handler. */
  blocked_sigs = 0;
#ifdef SHELL
  if (interrupt_immediately || signal_is_trapped (SIGINT) || signal_is_trapped (SIGCHLD))
#endif
    {
      block_signals (&set, &oset);
      blocked_sigs = 1;
    }

  siz = 1 << (nu + 3);	/* size of desired block for nextf[nu] */

  if (siz < 0)
    goto morecore_done;		/* oops */

#ifdef MALLOC_STATS
  _mstats.nmorecore[nu]++;
#endif

  /* Try to split a larger block here, if we're within the range of sizes
     to split. */
  if (nu >= SPLIT_MIN)
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

  mp = (union mhead *) sbrk (sbrk_amt);

  /* Totally out of memory. */
  if ((long)mp == -1)
    goto morecore_done;

  /* shouldn't happen, but just in case -- require 8-byte alignment */
  if ((long)mp & 7)
    {
      mp = (union mhead *) (((long)mp + 7) & ~7);
      nblks--;
    }

  /* save new header and link the nblks blocks together */
  nextf[nu] = mp;
  while (1)
    {
      mp->mh_alloc = ISFREE;
      mp->mh_index = nu;
      if (--nblks <= 0) break;
      CHAIN (mp) = (union mhead *)((char *)mp + siz);
      mp = (union mhead *)((char *)mp + siz);
    }
  CHAIN (mp) = 0;

morecore_done:
  if (blocked_sigs)
    unblock_signals (&set, &oset);
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
  write (1, "malloc_debug_dummy\n", 19);
}

static PTR_T
internal_malloc (n, file, line, flags)		/* get a block */
     size_t n;
     const char *file;
     int line, flags;
{
  register union mhead *p;
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
     multiple of 8, then figure out which nextf[] area to use.  Try to
     be smart about where to start searching -- if the number of bytes
     needed is greater than the page size, we can start at pagebucket. */
  nbytes = ALLOCATED_BYTES(n);
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

  /* Silently reject too-large requests. */
  if (nunits >= NBUCKETS)
    return ((PTR_T) NULL);

  /* In case this is reentrant use of malloc from signal handler,
     pick a block size that no other malloc level is currently
     trying to allocate.  That's the easiest harmless way not to
     interfere with the other level of execution.  */
#ifdef MALLOC_STATS
  if (busy[nunits]) _mstats.nrecurse++;
#endif
  while (busy[nunits]) nunits++;
  busy[nunits] = 1;

  if (nunits > maxbuck)
    maxbuck = nunits;

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
    xbotch ((PTR_T)0, 0, "malloc: block on free list clobbered", file, line);

  /* Fill in the info, and set up the magic numbers for range checking. */
  p->mh_alloc = ISALLOC;
  p->mh_magic2 = MAGIC2;
  p->mh_nbytes = n;
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

#ifdef MALLOC_TRACE
  if (malloc_trace && (flags & MALLOC_NOTRACE) == 0)
    mtrace_alloc ("malloc", p + 1, n, file, line);
#endif

#ifdef MALLOC_REGISTER
  if (malloc_register && (flags & MALLOC_NOREG) == 0)
    mregister_alloc ("malloc", p + 1, n, file, line);
#endif

  return (char *) (p + 1);	/* XXX - should be cast to PTR_T? */
}

static void
internal_free (mem, file, line, flags)
     PTR_T mem;
     const char *file;
     int line, flags;
{
  register union mhead *p;
  register char *ap;
  register int nunits;
  register unsigned int nbytes;
  int ubytes;		/* caller-requested size */

  if ((ap = (char *)mem) == 0)
    return;

  p = (union mhead *) ap - 1;

  if (p->mh_alloc == ISMEMALIGN)
    {
      ap -= p->mh_nbytes;
      p = (union mhead *) ap - 1;
    }

#if defined (MALLOC_TRACE) || defined (MALLOC_REGISTER)
  if (malloc_trace || malloc_register)
    ubytes = p->mh_nbytes;
#endif

  if (p->mh_alloc != ISALLOC)
    {
      if (p->mh_alloc == ISFREE)
	xbotch (mem, ERR_DUPFREE,
		"free: called with already freed block argument", file, line);
      else
	xbotch (mem, ERR_UNALLOC,
		"free: called with unallocated block argument", file, line);
    }

  ASSERT (p->mh_magic2 == MAGIC2);

  nunits = p->mh_index;
  nbytes = ALLOCATED_BYTES(p->mh_nbytes);
  /* Since the sizeof(u_bits32_t) bytes before the memory handed to the user
     are now used for the number of bytes allocated, a simple check of
     mh_magic2 is no longer sufficient to catch things like p[-1] = 'x'.
     We sanity-check the value of mh_nbytes against the size of the blocks
     in the appropriate bucket before we use it.  This can still cause problems
     and obscure errors if mh_nbytes is wrong but still within range; the
     checks against MAGIC1 will probably fail then.  Using MALLOC_REGISTER
     will help here, since it saves the original number of bytes requested. */
  if (IN_BUCKET(nbytes, nunits) == 0)
    xbotch (mem, ERR_UNDERFLOW,
	    "free: underflow detected; mh_nbytes out of range", file, line);

  ap += p->mh_nbytes;
  ASSERT (*ap++ == MAGIC1); ASSERT (*ap++ == MAGIC1);
  ASSERT (*ap++ == MAGIC1); ASSERT (*ap   == MAGIC1);

#ifdef MEMSCRAMBLE
  zmemset (mem, 0xcf, p->mh_nbytes);
#endif

  ASSERT (nunits < NBUCKETS);
  p->mh_alloc = ISFREE;

  if (busy[nunits] == 1)
    return;	/* this is bogus, but at least it won't corrupt the chains */

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

#ifdef MALLOC_TRACE
  if (malloc_trace && (flags & MALLOC_NOTRACE) == 0)
    mtrace_free (mem, ubytes, file, line);
#endif

#ifdef MALLOC_REGISTER
  if (malloc_register && (flags & MALLOC_NOREG) == 0)
    mregister_free (mem, ubytes, file, line);
#endif
}

static PTR_T
internal_realloc (mem, n, file, line, flags)
     PTR_T mem;
     register size_t n;
     const char *file;
     int line, flags;
{
  register union mhead *p;
  register u_bits32_t tocopy;
  register unsigned int nbytes;
  register int nunits;
  register char *m;

#ifdef MALLOC_STATS
  _mstats.nrealloc++;
#endif

  if (n == 0)
    {
      internal_free (mem, file, line, MALLOC_INTERNAL);
      return (NULL);
    }
  if ((p = (union mhead *) mem) == 0)
    return internal_malloc (n, file, line, MALLOC_INTERNAL);

  p--;
  nunits = p->mh_index;
  ASSERT (nunits < NBUCKETS);

  if (p->mh_alloc != ISALLOC)
    xbotch (mem, ERR_UNALLOC,
	    "realloc: called with unallocated block argument", file, line);

  ASSERT (p->mh_magic2 == MAGIC2);
  nbytes = ALLOCATED_BYTES(p->mh_nbytes);
  /* Since the sizeof(u_bits32_t) bytes before the memory handed to the user
     are now used for the number of bytes allocated, a simple check of
     mh_magic2 is no longer sufficient to catch things like p[-1] = 'x'.
     We sanity-check the value of mh_nbytes against the size of the blocks
     in the appropriate bucket before we use it.  This can still cause problems
     and obscure errors if mh_nbytes is wrong but still within range; the
     checks against MAGIC1 will probably fail then.  Using MALLOC_REGISTER
     will help here, since it saves the original number of bytes requested. */
  if (IN_BUCKET(nbytes, nunits) == 0)
    xbotch (mem, ERR_UNDERFLOW,
	    "realloc: underflow detected; mh_nbytes out of range", file, line);

  m = (char *)mem + (tocopy = p->mh_nbytes);
  ASSERT (*m++ == MAGIC1); ASSERT (*m++ == MAGIC1);
  ASSERT (*m++ == MAGIC1); ASSERT (*m   == MAGIC1);

  /* See if desired size rounds to same power of 2 as actual size. */
  nbytes = ALLOCATED_BYTES(n);

  /* If ok, use the same block, just marking its size as changed.  */
  if (IN_BUCKET(nbytes, nunits))
    {
      m = (char *)mem + tocopy;
      *m++ = 0;  *m++ = 0;  *m++ = 0;  *m++ = 0;
      p->mh_nbytes = n;
      m = (char *)mem + n;
      *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;
      return mem;
    }

#ifdef MALLOC_STATS
  _mstats.nrcopy++;
#endif

  if (n < tocopy)
    tocopy = n;

  if ((m = internal_malloc (n, file, line, MALLOC_INTERNAL|MALLOC_NOTRACE|MALLOC_NOREG)) == 0)
    return 0;
  FASTCOPY (mem, m, tocopy);
  internal_free (mem, file, line, MALLOC_INTERNAL);

#ifdef MALLOC_TRACE
  if (malloc_trace && (flags & MALLOC_NOTRACE) == 0)
    mtrace_alloc ("realloc", m, n, file, line);
#endif

#ifdef MALLOC_REGISTER
  if (malloc_register && (flags & MALLOC_NOREG) == 0)
    mregister_alloc ("realloc", m, n, file, line);
#endif

  return m;
}

static PTR_T
internal_memalign (alignment, size, file, line, flags)
     unsigned int alignment;
     size_t size;
     const char *file;
     int line, flags;
{
  register char *ptr;
  register char *aligned;
  register union mhead *p;

  ptr = internal_malloc (size + alignment, file, line, MALLOC_INTERNAL);

  if (ptr == 0)
    return 0;
  /* If entire block has the desired alignment, just accept it.  */
  if (((long) ptr & (alignment - 1)) == 0)
    return ptr;
  /* Otherwise, get address of byte in the block that has that alignment.  */
#if 0
  aligned = (char *) (((long) ptr + alignment - 1) & -alignment);
#else
  aligned = (char *) (((long) ptr + alignment - 1) & (~alignment + 1));
#endif

  /* Store a suitable indication of how to free the block,
     so that free can find the true beginning of it.  */
  p = (union mhead *) aligned - 1;
  p->mh_nbytes = aligned - ptr;
  p->mh_alloc = ISMEMALIGN;

  return aligned;
}

#if !defined (NO_VALLOC)
/* This runs into trouble with getpagesize on HPUX, and Multimax machines.
   Patching out seems cleaner than the ugly fix needed.  */
static PTR_T
internal_valloc (size, file, line, flags)
     size_t size;
     const char *file;
     int line, flags;
{
  return internal_memalign (getpagesize (), size, file, line, flags|MALLOC_INTERNAL);
}
#endif /* !NO_VALLOC */

#ifndef NO_CALLOC
static PTR_T
internal_calloc (n, s, file, line, flags)
     size_t n, s;
     const char *file;
     int line, flags;
{
  size_t total;
  PTR_T result;

  total = n * s;
  result = internal_malloc (total, file, line, flags|MALLOC_INTERNAL);
  if (result)
    zmemset (result, 0, total);
  return result;  
}

static void
internal_cfree (p, file, line, flags)
     PTR_T p;
     const char *file;
     int line, flags;
{
  internal_free (p, file, line, flags|MALLOC_INTERNAL);
}
#endif /* !NO_CALLOC */

#ifdef MALLOC_STATS

int
malloc_free_blocks (size)
     int size;
{
  int nfree;
  register union mhead *p;

  nfree = 0;
  for (p = nextf[size]; p; p = CHAIN (p))
    nfree++;

  return nfree;
}
#endif

#if defined (SHELL)
PTR_T
sh_malloc (bytes, file, line)
     size_t bytes;
     const char *file;
     int line;
{
  return internal_malloc (bytes, file, line, MALLOC_WRAPPER);
}

PTR_T
sh_realloc (ptr, size, file, line)
     PTR_T ptr;
     size_t size;
     const char *file;
     int line;
{
  return internal_realloc (ptr, size, file, line, MALLOC_WRAPPER);
}

void
sh_free (mem, file, line)
     PTR_T mem;
     const char *file;
     int line;
{
  internal_free (mem, file, line, MALLOC_WRAPPER);
}

PTR_T
sh_memalign (alignment, size, file, line)
     unsigned int alignment;
     size_t size;
     const char *file;
     int line;
{
  return internal_memalign (alignment, size, file, line, MALLOC_WRAPPER);
}

#ifndef NO_CALLOC
PTR_T
sh_calloc (n, s, file, line)
     size_t n, s;
     const char *file;
     int line;
{
  return internal_calloc (n, s, file, line, MALLOC_WRAPPER);
}

void
sh_cfree (mem, file, line)
     PTR_T mem;
     const char *file;
     int line;
{
  internal_cfree (mem, file, line, MALLOC_WRAPPER);
}
#endif

#ifndef NO_VALLOC
PTR_T
sh_valloc (size, file, line)
     size_t size;
     const char *file;
     int line;
{
  return internal_valloc (size, file, line, MALLOC_WRAPPER);
}
#endif

#endif

/* Externally-available functions that call their internal counterparts. */

PTR_T
malloc (size)
     size_t size;
{
  return internal_malloc (size, (char *)NULL, 0, 0);
}

PTR_T
realloc (mem, nbytes)
     PTR_T mem;
     size_t nbytes;
{
  return internal_realloc (mem, nbytes, (char *)NULL, 0, 0);
}

void
free (mem)
     PTR_T mem;
{
  internal_free (mem,  (char *)NULL, 0, 0);
}

PTR_T
memalign (alignment, size)
     unsigned int alignment;
     size_t size;
{
  return internal_memalign (alignment, size, (char *)NULL, 0, 0);
}

#ifndef NO_VALLOC
PTR_T
valloc (size)
     size_t size;
{
  return internal_valloc (size, (char *)NULL, 0, 0);
}
#endif

#ifndef NO_CALLOC
PTR_T
calloc (n, s)
     size_t n, s;
{
  return internal_calloc (n, s, (char *)NULL, 0, 0);
}

void
cfree (mem)
     PTR_T mem;
{
  internal_cfree (mem, (char *)NULL, 0, 0);
}
#endif
