#include "chat.h"
#include "common.h"
#include "client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	struct timeval tv;

	tv.tv_sec = 2;  /* 30 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	return sockfd;
}

int client(char* username, char* server, char* port){
	int socket = server_connect(server,port);
	if(socket <= 0){
		printf("An error occured connecting, try again\n");
		exit(1);
	}
	int response = register_username(socket, username);
	if( response < 0){
		if(response == -1){
			printf("An error occured with the registration request\n");
			exit(1);
		}else if(response == -2){
			printf("Username already exists\n");
			exit(1);
		}else{
			printf("Unknown error!!!\n");
			exit(1);
		}
	}else{
		printf("We registerd succesfully. New id: %d\n",response);
		user_id = response;
	}


	struct message msg = { 23, 0, "This is a test message" };
	msg.length = strlen(msg.message);

	if(send_message(socket, &msg) <  0){
		return -1;
	}

	sleep(30);

	return 0;
}

// Returns 0 on success, -1 on send failure, -2 if username is already registered
int register_username(int server, char* username){
	if(username == NULL){
		return -3;
	}
	struct join_request req;
	memset(&req,0,sizeof(struct join_request));
	int len = strlen(username);
	req.name_length = len;
	strncpy(req.name,username,len);

	printf("Sending request...\n");
	if(send_join_request(server, &req) < 0){
		printf("Failed to send registration request\n");
		return -1;
	}

	struct header header;
	memset(&header,0,sizeof(header));

	if(get_header(server, &header) < 0){
		printf("Failed to retrieve registration response\n");
		return -1;
	}

	if(header.type != JOIN_RESP){
		printf("Invalid return type from server\n");
		return -1;
	}

	struct join_response response;
	memset(&response,0,sizeof(struct join_response));

	if(get_data(server, &response, (int)header.length) < 0){
		printf("Failed to get response from server\n");
		return -1;
	}
	switch(response.status){
		case OK:
			return response.user_id;
		case USED:
			return -2;
		case INVALID:
			return -3;
		default:
			return -4;
	}
}

void usage(const char* message){
	if(message != NULL){
		printf("ERROR: %s\n\n",message);
	}
	printf("./client <username> <host> <port>\n");
	printf("\tusername\tYour desired username (15 characters max)\n");
	printf("\thost\tThe host to connect to\n");
	printf("\tport\tThe port to connect to\n");
	if(message != NULL)
		exit(1);
}

int main(int argc, char** argv){
	if(argc < 3){
		usage(NULL);
	}

	client(argv[1],argv[2],argv[3]);

}
