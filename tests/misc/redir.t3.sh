#
# Test the effect of input buffering on the shell's input
#
echo this is redir.t3.sh

exec 0< redir.t3.sub

echo after exec in redir.t3.sh
