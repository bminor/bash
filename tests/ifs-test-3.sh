OIFS=$IFS
IFS=":$IFS"
foo=`echo a:b:c`
IFS=$OIFS

for i in $foo
do
	echo $i
done
