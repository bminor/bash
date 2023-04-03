/* trap.h -- data structures used in the trap mechanism. */

/* Copyright (C) 1993-2023 Free Software Foundation, Inc.

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

#if !defined (_TRAP_H_)
#define _TRAP_H_

#include "stdc.h"

#if !defined (SIG_DFL)
#include "bashtypes.h"
#include <signal.h>
#endif /* SIG_DFL */

#if !defined (NSIG)
#define NSIG 64
#endif /* !NSIG */

#define NO_SIG -1
#define DEFAULT_SIG	SIG_DFL
#define IGNORE_SIG	SIG_IGN

/* Special shell trap names. */
#define DEBUG_TRAP	NSIG
#define ERROR_TRAP	NSIG+1
#define RETURN_TRAP	NSIG+2
#define EXIT_TRAP 	0

/* system signals plus special bash traps */
#define BASH_NSIG	NSIG+3

/* Flags values for decode_signal() */
#define DSIG_SIGPREFIX	0x01		/* don't allow `SIG' PREFIX */
#define DSIG_NOCASE	0x02		/* case-insensitive comparison */

/* A value which can never be the target of a trap handler. */
#define IMPOSSIBLE_TRAP_HANDLER (SigHandler *)initialize_traps

#define signal_object_p(x,f) (decode_signal (x,f) != NO_SIG)

#define TRAP_STRING(s) \
  (signal_is_trapped (s) && signal_is_ignored (s) == 0) ? trap_list[s] \
							: (char *)NULL

extern char *trap_list[];

extern int trapped_signal_received;
extern int wait_signal_received;
extern int running_trap;
extern int trap_return_context;
extern int trap_saved_exit_value;
extern int suppress_debug_trap_verbose;

/* Externally-visible functions declared in trap.c. */
extern void initialize_traps (void);

extern void run_pending_traps (void);

extern void queue_sigchld_trap (int);
extern void set_impossible_sigchld_trap (void);
extern void set_sigchld_trap (const char *);

extern void set_debug_trap (const char *);
extern void set_error_trap (const char *);
extern void set_return_trap (const char *);

extern void uw_set_error_trap (void *);
extern void uw_maybe_set_sigchld_trap (void *);
extern void uw_maybe_set_debug_trap (void *);
extern void uw_maybe_set_error_trap (void *);
extern void uw_maybe_set_return_trap (void *);

extern void set_sigint_trap (const char *);
extern void set_signal (int, const char *);

extern void restore_default_signal (int);
extern void ignore_signal (int);
extern int run_exit_trap (void);
extern void run_trap_cleanup (int);
extern int run_debug_trap (void);
extern void run_error_trap (void);
extern void run_return_trap (void);

extern void free_trap_strings (void);
extern void reset_signal_handlers (void);
extern void restore_original_signals (void);
extern void restore_traps (void);

extern void get_original_signal (int);
extern void get_all_original_signals (void);

extern char *signal_name (int);

extern int decode_signal (const char *, int);
extern void run_interrupt_trap (int);
extern int maybe_call_trap_handler (int);
extern int signal_is_special (int);
extern int signal_is_trapped (int);
extern int signal_is_pending (int);
extern int signal_is_ignored (int);
extern int signal_is_hard_ignored (int);
extern void set_signal_hard_ignored (int);
extern void set_signal_ignored (int);
extern void set_signal_async_ignored (int);
extern int signal_is_async_ignored (int);
extern int signal_in_progress (int);

extern void set_trap_state (int);

extern int next_pending_trap (int);
extern int first_pending_trap (void);
extern void clear_pending_traps (void);
extern int any_signals_trapped (void);
extern void check_signals (void);
extern void check_signals_and_traps (void);

#endif /* _TRAP_H_ */
