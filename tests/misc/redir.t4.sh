echo "Point 1"
exec 3</etc/passwd
exec 4>a
exec 5>b
echo "Point 2"
echo to a 1>&4
echo to b 1>&5
exec 11</etc/printcap
echo "Point 3"
echo to a 1>&4
echo to b 1>&5
exit 0
