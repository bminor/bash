#
# varenv.sh
#
# Test the behavior of the shell with respect to variable and environment
# assignments
#
expect()
{
	echo expect "$@"
}

a=1
b=2
c=3
d=4
e=5
f=6 g=7 h=8

a=3 b=4 $CHMOD $MODE $FN

# This should echo "3 4" according to Posix.2
expect "3 4"
echo $a $b

set -k

# Assignment statements made when no words are left affect the shell's
# environment
a=5 b=6 $CHMOD c=7 $MODE d=8 $FN e=9

expect "5 6 7 8 9"
echo $a $b $c $d $e

$CHMOD f=7 $MODE g=8 $FN h=9
expect "7 8 9"
echo $f $g $h

set +k

# The temporary environment does not affect variable expansion, only the
# environment given to the command

export HOME=/usr/chet
expect $HOME
echo $HOME

expect $HOME
HOME=/a/b/c /bin/echo $HOME

expect $HOME
echo $HOME

# This should echo /a/b/c
expect /a/b/c
HOME=/a/b/c printenv HOME

set -k

# This should echo $HOME 9, NOT /a/b/c 9

expect "$HOME"
HOME=/a/b/c /bin/echo $HOME c=9
expect "$HOME 7"
echo $HOME $c

# I claim the next two echo calls should give identical output.
# ksh agrees, the System V.3 sh does not

expect "/a/b/c 9 /a/b/c"
HOME=/a/b/c $ECHO a=$HOME c=9
echo $HOME $c $a

expect "/a/b/c 9 /a/b/c"
HOME=/a/b/c a=$HOME c=9
echo $HOME $c $a
set +k

# How do assignment statements affect subsequent assignments on the same
# line?
expect "/a/b/c /a/b/c"
HOME=/a/b/c a=$HOME
echo $HOME $a

# The system V.3 sh does this wrong; the last echo should output "1 1",
# but the system V.3 sh has it output "2 2".  Posix.2 says the assignment
# statements are processed left-to-right.  bash and ksh output the right
# thing
c=1
d=2
expect "1 2"
echo $c $d
d=$c c=$d
expect "1 1"
echo $c $d
