#! /bin/sh
#
# update-config.sh - fetch new versions of config.guess and config.sub
# from the master GNU git tree
#
cd /usr/src/local/chet/src/bash/src/support

mv config.guess config.guess.save
mv config.sub config.sub.save

wget -v http://git.savannah.gnu.org/cgit/config.git/plain/config.guess
wget -v http://git.savannah.gnu.org/cgit/config.git/plain/config.sub
