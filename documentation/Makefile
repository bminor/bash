# This Makefile is for the Bash/documentation directory -*- text -*-.
#
RM	    = rm -f

INSTALL_DATA = ../support/install.sh -c -m 644

# unused
TEXINDEX    = texindex
TEX	    = tex

MAKEINFO    = makeinfo
TEXI2DVI    = ../support/texi2dvi
QUIETPS	    = #set this to -q to shut up dvips
DVIPS	    = dvips -D 300 $(QUIETPS) -o $@	# tricky
TEXINPUTS   = ./../lib/readline/doc

# Change to groff -Tascii if you don't have nroff
NROFF	    = nroff

# This should be a program that converts troff to postscript
GROFF	    = groff

HSUSER      = ./../lib/readline/doc/hsuser.texinfo
RLUSER      = ./../lib/readline/doc/rluser.texinfo

.SUFFIXES:	.1 .3 .ms .ps .txt .dvi

.1.ps:
	$(RM) $@
	${GROFF} -man $< > $@

.1.txt:
	$(RM) $@
	${NROFF} -man $< > $@

.ms.ps:
	$(RM) $@
	${GROFF} -ms $< > $@

.ms.txt:
	$(RM) $@
	${NROFF} -ms $< > $@

.3.ps:
	$(RM) $@
	${GROFF} -man $< > $@

.3.txt:
	$(RM) $@
	${NROFF} -man $< > $@

all: ps info dvi text

ps: bash.ps readline.ps article.ps
dvi: features.dvi features.ps 
info: features.info
text: bash.txt builtins.txt readline.txt article.txt

features.dvi: features.texi $(HSUSER) $(RLUSER)
	TEXINPUTS=.:$(TEXINPUTS):$$TEXINPUTS $(TEXI2DVI) features.texi

features.ps: features.dvi
	$(RM) $@
	$(DVIPS) features.dvi

features.info: features.texi $(HSUSER) $(RLUSER)
	$(MAKEINFO) --no-split -I$(TEXINPUTS) features.texi

bash.txt: bash.1
bash.ps: bash.1
builtins.txt: builtins.1 bash.1
readline.txt: readline.3
readline.ps: readline.3
article.ps: article.ms

hsuser.texinfo: ../lib/readline/doc/hsuser.texinfo
	ln -s ../lib/readline/doc/hsuser.texinfo .

rluser.texinfo: ../lib/readline/doc/rluser.texinfo
	ln -s ../lib/readline/doc/rluser.texinfo .

clean:
	$(RM) *.aux *.cp *.fn *.ky *.log *.pg *.toc *.tp *.vr *.cps *.pgs \
	*.fns *.kys *.tps *.vrs *.o core rluser.texinfo hsuser.texinfo

distclean mostlyclean:	clean

realclean maintainer-clean:	clean
	rm -f *.dvi *.info *.ps *.txt

installdirs:
	-[ -d $(mandir) ] || mkdir $(mandir)
	-[ -d $(man3dir) ] || mkdir $(man3dir)
	-[ -d $(infodir) ] || mkdir $(infodir)

install: all installdirs
	$(INSTALL_DATA) bash.1 $(mandir)
	$(INSTALL_DATA) readline.3 $(man3dir)
	$(INSTALL_DATA) features.info $(infodir)/bash.info

uninstall:
	$(RM) $(mandir)/bash.1
	$(RM) $(man3dir)/readline.3 $(infodir)/bash.info
