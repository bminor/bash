# Copyright (C) 1991 Free Software Foundation, Inc.
# This file is part of the GNU C Library.

# The GNU C Library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Library General Public License
# as published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.

# The GNU C Library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Library General Public License for more details.

# You should have received a copy of the GNU Library General Public
# License along with the GNU C Library; see the file COPYING.LIB.  If
# not, write to the Free Software Foundation, Inc., 675 Mass Ave,
# Cambridge, MA 02139, USA.

# Makefile for standalone distribution of malloc.

srcdir = .
VPATH = .:$(srcdir)

all: libmalloc.a

sources = calloc.c cfree.c free.c malloc.c mcheck.c morecore.c \
	  memalign.c mstats.c mtrace.c realloc.c valloc.c
objects = calloc.o cfree.o free.o malloc.o mcheck.o morecore.o \
	  memalign.o mstats.o mtrace.o realloc.o valloc.o
headers = malloc.h getpagesize.h

libmalloc.a: $(objects)
	ar crv $@ $(objects)
	ranlib $@

.c.o:
	$(CC) $(CFLAGS) $(CPPFLAGS) -I. -c $< $(OUTPUT_OPTION)

.PHONY: clean realclean malloc-clean malloc-realclean
clean malloc-clean:
	-rm -f libmalloc.a $(objects) core
realclean malloc-realclean: clean
	-rm -f TAGS tags *~

calloc.o: malloc.h
free.o: malloc.h
malloc.o: malloc.h
mcheck.o: malloc.h
memalign.o: malloc.h
mstats.o: malloc.h
mtrace.o: malloc.h
realloc.o: malloc.h
valloc.o: malloc.h getpagesize.h
