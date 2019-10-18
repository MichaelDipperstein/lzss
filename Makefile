############################################################################
# Makefile for bitfile library and sample
#
############################################################################

CC = gcc
LD = gcc
CFLAGS = -O2 -Wall -Wextra -ansi -pedantic -c
LDFLAGS = -O2 -o

# libraries
LIBS = -L. -lbitfile

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

all:	sample$(EXE)

sample$(EXE):	sample.o libbitfile.a
	$(LD) $< $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c bitfile.h
	$(CC) $(CFLAGS) $<

libbitfile.a:	bitfile.o
	ar crv libbitfile.a bitfile.o
	ranlib libbitfile.a

bitfile.o:	bitfile.c bitfile.h
	$(CC) $(CFLAGS) $<

docs:		doxygen.conf bitfile.c bitfile.h sample.c
		rm -rf docs
		doxygen $<

clean:
	$(DEL) *.o
	$(DEL) *.a
	$(DEL) sample$(EXE)
	$(DEL) testfile
