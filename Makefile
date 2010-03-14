############################################################################
# Makefile for lzss encode/decode library and sample program
#
#   $Id: Makefile,v 1.2 2004/02/22 17:12:41 michael Exp $
#   $Log: Makefile,v $
#   Revision 1.2  2004/02/22 17:12:41  michael
#   - Separated encode/decode, match finding, and main.
#   - Use bitfiles for reading/writing files
#   - Use traditional LZSS encoding where the coded/uncoded bits
#     precede the symbol they are associated with, rather than
#     aggregating the bits.
#
#   Revision 1.1.1.1  2004/01/21 06:25:49  michael
#   Initial version
#
#
############################################################################
CC = gcc
LD = gcc
CFLAGS = -O2 -Wall -ansi -c
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
	DEL = rm
endif

# define the method to be used for searching for matches (choose one)
# brute force
FMOBJ = brute.o
# linked list
# FMOBJ = list.o
# hash table
# FMOBJ = hash.o

all:		lzsample$(EXE)

lzsample$(EXE):	lzsample.o lzss.o bitfile.o getopt.o $(FMOBJ)
		$(LD) $^ $(LDFLAGS) $@

lzsample.o:	lzsample.c lzss.h getopt.h
		$(CC) $(CFLAGS) $<

lzss.o:		lzss.c lzlocal.h bitfile.h
		$(CC) $(CFLAGS) $<

brute.o:	brute.c lzlocal.h
		$(CC) $(CFLAGS) $<

list.o:	        list.c lzlocal.h
		$(CC) $(CFLAGS) $<

hash.o:	        hash.c lzlocal.h
		$(CC) $(CFLAGS) $<

getopt.o:	getopt.c getopt.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) lzsample$(EXE)
