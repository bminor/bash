/* mbsncmp - multibyte string comparison. */

/* Copyright (C) 1995-2023 Free Software Foundation, Inc.

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

#include <config.h>

#if !defined (HAVE_MBSNCMP) && defined (HANDLE_MULTIBYTE)

#include <stdlib.h>
#include <stddef.h>
#include <string.h>

#include <wchar.h>

extern int locale_utf8locale;

/* Compare MBS1 and MBS2 up to N multibyte characters. */
int
mbsncmp (const char *mbs1, const char *mbs2, size_t n)
{
  int len1, len2, mb_cur_max;
  wchar_t c1, c2;
  mbstate_t state1 = { 0 }, state2 = { 0 };

  len1 = len2 = 0;
  mb_cur_max = MB_CUR_MAX;

  if (n == 0)
    return 0;

  do
    {
      len1 = mbrtowc (&c1, mbs1, mb_cur_max, &state1);
      len2 = mbrtowc (&c2, mbs2, mb_cur_max, &state2);

      if (len1 == 0)
	return len2 == 0 ? 0 : -1;
      else if (len2 == 0)
	return 1;
      else if (len1 > 0 && len2 < 0)
        return -1;
      else if (len1 < 0 && len2 > 0)
        return 1;
      else if (len1 < 0 && len2 < 0)
	{
	  len1 = strlen (mbs1);
	  len2 = strlen (mbs2);
	  return (len1 == len2 ? memcmp (mbs1, mbs2, len1)
			       : ((len1 < len2) ? (memcmp (mbs1, mbs2, len1) > 0 ? 1 : -1)
						: (memcmp (mbs1, mbs2, len2) >= 0 ? 1 : -1)));
	}

      mbs1 += len1;
      mbs2 += len2;
      n--;
      if (c1 != c2)
        break;
    }
  while (n > 0);

  return c1 - c2;
}

#endif
