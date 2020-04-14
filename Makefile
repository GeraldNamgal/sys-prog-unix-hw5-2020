#
# Makefile for smsh
#  as of 2017-03-31
#

GCC = gcc -Wall  -g


OBJS = smsh5.o splitline2.o process.o varlib.o controlflow.o builtin.o \
		flexstr2.o

smsh: $(OBJS)
	$(GCC) -o smsh $(OBJS)

builtin.o: builtin.c smsh.h varlib.h builtin.h 
	$(GCC) -c -Wall builtin.c

controlflow.o: controlflow.c smsh.h process.h 
	$(GCC) -c -Wall controlflow.c

flexstr2.o: flexstr2.c flexstr2.h splitline2.h 
	$(GCC) -c -Wall flexstr2.c

process.o: process.c smsh.h builtin.h varlib.h controlflow.h process.h 
	$(GCC) -c -Wall process.c

smsh5.o: smsh5.c smsh.h splitline2.h varlib.h process.h 
	$(GCC) -c -Wall smsh5.c

splitline2.o: splitline2.c splitline2.h smsh.h flexstr2.h 
	$(GCC) -c -Wall splitline2.c

varlib.o: varlib.c varlib.h 
	$(GCC) -c -Wall varlib.c

clean:
	rm -f *.o smsh
