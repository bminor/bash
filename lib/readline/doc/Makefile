# This makefile for History library documentation is in -*- text -*- mode.
# Emacs likes it that way.

DOC_SUPPORT = ../../doc-support/
TEXINDEX    = $(DOC_SUPPORT)/texindex

TEX    = tex

RLSRC = rlman.texinfo rluser.texinfo rltech.texinfo
HISTSRC = hist.texinfo hsuser.texinfo hstech.texinfo

DVIOBJ = readline.dvi history.dvi
INFOOBJ = readline.info history.info
PSOBJ = readline.ps history.ps

all: info dvi

readline.dvi: $(RLSRC)
	$(TEX) rlman.texinfo
	$(TEXINDEX) rlman.??
	$(TEX) rlman.texinfo
	mv rlman.dvi readline.dvi

readline.info: $(RLSRC)
	makeinfo rlman.texinfo

history.dvi: ${HISTSRC}
	$(TEX) hist.texinfo
	$(TEXINDEX) hist.??
	$(TEX) hist.texinfo
	mv hist.dvi history.dvi

history.info: ${HISTSRC}
	makeinfo hist.texinfo

readline.ps:	readline.dvi
	dvips -D 300 -o $@ readline.dvi

history.ps:	history.dvi
	dvips -D 300 -o $@ history.dvi

info:	$(INFOOBJ)
dvi:	$(DVIOBJ)
ps:	$(PSOBJ)


$(TEXINDEX):
	(cd $(DOC_SUPPORT); $(MAKE) $(MFLAGS) CFLAGS='$(CFLAGS)' texindex)

distclean mostlyclean clean:
	rm -f *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr *.cps *.pgs \
	      *.fns *.kys *.tps *.vrs *.o core

maintainer-clean realclean:	clean
	rm -f *.dvi *.info *.info-*  *.ps
