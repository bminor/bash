.SH Shell Compatibility Mode
Bash-4.0 introduced the concept of a `shell compatibility level', specified
as a set of options to the shopt builtin
.BR compat31 ,
.BR compat32 ,
.BR compat40 ,
.BR compat41 ,
and so on).
There is only one current
compatibility level -- each option is mutually exclusive.
The compatibility level is intended to allow users to select behavior
from previous versions that is incompatible with newer versions
while they migrate scripts to use current features and
behavior. It's intended to be a temporary solution.
.PP
This section does not mention behavior that is standard for a particular
version (e.g., setting \fBcompat32\fP means that quoting the rhs of the regexp
matching operator quotes special regexp characters in the word, which is
default behavior in bash-3.2 and above). 
.PP
If a user enables, say, \fBcompat32\fP, it may affect the behavior of other
compatibility levels up to and including the current compatibility level.
The idea is that each compatibility level controls behavior that changed
in that version of \fBbash\fP,
but that behavior may have been present in earlier versions.
For instance, the change to use locale-based comparisons with the \fB[[\fP
command came in bash-4.1, and earlier versions used ASCII-based comparisons,
so enabling \fBcompat32\fP will enable ASCII-based comparisons as well.
That granularity may not be sufficient for
all uses, and as a result users should employ compatibility levels carefully.
Read the documentation for a particular feature to find out the
current behavior.
.PP
Bash-4.3 introduced a new shell variable:
.SM
.BR BASH_COMPAT .
The value assigned
to this variable (a decimal version number like 4.2, or an integer
corresponding to the \fBcompat\fP\fINN\fP option, like 42) determines the
compatibility level.
.PP
Starting with bash-4.4, Bash has begun deprecating older compatibility
levels.
Eventually, the options will be removed in favor of
.SM
.BR BASH_COMPAT .
.PP
Bash-5.0 is the final version for which there will be an individual shopt
option for the previous version. Users should use
.SM
.B BASH_COMPAT
on bash-5.0 and later versions.
.PP
The following table describes the behavior changes controlled by each
compatibility level setting.
The \fBcompat\fP\fINN\fP tag is used as shorthand for setting the
compatibility level
to \fINN\fP using one of the following mechanisms.
For versions prior to bash-5.0, the compatibility level may be set using
the corresponding \fBcompat\fP\fINN\fP shopt option.
For bash-4.3 and later versions, the
.SM
.B BASH_COMPAT
variable is preferred,
and it is required for bash-5.1 and later versions.
.PP

.TP
\fBcompat31\fP
.IP \(bu
quoting the rhs of the \fB[[\fP command's regexp matching operator (=~)
has no special effect

.TP
\fBcompat32\fP
.IP \(bu
interrupting a command list such as "a ; b ; c" causes the execution
of the next command in the list (in bash-4.0 and later versions,
the shell acts as if it received the interrupt, so
interrupting one command in a list aborts the execution of the
entire list)

.TP
\fBcompat40\fP
.IP \(bu
the \fB<\fP and \fB>\fP operators to the \fB[[\fP command do not
consider the current locale when comparing strings; they use ASCII
ordering.
Bash versions prior to bash-4.1 use ASCII collation and
.IR strcmp (3);
bash-4.1 and later use the current locale's collation sequence and
.IR strcoll (3).

.TP
\fBcompat41\fP
.IP \(bu
in \fIposix\fP mode, \fBtime\fP may be followed by options and still be
recognized as a reserved word (this is POSIX interpretation 267)
.IP \(bu
in \fIposix\fP mode, the parser requires that an even number of single
quotes occur in the \fIword\fP portion of a double-quoted
parameter expansion and treats them specially, so that characters within
the single quotes are considered quoted
(this is POSIX interpretation 221)

.TP
\fBcompat42\fP
.IP \(bu
the replacement string in double-quoted pattern substitution does not
undergo quote removal, as it does in versions after bash-4.2
.IP \(bu
in posix mode, single quotes are considered special when expanding
the \fIword\fP portion of a double-quoted parameter expansion
and can be used to quote a closing brace or other special character
(this is part of POSIX interpretation 221);
in later versions, single quotes
are not special within double-quoted word expansions

.TP
\fBcompat43\fP
the shell does not print a warning message if an attempt is made to
use a quoted compound assignment as an argument to declare
(declare -a foo='(1 2)'). Later versions warn that this usage is
deprecated
.IP \(bu
word expansion errors are considered non-fatal errors that cause the
current command to fail, even in posix mode
(the default behavior is to make them fatal errors that cause the shell
to exit)
.IP \(bu
when executing a shell function, the loop state (while/until/etc.)
is not reset, so \fBbreak\fP or \fBcontinue\fP in that function will break
or continue loops in the calling context. Bash-4.4 and later reset
the loop state to prevent this

.TP
\fBcompat44\fP
.IP \(bu
the shell sets up the values used by
.SM
.B BASH_ARGV
and
.SM
.B BASH_ARGC
so they can expand to the shell's positional parameters even if extended
debug mode is not enabled
.IP \(bu
a subshell inherits loops from its parent context, so \fBbreak\fP
or \fBcontinue\fP will cause the subshell to exit.
Bash-5.0 and later reset the loop state to prevent the exit
.IP \(bu
variable assignments preceding builtins like \fBexport\fP and \fBreadonly\fP
that set attributes continue to affect variables with the same
name in the calling environment even if the shell is not in posix
mode

.TP
\fBcompat50\fP
.IP \(bu
Bash-5.1 changed the way
.SM
.B $RANDOM
is generated to introduce slightly
more randomness. If the shell compatibility level is set to 50 or
lower, it reverts to the method from bash-5.0 and previous versions,
so seeding the random number generator by assigning a value to
.SM
.B RANDOM
will produce the same sequence as in bash-5.0
