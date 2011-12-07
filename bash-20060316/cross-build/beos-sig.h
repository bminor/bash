/* This file is used when cross compiling bash to run native on BeOS. */

#ifndef __GNUC__
# error cross compiling requires gcc
#endif

/* A translation list so we can be polite to our users.
   Use gcc labelled initializers to set up the array.  Note that
   some entries might wind up being NULL. */

char *signal_names[NSIG + 3] = {
  [0]         "EXIT",
  [SIGHUP]    "SIGHUP",
  [SIGINT]    "SIGINT",
  [SIGQUIT]   "SIGQUIT",
  [SIGILL]    "SIGILL",
  [SIGCHLD]   "SIGCHLD",
  [SIGABRT]   "SIGABRT",
  [SIGPIPE]   "SIGPIPE",
  [SIGFPE]    "SIGFPE",
  [SIGKILL]   "SIGKILL",
  [SIGSTOP]   "SIGSTOP",
  [SIGSEGV]   "SIGSEGV",
  [SIGCONT]   "SIGCONT",
  [SIGTSTP]   "SIGTSTP",
  [SIGALRM]   "SIGALRM",
  [SIGTERM]   "SIGTERM",
  [SIGTTIN]   "SIGTTIN",
  [SIGTTOU]   "SIGTTOU",
  [SIGUSR1]   "SIGUSR1",
  [SIGUSR2]   "SIGUSR2",
  [SIGWINCH]  "SIGWINCH",
  [SIGKILLTHR]        "SIGKILLTHR",
  [NSIG]      "DEBUG",
  [NSIG + 1]  "ERR",
  [NSIG + 2]  (char *)0x0,
};
