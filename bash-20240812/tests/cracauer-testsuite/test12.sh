#!./testshell

echo 'Test 12 (Variant of test10):'
echo 'You should be able to end the script with one SIGINT'
(while :; do ./catcher ; done)
