#include "chat.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <unistd.h>

int server_connect(char* server, char* port){
	struct addrinfo hints, *res;
	int sockfd;

	// first, load up address structs with getaddrinfo():

	memset(&hints, 0, sizeof hints);
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	getaddrinfo(server, port, &hints, &res);

	// make a socket:

	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(sockfd <= 0){
		perror("Failed to create socket");
		return -1;
	}

	// connect!
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) != 0){
		perror("Failed to connect to server");
		return -1;
	}	

	return sockfd;
}

void client(char* server, char* port){
	int socket = server_connect(server,port);
	if(socket <= 0){
		printf("An error occured connecting, try again\n");
		exit(1);
	}

	sleep(2);

	struct message msg = { 23, "This is a test message" };

	send_message(socket, &msg);


}

void usage(const char* message){
	if(message != NULL){
		printf("ERROR: %s\n\n",message);
	}
	printf("./client <host> <port>\n");
	if(message != NULL)
		exit(1);
}

int main(int argc, char** argv){
	if(argc < 3){
		usage(NULL);
	}

	client(argv[1],argv[2]);

}
