/* dynamic memory allocation for GNU. */

/*  Copyright (C) 1985, 1987 Free Software Foundation, Inc.

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
 * Call malloc_stats to get info on memory stats if MSTATS turned on.
 * realloc knows how to return same block given, just changing its size,
 * if the power of 2 is correct.
 */

/*
 * nextf[i] is the pointer to the next free block of size 2^(i+3).  The
 * smallest allocatable block is 8 bytes.  The overhead information will
 * go in the first int of the block, and the returned pointer will point
 * to the second.
 *
#ifdef MSTATS
 * nmalloc[i] is the difference between the number of mallocs and frees
 * for a given block size.
#endif
 */

/* Define this to have free() write 0xcf into memory as it's freed, to
   uncover callers that refer to freed memory. */
/* SCO 3.2v4 getcwd and possibly other libc routines fail with MEMSCRAMBLE */
#if !defined (NO_MEMSCRAMBLE)
#  define MEMSCRAMBLE
#endif

#if defined (emacs) || defined (HAVE_CONFIG_H)
#  include <config.h>
#endif /* emacs */

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

/* Define getpagesize () if the system does not.  */
#ifndef HAVE_GETPAGESIZE
#  include "getpagesize.h"
#endif

#if defined (HAVE_RESOURCE)
#  include <sys/time.h>
#  include <sys/resource.h>
#endif /* HAVE_RESOURCE */

/* Check for the needed symbols.  If they aren't present, this
   system's <sys/resource.h> isn't very useful to us. */
#if !defined (RLIMIT_DATA)
#  undef HAVE_RESOURCE
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

#define start_of_data() &etext

#define ISALLOC ((char) 0xf7)	/* magic byte that implies allocation */
#define ISFREE ((char) 0x54)	/* magic byte that implies free block */
				/* this is for error checking only */
#define ISMEMALIGN ((char) 0xd6)  /* Stored before the value returned by
				     memalign, with the rest of the word
				     being the distance to the true
				     beginning of the block.  */
extern char etext;

#if !defined (SBRK_DECLARED)
extern char *sbrk ();
#endif /* !SBRK_DECLARED */

/* These two are for user programs to look at, when they are interested.  */
unsigned int malloc_sbrk_used;       /* amount of data space used now */
unsigned int malloc_sbrk_unused;     /* amount more we can have */

/* start of data space; can be changed by calling init_malloc */
static char *data_space_start;

static void get_lim_data ();

#ifdef MSTATS
static int nmalloc[30];
static int nmal, nfre;
#endif /* MSTATS */

/* If range checking is not turned on, all we have is a flag indicating
   whether memory is allocated, an index in nextf[], and a size field; to
   realloc() memory we copy either size bytes or 1<<(index+3) bytes depending
   on whether the former can hold the exact size (given the value of
   'index').  If range checking is on, we always need to know how much space
   is allocated, so the 'size' field is never used. */

struct mhead {
	char     mh_alloc;	/* ISALLOC or ISFREE */
	char     mh_index;	/* index in nextf[] */
/* Remainder are valid only when block is allocated */
	unsigned short mh_size;	/* size, if < 0x10000 */
#ifdef RCHECK
	unsigned int mh_nbytes;	/* number of bytes allocated */
	int      mh_magic4;	/* should be == MAGIC4 */
#endif /* RCHECK */
};

/* Access free-list pointer of a block.
  It is stored at block + 4.
  This is not a field in the mhead structure
  because we want sizeof (struct mhead)
  to describe the overhead for when the block is in use,
  and we do not want the free-list pointer to count in that.  */

#define CHAIN(a) \
  (*(struct mhead **) (sizeof (char *) + (char *) (a)))

#ifdef RCHECK
#  include <stdio.h>
#  if !defined (botch)
#    define botch(x) abort ()
#  else
extern void botch();
#  endif /* botch */

#  if !defined (__STRING)
#    if defined (__STDC__)
#      define __STRING(x) #x
#    else
#      define __STRING(x) "x"
#    endif
#  endif

  /* To implement range checking, we write magic values in at the beginning
     and end of each allocated block, and make sure they are undisturbed
     whenever a free or a realloc occurs. */

  /* Written in each of the 4 bytes following the block's real space */
#  define MAGIC1 0x55
  /* Written in the 4 bytes before the block's real space */
#  define MAGIC4 0x55555555
#  define ASSERT(p) if (!(p)) botch(__STRING(p)); else
#  define EXTRA  4		/* 4 bytes extra for MAGIC1s */
#else /* !RCHECK */
#  define ASSERT(p)
#  define EXTRA  0
#endif /* RCHECK */

/* nextf[i] is free list of blocks of size 2**(i + 3)  */

static struct mhead *nextf[30];

/* busy[i] is nonzero while allocation of block size i is in progress.  */

static char busy[30];

/* Number of bytes of writable memory we can expect to be able to get */
static unsigned int lim_data;

/* Level number of warnings already issued.
  0 -- no warnings issued.
  1 -- 75% warning already issued.
  2 -- 85% warning already issued.
*/
static int warnlevel;

/* Function to call to issue a warning;
   0 means don't issue them.  */
static void (*warnfunction) ();

/* nonzero once initial bunch of free blocks made */
static int gotpool;

char *_malloc_base;

static void getpool ();

/* Cause reinitialization based on job parameters;
  also declare where the end of pure storage is. */
void
malloc_init (start, warnfun)
     char *start;
     void (*warnfun) ();
{
  if (start)
    data_space_start = start;
  lim_data = 0;
  warnlevel = 0;
  warnfunction = warnfun;
}

/* Return the maximum size to which MEM can be realloc'd
   without actually requiring copying.  */

int
malloc_usable_size (mem)
     char *mem;
{
  int blocksize = 8 << (((struct mhead *) mem) - 1) -> mh_index;

  return blocksize - sizeof (struct mhead) - EXTRA;
}

static void
morecore (nu)			/* ask system for more memory */
     register int nu;		/* size index to get more of  */
{
  register char *cp;
  register int nblks;
  register unsigned int siz;

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

  if (!data_space_start)
    {
      data_space_start = start_of_data ();
    }

  if (lim_data == 0)
    get_lim_data ();

 /* On initial startup, get two blocks of each size up to 1k bytes */
  if (!gotpool)
    { getpool (); getpool (); gotpool = 1; }

  /* Find current end of memory and issue warning if getting near max */

  cp = sbrk (0);
  siz = cp - data_space_start;
  malloc_sbrk_used = siz;
  malloc_sbrk_unused = lim_data - siz;

  if (warnfunction)
    switch (warnlevel)
      {
      case 0: 
	if (siz > (lim_data / 4) * 3)
	  {
	    warnlevel++;
	    (*warnfunction) ("Warning: past 75% of memory limit");
	  }
	break;
      case 1: 
	if (siz > (lim_data / 20) * 17)
	  {
	    warnlevel++;
	    (*warnfunction) ("Warning: past 85% of memory limit");
	  }
	break;
      case 2: 
	if (siz > (lim_data / 20) * 19)
	  {
	    warnlevel++;
	    (*warnfunction) ("Warning: past 95% of memory limit");
	  }
	break;
      }

  if ((int) cp & 0x3ff)	/* land on 1K boundaries */
    sbrk (1024 - ((int) cp & 0x3ff));

 /* Take at least 2k, and figure out how many blocks of the desired size
    we're about to get */
  nblks = 1;
  if ((siz = nu) < 8)
    nblks = 1 << ((siz = 8) - nu);

  if ((cp = sbrk (1 << (siz + 3))) == (char *) -1)
    return;			/* no more room! */

  if ((int) cp & 7)
    {		/* shouldn't happen, but just in case */
      cp = (char *) (((int) cp + 8) & ~7);
      nblks--;
    }

 /* save new header and link the nblks blocks together */
  nextf[nu] = (struct mhead *) cp;
  siz = 1 << (nu + 3);
  while (1)
    {
      ((struct mhead *) cp) -> mh_alloc = ISFREE;
      ((struct mhead *) cp) -> mh_index = nu;
      if (--nblks <= 0) break;
      CHAIN ((struct mhead *) cp) = (struct mhead *) (cp + siz);
      cp += siz;
    }
  CHAIN ((struct mhead *) cp) = 0;

#if defined (HAVE_BSD_SIGNALS)
  sigsetmask (oldmask);
#else
#  if defined (HAVE_POSIX_SIGNALS)
  sigprocmask (SIG_SETMASK, &oset, (sigset_t *)NULL);
#  endif
#endif /* HAVE_BSD_SIGNALS */
}

static void
getpool ()
{
  register int nu;
  register char *cp = sbrk (0);

  if ((int) cp & 0x3ff)	/* land on 1K boundaries */
    sbrk (1024 - ((int) cp & 0x3ff));

  /* Record address of start of space allocated by malloc.  */
  if (_malloc_base == 0)
    _malloc_base = cp;

  /* Get 2k of storage */

  cp = sbrk (04000);
  if (cp == (char *) -1)
    return;

  /* Divide it into an initial 8-word block
     plus one block of size 2**nu for nu = 3 ... 10.  */

  CHAIN (cp) = nextf[0];
  nextf[0] = (struct mhead *) cp;
  ((struct mhead *) cp) -> mh_alloc = ISFREE;
  ((struct mhead *) cp) -> mh_index = 0;
  cp += 8;

  for (nu = 0; nu < 7; nu++)
    {
      CHAIN (cp) = nextf[nu];
      nextf[nu] = (struct mhead *) cp;
      ((struct mhead *) cp) -> mh_alloc = ISFREE;
      ((struct mhead *) cp) -> mh_index = nu;
      cp += 8 << nu;
    }
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

char *
malloc (n)		/* get a block */
     unsigned int n;
{
  register struct mhead *p;
  register unsigned int nbytes;
  register int nunits = 0;

  /* Figure out how many bytes are required, rounding up to the nearest
     multiple of 4, then figure out which nextf[] area to use */
  nbytes = (n + sizeof *p + EXTRA + 3) & ~3;
  {
    register unsigned int   shiftr = (nbytes - 1) >> 2;

    while (shiftr >>= 1)
      nunits++;
  }

  /* In case this is reentrant use of malloc from signal handler,
     pick a block size that no other malloc level is currently
     trying to allocate.  That's the easiest harmless way not to
     interfere with the other level of execution.  */
  while (busy[nunits]) nunits++;
  busy[nunits] = 1;

  /* If there are no blocks of the appropriate size, go get some */
  /* COULD SPLIT UP A LARGER BLOCK HERE ... ACT */
  if (nextf[nunits] == 0)
    morecore (nunits);

  /* Get one block off the list, and set the new list head */
  if ((p = nextf[nunits]) == 0)
    {
      busy[nunits] = 0;
      return 0;
    }
  nextf[nunits] = CHAIN (p);
  busy[nunits] = 0;

  /* Check for free block clobbered */
  /* If not for this check, we would gobble a clobbered free chain ptr */
  /* and bomb out on the NEXT allocate of this size block */
  if (p -> mh_alloc != ISFREE || p -> mh_index != nunits)
#ifdef RCHECK
    botch ("block on free list clobbered");
#else /* not RCHECK */
    abort ();
#endif /* not RCHECK */

  /* Fill in the info, and if range checking, set up the magic numbers */
  p -> mh_alloc = ISALLOC;
#ifdef RCHECK
  p -> mh_nbytes = n;
  p -> mh_magic4 = MAGIC4;
  {
    register char  *m = (char *) (p + 1) + n;

    *m++ = MAGIC1, *m++ = MAGIC1, *m++ = MAGIC1, *m = MAGIC1;
  }
#else /* not RCHECK */
  p -> mh_size = n;
#endif /* not RCHECK */
#ifdef MEMSCRAMBLE
  zmemset ((char *)(p + 1), 0xdf, n);	/* scramble previous contents */
#endif
#ifdef MSTATS
  nmalloc[nunits]++;
  nmal++;
#endif /* MSTATS */
  return (char *) (p + 1);
}

void
free (mem)
     char *mem;
{
  register struct mhead *p;
  {
    register char *ap = mem;

    if (ap == 0)
      return;

    p = (struct mhead *) ap - 1;

    if (p -> mh_alloc == ISMEMALIGN)
      {
#ifdef RCHECK
	ap -= p->mh_nbytes;
#else
	ap -= p->mh_size;	/* XXX */
#endif
	p = (struct mhead *) ap - 1;
      }

#ifndef RCHECK
    if (p -> mh_alloc != ISALLOC)
      abort ();

#else /* RCHECK */
    if (p -> mh_alloc != ISALLOC)
      {
	if (p -> mh_alloc == ISFREE)
	  botch ("free: Called with already freed block argument\n");
	else
	  botch ("free: Called with unallocated block argument\n");
      }

    ASSERT (p -> mh_magic4 == MAGIC4);
    ap += p -> mh_nbytes;
    ASSERT (*ap++ == MAGIC1); ASSERT (*ap++ == MAGIC1);
    ASSERT (*ap++ == MAGIC1); ASSERT (*ap   == MAGIC1);
#endif /* RCHECK */
  }
#ifdef MEMSCRAMBLE
  {
    register int n;
    
#ifdef RCHECK
    n = p->mh_nbytes;
#else /* not RCHECK */
    n = p->mh_size;
#endif /* not RCHECK */
    zmemset (mem, 0xcf, n);
  }
#endif
  {
    register int nunits = p -> mh_index;

    ASSERT (nunits <= 29);
    p -> mh_alloc = ISFREE;

    /* Protect against signal handlers calling malloc.  */
    busy[nunits] = 1;
    /* Put this block on the free list.  */
    CHAIN (p) = nextf[nunits];
    nextf[nunits] = p;
    busy[nunits] = 0;

#ifdef MSTATS
    nmalloc[nunits]--;
    nfre++;
#endif /* MSTATS */
  }
}

char *
realloc (mem, n)
     char *mem;
     register unsigned int n;
{
  register struct mhead *p;
  register unsigned int tocopy;
  register unsigned int nbytes;
  register int nunits;

  if ((p = (struct mhead *) mem) == 0)
    return malloc (n);
  p--;
  nunits = p -> mh_index;
  ASSERT (p -> mh_alloc == ISALLOC);
#ifdef RCHECK
  ASSERT (p -> mh_magic4 == MAGIC4);
  {
    register char *m = mem + (tocopy = p -> mh_nbytes);
    ASSERT (*m++ == MAGIC1); ASSERT (*m++ == MAGIC1);
    ASSERT (*m++ == MAGIC1); ASSERT (*m   == MAGIC1);
  }
#else /* not RCHECK */
  if (p -> mh_index >= 13)
    tocopy = (1 << (p -> mh_index + 3)) - sizeof *p;
  else
    tocopy = p -> mh_size;
#endif /* not RCHECK */

  /* See if desired size rounds to same power of 2 as actual size. */
  nbytes = (n + sizeof *p + EXTRA + 7) & ~7;

  /* If ok, use the same block, just marking its size as changed.  */
  if (nbytes > (4 << nunits) && nbytes <= (8 << nunits))
    {
#ifdef RCHECK
      register char *m = mem + tocopy;
      *m++ = 0;  *m++ = 0;  *m++ = 0;  *m++ = 0;
      p-> mh_nbytes = n;
      m = mem + n;
      *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;  *m++ = MAGIC1;
#else /* not RCHECK */
      p -> mh_size = n;
#endif /* not RCHECK */
      return mem;
    }

  if (n < tocopy)
    tocopy = n;
  {
    register char *new;

    if ((new = malloc (n)) == 0)
      return 0;
    FASTCOPY (mem, new, tocopy);
    free (mem);
    return new;
  }
}

char *
memalign (alignment, size)
     unsigned int alignment, size;
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
  p -> mh_size = aligned - ptr;
  p -> mh_alloc = ISMEMALIGN;
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

#ifdef MSTATS
/* Return statistics describing allocation of blocks of size 2**n. */

struct mstats_value
  {
    int blocksize;
    int nfree;
    int nused;
  };

struct mstats_value
malloc_stats (size)
     int size;
{
  struct mstats_value v;
  register int i;
  register struct mhead *p;

  v.nfree = 0;

  if (size < 0 || size >= 30)
    {
      v.blocksize = 0;
      v.nused = 0;
      return v;
    }

  v.blocksize = 1 << (size + 3);
  v.nused = nmalloc[size];

  for (p = nextf[size]; p; p = CHAIN (p))
    v.nfree++;

  return v;
}
#endif /* MSTATS */

/*
 *	This function returns the total number of bytes that the process
 *	will be allowed to allocate via the sbrk(2) system call.  On
 *	BSD systems this is the total space allocatable to stack and
 *	data.  On USG systems this is the data space only.
 */

#if !defined (HAVE_RESOURCE)
extern long ulimit ();

static void
get_lim_data ()
{    
  lim_data = ulimit (3, 0);
  lim_data -= (long) data_space_start;
}

#else /* HAVE_RESOURCE */
static void
get_lim_data ()
{
  struct rlimit XXrlimit;

  getrlimit (RLIMIT_DATA, &XXrlimit);
#ifdef RLIM_INFINITY
  lim_data = XXrlimit.rlim_cur & RLIM_INFINITY; /* soft limit */
#else
  lim_data = XXrlimit.rlim_cur;	/* soft limit */
#endif
}

#endif /* HAVE_RESOURCE */
