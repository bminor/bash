/* config-top.h */

/* This contains various user-settable options not under the control of
   autoconf. */

/* Define CONTINUE_AFTER_KILL_ERROR if you want the kill command to
   continue processing arguments after one of them fails.  This is
   what POSIX.2 specifies. */
#define CONTINUE_AFTER_KILL_ERROR

/* Define BREAK_COMPLAINS if you want the non-standard, but useful
   error messages about `break' and `continue' out of context. */
#define BREAK_COMPLAINS

/* Define BUFFERED_INPUT if you want the shell to do its own input
   buffering, rather than using stdio.  Do not undefine this; it's
   required to preserve semantics required by POSIX. */
#define BUFFERED_INPUT

/* Define ONESHOT if you want sh -c 'command' to avoid forking to execute
   `command' whenever possible.  This is a big efficiency improvement. */
#define ONESHOT

/* Define V9_ECHO if you want to give the echo builtin backslash-escape
   interpretation using the -e option, in the style of the Bell Labs 9th
   Edition version of echo.  You cannot emulate the System V echo behavior
   without this option. */
#define V9_ECHO

/* Define DONT_REPORT_SIGPIPE if you don't want to see `Broken pipe' messages
   when a job like `cat jobs.c | exit 1' is executed. */
/* #define DONT_REPORT_SIGPIPE */

/* The default value of the PATH variable. */
#ifndef DEFAULT_PATH_VALUE
#define DEFAULT_PATH_VALUE \
  "/usr/gnu/bin:/usr/local/bin:/usr/ucb:/bin:/usr/bin:."
#endif

/* The value for PATH when invoking `command -p'.  This is only used when
   the Posix.2 confstr () function, or CS_PATH define are not present. */
#ifndef STANDARD_UTILS_PATH
#define STANDARD_UTILS_PATH \
  "/bin:/usr/bin:/usr/ucb:/sbin:/usr/sbin:/etc:/usr/etc"
#endif

/* Default primary and secondary prompt strings. */
#define PPROMPT "\\s-\\v\\$ "
#define SPROMPT "> "

/* System-wide .bashrc file for interactive shells. */
/* #define SYS_BASHRC "/etc/bash.bashrc" */

/* System-wide .bash_logout for login shells. */
/* #define SYS_BASH_LOGOUT "/etc/bash.bash_logout" */

/* Define this to make non-interactive shells begun with argv[0][0] == '-'
   run the startup files when not in posix mode. */
/* #define NON_INTERACTIVE_LOGIN_SHELLS */
