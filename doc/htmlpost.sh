#! /bin/sh
#
# Some of these should really be done by man2html
#
# The ~/xxx links don't really work -- netscape doesn't expand the home
# directory of the user running navigator
#

sed -e 's|<B>gnu.bash.bug</B>|<A HREF="news:gnu.bash.bug">gnu.bash.bug</A>|g' \
    -e 's|<[iI]>/bin/bash</[iI]>|<A HREF="file:/bin/bash"><i>/bin/bash</i></A>|g' \
    -e 's|<[iI]>/etc/profile</[iI]>|<A HREF="file:/etc/profile"><i>/etc/profile</i></A>|g' \
    -e 's|<[iI]>~/.bash_profile</[iI]>|<A HREF="file:~/.bash_profile"><i>~/.bash_profile</i></A>|g' \
    -e 's|<[iI]>~/.bash_login</[iI]>|<A HREF="file:~/.bash_login"><i>~/.bash_login</i></A>|g' \
    -e 's|<[iI]>~/.profile</[iI]>|<A HREF="file:~/.profile"><i>~/.profile</i></A>|g' \
    -e 's|<[iI]>~/.bashrc</[iI]>|<A HREF="file:~/.bashrc"><i>~/.bashrc</i></A>|g' \
    -e 's|<[iI]>~/.bash_logout</[iI]>|<A HREF="file:~/.bash_logout"><i>~/.bash_logout</i></A>|g' \
    -e 's|<[iI]>~/.bash_history</[iI]>|<A HREF="file:~/.bash_history"><i>~/.bash_history</i></A>|g' \
    -e 's|<[iI]>~/.inputrc</[iI]>|<A HREF="file:~/.inputrc"><i>~/.inputrc</i></A>|g' \
    -e 's|<[iI]>/etc/inputrc</[iI]>|<A HREF="file:/etc/inputrc"><i>/etc/inputrc</i></A>|g'
