FLAGS = -g -std=gnu99
CC = gcc $(FLAGS)

all: ../bin/server ../bin/client

server: ../bin/server
client: ../bin/client

../bin/server: ../bin/common.o server.c
	$(CC) -o ../bin/server ../bin/common.o server.c

../bin/client: ../bin/common.o client.c interface.h interface.c
	$(CC)  -lncurses -o ../bin/client ../bin/common.o interface.c client.c

../bin/common.o: common.c common.h chat.h
	$(CC) -c common.c -o ../bin/common.o

clean:
	rm -rf ../bin/server ../bin/client ../bin/*.o
