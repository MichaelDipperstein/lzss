############################################################################
#   Makefile for oplist command line library sample
############################################################################
CC = gcc
LD = gcc
CFLAGS = -I. -O3 -Wall -Wextra -pedantic -ansi -c
LDFLAGS = -O3 -o

CXX = g++
CXXFLAGS = -I. -O3 -Wall -Wextra -c

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

all:	sample$(EXE) sample_cpp$(EXE) liboptlist.a

sample$(EXE):	sample.o liboptlist.a
	$(LD) $< $(LIBS) $(LDFLAGS) $@

sample.o:	sample.c optlist.h
	$(CC) $(CFLAGS) $<

sample_cpp$(EXE):	sample_cpp.o liboptlist.a
	$(CXX) $< $(LIBS) $(LDFLAGS) $@

sample_cpp.o:	sample.cpp optlist.h
	$(CXX) $(CXXFLAGS) $< -o $@

liboptlist.a:	optlist.o
	ar crv liboptlist.a optlist.o
	ranlib liboptlist.a

optlist.o:	optlist.c optlist.h
	$(CC) $(CFLAGS) $<

clean:
	$(DEL) *.o
	$(DEL) sample$(EXE) sample_cpp$(EXE) liboptlist.a
