############################################################################
# Makefile for bitfile library and sample
#
#   $Id: Makefile,v 1.2 2007/07/10 05:32:53 michael Exp $
#   $Log: Makefile,v $
#   Revision 1.2  2007/07/10 05:32:53  michael
#   Use -pedantic option when compiling code.
#
#   Revision 1.1.1.1  2004/02/09 05:31:42  michael
#   Initial release
#
############################################################################

CC = gcc
LD = gcc
CFLAGS = -O2 -Wall -Wextra -ansi -pedantic -c
LDFLAGS = -O2 -o

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
	OS = Windows
endif

ifeq ($(OS),Windows)
	EXE = .exe
	DEL = del
else	#assume Linux/Unix
	EXE =
	DEL = rm -f
endif

all:		sample$(EXE)

sample$(EXE):	sample.o bitfile.o
		$(LD) $^ $(LDFLAGS) $@

sample.o:	sample.c bitfile.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) sample$(EXE)
		$(DEL) testfile
