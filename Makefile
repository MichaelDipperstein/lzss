############################################################################
# Makefile for oplist command line library sample
#
#   $Id: Makefile,v 1.1.1.2 2007/09/04 04:45:42 michael Exp $
#   $Log: Makefile,v $
#   Revision 1.1.1.2  2007/09/04 04:45:42  michael
#   Added FreeOptList.
#
#   Revision 1.1.1.1  2007/08/07 05:01:48  michael
#   Initial Release
#
#
############################################################################
CC = gcc
LD = gcc
CFLAGS = -I. -O3 -Wall -Wextra -pedantic -ansi -c
LDFLAGS = -O3 -o

# libraries
LIBS = -L. -loptlist

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
    OS = Windows
endif

ifeq ($(OS),Windows)
    EXE = .exe
    DEL = del
else    #assume Linux/Unix
    EXE =
    DEL = rm -f
endif

all:		sample$(EXE) liboptlist.a

sample$(EXE):	sample.o liboptlist.a
	$(LD) $< $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c optlist.h
	$(CC) $(CFLAGS) $<

liboptlist.a:	optlist.o
	ar crv liboptlist.a optlist.o
	ranlib liboptlist.a

optlist.o:	optlist.c optlist.h
	$(CC) $(CFLAGS) $<

clean:
	$(DEL) *.o
	$(DEL) *.a
	$(DEL) sample$(EXE)
