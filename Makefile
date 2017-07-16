############################################################################
# Makefile for lzss encode/decode library and sample program
############################################################################
CC = gcc
LD = gcc
CFLAGS = -I. -O3 -Wall -Wextra -pedantic -ansi -c
LDFLAGS = -O3 -o

# libraries
LIBS = -L. -Lbitfile -Loptlist -llzss -lbitfile -loptlist

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

all:		sample$(EXE)

sample$(EXE):	sample.o liblzss.a optlist/liboptlist.a bitfile/libbitfile.a
		$(LD) $< $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c lzss.h optlist/optlist.h
		$(CC) $(CFLAGS) $<

liblzss.a:	$(LZOBJS)
		ar crv liblzss.a $(LZOBJS)
		ranlib liblzss.a

lzss.o:	lzss.c lzlocal.h bitfile/bitfile.h
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

bitfile/libbitfile.a:
		cd bitfile && $(MAKE) libbitfile.a

optlist/liboptlist.a:
		cd optlist && $(MAKE) liboptlist.a

clean:
		$(DEL) *.o
		$(DEL) *.a
		$(DEL) sample$(EXE)
		cd optlist && $(MAKE) clean
		cd bitfile && $(MAKE) clean
