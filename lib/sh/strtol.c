/* strtol - Convert string representation of a number into an integer value.
   Copyright (C) 1997 Free Software Foundation, Inc.

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the
   Free Software Foundation; either version 2, or (at your option) any
   later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.  */

#include <config.h>

#if !defined (HAVE_STRTOL)

#include <ctype.h>
#include <errno.h>

#ifndef errno
extern int errno;
#endif

#ifndef __set_errno
#  define __set_errno(Val) errno = (Val)
#endif

#ifdef HAVE_LIMITS_H
#  include <limits.h>
#endif

#include <bashansi.h>

#ifndef NULL
#  define NULL 0
#endif

/* Nonzero if we are defining `strtoul', operating on unsigned integers.  */
#ifndef UNSIGNED
# define UNSIGNED 0
# define RETTYPE long
#else
# define RETTYPE unsigned long
#endif

/* Determine the name.  */
#if UNSIGNED
#  define strtol strtoul
#endif

#ifndef CHAR_BIT
#  define CHAR_BIT 8
#endif

#ifndef ULONG_MAX
#  define ULONG_MAX ((unsigned long) ~(unsigned long) 0)
#  define ULONG_MIN ((unsigned long) 0 - ULONG_MAX)
#endif

#ifndef LONG_MAX
#  define LONG_MAX ((long) (ULONG_MAX >> 1))
#  define LONG_MIN ((long) (0 - LONG_MAX))
#endif

/* Convert NPTR to an `unsigned long int' or `long int' in base BASE.
   If BASE is 0 the base is determined by the presence of a leading
   zero, indicating octal or a leading "0x" or "0X", indicating hexadecimal.
   If BASE is < 2 or > 36, it is reset to 10.
   If ENDPTR is not NULL, a pointer to the character after the last
   one converted is stored in *ENDPTR.  */

RETTYPE
strtol (nptr, endptr, base)
     const char *nptr;
     char **endptr;
     int base;
{
  int negative;
  register unsigned long cutoff, i;
  register unsigned int cutlim;
  register const char *s;
  register unsigned char c;
  const char *save, *end;
  int overflow;

  if (base < 0 || base == 1 || base > 36)
    base = 10;

  save = s = nptr;

  /* Skip white space.  */
  while (isspace (*s))
    ++s;
  if (*s == '\0')
    goto noconv;

  /* Check for a sign.  */
  if (*s == '-' || *s == '+')
    {
      negative = (*s == '-');
      ++s;
    }
  else
    negative = 0;

  if (base == 16 && *s == '0' && toupper (s[1]) == 'X')
    s += 2;

  /* If BASE is zero, figure it out ourselves.  */
  if (base == 0)
    if (*s == '0')
      {
	if (toupper (s[1]) == 'X')
	  {
	    s += 2;
	    base = 16;
	  }
	else
	  base = 8;
      }
    else
      base = 10;

  /* Save the pointer so we can check later if anything happened.  */
  save = s;

  end = NULL;

  cutoff = ULONG_MAX / (unsigned long int) base;
  cutlim = ULONG_MAX % (unsigned long int) base;

  overflow = 0;
  i = 0;
  for (c = *s; c != '\0'; c = *++s)
    {
      if (s == end)
	break;

      if (c >= '0' && c <= '9')
	c -= '0';
      else if (isalpha (c))
	c = toupper (c) - 'A' + 10;
      else
	break;

      if ((int) c >= base)
	break;

      /* Check for overflow.  */
      if (i > cutoff || (i == cutoff && c > cutlim))
	overflow = 1;
      else
	{
	  i *= (unsigned long int) base;
	  i += c;
	}
    }

  /* Check if anything actually happened.  */
  if (s == save)
    goto noconv;

  /* Store in ENDPTR the address of one character
     past the last character we converted.  */
  if (endptr != NULL)
    *endptr = (char *) s;

#if !UNSIGNED
  /* Check for a value that is within the range of
     `unsigned long int', but outside the range of `long int'.  */
  if (overflow == 0
      && i > (negative
	      ? -((unsigned long) (LONG_MIN + 1)) + 1
	      : (unsigned long) LONG_MAX))
    overflow = 1;
#endif

  if (overflow)
    {
      __set_errno (ERANGE);
#if UNSIGNED
      return ULONG_MAX;
#else
      return negative ? LONG_MIN : LONG_MAX;
#endif
    }

  /* Return the result with the appropriate sign.  */
  return (negative ? -i : i);

noconv:
  /* We must handle a special case here: the base is 0 or 16 and the
     first two characters are '0' and 'x', but the rest are no
     hexadecimal digits.  This is no error case.  We return 0 and
     ENDPTR points to the `x'.  */
  if (endptr != NULL)
    {
      if (save - nptr >= 2 && toupper (save[-1]) == 'X' && save[-2] == '0')
	*endptr = (char *) &save[-1];
      else
        /*  There was no number to convert.  */
        *endptr = (char *) nptr;
    }

  return 0L;
}

#endif /* !HAVE_STRTOL */
