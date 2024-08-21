## -*- text -*- ####################################################
#								   #
# Makefile for the GNU Tilde Library.				   #
#								   #
####################################################################

# This Makefile is hand made from a template file, found in
# ../template.  Each library must provide several Makefile
# targets: `all', `clean', `documentation', `install', and
# `what-tar'.  The `what-tar' target reports the names of the
# files that need to be included in a tarfile to build the full
# code and documentation for this library.

# Please note that the values for INCLUDES, CC, AR, RM, CP,
# RANLIB, and selfdir are passed in from ../Makefile, and do
# not need to be defined here.
RM = rm

srcdir = .
VPATH = .:$(srcdir)

# Here is a rule for making .o files from .c files that doesn't force
# the type of the machine (like -sun3) into the flags.
.c.o:
	$(CC) -c $(CFLAGS) $(INCLUDES) $(LOCAL_DEFINES) $(CPPFLAGS) $<

# LOCAL_DEFINES are flags that are specific to this library.
# Define -DUSG if you are using a System V operating system.
LOCAL_DEFINES = $(LOCAL_INCLUDES) #-DUSG

# For libraries which include headers from other libraries.
LOCAL_INCLUDES = -I..

# The name of the library target.
LIBRARY_NAME = libtilde.a

# The C code source files for this library.
CSOURCES = $(srcdir)/tilde.c

# The header files for this library.
HSOURCES = $(srcdir)/tilde.h

OBJECTS = tilde.o

# The texinfo files which document this library.
DOCSOURCE = doc/tilde.texi
DOCOBJECT = doc/tilde.dvi
DOCSUPPORT = doc/Makefile
DOCUMENTATION = $(DOCSOURCE) $(DOCOBJECT) $(DOCSUPPORT)

SUPPORT = Makefile ChangeLog $(DOCSUPPORT)

SOURCES  = $(CSOURCES) $(HSOURCES) $(DOCSOURCE)

THINGS_TO_TAR = $(SOURCES) $(SUPPORT)

######################################################################

all: $(LIBRARY_NAME)

$(LIBRARY_NAME): $(OBJECTS)
	$(RM) -f $@
	$(AR) cq $@ $(OBJECTS)
	-[ -n "$(RANLIB)" ] && $(RANLIB) $@

what-tar:
	@for file in $(THINGS_TO_TAR); do \
	  echo $(selfdir)$$file; \
	done

documentation: force
	-(cd doc; $(MAKE) $(MFLAGS))
force:

# The rule for 'includes' is written funny so that the if statement
# always returns TRUE unless there really was an error installing the
# include files.
install:
	-$(MV) $(bindir)/$(LIBRARY_NAME) $(bindir)/$(LIBRARY_NAME)-old
	$(CP) $(LIBRARY_NAME) $(bindir)/$(LIBRARY_NAME)
	-[ -n "$(RANLIB)" ] && $(RANLIB) -t $(bindir)/$(LIBRARY_NAME)

clean:
	$(RM) -f $(OBJECTS) $(LIBRARY_NAME)
	-(cd doc; $(MAKE) $(MFLAGS) clean)


######################################################################
#								     #
#  Dependencies for the object files which make up this library.     #
#								     #
######################################################################

tilde.o: tilde.h tilde.c
