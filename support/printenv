#! /bin/sh -

if [ $# -eq 0 ]; then
	env
	exit
elif eval [ "\${$1-unset}" = "unset" ]; then
	exit 1
else
	eval echo \$$1
	exit 0
fi
