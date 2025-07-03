/* strptime - take a date-time string and turn it into seconds since the epoch. */

/* See Makefile for compilation details. */

/*
   Copyright (C) 2023-2025 Free Software Foundation, Inc.

   This file is part of GNU Bash.
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

#include "bashtypes.h"
#include "posixtime.h"

#include <stdio.h>

#include "builtins.h"
#include "shell.h"
#include "bashgetopt.h"
#include "common.h"

struct date_modifier
{
  char *shorthand;
  int incr;
};

static struct date_modifier date_time_modifiers[] =
{
  { "now",	0 },
  { "today",	0 },
  { "tomorrow",	24*60*60 },
  { "yesterday", -24*60*60 },
  { "day after tomorrow", 48*60*60 },
  { "two days ago", -48*60*60 },
  { "next week", 7*24*60*60 },
  { "last week", -7*24*60*60 },
  { "the day after tomorrow", 48*60*60 },
  { 0, 0 }
};

static char * const date_time_formats[] =
{
  "%a %b %d %T %Z %Y",		/* Unix date */
  "%a %b %d %T %Y",		/* Wkd Mon DD HH:MM:SS YYYY */
  "%FT%T%z",			/* ISO8601 time YYYY-mm-ddTHH:MM:SSzone */
  "%FT%R%z",			/* ISO8601 time YYYY-mm-ddTHH:MMzone */
  "%G-%m-%dT%T%z",		/* ISO8601 time YYYY-mm-ddTHH:MM:SSzone */
  "%G-%m-%dT%R%z",		/* ISO8601 time YYYY-mm-ddTHH:MMzone */
  "%G-%m-%d",			/* ISO8601 time YYYY-mm-dd */
  /* Can't do 8601 time zone offset with colon or fractions of a second */
  "%a, %d %b %Y %T %Z",		/* RFC822/RFC2822 time */
  "%a, %d %b %Y %T %z",		/* RFC822/RFC2822 time */
  "%D %T",			/* mm/dd/yy HH:MM:SS */
  "%D %R",			/* mm/dd/yy HH:MM */
  "%D %r",			/* mm/dd/yy HH:MM:SS a.m.  */
  "%D %I:%M %p",		/* mm/dd/yy HH:MM p.m.  */
  "%m/%d/%Y %T",		/* mm/dd/YYYY HH:MM:SS */
  "%m/%d/%Y %R",		/* mm/dd/YYYY HH:MM */
  "%m/%d/%Y %r",		/* mm/dd/YYYY HH:MM:SS a.m */
  "%m/%d/%Y %I:%M %p",		/* mm/dd/YYYY HH:MM p.m. */
  "%m-%d-%Y %T",		/* mm-dd-YYYY HH:MM:SS */
  "%m-%d-%Y %R",		/* mm-dd-YYYY HH:MM */
  "%m-%d-%Y %r",		/* mm-dd-YYYY HH:MM:SS a.m. */
  "%m-%d-%Y %I:%M %p",		/* mm-dd-YYYY HH:MM p.m. */
  "%Y/%m/%d %T",		/* YYYY/mm/dd HH:MM:SS */
  "%Y/%m/%d %R",		/* YYYY/mm/dd HH:MM */
  "%Y/%m/%d %r",		/* YYYY/mm/dd hh:MM:SS a.m. */
  "%F %T",			/* YYYY-mm-dd HH:MM:SS */
  "%F %r",			/* YYYY-mm-dd HH:MM:SS p.m. */
  "%F %R",			/* YYYY-mm-dd HH:MM */
  "%F %I:%M %p",		/* YYYY-mm-dd HH:MM a.m. */
  /* From coreutils-9.2 date */
  "%Y-%m-%dT%H:%M:%S%z",	/* ISO8601 time */
  "%Y-%m-%dT%H%z",		/* ISO8601 time */
  "%Y-%m-%dT%H:%M%z",		/* ISO8601 time */
  "%Y-%m-%dT%H:%M:%S%Z",	/* ISO8601 time but with timezone name */
  "%Y-%m-%dT%H%Z",		/* ISO8601 time but with timezone name */
  "%Y-%m-%dT%H:%M%Z",		/* ISO8601 time but with timezone name */
  /* RFC 3339 time */
  "%Y-%m-%d %H:%M:%S%z",	/* RFC 3339 time */
  "%Y-%m-%d %H:%M:%S%Z",	/* RFC 3339 time but with timezone name */
#if 0
  "%Y-%m-%dT%H:%M:%S%z",	/* RFC 3339 time, same as first ISO8601 time */
#endif
  /* more oddball formats */
  "%m.%d.%Y %T",		/* mm.dd.YYYY HH:MM:SS */
  "%m.%d.%Y %R",		/* mm.dd.YYYY HH:MM */
  "%m.%d.%Y %r",		/* mm.dd.YYYY HH:MM:SS a.m. */
  "%m.%d.%Y %I:%M %p",		/* mm.dd.YYYY HH:MM p.m. */
  "%m/%d/%Y",			/* mm/dd/YYYY */
  "%d %B %Y %T",		/* dd Month YYYY HH:MM:SS */
  "%d %B %Y %R",		/* dd Month YYYY HH:MM */
  "%d %B %Y %r",		/* dd Month YYYY HH:MM:SS a.m. */
  "%d %B %Y %I:%M %p",		/* dd Month YYYY HH:MM p.m. */
  "%d %b %Y %T",		/* dd Mon YYYY HH:MM:SS */
  "%d %b %Y %R",		/* dd Mon YYYY HH:MM */
  "%d %b %Y %r",		/* dd Mon YYYY HH:MM:SS a.m. */
  "%d %b %Y %I:%M %p",		/* dd Mon YYYY HH:MM p.m. */
  "%b %d, %Y %T",		/* Mon dd, YYYY HH:MM:SS */
  "%b %d, %Y %R",		/* Mon dd, YYYY HH:MM */
  "%b %d, %Y %r",		/* Mon dd, YYYY HH:MM:SS a.m. */
  "%b %d, %Y %I:%M %p",		/* Mon dd, YYYY HH:MM p.m. */
  "%m-%b-%Y",			/* dd-Mon-YYYY */
  "%m-%b-%Y %T",		/* dd-Mon-YYYY HH:MM:SS */
  "%m-%b-%Y %R",		/* dd-Mon-YYYY HH:MM */
  "%m-%b-%Y %r",		/* dd-Mon-YYYY HH:MM:SS a.m. */
  "%m-%b-%Y %I:%M %p",		/* dd-Mon-YYYY HH:MM p.m. */
  "%d/%b/%Y:%T %z",		/* NCSA log format dd/Mon/YYYY:HH:MM:SS zone */
  "%d/%b/%Y:%T%z",		/* NCSA log format dd/Mon/YYYY:HH:MM:SSzone */
  /* No delimiters */
  "%Y%m%d %T",			/* YYYYMMDD HH:MM:SS */
  "%Y%m%d %R",			/* YYYYMMDD HH:MM */
  "%Y%m%d %r",			/* YYYYMMDD HH:MM:SS a.m. */
  "%Y%m%d %I:%M %p",		/* YYYYMMDD HH:MM p.m. */
  "%Y%m%d %H:%M:%S%z",		/* YYYYMMDD HH:MM:SSzone */
  "%Y%m%dT%H:%M:%S%z",		/* YYYYMMDDTHH:MM:SSzone */
  "%Y%m%dT%T",			/* YYYYMMDDTHH:MM:SS */
  "%Y%m%dT%R",			/* YYYYMMDDTHH:MM */
  /* Non-US formats */
  "%d-%m-%Y",			/* dd-mm-YYYY */
  "%d-%m-%Y %T",		/* dd-mm-YYYY HH:MM:SS */
  "%d-%m-%Y %R",		/* dd-mm-YYYY HH:MM */
  "%d-%m-%Y %r",		/* dd-mm-YYYY HH:MM:SS a.m. */    
  "%d-%m-%Y %I:%M %p",		/* dd-mm-YYYY HH:MM p.m. */
  "%d/%m/%Y %T",		/* dd/mm/YYYY HH:MM:SS */
  "%d/%m/%Y %R",		/* dd/mm/YYYY HH:MM */
  "%d/%m/%Y %r",		/* dd/mm/YYYY HH:MM:SS a.m. */
  "%d/%m/%Y %I:%M %p",		/* dd/mm/YYYY HH:MM p.m. */
  "%Y-%d-%m %T",		/* YYYY-dd-mm HH:MM:SS */
  "%Y-%d-%m %R",		/* YYYY-dd-mm HH:MM */
  "%d-%m-%Y %T",		/* dd-mm-YYYY HH:MM:SS */
  "%d-%m-%Y %R",		/* dd-mm-YYYY HH:MM */
  "%d-%m-%Y %r",		/* dd-mm-YYYY HH:MM:SS a.m. */
  "%d-%m-%Y %I:%M %p",		/* dd-mm-YYYY HH:MM p.m. */
  "%d.%m.%Y %T",		/* dd.mm.YYYY HH:MM:SS */
  "%d.%m.%Y %R",		/* dd.mm.YYYY HH:MM */
  "%d.%m.%Y %r",		/* dd.mm.YYYY HH:MM:SS a.m. */    
  "%d.%m.%Y %I:%M %p",		/* dd.mm.YYYY HH:MM p.m. */
  /* Some fallbacks */
  "%F",				/* YYYY-mm-dd ISO8601 time */
  "%T",				/* HH:MM:SS */
  "%H.%M.%S",			/* HH.MM.SS */
  0
};

static void
inittime (time_t *clock, struct tm *timeptr)
{
  struct tm *loctime;

  /* Initialize to local time */
  loctime = localtime (clock);

  if (loctime == 0)
    {
      timeptr->tm_hour = timeptr->tm_min = timeptr->tm_sec = 0;
      return;
    }

  memcpy (timeptr, loctime, sizeof (struct tm));

  /* but default to midnight */
  timeptr->tm_hour = timeptr->tm_min = timeptr->tm_sec = 0;
  /* and let the system figure out the right DST offset */
  timeptr->tm_isdst = -1;
}

int
strptime_builtin (WORD_LIST *list)
{
  char *s;
  struct tm t, *tm;
  time_t now, secs;
  char *datestr, *format;
  int i, opt;

  format = NULL;
  reset_internal_getopt ();
  while ((opt = internal_getopt (list, "f:")) != -1)
    {
      switch (opt)
        {
          case 'f':
	    format = list_optarg;
	    break;
	  CASE_HELPOPT;
	  default:
	    builtin_usage ();
	    return (EX_USAGE);
        }
    }

  list = loptend;

  if (list == 0)
    {
      builtin_usage ();
      return (EX_USAGE);
    }

  datestr = string_list (list);
  if (datestr == 0 || *datestr == 0)
    return (EXECUTION_SUCCESS);

  now = getnow ();
  secs = -1;
  for (i = 0; date_time_modifiers[i].shorthand; i++)
    {
      if (STREQ (datestr, date_time_modifiers[i].shorthand))
	{
	  secs = now + date_time_modifiers[i].incr;
	  printf ("%ld\n", secs);    
	  return (EXECUTION_SUCCESS);
	}
    }

  /* init struct tm */
  inittime (&now, &t);
  if (format)
    {
      s = strptime (datestr, format, &t);
      if (s == 0 || s == datestr)
	{
	  builtin_error ("%s: unrecognized format", datestr);
	  return (EXECUTION_FAILURE);
	}
    }
  else
    {
      for (i = 0; date_time_formats[i]; i++)
	{
	  s = strptime (datestr, date_time_formats[i], &t);
	  if (s == 0 || s == datestr)
	    continue;
	  break;
        }
      if (date_time_formats[i] == 0)
	{
	  builtin_error ("%s: unrecognized format", datestr);
	  return (EXECUTION_FAILURE);

	}
    }

  /* Found something. */
  secs = mktime (&t);
  if (s && *s)
    builtin_warning("%s: not completely converted (%s)", datestr, s);

  printf ("%ld\n", secs);    
  return (EXECUTION_SUCCESS);
}

char *strptime_doc[] = {
	"Convert a date-time string to seconds since the epoch.",
	"",
	"Take DATE-TIME, a date-time string, and parse it using FORMAT, a",
	"date and time format accepted by strptime(3). If FORMAT is not supplied,",
	"attempt to parse DATE-TIME against a set of common date-time formats,",
	"not all of which may be acceptable to strptime(3).",
	"If the string matches one of the formats, convert it into seconds",
	"since the epoch and display the result.",
	(char *)NULL
};

/* The standard structure describing a builtin command.  bash keeps an array
   of these structures.  The flags must include BUILTIN_ENABLED so the
   builtin can be used. */
struct builtin strptime_struct = {
	"strptime",		/* builtin name */
	strptime_builtin,		/* function implementing the builtin */
	BUILTIN_ENABLED,	/* initial flags for builtin */
	strptime_doc,		/* array of long documentation strings. */
	"strptime [-f format] date-time",	/* usage synopsis; becomes short_doc */
	0			/* reserved for internal use */
};
