CC=gcc
CFLAGS=-c -Wall -g -DDEBUG

all: PMan

PMan: PMan.o linkedlist.o
	$(CC) PMan.o linkedlist.o -o PMan

PMan.o: PMan.c linkedlist.h
	$(CC) $(CFLAGS) PMan.c

linkedlist.o: linkedlist.c linkedlist.h
	$(CC) $(CFLAGS) linkedlist.c

.PHONY: clean

clean:
	rm -rf *.o *.exe PMan linkedlist