#! /bin/bash
#
# Simple program to make new version files for the shell.
#
# Chet Ramey
# chet@po.cwru.edu

# Copyright (C) 1996-2002 Free Software Foundation, Inc.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

PROGNAME=`basename $0`
USAGE="$PROGNAME [-M] [-m] [-p] [version]"

while [ $# -gt 0 ]; do
	case "$1" in
	-n)	shift; debug=yes;;
	-M)	shift; major_version=yes ;;
	-m)	shift; minor_version=yes ;;
	-p)	shift; patch_level=yes ;;
	-*)	echo "$PROGNAME: usage: $USAGE" >&2 ; exit 2 ;;
	*)	break;;
	esac
done

if [ -n "$1" ]; then
	NEWVERSION="$1";
fi

DISTFILE=_distribution
PATCHFILE=_patchlevel

dist_version=`cat $DISTFILE`
PATCH=`cat $PATCHFILE`

# making new version files from a new version argument
if [ -n "$NEWVERSION" ]; then
	case "$NEWVERSION" in
	*.*.*)	PATCH=${NEWVERSION##*.} ;;
	*)	PATCH=0 ;;
	esac

	case "$NEWVERSION" in
	*.*.*)	vers=${NEWVERSION%.*} ;;
	*)	vers=${NEWVERSION} ;;
	esac

	case "$vers" in
	*.*)	MAJOR=${vers%.*} MINOR=${vers##*.} ;;
	*)	MAJOR=${vers} MINOR=0 ;;
	esac

	if [ -n "$debug" ]; then
		echo "echo ${MAJOR}.${MINOR} > $DISTFILE"
		echo "echo ${PATCH} > $PATCHFILE"
	else
		echo ${MAJOR}.${MINOR} > $DISTFILE
		echo ${PATCH} > $PATCHFILE
	fi

	echo "$PROGNAME: running autoconf..."
	autoconf

	exit 0
fi

case "$dist_version" in
*.*.*)	vers=${dist_version%.*} ;;
*.*)	vers=${dist_version} ;;
esac

case "$vers" in
*.*)	MAJOR=${vers%.*} MINOR=${vers##*.} ;;
*)	MAJOR=${vers} MINOR=0 ;;
esac

# incrementing the major version
if [ -n "$major_version" ]; then
	MAJOR=`expr $MAJOR + 1`
fi

if [ -n "$minor_version" ]; then
	$MINOR=`echo $minor_version | awk '{printf "%02d\n", $1+1}'`
fi
	
if [ -n "$patch_level" ]; then
	PATCH=`expr $PATCH + 1`
fi

if [ -n "$debug" ]; then
	echo "echo ${MAJOR}.${MINOR} > $DISTFILE"
	echo "echo ${PATCH} > $PATCHFILE"
else
	echo ${MAJOR}.${MINOR} > $DISTFILE
	echo ${PATCH} > $PATCHFILE
fi

echo "$PROGNAME: running autoconf..."
autoconf

exit 0
