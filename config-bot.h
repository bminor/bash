/* config.h.bot */
/* modify settings or make new ones based on what autoconf tells us. */

#if !defined (HAVE_VPRINTF) && defined (HAVE_DOPRNT)
#  define USE_VFPRINTF_EMULATION
#  define HAVE_VPRINTF
#endif

/* Ultrix botches type-ahead when switching from canonical to
   non-canonical mode, at least through version 4.3 */
#if !defined (HAVE_TERMIOS_H) || !defined (HAVE_TCGETATTR) || defined (ultrix)
#  define TERMIOS_MISSING
#endif

/* If we have a getcwd(3), but it calls popen(), #undef HAVE_GETCWD so
   the replacement in getcwd.c will be built. */
#if defined (HAVE_GETCWD) && defined (GETCWD_BROKEN)
#  undef HAVE_GETCWD
#endif

#if defined (HAVE_SYS_RESOURCE_H) && defined (HAVE_GETRLIMIT)
#  define HAVE_RESOURCE
#endif

#if !defined (GETPGRP_VOID)
#  define HAVE_BSD_PGRP
#endif

#if !defined (HAVE_DEV_FD) && defined (NAMED_PIPES_MISSING)
#  undef PROCESS_SUBSTITUTION
#endif

/* If the shell is called by this name, it will become restricted. */
#if defined (RESTRICTED_SHELL)
#  define RESTRICTED_SHELL_NAME "rbash"
#endif

/* BANG_HISTORY requires HISTORY. */
#if defined (BANG_HISTORY) && !defined (HISTORY)
#  define HISTORY
#endif /* BANG_HISTORY && !HISTORY */

#if defined (READLINE) && !defined (HISTORY)
#  define HISTORY
#endif

#if defined (PROGRAMMABLE_COMPLETION) && !defined (READLINE)
#  undef PROGRAMMABLE_COMPLETION
#endif

#if !defined (V9_ECHO)
#  undef DEFAULT_ECHO_TO_XPG
#endif

#if defined (JOB_CONTROL_MISSING)
#  undef JOB_CONTROL
#endif

#if defined (__STDC__) && defined (HAVE_STDARG_H)
#  define PREFER_STDARG
#  define USE_VARARGS
#else
#  if defined (HAVE_VARARGS_H)
#    define PREFER_VARARGS
#    define USE_VARARGS
#  endif
#endif

#if defined (STRCOLL_BROKEN)
#  undef HAVE_STRCOLL
#endif

#if defined (HAVE_SYS_SOCKET_H) && defined (HAVE_GETPEERNAME) && defined (HAVE_NETINET_IN_H)
#  define HAVE_NETWORK
#endif

#if !defined (PROMPT_STRING_DECODE)
#  undef PPROMPT
#  define PPROMPT "$ "
#endif
