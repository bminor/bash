# Hey Emacs, this Makefile is in -*- makefile -*- mode!
#
# Makefile for Bash.
# If your cpp doesn't like -P, just get rid of it (the -P, not cpp).
# If you wish to use Gcc, then type `make CC=gcc CPPNAME='$(CC) -E''.
# If you wish to use GNU's Make, then change `MAKE'.
# If you don't like the destination, then change `bindir'.
# The file that you most likely want to look at is cpp-Makefile.
#
# If you haven't read README, now might be a good time.

# Include some boilerplate Gnu makefile definitions.
prefix = /usr/local
exec_prefix = $(prefix)
bindir = $(exec_prefix)/bin
srcdir = .
VPATH = $(srcdir)

# MAKE    = make
RM      = rm -f
SHELL   = /bin/sh
GAWK     = awk
# GAWK     = gawk

# Force CPPNAME to be the name of your C preprocesor if Bash can't
# find it.  For instance, `CPPNAME=/usr/libexec/cpp' on 4.4 BSD.
# If all else fails, set CPPNAME=$(CC) -E
CPPNAME =
CPP     = `$(SHELL) $(CPPMAGIC) $(GETCPPSYMS) "$(CPPNAME)"` -P

CPP_MAKEFILE = $(srcdir)/cpp-Makefile
ANSI_MAKEFILE = ansi-Makefile

# CPPFLAGS = $(SYSTEM) $(CPP_DEFINES)
CPPFLAGS = $(CPP_DEFINES) -I. -I$(srcdir)
CPP_ARGS = -DCPP_CC="$(CC)"

SUPPORTDIR     = ./support/
SUPPORTSRC     = $(srcdir)/support/

MKSYSDEFS      = $(SUPPORTSRC)mksysdefs
CPPMAGIC       = $(SUPPORTSRC)cppmagic
CAT_S          = $(SUPPORTSRC)cat-s
GETCPPSYMS     = $(SUPPORTDIR)getcppsyms
GETCPPSYMS_SRC = $(SUPPORTSRC)getcppsyms.c

# Here is a command which compresses runs of multiple blank lines to a
# single blank line.  "cat -s" works for BSD systems, but not for USG
# systems.  You can use an awk script if you like.  If you have too
# much trouble with this, just forget it.  It is for making
# bash-Makefile pretty and readable; something that isn't strictly
# necessary.
# SQUASH_BLANKS = cat -s
#
SQUASH_BLANKS = $(GAWK) -f $(CAT_S)

all:	.notified bash-Makefile
	$(MAKE) -f bash-Makefile $(MFLAGS) $(MAKEARGS) srcdir=$(srcdir)

bash-Makefile: $(CPP_MAKEFILE) Makefile machines.h sysdefs.h config.h
	@-if [ -f ansi-Makefile ]; then \
	    echo "cp ansi-Makefile tmp-Makefile.c"; \
	    cp ansi-Makefile tmp-Makefile.c; else \
	    echo "cp $(CPP_MAKEFILE) tmp-Makefile.c"; \
	    cp $(CPP_MAKEFILE) tmp-Makefile.c; \
	  fi
	$(RM) $(GETCPPSYMS)
	$(SHELL) $(SUPPORTSRC)mkdirs support
	$(CC) -o $(GETCPPSYMS) $(GETCPPSYMS_SRC)
	rm -f bash-Makefile
	@$(SHELL) -c 'echo $(CPP) $(CPPFLAGS) $(CPP_ARGS) tmp-Makefile.c \| $(SQUASH_BLANKS) \> bash-Makefile'
	@$(SHELL) -c '$(CPP) $(CPPFLAGS) $(CPP_ARGS) tmp-Makefile.c | $(SQUASH_BLANKS) >bash-Makefile'
	rm -f tmp-Makefile.c
	@test -s bash-Makefile || { rm -f bash-Makefile ; exit 1; }

sysdefs.h: $(MKSYSDEFS)
	$(SHELL) $(MKSYSDEFS) -s $(srcdir)

# This is also performed by support/mksysdefs, but there's no way to change
# it if cpp-Makefile is changed without changing anything else, since there
# are no dependencies.  This lets you run `make ansi-Makefile'.
ansi-Makefile: $(CPP_MAKEFILE)
	grep -v '/\*\*/' $(CPP_MAKEFILE) > $@

# Subsequent lines contain targets that are correctly handled by an
# existing bash-Makefile.

install uninstall newversion architecture: bash-Makefile
	$(MAKE) -f bash-Makefile $(MFLAGS) $(MAKEARGS) bindir=$(bindir) \
		prefix=$(prefix) $@

tests DEFINES tags documentation: bash-Makefile directory-frob
	$(MAKE) -f bash-Makefile $(MFLAGS) $(MAKEARGS) $@

clean distclean realclean mostlyclean maintainer-clean: bash-Makefile directory-frob
	rm -f .notified
	$(MAKE) -f bash-Makefile $(MFLAGS) $(MAKEARGS) $@

directory-frob:

.NOEXPORT:

.notified:
	@echo ""
	@echo "   You are about to make this version of GNU Bash for"
	@echo "   this architecture for the first time.  If you haven't"
	@echo "   yet read the README file, you may want to do so.  If"
	@echo "   you wish to report a bug in Bash, or in the installation"
	@echo "   procedure, please run the bashbug script and include:"
	@echo ""
	@echo "	* a description of the bug,"
	@echo "	* a recipe for recreating the bug reliably,"
	@echo "	* a fix for the bug if you have one!"
	@echo ""
	@touch .notified
