/* This file is used when cross-compiling for the CYGWIN32 environment on
   a Unix machine.  It gets copied to signames.h in the build directory. */
#include <sys/types.h>
#include <signal.h>

#ifndef __GNUC__
# error cross compiling requires gcc
#endif

/* A translation list so we can be polite to our users.  Use gcc
   labelled initializers to set up the array.  Note that some entries
   might wind up being NULL.  */

char *signal_names[NSIG + 3] = {
  [0]		"EXIT",

#ifdef SIGLOST
  [SIGLOST]	"SIGLOST",
#endif

#ifdef SIGMSG
  [SIGMSG]	"SIGMSG",
#endif

#ifdef SIGDANGER
  [SIGDANGER]	"SIGDANGER",
#endif

#ifdef SIGMIGRATE
  [SIGMIGRATE]	"SIGMIGRATE",
#endif

#ifdef SIGPRE
  [SIGPRE]	"SIGPRE",
#endif

#ifdef SIGVIRT
  [SIGVIRT]	"SIGVIRT",
#endif

#ifdef SIGALRM1
  [SIGALRM1]	"SIGALRM1",
#endif

#ifdef SIGWAITING
  [SIGWAITING]	"SIGWAITING",
#endif

#ifdef SIGGRANT
  [SIGGRANT]	"SIGGRANT",
#endif

#ifdef SIGKAP
  [SIGKAP]	"SIGKAP",
#endif

#ifdef SIGRETRACT
  [SIGRETRACT]	"SIGRETRACT",
#endif

#ifdef SIGSOUND
  [SIGSOUND]	"SIGSOUND",
#endif

#ifdef SIGSAK
  [SIGSAK]	"SIGSAK",
#endif

#ifdef SIGLWP
  [SIGLWP]	"SIGLWP",
#endif

#ifdef SIGFREEZE
  [SIGFREEZE]	"SIGFREEZE",
#endif

#ifdef SIGTHAW
  [SIGTHAW]	"SIGTHAW",
#endif

#ifdef SIGCANCEL
  [SIGCANCEL]	"SIGCANCEL",
#endif

#ifdef SIGDIL
  [SIGDIL]	"SIGDIL",
#endif

#ifdef SIGCLD
#ifndef SIGCHLD
  [SIGCLD]	"SIGCLD",
#else
#if SIGCHLD != SIGCLD
  [SIGCLD]	"SIGCLD",
#endif
#endif
#endif

#ifdef SIGPWR
  [SIGPWR]	"SIGPWR",
#endif

#ifdef SIGPOLL
#ifndef SIGIO
  [SIGPOLL]	"SIGPOLL",
#else
#if SIGIO != SIGPOLL
  [SIGPOLL]	"SIGPOLL",
#endif
#endif
#endif

#ifdef SIGWINDOW
  [SIGWINDOW]	"SIGWINDOW",
#endif

#ifdef SIGHUP
  [SIGHUP]	"SIGHUP",
#endif

#ifdef SIGINT
  [SIGINT]	"SIGINT",
#endif

#ifdef SIGQUIT
  [SIGQUIT]	"SIGQUIT",
#endif

#ifdef SIGILL
  [SIGILL]	"SIGILL",
#endif

#ifdef SIGTRAP
  [SIGTRAP]	"SIGTRAP",
#endif

#ifdef SIGIOT
#ifndef SIGABRT
  [SIGIOT]	"SIGIOT",
#else
#if SIGABRT != SIGIOT
  [SIGIOT]	"SIGIOT",
#endif
#endif
#endif

#ifdef SIGABRT
  [SIGABRT]	"SIGABRT",
#endif

#ifdef SIGEMT
  [SIGEMT]	"SIGEMT",
#endif

#ifdef SIGFPE
  [SIGFPE]	"SIGFPE",
#endif

#ifdef SIGKILL
  [SIGKILL]	"SIGKILL",
#endif

#ifdef SIGBUS
  [SIGBUS]	"SIGBUS",
#endif

#ifdef SIGSEGV
  [SIGSEGV]	"SIGSEGV",
#endif

#ifdef SIGSYS
  [SIGSYS]	"SIGSYS",
#endif

#ifdef SIGPIPE
  [SIGPIPE]	"SIGPIPE",
#endif

#ifdef SIGALRM
  [SIGALRM]	"SIGALRM",
#endif

#ifdef SIGTERM
  [SIGTERM]	"SIGTERM",
#endif

#ifdef SIGURG
  [SIGURG]	"SIGURG",
#endif

#ifdef SIGSTOP
  [SIGSTOP]	"SIGSTOP",
#endif

#ifdef SIGTSTP
  [SIGTSTP]	"SIGTSTP",
#endif

#ifdef SIGCONT
  [SIGCONT]	"SIGCONT",
#endif

#ifdef SIGCHLD
  [SIGCHLD]	"SIGCHLD",
#endif

#ifdef SIGTTIN
  [SIGTTIN]	"SIGTTIN",
#endif

#ifdef SIGTTOU
  [SIGTTOU]	"SIGTTOU",
#endif

#ifdef SIGIO
  [SIGIO]	"SIGIO",
#endif

#ifdef SIGXCPU
  [SIGXCPU]	"SIGXCPU",
#endif

#ifdef SIGXFSZ
  [SIGXFSZ]	"SIGXFSZ",
#endif

#ifdef SIGVTALRM
  [SIGVTALRM]	"SIGVTALRM",
#endif

#ifdef SIGPROF
  [SIGPROF]	"SIGPROF",
#endif

#ifdef SIGWINCH
  [SIGWINCH]	"SIGWINCH",
#endif

#ifdef SIGINFO
  [SIGINFO]	"SIGINFO",
#endif

#ifdef SIGUSR1
  [SIGUSR1]	"SIGUSR1",
#endif

#ifdef SIGUSR2
  [SIGUSR2]	"SIGUSR2",
#endif

  [NSIG]	"DEBUG",

  [NSIG + 1]	"ERR",

  [NSIG + 2]	(char *)0x0
};
