#!/bin/sh -
#
# bashbug - create a bug report and mail it to bug-bash@prep.ai.mit.edu
#
# configuration section:
#	these variables are filled in by the make target in cpp-Makefile
#
MACHINE="@MACHINE@"
OS="@OS@"
CC="@CC@"
CFLAGS="@CFLAGS@"
RELEASE="@RELEASE@"
PATCHLEVEL="@PATCHLEVEL@"

PATH=/bin:/usr/bin:usr/local/bin:$PATH
export PATH

TEMP=/tmp/bashbug.$$

BUGADDR=${1-bug-bash@prep.ai.mit.edu}

: ${EDITOR=emacs}

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

Bash Version: $RELEASE
Patch Level: $PATCHLEVEL

Description:
        [Detailed description of the problem, suggestion, or complaint.]

Repeat-By:
        [Describe the sequence of events that causes the problem
        to occur.]

Fix:
        [Description of how to fix the problem.  If you don't know a
        fix for the problem, don't include this section.]
EOF

chmod u+w $TEMP
cp $TEMP $TEMP.x

if $EDITOR $TEMP
then
	if cmp -s $TEMP $TEMP.x
	then
		echo "File not changed, no bug report submitted."
		exit
	fi

	${RMAIL} $BUGADDR < $TEMP || {
		cat $TEMP >> $HOME/dead.bashbug
		echo "$0: mail failed: report saved in $HOME/dead.bashbug" >&2
	}
fi

exit 0
