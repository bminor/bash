/* signames.c -- Create and write `signames.h', which contains an array of
   signal names. */

/* Copyright (C) 1992 Free Software Foundation, Inc.

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
#include <sys/types.h>
#include <signal.h>
#if defined (HAVE_STDLIB_H)
#  include <stdlib.h>
#else
#  include "ansi_stdlib.h"
#endif /* HAVE_STDLIB_H */

#if !defined (NSIG)
#  define NSIG 64
#endif

char *signal_names[2 * NSIG];

char *progname;

initialize_signames ()
{
  register int i;

  for (i = 1; i < sizeof(signal_names)/sizeof(signal_names[0]); i++)
    signal_names[i] = (char *)NULL;

  /* `signal' 0 is what we do on exit. */
  signal_names[0] = "EXIT";

  /* Place signal names which can be aliases for more common signal
     names first.  This allows (for example) SIGEMT to overwrite SIGGRANT. */
#if defined (SIGGRANT)	/* HFT monitor mode granted */
  signal_names[SIGGRANT] = "SIGGRANT";
#endif

#if defined (SIGRETRACT) /* HFT monitor mode retracted */
  signal_names[SIGRETRACT] = "SIGRETRACT";
#endif

#if defined (SIGHUP)	/* hangup */
  signal_names[SIGHUP] = "SIGHUP";
#endif

#if defined (SIGINT)	/* interrupt */
  signal_names[SIGINT] = "SIGINT";
#endif

#if defined (SIGQUIT)	/* quit */
  signal_names[SIGQUIT] = "SIGQUIT";
#endif

#if defined (SIGILL)	/* illegal instruction (not reset when caught) */
  signal_names[SIGILL] = "SIGILL";
#endif

#if defined (SIGTRAP)	/* trace trap (not reset when caught) */
  signal_names[SIGTRAP] = "SIGTRAP";
#endif

#if defined (SIGABRT)	/* Cause current process to dump core. */
  signal_names[SIGABRT] = "SIGABRT";
#endif

#if defined (SIGIOT)	/* IOT instruction */
  signal_names[SIGIOT] = "SIGIOT";
#endif

#if defined (SIGEMT)	/* EMT instruction */
  signal_names[SIGEMT] = "SIGEMT";
#endif

#if defined (SIGFPE)	/* floating point exception */
  signal_names[SIGFPE] = "SIGFPE";
#endif

#if defined (SIGKILL)	/* kill (cannot be caught or ignored) */
  signal_names[SIGKILL] = "SIGKILL";
#endif

#if defined (SIGBUS)	/* bus error */
  signal_names[SIGBUS] = "SIGBUS";
#endif

#if defined (SIGSEGV)	/* segmentation violation */
  signal_names[SIGSEGV] = "SIGSEGV";
#endif

#if defined (SIGSYS)	/* bad argument to system call */
  signal_names[SIGSYS] = "SIGSYS";
#endif

#if defined (SIGPIPE)	/* write on a pipe with no one to read it */
  signal_names[SIGPIPE] = "SIGPIPE";
#endif

#if defined (SIGALRM)	/* alarm clock */
  signal_names[SIGALRM] = "SIGALRM";
#endif

#if defined (SIGTERM)	/* software termination signal from kill */
  signal_names[SIGTERM] = "SIGTERM";
#endif

#if defined (SIGCLD)	/* Like SIGCHLD.  */
  signal_names[SIGCLD] = "SIGCLD";
#endif

#if defined (SIGPWR)	/* Magic thing for some machines. */
  signal_names[SIGPWR] = "SIGPWR";
#endif

#if defined (SIGPOLL)	/* For keyboard input?  */
  signal_names[SIGPOLL] = "SIGPOLL";
#endif

#if defined (SIGURG)	/* urgent condition on IO channel */
  signal_names[SIGURG] = "SIGURG";
#endif

#if defined (SIGSTOP)	/* sendable stop signal not from tty */
  signal_names[SIGSTOP] = "SIGSTOP";
#endif

#if defined (SIGTSTP)	/* stop signal from tty */
  signal_names[SIGTSTP] = "SIGTSTP";
#endif

#if defined (SIGCONT)	/* continue a stopped process */
  signal_names[SIGCONT] = "SIGCONT";
#endif

#if defined (SIGCHLD)	/* to parent on child stop or exit */
  signal_names[SIGCHLD] = "SIGCHLD";
#endif

#if defined (SIGTTIN)	/* to readers pgrp upon background tty read */
  signal_names[SIGTTIN] = "SIGTTIN";
#endif

#if defined (SIGTTOU)	/* like TTIN for output if (tp->t_local&LTOSTOP) */
  signal_names[SIGTTOU] = "SIGTTOU";
#endif

#if defined (SIGIO)	/* input/output possible signal */
  signal_names[SIGIO] = "SIGIO";
#endif

#if defined (SIGXCPU)	/* exceeded CPU time limit */
  signal_names[SIGXCPU] = "SIGXCPU";
#endif

#if defined (SIGXFSZ)	/* exceeded file size limit */
  signal_names[SIGXFSZ] = "SIGXFSZ";
#endif

#if defined (SIGVTALRM)	/* virtual time alarm */
  signal_names[SIGVTALRM] = "SIGVTALRM";
#endif

#if defined (SIGPROF)	/* profiling time alarm */
  signal_names[SIGPROF] = "SIGPROF";
#endif

#if defined (SIGWINCH)	/* window changed */
  signal_names[SIGWINCH] = "SIGWINCH";
#endif

#if defined (SIGLOST)	/* resource lost (eg, record-lock lost) */
  signal_names[SIGLOST] = "SIGLOST";
#endif

#if defined (SIGUSR1)	/* user defined signal 1 */
  signal_names[SIGUSR1] = "SIGUSR1";
#endif

#if defined (SIGUSR2)	/* user defined signal 2 */
  signal_names[SIGUSR2] = "SIGUSR2";
#endif

#if defined (SIGMSG)	/* HFT input data pending */
  signal_names[SIGMSG] = "SIGMSG";
#endif

#if defined (SIGPWR)	/* power failure imminent (save your data) */
  signal_names[SIGPWR] = "SIGPWR";
#endif

#if defined (SIGDANGER)	/* system crash imminent */
  signal_names[SIGDANGER] = "SIGDANGER";
#endif

#if defined (SIGMIGRATE) /* migrate process to another CPU */
  signal_names[SIGMIGRATE] = "SIGMIGRATE";
#endif

#if defined (SIGPRE)	/* programming error */
  signal_names[SIGPRE] = "SIGPRE";
#endif

#if defined (SIGSOUND)	/* HFT sound sequence has completed */
  signal_names[SIGSOUND] = "SIGSOUND";
#endif

#if defined (SIGWINDOW)
  signal_names[SIGWINDOW] = "SIGWINDOW";
#endif

#if defined (SIGDIL)
  signal_names[SIGDIL] = "SIGDIL";
#endif

#if defined (SIGSAK)	/* Secure Attention Key */
  signal_names[SIGSAK] = "SIGSAK";
#endif

  for (i = 0; i < NSIG; i++)
    if (signal_names[i] == (char *)NULL)
      {
	signal_names[i] = (char *)malloc (18);
	sprintf (signal_names[i], "SIGJUNK(%d)", i);
      }
}

write_signames (stream)
     FILE *stream;
{
  register int i;

  fprintf (stream, "/* This file was automatically created by %s.\n",
	   progname);
  fprintf (stream, "   Do not edit.  Edit signames.c instead. */\n\n");
  fprintf (stream,
	   "/* A translation list so we can be polite to our users. */\n");
  fprintf (stream, "char *signal_names[NSIG + 2] = {\n");

  for (i = 0; i < NSIG; i++)
    fprintf (stream, "    \"%s\",\n", signal_names[i]);

  fprintf (stream, "    (char *)0x0,\n");
  fprintf (stream, "};\n");
}

main (argc, argv)
     int argc;
     char **argv;
{
  char *stream_name;
  FILE *stream;

  progname = argv[0];

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

  initialize_signames ();
  write_signames (stream);
  exit (0);
}
