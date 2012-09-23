FLAGS = -g 
CC = gcc $(FLAGS)

all: ../bin/server ../bin/client

server: ../bin/server
client: ../bin/client

../bin/server: ../bin/common.o
	$(CC) -o ../bin/server ../bin/common.o server.c
../bin/client: ../bin/common.o
	$(CC) -o ../bin/client ../bin/common.o client.c

../bin/server.o: server.c ../bin/common.o 
	$(CC) -o ../bin/server.o server.c ../bin/common.o
../bin/client.o: client.c ../bin/common.o
	$(CC) -o ../bin/client.o client.c ../bin/common.o

../bin/common.o: common.c
	$(CC) -c common.c -o ../bin/common.o

clean:
	rm -rf ../bin/server ../bin/client bin/*.o
