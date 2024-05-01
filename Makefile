CC = gcc
CFLAGS = -Wall -g

all: orchestrator client

orchestrator: orchestrator.o

client: client.o

orchestrator.o: orchestrator.c defs.h

client.o: client.c defs.h

clean:
	rm -f -r *.dSYM fifo* orchestrator client *.o
