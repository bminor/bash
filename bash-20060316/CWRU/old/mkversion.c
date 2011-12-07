/* Simple program to make new version numbers for the shell.
   Big deal, but it was getting out of hand to do everything
   in the makefile. */

/* Copyright (C) 1989 Free Software Foundation, Inc.

This file is part of GNU Bash, the Bourne Again SHell.

Bash is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License as published by the Free
Software Foundation; either version 1, or (at your option) any later
version.

Bash is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License along
with Bash; see the file COPYING.  If not, write to the Free Software
Foundation, 675 Mass Ave, Cambridge, MA 02139, USA. */

#include "config.h"

#include <sys/types.h>
#include "posixstat.h"
#include <stdio.h>
#include "bashansi.h"

char *progname;
char *dir;
char *status;

FILE *must_open ();

main (argc, argv)
     int argc;
     char **argv;
{
  FILE *file;
  float distver = 0.0;
  int buildver = 0, patchlevel = 0;
  int dist = 0, build = 0, patch = 0;
  int dist_inc = 0, build_inc = 0, patch_inc = 0;
  int dot_dist_needs_making = 0;
  int arg_index = 1;
  struct stat sb;

  progname = argv[0];

  status = dir = (char *)0;
  while (arg_index < argc && argv[arg_index][0] == '-')
    {
      if (strcmp (argv[arg_index], "-dist") == 0)
        {
	  dist++;
	  dist_inc++;
        }
      else if (strcmp (argv[arg_index], "-build") == 0)
        {
	  build++;
	  build_inc++;
        }
      else if (strcmp (argv[arg_index], "-patch") == 0)
        {
          patch++;
	  patch_inc++;
        }
      else if (strcmp (argv[arg_index], "-dir") == 0)
	{
	  dir = argv[++arg_index];
	  if (dir == 0)
	    {
	      fprintf (stderr, "%s: `-dir' requires an argument\n", progname);
	      exit (1);
	    }
	  if (stat (dir, &sb) < 0)
	    {
	      fprintf (stderr, "%s: cannot stat %s\n", progname, dir);
	      exit (1);
	    }
	  if ((sb.st_mode & S_IFMT) != S_IFDIR)
	    {
	      fprintf (stderr, "%s: not a directory\n", progname);
	      exit (1);
	    }
	}
      else if (strcmp (argv[arg_index], "-status") == 0)
        {
          status = argv[++arg_index];
	  if (status == 0)
	    {
	      fprintf (stderr, "%s: `-status' requires an argument\n", progname);
	      exit (1);
	    }
        }
      else
	{
	  fprintf (stderr, "%s: unknown option: %s\n", progname, argv[arg_index]);
	  fprintf (stderr, "usage: %s [-dist|-patch|-build] [-dir directory]\n", progname);
	  exit (1);
	}
      arg_index++;
    }

  if (get_float_from_file (".distribution", &distver, 1) == 0)
    dot_dist_needs_making++;

  if (get_int_from_file (".patchlevel", &patchlevel, 1) == 0)
    {
      patchlevel = 0;
      patch_inc = 0;
    }

  if (get_int_from_file (".build", &buildver, 0) == 0)
    buildver = 0;

  /* Setting distribution version. */
  if (dist && arg_index < argc)
    if (sscanf (argv[arg_index], "%f", &distver) != 1)
      {
	fprintf (stderr, "%s: Bad input `%s'.  Expected float value for -dist.\n",
		 progname, argv[arg_index]);
	exit (1);
      }
    else
      {
	arg_index++;
	dist_inc = 0;
      }

  /* Setting patchlevel via argument. */
  if (patch && arg_index < argc)
    if (sscanf (argv[arg_index], "%d", &patchlevel) != 1)
      {
	fprintf (stderr, "%s: Bad input `%s'.  Expected int value for -patch.\n",
		 progname, argv[arg_index]);
	exit (1);
      }
    else
      {
	arg_index++;
	patch_inc = 0;
      }
    
  if (build && arg_index < argc)
    if (sscanf (argv[arg_index], "%d", &buildver) != 1)
      {
	fprintf (stderr, "%s: Bad input `%s'.  Expected int value for -build.\n",
		 progname, argv[arg_index]);
	exit (1);
      }
    else
      {
	arg_index++;
	build_inc = 0;
      }

  if (dot_dist_needs_making && !distver)
    {
      fprintf (stderr, "%s: There is no `.distribution' file to infer from.\n", progname);
      exit (1);
    }

  if (dist_inc)
    distver = distver + 0.01;

  if (patch_inc)
    patchlevel++;

  if (build_inc)
    buildver++;

  file = must_open ("newversion.h", "w");

  /* Output the leading comment. */
  fprintf (file, 
"/* Version control for the shell.  This file gets changed when you say\n\
   `make newversion' to the Makefile.  It is created by mkversion. */\n");

  fprintf (file, "\n/* The distribution version number of this shell. */\n");
  fprintf (file, "#define DISTVERSION \"%.2f\"\n", distver);

  fprintf (file, "\n/* The patch level of this version of the shell. */\n");
  fprintf (file, "#define PATCHLEVEL %d\n", patchlevel);

  fprintf (file, "\n/* The last built version of this shell. */\n");
  fprintf (file, "#define BUILDVERSION %d\n", buildver);

  if (status)
    {
      fprintf (file, "\n/* The release status of this shell. */\n");
      fprintf (file, "#define RELSTATUS \"%s\"\n", status);
    }

  fprintf (file, "\n/* A version string for use by sccs and the what command. */\n\n");
  if (status)
    fprintf (file, "#define SCCSVERSION \"@(#)Bash version %.2f.%d(%d) %s GNU\"\n\n",
      distver, patchlevel, buildver, status);
  else
    fprintf (file, "#define SCCSVERSION \"@(#)Bash version %.2f.%d(%d) GNU\"\n\n",
      distver, patchlevel, buildver);

  fclose (file);

  file = must_open (".build", "w");
  fprintf (file, "%d\n", buildver);
  fclose (file);

  /* Making a new distribution. */
  if (dist)
    {
      file = must_open (".distribution", "w");
      fprintf (file, "%.2f\n", distver);
      fclose (file);
    }

  /* Releasing a new patch level. */
  if (patch)
    {
      file = must_open (".patchlevel", "w");
      fprintf (file, "%d\n", patchlevel);
      fclose (file);
    }

  exit (0);
}

char *
makename (fn, from_srcdir)
     char *fn;
{
  char *ret;
  int dlen;

  dlen = (from_srcdir && dir) ? strlen (dir) + 1 : 0;
  ret = (char *)malloc (dlen + strlen (fn) + 1);
  if (ret == 0)
    {
      fprintf (stderr, "%s: malloc failed\n", progname);
      exit (1);
    }
  if (from_srcdir && dir)
    sprintf (ret, "%s/%s", dir, fn);
  else
    (void)strcpy (ret, fn);

  return ret;
}

get_float_from_file (filename, var, from_srcdir)
     char *filename;
     float *var;
     int from_srcdir;
{
  FILE *stream;
  int result;
  char *name;

  name = makename (filename, from_srcdir);
  stream = fopen (name, "r");
  free (name);
  if (stream == (FILE *)NULL)
    return (0);
  result = fscanf (stream, "%f\n", var);
  fclose (stream);
  return (result == 1);
}

get_int_from_file (filename, var, from_srcdir)
     char *filename;
     int *var, from_srcdir;
{
  FILE *stream;
  int result;
  char *name;

  name = makename (filename, from_srcdir);
  stream = fopen (name, "r");
  free (name);
  if (stream == (FILE *)NULL)
    return (0);
  result = fscanf (stream, "%d\n", var);
  fclose (stream);
  return (result == 1);
}

FILE *
must_open (name, mode)
     char *name, *mode;
{
  FILE *temp = fopen (name, mode);

  if (!temp)
    {
      fprintf (stderr, "%s: Cannot open `%s' for mode `%s'.\n",
	       progname, name, mode);
      fprintf
	(stderr,
	 "Perhaps you don't have %s permission to the file or directory.\n",
	 (strcmp (mode, "w") == 0) ? "write" : "read");
      exit (3);
    }
  return (temp);
}
