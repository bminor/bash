#! /bin/bash
#
# aliasconv.sh - convert csh aliases to bash aliases and functions
#
# usage: aliasconv.sh
#
# Chet Ramey
# chet@po.cwru.edu
#
trap 'rm -f /tmp/cb$$.?' 0 1 2 3 6 15

T='	'

cat << \EOF >/tmp/cb$$.1
mkalias ()
{
	case $2 in
	'')	echo alias ${1}="''" ;;
	*[#\!]*)
		comm=`echo $2 | sed  's/\\!\*/"$\@"/g
				      s/\\!:\([1-9]\)/"$\1"/g
			              s/#/\#/g'`
		echo $1 \(\) "{" command "$comm"  "; }"
		;;
	*)	echo alias ${1}=\'`echo "${2}" | sed "s:':'\\\\\\\\'':"`\' ;;
	esac
}
EOF

# the first thing we want to do is to protect single quotes in the alias,
# since they whole thing is going to be surrounded by single quotes when
# passed to mkalias

sed -e "s:':\\'\\\'\\':" -e "s/^\([a-zA-Z0-9_-]*\)$T\(.*\)$/mkalias \1 '\2'/" >>/tmp/cb$$.1

sh /tmp/cb$$.1 | sed -e 's/\$cwd/\$PWD/g' \
		     -e 's/\$term/\$TERM/g' \
		     -e 's/\$home/\$HOME/g' \
		     -e 's/\$user/\$USER/g' \
		     -e 's/\$prompt/\$PS1/g'

exit 0
