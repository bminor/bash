#!./testcsh -f

echo 'Test 2: You should not be able to end cat using anything besides SIGKILL'

source lib.csh

onintr -

while (1)
    echo cat
    cat
    echo Exited with $?
end
