CC = gcc
CFLAGS = -Wall -g 

all: folders orchestrator client

orchestrator: bin/orchestrator

client: bin/client

folders:
	@mkdir -p src obj bin tmp

bin/client: obj/client.o
	$(CC) $(CFLAGS) obj/client.o -o bin/client

obj/client.o: src/client.c
	$(CC) $(CFLAGS) -c src/client.c -o obj/client.o

bin/orchestrator: obj/orchestrator.o
	$(CC) $(CFLAGS) obj/orchestrator.o -o bin/orchestrator

obj/orchestrator.o: src/orchestrator.c
	$(CC) $(CFLAGS) -c src/orchestrator.c -o obj/orchestrator.o

clean:
	rm -f obj/* tmp/* bin/{client,orchestrator}