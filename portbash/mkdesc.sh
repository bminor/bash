#! /bin/sh

if [ -f /unix ]; then
	echo '#define M_OS "USG"'
fi

sh signals.sh
sh stdio.sh
sh strings.sh
sh syscalls.sh
sh libc.sh
