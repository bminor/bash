/* mailcheck.c -- The check is in the mail... */

/* Copyright (C) 1987,1989 Free Software Foundation, Inc.

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

#include <stdio.h>
#include "bashtypes.h"
#include "posixstat.h"
#include <sys/param.h>
#include "bashansi.h"
#include "shell.h"
#include "maxpath.h"
#include "execute_cmd.h"
#include <tilde/tilde.h>

#ifndef NOW
#define NOW ((time_t)time ((time_t *)0))
#endif

typedef struct {
  char *name;
  time_t access_time;
  time_t mod_time;
  long file_size;
} FILEINFO;

/* The list of remembered mail files. */
FILEINFO **mailfiles = (FILEINFO **)NULL;

/* Number of mail files that we have. */
int mailfiles_count = 0;

/* The last known time that mail was checked. */
int last_time_mail_checked = 0;

/* Returns non-zero if it is time to check mail. */
int
time_to_check_mail ()
{
  char *temp = get_string_value ("MAILCHECK");
  time_t now = NOW;
  long seconds = -1L;

  /* Skip leading whitespace in MAILCHECK. */
  if (temp)
    {
      while (whitespace (*temp))
	temp++;

      seconds = atoi (temp);
    }

  /* Negative number, or non-numbers (such as empty string) cause no
     checking to take place. */
  if (seconds < 0)
    return (0);

  /* Time to check if MAILCHECK is explicitly set to zero, or if enough
     time has passed since the last check. */
  return (!seconds || ((now - last_time_mail_checked) >= seconds));
}

/* Okay, we have checked the mail.  Perhaps I should make this function
   go away. */
void
reset_mail_timer ()
{
  last_time_mail_checked = NOW;
}

/* Locate a file in the list.  Return index of
   entry, or -1 if not found. */
static int
find_mail_file (file)
     char *file;
{
  register int i;

  for (i = 0; i < mailfiles_count; i++)
    if (STREQ ((mailfiles[i])->name, file))
      return i;

  return -1;
}

/* Add this file to the list of remembered files and return its index
   in the list of mail files. */
static int
add_mail_file (file)
     char *file;
{
  struct stat finfo;
  char *filename;
  int i;

  filename = full_pathname (file);
  i = find_mail_file (file);
  if (i > -1)
    {
      if (stat (filename, &finfo) == 0)
	{
	  mailfiles[i]->mod_time = finfo.st_mtime;
	  mailfiles[i]->access_time = finfo.st_atime;
	  mailfiles[i]->file_size = (long)finfo.st_size;
	}
      free (filename);
      return i;
    }

  i = mailfiles_count++;
  mailfiles = (FILEINFO **)xrealloc
		(mailfiles, mailfiles_count * sizeof (FILEINFO *));

  mailfiles[i] = (FILEINFO *)xmalloc (sizeof (FILEINFO));
  mailfiles[i]->name = filename;
  if (stat (filename, &finfo) == 0)
    {
      mailfiles[i]->access_time = finfo.st_atime;
      mailfiles[i]->mod_time = finfo.st_mtime;
      mailfiles[i]->file_size = finfo.st_size;
    }
  else
    {
      mailfiles[i]->access_time = mailfiles[i]->mod_time = (time_t)-1;
      mailfiles[i]->file_size = -1L;
    }
  return i;
}

/* Reset the existing mail files access and modification times to zero. */
void
reset_mail_files ()
{
  register int i;

  for (i = 0; i < mailfiles_count; i++)
    {
      mailfiles[i]->access_time = mailfiles[i]->mod_time = 0;
      mailfiles[i]->file_size = 0L;
    }
}

/* Free the information that we have about the remembered mail files. */
void
free_mail_files ()
{
  register int i;

  for (i = 0; i < mailfiles_count; i++)
    {
      free (mailfiles[i]->name);
      free (mailfiles[i]);
    }

  if (mailfiles)
    free (mailfiles);

  mailfiles_count = 0;
  mailfiles = (FILEINFO **)NULL;
}

/* Return non-zero if FILE's mod date has changed and it has not been
   accessed since modified. */
static int
file_mod_date_changed (file)
     char *file;
{
  time_t time = (time_t)0;
  struct stat finfo;
  int i;

  i = find_mail_file (file);
  if (i != -1)
    time = mailfiles[i]->mod_time;

  if ((stat (file, &finfo) == 0) && (finfo.st_size > 0))
    return (time != finfo.st_mtime);

  return (0);
}

/* Return non-zero if FILE's access date has changed. */
static int
file_access_date_changed (file)
     char *file;
{
  time_t time = (time_t)0;
  struct stat finfo;
  int i;

  i = find_mail_file (file);
  if (i != -1)
    time = mailfiles[i]->access_time;

  if ((stat (file, &finfo) == 0) && (finfo.st_size > 0))
    return (time != finfo.st_atime);

  return (0);
}

/* Return non-zero if FILE's size has increased. */
static int
file_has_grown (file)
     char *file;
{
  long size = 0L;
  struct stat finfo;
  int i;

  i = find_mail_file (file);
  if (i != -1)
    size = mailfiles[i]->file_size;

  return ((stat (file, &finfo) == 0) && (finfo.st_size > size));
}

char *
make_default_mailpath ()
{
  char *mp;

  mp = xmalloc (1 + sizeof (DEFAULT_MAIL_PATH) + strlen (current_user.user_name));
  strcpy (mp, DEFAULT_MAIL_PATH);
  strcpy (mp + sizeof (DEFAULT_MAIL_PATH) - 1, current_user.user_name);
  return (mp);
}

/* Return the colon separated list of pathnames to check for mail. */
static char *
get_mailpaths ()
{
  char *mailpaths;

  mailpaths = get_string_value ("MAILPATH");

  if (!mailpaths)
    mailpaths = get_string_value ("MAIL");

  if (mailpaths)
    return (savestring (mailpaths));
    
  return (make_default_mailpath ());
}

/* Take an element from $MAILPATH and return the portion from
   the first unquoted `?' or `%' to the end of the string.  This is the
   message to be printed when the file contents change. */
static char *
parse_mailpath_spec (str)
     char *str;
{
  char *s;
  int pass_next;

  for (s = str, pass_next = 0; s && *s; s++)
    {
      if (pass_next)
	{
	  pass_next = 0;
	  continue;
	}
      if (*s == '\\')
	{
	  pass_next++;
	  continue;
	}
      if (*s == '?' || *s == '%')
        return s;
    }
  return ((char *)NULL);
}
      
/* Remember the dates of the files specified by MAILPATH, or if there is
   no MAILPATH, by the file specified in MAIL.  If neither exists, use a
   default value, which we randomly concoct from using Unix. */
void
remember_mail_dates ()
{
  char *mailpaths;
  char *mailfile, *mp;
  int i = 0;

  mailpaths = get_mailpaths ();  
  while (mailfile = extract_colon_unit (mailpaths, &i))
    {
      mp = parse_mailpath_spec (mailfile);
      if (mp && *mp)
	*mp = '\0';
      add_mail_file (mailfile);
      free (mailfile);
    }
  free (mailpaths);
}

/* check_mail () is useful for more than just checking mail.  Since it has
   the paranoids dream ability of telling you when someone has read your
   mail, it can just as easily be used to tell you when someones .profile
   file has been read, thus letting one know when someone else has logged
   in.  Pretty good, huh? */

/* Check for mail in some files.  If the modification date of any
   of the files in MAILPATH has changed since we last did a
   remember_mail_dates () then mention that the user has mail.
   Special hack:  If the shell variable MAIL_WARNING is on and the
   mail file has been accessed since the last time we remembered, then
   the message "The mail in <mailfile> has been read" is printed. */
void
check_mail ()
{
  char *current_mail_file, *you_have_mail_message;
  char *mailpaths, *mp;
  int file_index = 0;
  char *dollar_underscore;

  dollar_underscore = get_string_value ("_");

  if (dollar_underscore)
    dollar_underscore = savestring (dollar_underscore);

  mailpaths = get_mailpaths ();
  while ((current_mail_file = extract_colon_unit (mailpaths, &file_index)))
    {
      char *t;
      int use_user_notification;

      if (!*current_mail_file)
	{
	  free (current_mail_file);
	  continue;
	}

      t = full_pathname (current_mail_file);
      free (current_mail_file);
      current_mail_file = t;

      use_user_notification = 0;
      you_have_mail_message = "You have mail in $_";

      mp = parse_mailpath_spec (current_mail_file);
      if (mp && *mp)
	{
	  *mp = '\0';
	  you_have_mail_message = ++mp;
	  use_user_notification++;
	}

      if (file_mod_date_changed (current_mail_file))
	{
	  WORD_LIST *tlist;
	  int i, file_is_bigger;
	  bind_variable ("_", current_mail_file);
#define atime mailfiles[i]->access_time
#define mtime mailfiles[i]->mod_time

	  /* Have to compute this before the call to add_mail_file, which
	     resets all the information. */
	  file_is_bigger = file_has_grown (current_mail_file);

	  i = add_mail_file (current_mail_file);

	  if (i == -1)
	    continue;		/* if this returns -1 , it is a bug */

	  /* If the user has just run a program which manipulates the
	     mail file, then don't bother explaining that the mail
	     file has been manipulated.  Since some systems don't change
	     the access time to be equal to the modification time when
	     the mail in the file is manipulated, check the size also.  If
	     the file has not grown, continue. */
	  if ((atime >= mtime) && !file_is_bigger)
	    {
	      free (current_mail_file);
	      continue;
	    }

	  /* If the mod time is later than the access time and the file
	     has grown, note the fact that this is *new* mail. */
	  if (!use_user_notification && (atime < mtime) && file_is_bigger)
	    you_have_mail_message = "You have new mail in $_";
#undef atime
#undef mtime

	  if ((tlist = expand_string (you_have_mail_message, 1)))
	    {
	      char *tem = string_list (tlist);
	      printf ("%s\n", tem);
	      free (tem);
	      dispose_words (tlist);
	    }
	  else
	    printf ("\n");
	}

      if (find_variable ("MAIL_WARNING") &&
	    file_access_date_changed (current_mail_file))
	{
	  add_mail_file (current_mail_file);
	  printf ("The mail in %s has been read!\n", current_mail_file);
	}

      free (current_mail_file);
    }
  free (mailpaths);

  if (dollar_underscore)
    {
      bind_variable ("_", dollar_underscore);
      free (dollar_underscore);
    }
  else
    unbind_variable ("_");
}
