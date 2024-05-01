CC = gcc
CFLAGS = -Wall -g

all: server1 client1

server1: server1.o

client1: client1.o

server1.o: server1.c defs.h

client1.o: client1.c defs.h

clean:
	rm -f -r *.dSYM fifo* server1 client1 *.o
