#!/bin/sh -
#
# bashbug - create a bug report and mail it to the bug address
#
# The bug address depends on the release status of the shell.  Versions
# with status `alpha' or `beta' mail bug reports to chet@po.cwru.edu.
# Other versions send mail to bug-bash@gnu.org.
#
# configuration section:
#	these variables are filled in by the make target in cpp-Makefile
#
MACHINE="!MACHINE!"
OS="!OS!"
CC="!CC!"
CFLAGS="!CFLAGS!"
RELEASE="!RELEASE!"
PATCHLEVEL="!PATCHLEVEL!"
RELSTATUS="!RELSTATUS!"
MACHTYPE="!MACHTYPE!"

PATH=/bin:/usr/bin:/usr/local/bin:$PATH
export PATH

TEMP=/tmp/bbug.$$

# Figure out how to echo a string without a trailing newline
N=`echo 'hi there\c'`
case "$N" in
*c)	n=-n c= ;;
*)	n= c='\c' ;;
esac

BASHTESTERS="bash-testers@po.cwru.edu"

case "$RELSTATUS" in
alpha*|beta*)	BUGBASH=chet@po.cwru.edu ;;
*)		BUGBASH=bug-bash@gnu.org ;;
esac

case "$RELSTATUS" in
alpha*|beta*)	echo "$0: This is a testing release.  Would you like your bug report"
		echo "$0: to be sent to the bash-testers mailing list?"
		echo $n "$0: Send to bash-testers? $c"
		read ans
		case "$ans" in
		y*|Y*)	BUGBASH="${BUGBASH},${BASHTESTERS}" ;;
		esac ;;
esac

BUGADDR="${1-$BUGBASH}"

: ${EDITOR=emacs}

: ${USER=${LOGNAME-`whoami`}}

trap 'rm -f $TEMP $TEMP.x; exit 1' 1 2 3 13 15
trap 'rm -f $TEMP $TEMP.x' 0

UN=
if (uname) >/dev/null 2>&1; then
	UN=`uname -a`
fi

if [ -f /usr/lib/sendmail ] ; then
	RMAIL="/usr/lib/sendmail"
elif [ -f /usr/sbin/sendmail ] ; then
	RMAIL="/usr/sbin/sendmail"
else
	RMAIL=rmail
fi

# this is raceable
rm -f $TEMP

cat > $TEMP <<EOF
From: ${USER}
To: ${BUGADDR}
Subject: [50 character or so descriptive subject here (for reference)]

Configuration Information [Automatically generated, do not change]:
Machine: $MACHINE
OS: $OS
Compiler: $CC
Compilation CFLAGS: $CFLAGS
uname output: $UN
Machine Type: $MACHTYPE

Bash Version: $RELEASE
Patch Level: $PATCHLEVEL
Release Status: $RELSTATUS

Description:
	[Detailed description of the problem, suggestion, or complaint.]

Repeat-By:
	[Describe the sequence of events that causes the problem
	to occur.]

Fix:
	[Description of how to fix the problem.  If you don't know a
	fix for the problem, don't include this section.]
EOF

# this is still raceable
rm -f $TEMP.x
cp $TEMP $TEMP.x
chmod u+w $TEMP

trap '' 2		# ignore interrupts while in editor

until $EDITOR $TEMP; do
	echo "$0: editor \`$EDITOR' exited with nonzero status."
	echo "$0: Perhaps it was interrupted."
	echo "$0: Type \`y' to give up, and lose your bug report;"
	echo "$0: type \`n' to re-enter the editor."
	echo $n "$0: Do you want to give up? $c"

	read ans
	case "$ans" in
	[Yy]*) exit 1 ;;
	esac
done

trap 'rm -f $TEMP $TEMP.x; exit 1' 2	# restore trap on SIGINT

if cmp -s $TEMP $TEMP.x
then
	echo "File not changed, no bug report submitted."
	exit
fi

echo $n "Send bug report? [y/n] $c"
read ans
case "$ans" in
[Nn]*)	exit 0 ;;
esac

${RMAIL} $BUGADDR < $TEMP || {
	cat $TEMP >> $HOME/dead.bashbug
	echo "$0: mail failed: report saved in $HOME/dead.bashbug" >&2
}

exit 0
