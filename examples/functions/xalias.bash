# xalias - convert csh alias commands to bash functions
# from Mohit Aron <aron@cs.rice.edu>
# posted to usenet as <4i5p17$bnu@larry.rice.edu>
function xalias ()
{
	if [ "x$2" = "x" ] 
	then
		declare -f $1
	else
		echo $2 | egrep -q '(\!|#)'
		if [ $? -eq 0 ]
		then
			comm=$(echo $2 | sed  's/\\!\*/\"$\@\"/g
					       s/\\!:\([1-9]\)/\"$\1\"/g
				               s/#/\\#/g')
		else
			comm="$2 \"\$@\""
		fi
		eval function $1 \(\) "{" command "$comm"  "; }"
	fi
}
