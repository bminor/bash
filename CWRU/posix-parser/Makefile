SRCS=	main.c parse.y make.c copy.c aux.c 
OBJS=	main.o parse.o make.o copy.o aux.o

PROG=	parse

AUX=	../../../sun4/error.o

CFLAGS=	-g -I. -I../.. -I. -DTEST -Dalloca=__builtin_alloca
LDFLAGS= -g

$(PROG):	$(OBJS)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(AUX) $(LIBS)

parse.o: parse.y
