/* endian.c -- A trick for determining the byte order of a machine. */

/* Copyright (C) 1993,2023 Free Software Foundation, Inc.

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

#include "config.h"

#include <sys/types.h>
#include <stdio.h>
#include "bashansi.h"

/* The name of this program, as taken from argv[0]. */
char *progname;

/* The name of the source file that this code is made from. */
char source_name[256];

/* The name of the define.  Either "BIG_ENDIAN" or "LITTLE_ENDIAN". */
char *endian_define;

char string[9];
char nstring[9];

/* Stuff "1234" into a long, and compare it against a character string
   "1234".  If the results are EQ, the machine is big endian like a 68000
   or Sparc, otherwise it is little endian, like a Vax, or 386. */
int
main (int argc, char **argv)
{
  register size_t i;
  FILE *stream = (FILE *)NULL;
  char *stream_name = "stdout";
  union {
      unsigned long l;
      char s[sizeof (long)];
  } u;

  progname = argv[0];

  for (i = strlen (progname); i > 0; i--)
    if (progname[i] == '/')
      {
	progname = progname + i + 1;
	break;
      }

  strcpy (source_name, progname);
  for (i = strlen (source_name); i > 0; i--)
    if (source_name[i] == '.')
      {
	source_name[i] = '\0';
	break;
      }

  strcat (source_name, ".c");

  if (argc == 1)
    {
      stream_name = "stdout";
      stream = stdout;
    }
  else if (argc == 2)
    {
      stream_name = argv[1];
      stream = fopen (stream_name, "w");
    }
  else
    {
      fprintf (stderr, "Usage: %s [output-file]\n", progname);
      exit (1);
    }

  if (!stream)
    {
      fprintf (stderr, "%s: %s Cannot be opened or written to.\n",
	       progname, stream_name);
      exit (2);
    }

  if (sizeof (long int) == 4)
    {
      u.l = 0x04030201L;
      (void) strcpy (string, "4321");
    }
  else if (sizeof (long int) == 8)
    {
#if defined (__GNUC__)
      unsigned long fake_out_gcc;

      fake_out_gcc = (0x08070605L << 31);
      fake_out_gcc = (fake_out_gcc << 1);
      u.l = fake_out_gcc | 0x04030201L;
#else
      u.l = (0x08070605L << 32) | 0x04030201L;
#endif /* !__GNUC__ */
      (void) strcpy (string, "87654321");
    }
  else
    {
      fprintf (stderr,
	       "%s: sizeof (long int) = %d, which isn't handled here.\n",
	       progname, sizeof (long int));
      exit (2);
    }

  for (i = 0; i < sizeof (long); i++)
    nstring[i] = u.s[i] + '0';
  nstring[i] = '\0';

  if (strcmp (nstring, string) == 0)
    endian_define = "BIG_ENDIAN";
  else
    endian_define = "LITTLE_ENDIAN";

  fprintf (stream, "/* %s - Define BIG or LITTLE endian. */\n\n", stream_name);
  fprintf (stream,
"/* This file was automatically created by `%s'.  You shouldn't\n\
   edit this file, because your changes will be overwritten.  Instead,\n\
   edit the source code file `%s'. */\n\n",
	   progname, source_name);

  fprintf (stream, "#if !defined (%s)\n", endian_define);
  fprintf (stream, "#  define %s\n", endian_define);
  fprintf (stream, "#endif /* %s */\n", endian_define);

  if (stream != stdout)
    fclose (stream);

  exit (0);
}
