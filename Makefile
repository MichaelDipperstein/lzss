############################################################################
# Makefile for lzss encode/decode library and sample program
#
#   $Id: Makefile,v 1.3 2004/11/08 05:54:18 michael Exp $
#   $Log: Makefile,v $
#   Revision 1.3  2004/11/08 05:54:18  michael
#   1. Split encode and decode routines for smarter linking
#   2. Renamed lzsample.c sample.c to match my other samples
#   3. Makefile now builds code as libraries for better LGPL compliance.
#
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
CFLAGS = -I. -O3 -Wall -ansi -c
LDFLAGS = -O3 -o

# libraries
LIBS = -L. -llzss -lgetopt

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
# FMOBJ = brute.o
# linked list
# FMOBJ = list.o
# hash table
FMOBJ = hash.o

LZOBJS = $(FMOBJ) lzencode.o lzdecode.o lzvars.o

all:		sample$(EXE) liblzss.a libgetopt.a

sample$(EXE):	sample.o liblzss.a libgetopt.a
		$(LD) $< $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c lzss.h getopt.h
		$(CC) $(CFLAGS) $<

liblzss.a:	$(LZOBJS) bitfile.o
		ar crv liblzss.a $(LZOBJS) bitfile.o
		ranlib liblzss.a

lzencode.o:	lzencode.c lzlocal.h bitfile.h
		$(CC) $(CFLAGS) $<

lzdecode.o:	lzdecode.c lzlocal.h bitfile.h
		$(CC) $(CFLAGS) $<

brute.o:	brute.c lzlocal.h
		$(CC) $(CFLAGS) $<

list.o:	        list.c lzlocal.h
		$(CC) $(CFLAGS) $<

hash.o:	        hash.c lzlocal.h
		$(CC) $(CFLAGS) $<

lzvars.o:	lzvars.c lzlocal.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
		$(CC) $(CFLAGS) $<

libgetopt.a:	getopt.o
		ar crv libgetopt.a getopt.o
		ranlib libgetopt.a

getopt.o:	getopt.c getopt.h
		$(CC) $(CFLAGS) $<

comp$(EXE):	comp.o $(FMOBJ) lzencode.o lzvars.o bitfile.o
		$(LD) $^ $(LDFLAGS) $@

comp.o:		comp.c lzss.h
		$(CC) $(CFLAGS) $<

decomp$(EXE):	decomp.o lzdecode.o lzvars.o bitfile.o 
		$(LD) $^ $(LDFLAGS) $@

decomp.o:	decomp.c lzss.h
		$(CC) $(CFLAGS) $<

clean:
		$(DEL) *.o
		$(DEL) *.a
		$(DEL) sample$(EXE)
