/* strscpy - null-terminated string copy with length checking. */

/* Copyright (C) 2023 Free Software Foundation, Inc.

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

#if defined (HAVE_UNISTD_H)
#  include <unistd.h>
#endif
#include <bashansi.h>

ssize_t
strscpy (char *d, const char *s, size_t len)
{
  size_t i;

  for (i = 0; i < len; i++)
    if ((d[i] = s[i]) == 0)
      return ((ssize_t)i);

  if (i != 0)
    d[--i] = '\0';

  return (-1);		/* strlen (s) > len */
}
