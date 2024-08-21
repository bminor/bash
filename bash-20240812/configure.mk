# Make sure the first target in the makefile is the right one
configure:	configure.ac aclocal.m4 config.h.in
	autoconf
