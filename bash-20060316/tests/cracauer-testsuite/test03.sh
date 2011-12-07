#!./testshell 

echo 'Test 3: A background job is being started, then the shell loops.'
echo '        You should be able to break the shell loop with C-c.'
echo '        This goes wrong if the shell blocks signals when'
echo '        starting any child. It should do so only for foreground'
echo '        jobs.'

if [ $ZSH_VERSION ] ; then
    source lib.sh
else
    . lib.sh
fi

echo Starting job
gzip < /dev/zero | wc &
echo 'Now try to break this loop'
endless
