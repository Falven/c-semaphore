#	Makefile for Readers and Writers
#	Francisco Aguilera
#	April 23, 2013

LIBSRC = usersem.c
LIBOBJ = usersem.o
LIBOUT = libusem.a
SRC = main.c
OUT = readers-writers
CC = /usr/bin/cc
INCLUDES = -I.
LDFLAGS = libusem.a -lpthread
DBGFLAGS = -g

all:	lib main

lib:	$(LIBSRC)
	$(CC) $(INCLUDES) -c $(LIBSRC) -o $(LIBOBJ)
	ar rcs $(LIBOUT) $(LIBOBJ)

P3:	$(SRC) $(LIBOBJ)
	$(CC) $(INCLUDES) $(SRC) $(LDFLAGS) -o $(OUT)

clean:
	rm -f $(OUT) $(LIBOBJ) $(LIBOUT)