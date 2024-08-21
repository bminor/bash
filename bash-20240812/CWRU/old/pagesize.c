/*
 * getpagesize - print the system pagesize
 *
 * Chet Ramey
 * chet@ins.cwru.edu
 */

#include <stdio.h>

/*
 * I know these systems have getpagesize(2)
 */

#if defined (Bsd) || defined (Ultrix) || defined (sun)
#  define HAVE_GETPAGESIZE
#endif

#if !defined (HAVE_GETPAGESIZE)

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#  if defined (_SC_PAGESIZE)
#    define getpagesize() sysconf(_SC_PAGESIZE)
#  endif /* _SC_PAGESIZE */
#endif

#if !defined (getpagesize)
#  include <sys/param.h>
#  if defined (PAGESIZE)
#     define getpagesize() PAGESIZE
#  else /* !PAGESIZE */
#    if defined (EXEC_PAGESIZE)
#      define getpagesize() EXEC_PAGESIZE
#    else /* !EXEC_PAGESIZE */
#      if defined (NBPG)
#        if !defined (CLSIZE)
#          define CLSIZE 1
#        endif /* !CLSIZE */
#        define getpagesize() (NBPG * CLSIZE)
#      else /* !NBPG */
#        if defined (NBPC)
#          define getpagesize() NBPC
#        endif /* NBPC */
#      endif /* !NBPG */
#    endif /* !EXEC_PAGESIZE */
#  endif /* !PAGESIZE */
#endif /* !getpagesize */

#if !defined (getpagesize)
#  define getpagesize() 4096  /* Just punt and use reasonable value */
#endif

#endif /* no EXEC_PAGESIZE */

#endif /* not HAVE_GETPAGESIZE */

main()
{
#if defined (HAVE_GETPAGESIZE) || defined (getpagesize)
  printf ("%ld\n", getpagesize ());
#else
  puts ("1024");
#endif
}
