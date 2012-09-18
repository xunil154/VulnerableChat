FLAGS = -g 
CC = gcc $(FLAGS)

all: bin/server bin/client

server: bin/server
client: bin/client

bin/server: bin/common.o
	$(CC) -o bin/server bin/common.o src/server.c
bin/client: bin/common.o
	$(CC) -o bin/client bin/common.o src/client.c

bin/server.o: src/server.c bin/common.o 
	$(CC) -o bin/server.o src/server.c bin/common.o
bin/client.o: src/client.c bin/common.o
	$(CC) -o bin/client.o src/client.c bin/common.o

bin/common.o: src/common.c
	$(CC) -c src/common.c -o bin/common.o

clean:
	rm -rf bin/chat bin/*.o
