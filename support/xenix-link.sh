:
# link bash for Xenix under SCO Unix
#
# For xenix 2.2:
#	CC="cc -xenix -lx" ./configure
#	edit config.h:
#		comment out the define for HAVE_DIRENT_H
#		enable the define for HAVE_SYS_NDIR_H to 1
#	make
#	CC="cc -xenix -lx" ./link.sh
#
# For xenix 2.3:
#	CC="cc -x2.3" ./configure
#	make
#	CC="cc -x2.3" ./link.sh

set -x

rm -f bash

if [ -z "$CC" ]
then
	if [ -f /unix -a ! -f /xenix ]
	then
		CC="cc -xenix"
	else
		CC=gcc
	fi
fi

try_dir=no
try_23=no
try_x=yes

case "$CC" in
*-ldir*) try_dir=yes ;;
esac

case "$CC" in
*-lx*) try_23=no ; try_x=yes ;;
esac

case "$CC" in
*-x2.3*|*-l2.3*) try_23=yes ; try_dir=yes ;;
esac

libs=
try="socket"
if [ $try_dir = yes ] ; then try="$try dir" ; fi
if [ $try_23 = yes ] ; then try="$try 2.3" ; fi
if [ $try_x = yes ] ; then try="$try x" ; fi
for name in $try
do
	if [ -r "/lib/386/Slib${name}.a" ] ; then libs="$libs -l$name" ; fi
done

$CC -o bash shell.o eval.o y.tab.o \
general.o make_cmd.o print_cmd.o dispose_cmd.o execute_cmd.o variables.o \
copy_cmd.o error.o expr.o flags.o nojobs.o subst.o hashcmd.o hashlib.o \
mailcheck.o trap.o input.o unwind_prot.o pathexp.o sig.o test.o \
version.o alias.o array.o braces.o bracecomp.o bashhist.o bashline.o \
getcwd.o siglist.o vprint.o oslib.o list.o stringlib.o locale.o \
xmalloc.o builtins/libbuiltins.a \
lib/readline/libreadline.a lib/readline/libhistory.a \
-ltermcap lib/glob/libglob.a lib/tilde/libtilde.a lib/malloc/libmalloc.a \
$libs

ls -l bash
