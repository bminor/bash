/*
 * rename - rename a file
 */

#include <config.h>

#if !defined (HAVE_RENAME)

#include <bashtypes.h>

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif

#include <stdc.h>

int
rename (from, to)
     const char *from, *to;
{
  unlink (to);
  if (link (from, to) < 0)
    return (-1);
  unlink (from);
  return (0);
}
#endif /* !HAVE_RENAME */
