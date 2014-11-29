############################################################################
# Makefile for lzss encode/decode library and sample program
############################################################################
CC = gcc
LD = gcc
CFLAGS = -I. -O3 -Wall -Wextra -pedantic -ansi -c
LDFLAGS = -O3 -o

# libraries
LIBS = -L. -llzss -loptlist

# Treat NT and non-NT windows the same
ifeq ($(OS),Windows_NT)
	OS = Windows
endif

ifeq ($(OS),Windows)
	ifeq ($(OSTYPE), cygwin)
		EXE = .exe
		DEL = rm
	else
		EXE = .exe
		DEL = del
	endif
else	#assume Linux/Unix
	EXE =
	DEL = rm -f
endif

# define the method to be used for searching for matches (choose one)
# brute force
# FMOBJ = brute.o

# linked list
# FMOBJ = list.o

# hash table
# FMOBJ = hash.o

# Knuth–Morris–Pratt search
# FMOBJ = kmp.o

# binary tree
FMOBJ = tree.o

LZOBJS = $(FMOBJ) lzss.o

all:		sample$(EXE) liblzss.a liboptlist.a

sample$(EXE):	sample.o liblzss.a liboptlist.a
		$(LD) $< $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c lzss.h optlist.h
		$(CC) $(CFLAGS) $<

liblzss.a:	$(LZOBJS) bitfile.o
		ar crv liblzss.a $(LZOBJS) bitfile.o
		ranlib liblzss.a

lzss.o:	lzss.c lzlocal.h bitfile.h
		$(CC) $(CFLAGS) $<

brute.o:	brute.c lzlocal.h
		$(CC) $(CFLAGS) $<

list.o:		list.c lzlocal.h
		$(CC) $(CFLAGS) $<

hash.o:		hash.c lzlocal.h
		$(CC) $(CFLAGS) $<

kmp.o:		kmp.c lzlocal.h
		$(CC) $(CFLAGS) $<

tree.o:		tree.c lzlocal.h
		$(CC) $(CFLAGS) $<

bitfile.o:	bitfile.c bitfile.h
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
