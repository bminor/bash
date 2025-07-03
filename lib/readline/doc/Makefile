# Derived by hand from the generated readline-src/doc/Makefile
# This makefile for Readline library documentation is in -*- text -*- mode.
# Emacs likes it that way.

#   Copyright (C) 1996-2002 Free Software Foundation, Inc.
#
#   This program is free software: you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation, either version 3 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

topdir = .
srcdir = .
VPATH = .

prefix = /usr/local
infodir = ${prefix}/info

mandir = ${prefix}/man
manpfx = man

man1ext = 1
man1dir = $(mandir)/$(manpfx)$(man1ext)
man3ext = 3
man3dir = $(mandir)/$(manpfx)$(man3ext)

SHELL = /bin/sh
RM = rm -f

INSTALL = /usr/bin/install -c
INSTALL_DATA = ${INSTALL} -m 644

BUILD_DIR = .
TEXINPUTDIR = $(srcdir)

MAKEINFO    = LANGUAGE= makeinfo
TEXI2DVI    = $(srcdir)/texi2dvi
TEXI2HTML   = $(srcdir)/texi2html
QUIETPS     = #set this to -q to shut up dvips
PSDPI       = 300	# I don't have any 600-dpi printers
DVIPS       = dvips -D ${PSDPI} $(QUIETPS) -o $@     # tricky
DVIPDF      = dvipdfm -o $@ -p ${PAPERSIZE}
PSPDF       = gs -sPAPERSIZE=${PAPERSIZE} -sDEVICE=pdfwrite -dNOPAUSE -dBATCH -sOutputFile=$@
# experimental; uses external texi2dvi for now; this needs pdftex to be present
TEXI2PDF    = texi2dvi --pdf

RLSRC = $(srcdir)/rlman.texi $(srcdir)/rluser.texi \
	$(srcdir)/rltech.texi $(srcdir)/version.texi \
	$(srcdir)/rluserman.texi
HISTSRC = $(srcdir)/history.texi $(srcdir)/hsuser.texi \
	  $(srcdir)/hstech.texi $(srcdir)/version.texi

# This should be a program that converts troff to an ascii-readable format
NROFF       = groff -Tascii -P -c

# This should be a program that converts troff to postscript
GROFF       = groff

DVIOBJ = readline.dvi history.dvi rluserman.dvi
INFOOBJ = readline.info history.info rluserman.info
PSOBJ = readline.ps history.ps rluserman.ps
HTMLOBJ = readline.html history.html rluserman.html
TEXTOBJ = readline.0 history.0
PDFOBJ = readline.pdf history.pdf rluserman.pdf readline_3.pdf history_3.pdf

INTERMEDIATE_OBJ = rlman.dvi

CREATED_DOCS = $(DVIOBJ) $(INFOOBJ) $(PSOBJ) $(HTMLOBJ) $(PDFOBJ) $(TEXTOBJ)

.SUFFIXES:      .0 .3 .ps .txt .dvi .html .pdf

.3.0:
	$(RM) $@
	-${NROFF} -man $< > $@

.3.pdf:
	$(RM) $@
	-${GROFF} -man -T pdf $< > $@

.3.ps:
	$(RM) $@
	-${GROFF} -man $< > $@

#.ps.pdf:
#	$(RM) $@
#	-${PSPDF} $<

.dvi.pdf:
	$(RM) $@
	-${DVIPDF} $<

all: info html text pdf ps dvi
nodvi: info html text
pdf: $(PDFOBJ)

readline.dvi: $(RLSRC)
	$(RM) $@
	TEXINPUTS=.:$(TEXINPUTDIR):$$TEXINPUTS $(TEXI2DVI) -o $@ $(srcdir)/rlman.texi

readline.info: $(RLSRC)
	$(RM) $@
	$(MAKEINFO) --no-split -I $(TEXINPUTDIR) -o $@ $(srcdir)/rlman.texi

readline.pdf: $(RLSRC)
	$(RM) $@
	$(TEXI2PDF) -I $(TEXINPUTDIR) -o $@ $(srcdir)/rlman.texi

rluserman.dvi: $(RLSRC)
	$(RM) $@
	TEXINPUTS=.:$(TEXINPUTDIR):$$TEXINPUTS $(TEXI2DVI) $(srcdir)/rluserman.texi

rluserman.info: $(RLSRC)
	$(RM) $@
	$(MAKEINFO) --no-split -I $(TEXINPUTDIR) -o $@ $(srcdir)/rluserman.texi

rluserman.pdf: $(RLSRC)
	$(RM) $@
	$(TEXI2PDF) -I $(TEXINPUTDIR) -o $@ $(srcdir)/rluserman.texi

history.dvi: ${HISTSRC}
	$(RM) $@
	TEXINPUTS=.:$(TEXINPUTDIR):$$TEXINPUTS $(TEXI2DVI) $(srcdir)/history.texi

history.info: ${HISTSRC}
	$(RM) $@
	$(MAKEINFO) --no-split -I $(TEXINPUTDIR) -o $@ $(srcdir)/history.texi

history.pdf: $(HISTSRC)
	$(RM) $@
	$(TEXI2PDF) -I $(TEXINPUTDIR) -o $@ $(srcdir)/history.texi

readline.ps:	readline.dvi
	$(RM) $@
	$(DVIPS) readline.dvi

rluserman.ps:	rluserman.dvi
	$(RM) $@
	$(DVIPS) rluserman.dvi

history.ps:	history.dvi
	$(RM) $@
	$(DVIPS) history.dvi

readline.html:	${RLSRC}
	$(RM) $@
	$(TEXI2HTML) -menu -monolithic -I $(TEXINPUTDIR) -o $@ $(srcdir)/rlman.texi

rluserman.html:	${RLSRC}
	$(TEXI2HTML) -menu -monolithic -I $(TEXINPUTDIR) $(srcdir)/rluserman.texi

history.html:	${HISTSRC}
	$(TEXI2HTML) -menu -monolithic -I $(TEXINPUTDIR) $(srcdir)/history.texi

history.0: $(srcdir)/history.3
readline.0: $(srcdir)/readline.3

history_3.pdf: $(srcdir)/history.3
	$(RM) $@
	-${GROFF} -man -T pdf $< > $@

readline_3.pdf: $(srcdir)/readline.3
	$(RM) $@
	-${GROFF} -man -T pdf $< > $@

info:	$(INFOOBJ)
dvi:	$(DVIOBJ)
ps:	$(PSOBJ)
html:	$(HTMLOBJ)
text:	$(TEXTOBJ)
pdf:	$(PDFOBJ)

clean:
	$(RM) *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr *.cps *.pgs \
	      *.fns *.kys *.tps *.vrs *.bt *.bts *.o core *.core

distclean: clean
	$(RM) $(CREATED_DOCS)
	$(RM) $(INTERMEDIATE_OBJ)

mostlyclean: clean

maintainer-clean: distclean

install:
	@echo "This documentation should not be installed."

uninstall:
