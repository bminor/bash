/*
 * ORIGINAL COPYRIGHT STATEMENT:
 *
 * Copyright (c) 1994 Winning Strategies, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Winning Strategies, Inc.
 * 4. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * POSIX.2 getconf utility
 *
 * Originally Written by:
 *	J.T. Conklin (jtc@wimsey.com), Winning Strategies, Inc.
 *
 * Heavily modified for inclusion in bash by
 *	Chet Ramey <chet@po.cwru.edu>
 */

#include <stdio.h>
#include <limits.h>
#include <locale.h>
#include <unistd.h>
#include <errno.h>
#include "bashansi.h"
#include "shell.h"
#include "builtins.h"
#include "stdc.h"
#include "common.h"
#include "bashgetopt.h"

struct conf_variable
{
  const char *name;
  enum { SYSCONF, CONFSTR, PATHCONF, CONSTANT } type;
  long value;
};

/* Some systems do not define these; use POSIX.2 minimum recommended values. */
#ifndef _POSIX2_COLL_WEIGHTS_MAX
#  define _POSIX2_COLL_WEIGHTS_MAX 2
#endif

static const struct conf_variable conf_table[] =
{
  /* POSIX.2 Configurable Variable Values */
  { "PATH",			CONFSTR,	_CS_PATH		},
  { "CS_PATH",			CONFSTR,	_CS_PATH		},

  /* POSIX.1 Configurable Variable Values (only Solaris?) */
#if defined (_CS_LFS_CFLAGS)
  { "LFS_CFLAGS",		CONFSTR,	_CS_LFS_CFLAGS		},
  { "LFS_LDFLAGS",		CONFSTR,	_CS_LFS_LDFLAGS		},
  { "LFS_LIBS",			CONFSTR,	_CS_LFS_LIBS		},
  { "LFS_LINTFLAGS",		CONFSTR,	_CS_LFS_LINTFLAGS	},
#endif
#if defined (_CS_LFS64_CFLAGS)
  { "LFS64_CFLAGS",		CONFSTR,	_CS_LFS64_CFLAGS	},
  { "LFS64_LDFLAGS",		CONFSTR,	_CS_LFS64_LDFLAGS	},
  { "LFS64_LIBS",		CONFSTR,	_CS_LFS64_LIBS		},
  { "LFS64_LINTFLAGS",		CONFSTR,	_CS_LFS64_LINTFLAGS	},
#endif

  /* Single UNIX Specification version 2 Configurable Variable Values */
#if defined (_CS_XBS5_ILP32_OFF32_CFLAGS)
  { "XBS5_ILP32_OFF32_CFLAGS",  	CONFSTR,	_CS_XBS5_ILP32_OFF32_CFLAGS },
  { "XBS5_ILP32_OFF32_LDFLAGS",		CONFSTR,	_CS_XBS5_ILP32_OFF32_LDFLAGS },
  { "XBS5_ILP32_OFF32_LIBS",    	CONFSTR,	_CS_XBS5_ILP32_OFF32_LIBS },
  { "XBS5_ILP32_OFF32_LINTFLAGS",       CONFSTR,	_CS_XBS5_ILP32_OFF32_LINTFLAGS },
  { "XBS5_ILP32_OFFBIG_CFLAGS", 	CONFSTR,	_CS_XBS5_ILP32_OFFBIG_CFLAGS },
  { "XBS5_ILP32_OFFBIG_LDFLAGS",	CONFSTR,	_CS_XBS5_ILP32_OFFBIG_LDFLAGS },
  { "XBS5_ILP32_OFFBIG_LIBS",   	CONFSTR,	_CS_XBS5_ILP32_OFFBIG_LIBS },
  { "XBS5_ILP32_OFFBIG_LINTFLAGS",      CONFSTR,	_CS_XBS5_ILP32_OFFBIG_LINTFLAGS },
  { "XBS5_LP64_OFF64_CFLAGS",   	CONFSTR,	_CS_XBS5_LP64_OFF64_CFLAGS },
  { "XBS5_LP64_OFF64_LDFLAGS",  	CONFSTR,	_CS_XBS5_LP64_OFF64_LDFLAGS },
  { "XBS5_LP64_OFF64_LIBS",     	CONFSTR,	_CS_XBS5_LP64_OFF64_LIBS },
  { "XBS5_LP64_OFF64_LINTFLAGS",	CONFSTR,	_CS_XBS5_LP64_OFF64_LINTFLAGS },
  { "XBS5_LPBIG_OFFBIG_CFLAGS", 	CONFSTR,	_CS_XBS5_LPBIG_OFFBIG_CFLAGS },
  { "XBS5_LPBIG_OFFBIG_LDFLAGS",	CONFSTR,	_CS_XBS5_LPBIG_OFFBIG_LDFLAGS },
  { "XBS5_LPBIG_OFFBIG_LIBS",   	CONFSTR,	_CS_XBS5_LPBIG_OFFBIG_LIBS },
  { "XBS5_LPBIG_OFFBIG_LINTFLAGS",      CONFSTR,	_CS_XBS5_LPBIG_OFFBIG_LINTFLAGS },
#endif /* _CS_XBS5_ILP32_OFF32_CFLAGS */

  /* POSIX.2 Utility Limit Minimum Values */
  { "POSIX2_BC_BASE_MAX",	CONSTANT,	_POSIX2_BC_BASE_MAX	},
  { "POSIX2_BC_DIM_MAX",	CONSTANT,	_POSIX2_BC_DIM_MAX	},
  { "POSIX2_BC_SCALE_MAX",	CONSTANT,	_POSIX2_BC_SCALE_MAX	},
  { "POSIX2_BC_STRING_MAX",	CONSTANT,	_POSIX2_BC_STRING_MAX	},
  { "POSIX2_COLL_WEIGHTS_MAX",	CONSTANT,	_POSIX2_COLL_WEIGHTS_MAX },
#if defined (_POSIX2_EQUIV_CLASS_MAX)
  { "POSIX2_EQUIV_CLASS_MAX",	CONSTANT,	_POSIX2_EQUIV_CLASS_MAX	},
#endif
  { "POSIX2_EXPR_NEST_MAX",	CONSTANT,	_POSIX2_EXPR_NEST_MAX	},
  { "POSIX2_LINE_MAX",		CONSTANT,	_POSIX2_LINE_MAX	},
  { "POSIX2_RE_DUP_MAX",	CONSTANT,	_POSIX2_RE_DUP_MAX	},
#if defined (_POSIX2_VERSION)
  { "POSIX2_VERSION",		CONSTANT,	_POSIX2_VERSION		},
#endif

  /* POSIX.1 Minimum Values */
  { "_POSIX_ARG_MAX",		CONSTANT,	_POSIX_ARG_MAX		},
  { "_POSIX_CHILD_MAX",		CONSTANT,	_POSIX_CHILD_MAX	},
  { "_POSIX_LINK_MAX",		CONSTANT,	_POSIX_LINK_MAX		},
  { "_POSIX_MAX_CANON",		CONSTANT,	_POSIX_MAX_CANON	},
  { "_POSIX_MAX_INPUT",		CONSTANT,	_POSIX_MAX_INPUT	},
  { "_POSIX_NAME_MAX",		CONSTANT,	_POSIX_NAME_MAX		},
  { "_POSIX_NGROUPS_MAX",	CONSTANT,	_POSIX_NGROUPS_MAX	},
  { "_POSIX_OPEN_MAX",		CONSTANT,	_POSIX_OPEN_MAX		},
  { "_POSIX_PATH_MAX",		CONSTANT,	_POSIX_PIPE_BUF		},
  { "_POSIX_PIPE_BUF",		CONSTANT,	_POSIX_PIPE_BUF		},
  { "_POSIX_SSIZE_MAX",		CONSTANT,	_POSIX_SSIZE_MAX	},
  { "_POSIX_STREAM_MAX",	CONSTANT,	_POSIX_STREAM_MAX	},
  { "_POSIX_TZNAME_MAX",	CONSTANT,	_POSIX_TZNAME_MAX	},

  /* POSIX.2 Symbolic Utility Limits */
  { "BC_BASE_MAX",		SYSCONF,	_SC_BC_BASE_MAX		},
  { "BC_DIM_MAX",		SYSCONF,	_SC_BC_DIM_MAX		},
  { "BC_SCALE_MAX",		SYSCONF,	_SC_BC_SCALE_MAX	},
  { "BC_STRING_MAX",		SYSCONF,	_SC_BC_STRING_MAX	},
  { "COLL_WEIGHTS_MAX",		SYSCONF,	_SC_COLL_WEIGHTS_MAX	},
  { "EXPR_NEST_MAX",		SYSCONF,	_SC_EXPR_NEST_MAX	},
  { "LINE_MAX",			SYSCONF,	_SC_LINE_MAX		},
  { "RE_DUP_MAX",		SYSCONF,	_SC_RE_DUP_MAX		},

  /* POSIX.2 Optional Facility Configuration Values */
  { "POSIX2_C_BIND",		SYSCONF,	_SC_2_C_BIND		},
  { "POSIX2_C_DEV",		SYSCONF,	_SC_2_C_DEV		},
#if defined (_SC_2_C_VERSION)
  { "POSIX2_C_VERSION",		SYSCONF,	_SC_2_C_VERSION		},
#endif
#if defined (_SC_2_CHAR_TERM)
  { "POSIX2_CHAR_TERM",		SYSCONF,	_SC_2_CHAR_TERM		},
#endif
  { "POSIX2_FORT_DEV",		SYSCONF,	_SC_2_FORT_DEV		},
  { "POSIX2_FORT_RUN",		SYSCONF,	_SC_2_FORT_RUN		},
  { "POSIX2_LOCALEDEF",		SYSCONF,	_SC_2_LOCALEDEF		},
  { "POSIX2_SW_DEV",		SYSCONF,	_SC_2_SW_DEV		},
#if defined (_SC2_UPE)
  { "POSIX2_UPE",		SYSCONF,	_SC_2_UPE		},
#endif
#if !defined (_POSIX2_VERSION) && defined (_SC_2_VERSION)
  { "POSIX2_VERSION"		SYSCONF,	_SC_2_VERSION		},
#endif

  /* POSIX.1 Configurable System Variables */
  { "ARG_MAX",			SYSCONF,	_SC_ARG_MAX 		},
  { "CHILD_MAX",		SYSCONF,	_SC_CHILD_MAX		},
  { "CLK_TCK",			SYSCONF,	_SC_CLK_TCK		},
  { "NGROUPS_MAX",		SYSCONF,	_SC_NGROUPS_MAX		},
  { "OPEN_MAX",			SYSCONF,	_SC_OPEN_MAX		},
  { "STREAM_MAX",		SYSCONF,	_SC_STREAM_MAX		},
  { "TZNAME_MAX",		SYSCONF,	_SC_TZNAME_MAX		},
  { "_POSIX_JOB_CONTROL",	SYSCONF,	_SC_JOB_CONTROL 	},
  { "_POSIX_SAVED_IDS",		SYSCONF,	_SC_SAVED_IDS		},
  { "_POSIX_VERSION",		SYSCONF,	_SC_VERSION		},

  /* POSIX.1 Optional Facility Configuration Values */
#if defined (_SC_ASYNCHRONOUS_IO)
  { "_POSIX_ASYNCHRONOUS_IO",	SYSCONF,	_SC_ASYNCHRONOUS_IO	},
#endif
#if defined (_SC_FSYNC)
  { "_POSIX_FSYNC",		SYSCONF,	_SC_FSYNC		},
#endif
#if defined (_SC_MAPPED_FILES)
  { "_POSIX_MAPPED_FILES",	SYSCONF,	_SC_MAPPED_FILES	},
#endif
#if defined (_SC_MEMLOCK)
  { "_POSIX_MEMLOCK",		SYSCONF,	_SC_MEMLOCK		},
#endif
#if defined (_SC_MEMLOCK_RANGE)
  { "_POSIX_MEMLOCK_RANGE",	SYSCONF,	_SC_MEMLOCK_RANGE	},
#endif
#if defined (_SC_MEMORY_PROTECTION)
  { "_POSIX_MEMORY_PROTECTION",	SYSCONF,	_SC_MEMORY_PROTECTION	},
#endif
#if defined (_SC_MESSAGE_PASSING)
  { "_POSIX_MESSAGE_PASSING",	SYSCONF,	_SC_MESSAGE_PASSING	},
#endif
#if defined (SC_PRIORITIZED_IO)
  { "_POSIX_PRIORITIZED_IO",	SYSCONF,	_SC_PRIORITIZED_IO	},
#endif
#if defined (_SC_PRIORITY_SCHEDULING)
  { "_POSIX_PRIORITY_SCHEDULING", SYSCONF,	_SC_PRIORITY_SCHEDULING	},
#endif
#if defined (_SC_REALTIME_SIGNALS)
  { "_POSIX_REALTIME_SIGNALS",	SYSCONF,	_SC_REALTIME_SIGNALS	},
#endif
#if defined (_SC_SEMAPHORES)
  { "_POSIX_SEMAPHORES",	SYSCONF,	_SC_SEMAPHORES		},
#endif
#if defined (_SC_SHARED_MEMORY_OBJECTS)
  { "_POSIX_SHARED_MEMORY_OBJECTS", SYSCONF,	_SC_SHARED_MEMORY_OBJECTS },
#endif
#if defined (_SC_SYNCHRONIZED_IO)
  { "_POSIX_SYNCHRONIZED_IO",	SYSCONF,	_SC_SYNCHRONIZED_IO	},
#endif
#if defined (_SC_TIMERS)
  { "_POSIX_TIMERS",		SYSCONF,	_SC_TIMERS		},
#endif
#if defined (_SC_THREADS)
  { "_POSIX_THREADS",		SYSCONF,	_SC_THREADS		},
  { "_POSIX_THREAD_ATTR_STACKADDR", SYSCONF,	_SC_THREAD_ATTR_STACKADDR },
  { "_POSIX_THREAD_ATTR_STACKSIZE", SYSCONF,	_SC_THREAD_ATTR_STACKSIZE },
  { "_POSIX_THREAD_PRIORITY_SCHEDULING", SYSCONF, _SC_THREAD_PRIORITY_SCHEDULING },
  { "_POSIX_THREAD_PRIO_INHERIT", SYSCONF,	_SC_THREAD_PRIO_INHERIT	},
  { "_POSIX_THREAD_PRIO_PROTECT", SYSCONF,	_SC_THREAD_PRIO_PROTECT	},
  { "_POSIX_THREAD_PROCESS_SHARED", SYSCONF,	_SC_THREAD_PROCESS_SHARED },
#  if defined (_SC_THREAD_SAFE_FUNCTIONS)
  { "_POSIX_THREAD_SAFE_FUNCTIONS", SYSCONF,	_SC_THREAD_SAFE_FUNCTIONS },
#  endif
#endif /* _SC_THREADS */

  /* XPG 4.2 Configurable System Variables. */
#if defined (_SC_ATEXIT_MAX)
  { "ATEXIT_MAX",		SYSCONF,	_SC_ATEXIT_MAX		},
#endif
#if defined (_SC_IOV_MAX)
  { "IOV_MAX",			SYSCONF,	_SC_IOV_MAX		},
#endif
#if defined (_SC_PAGESIZE)
  { "PAGESIZE",			SYSCONF,	_SC_PAGESIZE		},
#endif
#if defined (_SC_PAGE_SIZE)
  { "PAGE_SIZE",		SYSCONF,	_SC_PAGE_SIZE		},
#endif

#if defined (_SC_AIO_LISTIO_MAX)
  { "AIO_LISTIO_MAX",		SYSCONF,	_SC_AIO_LISTIO_MAX	},
  { "AIO_MAX",			SYSCONF,	_SC_AIO_MAX		},
  { "AIO_PRIO_DELTA_MAX",	SYSCONF,	_SC_AIO_PRIO_DELTA_MAX	},
  { "DELAYTIMER_MAX",		SYSCONF,	_SC_DELAYTIMER_MAX	},
#if defined (_SC_GETGR_R_SIZE_MAX)
  { "GETGR_R_SIZE_MAX",		SYSCONF,	_SC_GETGR_R_SIZE_MAX	},
#endif
#if defined (_SC_GETPW_R_SIZE_MAX)
  { "GETPW_R_SIZE_MAX",		SYSCONF,	_SC_GETPW_R_SIZE_MAX	},
#endif
  { "MQ_OPEN_MAX",		SYSCONF,	_SC_MQ_OPEN_MAX		},
  { "MQ_PRIO_MAX",		SYSCONF,	_SC_MQ_PRIO_MAX		},
  { "RTSIG_MAX",		SYSCONF,	_SC_RTSIG_MAX		},
  { "SEM_NSEMS_MAX",		SYSCONF,	_SC_SEM_NSEMS_MAX	},
  { "SEM_VALUE_MAX",		SYSCONF,	_SC_SEM_VALUE_MAX	},
  { "SIGQUEUE_MAX",		SYSCONF,	_SC_SIGQUEUE_MAX	},
  { "TIMER_MAX",		SYSCONF,	_SC_TIMER_MAX		},
#endif /* _SC_AIO_LISTIO_MAX */
#if defined (_SC_LOGIN_NAME_MAX)
  { "LOGIN_NAME_MAX",		SYSCONF,	_SC_LOGIN_NAME_MAX	},
#endif
#if defined (_SC_LOGNAME_MAX)
  { "LOGNAME_MAX",		SYSCONF,	_SC_LOGNAME_MAX		},
#endif
#if defined (_SC_TTY_NAME_MAX)
  { "TTY_NAME_MAX",		SYSCONF,	_SC_TTY_NAME_MAX	},
#endif

#if defined (_SC_PTHREAD_DESTRUCTOR_ITERATIONS)
  { "PTHREAD_DESTRUCTOR_ITERATIONS", SYSCONF,	_SC_THREAD_DESTRUCTOR_ITERATIONS },
  { "PTHREAD_KEYS_MAX",		SYSCONF,	_SC_THREAD_KEYS_MAX },
  { "PTHREAD_STACK_MIN",	SYSCONF,	_SC_THREAD_STACK_MIN },
  { "PTHREAD_THREADS_MAX",	SYSCONF,	_SC_THREAD_THREADS_MAX },
#endif /* _SC_PTHREAD_DESTRUCTOR_ITERATIONS */

  /* XPG 4.2 Optional Facility Configuration Values */
#if defined (_SC_XOPEN_UNIX)
  { "_XOPEN_UNIX",		SYSCONF,	_SC_XOPEN_UNIX		},
  { "_XOPEN_CRYPT",		SYSCONF,	_SC_XOPEN_CRYPT		},
  { "_XOPEN_ENH_I18N",		SYSCONF,	_SC_XOPEN_ENH_I18N	},
  { "_XOPEN_SHM",		SYSCONF,	_SC_XOPEN_SHM		},
  { "_XOPEN_VERSION",		SYSCONF,	_SC_XOPEN_VERSION	},
#  if defined (_SC_XOPEN_XCU_VERSION)
  { "_XOPEN_XCU_VERSION",	SYSCONF,	_SC_XOPEN_XCU_VERSION	},
#  endif
#endif
#if defined (_SC_XOPEN_REALTIME)
  { "_XOPEN_REALTIME",		SYSCONF,	_SC_XOPEN_REALTIME	},
  { "_XOPEN_REALTIME_THREADS",	SYSCONF,	_SC_XOPEN_REALTIME_THREADS },
#endif
#if defined (_SC_XOPEN_LEGACY)
  { "_XOPEN_LEGACY",		SYSCONF,	_SC_XOPEN_LEGACY	},
#endif /* _SC_XOPEN_LEGACY */

  /* Single UNIX Specification version 2 Optional Facility Configuration Values */
#if defined (_SC_XBS5_ILP32_OFF32)
  { "_XBS5_ILP32_OFF32",	SYSCONF,	_SC_XBS5_ILP32_OFF32	},
  { "_XBS5_ILP32_OFFBIG",	SYSCONF,	_SC_XBS5_ILP32_OFFBIG	},
  { "_XBS5_LP64_OFF64",		SYSCONF,	_SC_XBS5_LP64_OFF64	},
  { "_XBS5_LPBIG_OFFBIG",	SYSCONF,	_SC_XBS5_LPBIG_OFFBIG	},
#endif /* _SC_XBS5_ILP32_OFF32 */

  /* POSIX.1 Configurable Pathname Values */
  { "LINK_MAX",			PATHCONF,	_PC_LINK_MAX		},
  { "MAX_CANON",		PATHCONF,	_PC_MAX_CANON		},
  { "MAX_INPUT",		PATHCONF,	_PC_MAX_INPUT		},
  { "NAME_MAX",			PATHCONF,	_PC_NAME_MAX		},
  { "PATH_MAX",			PATHCONF,	_PC_PATH_MAX		},
  { "PIPE_BUF",			PATHCONF,	_PC_PIPE_BUF		},
  { "_POSIX_CHOWN_RESTRICTED",	PATHCONF,	_PC_CHOWN_RESTRICTED	},
  { "_POSIX_NO_TRUNC",		PATHCONF,	_PC_NO_TRUNC		},
  { "_POSIX_VDISABLE",		PATHCONF,	_PC_VDISABLE		},

  /* XPG 4.2 Configurable Pathname Values */
#if defined (_PC_FILESIZEBITS)
  { "FILESIZEBITS",		PATHCONF,	_PC_FILESIZEBITS },
#endif
#if defined (_PC_ASYNC_IO)
  { "_POSIX_ASYNC_IO",		PATHCONF,	_PC_ASYNC_IO },
#endif
#if defined (_PC_PRIO_IO)
  { "_POSIX_PRIO_IO",		PATHCONF,	_PC_PRIO_IO },
#endif
#if defined (_PC_SYNC_IO)
  { "_POSIX_SYNC_IO",		PATHCONF,	_PC_SYNC_IO },
#endif

  { NULL }
};

static int num_getconf_variables = sizeof(conf_table) / sizeof(struct conf_variable) - 1;

extern char *this_command_name;
extern char *xmalloc ();
extern char **make_builtin_argv ();

static void getconf_help ();
static int getconf_print ();
static int getconf_one ();
static int getconf_all ();

int
getconf_builtin (list)
     WORD_LIST *list;
{
  int c, r, opt, aflag;
  char **v;

  aflag = 0;
  reset_internal_getopt();
  while ((opt = internal_getopt (list, "ah")) != -1) {
  	switch (opt) {
  	case 'a':
  		aflag = 1;
  		break;
  	case 'h':
  		getconf_help();
  		return(EXECUTION_SUCCESS);
  	default:
  		builtin_usage();
  		return(EX_USAGE);
  	}
  }
 
  list = loptend;
  if ((aflag == 0 && list == 0) || (aflag && list) || list_length(list) > 2) {
  	builtin_usage();
  	return(EX_USAGE);
  }

  r = aflag ? getconf_all() : getconf_one(list);

  return r;
}

static void
getconf_help()
{
	const struct conf_variable *cp;
	register int i, column;

	builtin_usage();
	printf("Acceptable variable names are:\n");
	for (cp = conf_table; cp->name != NULL; cp++) {
		if (cp->type == PATHCONF)
			printf("%s pathname\n", cp->name);
		else
			printf("%s\n", cp->name);
	}
}

static int
getconf_print(cp, vpath, all)
struct conf_variable *cp;
char *vpath;
int all;
{
	long val;
	char *sval;
	size_t slen;

	switch (cp->type) {
	case CONSTANT:
		printf("%ld\n", cp->value);
		break;

	case CONFSTR:
		errno = 0;
		slen = confstr (cp->value, (char *) 0, (size_t) 0);
		if (slen == 0) {
			if (errno != 0) {
				if (all)
					printf ("getconf: %s\n", strerror(errno));
				else
					builtin_error ("%s", strerror(errno));
			} else
				printf ("undefined\n");
			return (EXECUTION_FAILURE);
		}
		sval = xmalloc(slen);

		confstr(cp->value, sval, slen);
		printf("%s\n", sval);
		free(sval);
		break;

	case SYSCONF:
		errno = 0;
		if ((val = sysconf(cp->value)) == -1) {
			if (errno != 0) {
				if (all)
					printf("getconf: %s\n", strerror (errno));
				else
					builtin_error ("%s", strerror (errno));
				return (EXECUTION_FAILURE);
			}

			printf ("undefined\n");
		} else {
			printf("%ld\n", val);
		}
		break;

	case PATHCONF:
		errno = 0;
		if ((val = pathconf(vpath, cp->value)) == -1) {
			if (errno != 0) {
				if (all)
					printf("getconf: %s: %s\n", vpath, strerror (errno));
				else
					builtin_error ("%s: %s", vpath, strerror (errno));
				return (EXECUTION_FAILURE);
			}

			printf ("undefined\n");
		} else {
			printf ("%ld\n", val);
		}
		break;
	}

	return (ferror(stdout) ? EXECUTION_FAILURE : EXECUTION_SUCCESS);
}

static int
getconf_all()
{
	const struct conf_variable *cp;
	int ret;

	ret = EXECUTION_SUCCESS;
	for (cp = conf_table; cp->name != NULL; cp++) {
		printf("%-35s", cp->name);
		if (getconf_print(cp, ".", 1) == EXECUTION_FAILURE)
			ret = EXECUTION_FAILURE;
	}
	return ret;
}

static int
getconf_one(list)
	WORD_LIST *list;
{
	const struct conf_variable *cp;
	char *vname, *vpath;

	vname = list->word->word;
	vpath = (list->next && list->next->word) ? list->next->word->word
						 : (char *)NULL;

	for (cp = conf_table; cp->name != NULL; cp++) {
		if (strcmp(vname, cp->name) == 0)
			break;
	}
	if (cp->name == NULL) {
		builtin_error ("%s: unknown variable", vname);
		return (EXECUTION_FAILURE);
	}

	if (cp->type == PATHCONF) {
		if (list->next == 0) {
			builtin_usage();
			return(EX_USAGE);
		}
	} else {
		if (list->next) {
			builtin_usage();
			return(EX_USAGE);
		}
	}

	return (getconf_print(cp, vpath, 0));
}

static char *getconf_doc[] = {
	"getconf writes the current value of a configurable system limit or",
	"option variable to the standard output.",
	(char *)NULL
};

struct builtin getconf_struct = {
	"getconf",
	getconf_builtin,
	BUILTIN_ENABLED,
	getconf_doc,
	"getconf -a or getconf -h or getconf sysvar or getconf pathvar pathname",
	0
};
