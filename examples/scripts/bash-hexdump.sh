#From: "dennis" <dennis@netstrata.com>
#To: <bash-maintainers@gnu.org>
#Subject: New example script: bash-hexdump
#Date: Mon, 4 Jan 2010 22:48:19 -0700
#Message-ID: <6dbec42d$64fcdbd2$4a32cf2d$@com>

#I've written a script that functions like "hexdump -C" or "hd". If you'd 
#like to include it in a future distribution of example Bash scripts, I have 
#included it here: 

#!/bin/bash
# bash-hexdump# pure Bash, no externals
# by Dennis Williamson - 2010-01-04
# in response to 
http://stackoverflow.com/questions/2003803/show-hexadecimal-numbers-of-a-file
# usage: bash-hexdump file
saveIFS="$IFS"
IFS=""                     # disables interpretation of \t, \n and space
saveLANG="$LANG"
LANG=C                     # allows characters > 0x7F
bytecount=0
valcount=0
printf "%08x  " $bytecount
while read  -d '' -r -n 1 char    # -d '' allows newlines, -r allows \
do
   ((bytecount++))
   # for information about the apostrophe in this printf command, see
   #  http://www.opengroup.org/onlinepubs/009695399/utilities/printf.html
   printf -v val "%02x" "'$char"
   echo -n "$val "
   ((valcount++))
   if [[ "$val" < 20 || "$val" > 7e ]]
   then
       string+="."                  # show unprintable characters as a dot
   else
       string+=$char
   fi
   if (( bytecount % 8 == 0 ))      # add a space down the middle
   then
       echo -n " "
   fi
   if (( bytecount % 16 == 0 ))   # print 16 values per line
   then
       echo "|$string|"
       string=''
       valcount=0
       printf "%08x  " $bytecount
   fi
done < "$1"

if [[ "$string" != "" ]]            # if the last line wasn't full, pad it out
then
   length=${#string}
   if (( length > 7 ))
   then
       ((length--))
   fi
   (( length += (16 - valcount) * 3 + 4))
   printf "%${length}s\n" "|$string|"
   printf "%08x  " $bytecount
fi
echo

LANG="$saveLANG";
IFS="$saveIFS"

exit 0
