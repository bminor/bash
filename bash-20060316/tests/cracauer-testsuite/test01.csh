#!./testcsh -f

echo 'Test 1: See whether child can work on SIGINT and SIGQUIT without'
echo '        terminating the shell around it. See if the shell is'
echo '        interruptable afterwards'

source lib.csh

docatcher
echo "No try to exit shell loop with C-c, C-\ or SIGTERM"
while (1)
end
