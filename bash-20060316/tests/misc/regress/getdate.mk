all:	getdate

getdate.c:	getdate.y
	yacc getdate.y
	mv y.tab.c getdate.c

getdate:	getdate.c
	$(CC) -o $@ getdate.c 
	rm -f getdate.c getdate.o
