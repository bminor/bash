#! /bin/sh
#
# show that setting a trap on SIGCHLD is not disastrous.
#

trap 'echo caught a child death' SIGCHLD

sleep 5 &
sleep 5 &
sleep 5 &

wait

exit 0
