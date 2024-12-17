/* strlcpy - null-terminated string copy with length checking. */

/* Copyright (C) 2023-2024 Free Software Foundation, Inc.

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

#include <bashansi.h>

size_t
strlcpy(char *dest, const char *src, size_t size)
{
  size_t ret;

  ret = strlen(src);
  if (size)
    {
      size_t len;
      len = (ret >= size) ? size - 1 : ret;
      memcpy (dest, src, len);
      dest[len] = '\0';
    }
  return ret;
}
