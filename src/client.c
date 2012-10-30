#include "chat.h"
#include "common.h"
#include "client.h"
#include "interface.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/**
 * Connect to the server on a given address and port
 * 	server: The server to connect to
 * 	port: the service to connect on
 * Returns a socket connected to the server, -1 on failure
 */
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

	tv.tv_sec = 2;  /* 2 Secs Timeout */
	tv.tv_usec = 0;  // Not init'ing this can cause strange errors

	setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

	return sockfd;
}

/**
 * Start the client chat program
 * It connects to the server and starts up the curses interface
 * Arguments:
 *	username: The desired username
 *	server: The server to connect to
 *	port: The port to connect on
 */
int client(char* username, char* server, char* port){
	// Connect to the server
	int socket = server_connect(server,port);
	if(socket <= 0){
		printf("An error occured connecting, try again\n");
		exit(1);
	}

	// Load info into our runtime configuration 
	config.self.socket = socket;
	strncpy(config.self.name,username,strlen(username));
	config.self.name_length = strlen(username);

	// Register ourselves with the server
	int response = register_username(username);
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
		config.self.id = response;
	}

	// Start the chat interfac
	interface(&config);

	return 0;
}

// Returns 0 on success, -1 on send failure, -2 if username is already registered
/**
 * Register our username with the server. This will grant us a slot
 * on the server as well as validate our username
 * Arguments:
 *	username: our desired username
 * Returns:
 *  Our new user id on success
 * 	-1 on network error
 *  -2 if our username is already used
 *  -3 if our username is invalid
 *  -4 on all other errors
 */
int register_username(char* username){
	if(username == NULL || config.self.socket <= 0){
		return -4;
	}

	// Build up our request message
	struct join_request req;
	memset(&req,0,sizeof(struct join_request));
	int len = strlen(username);
	req.name_length = len;
	strncpy(req.name,username,len);

	printf("Sending join request...\n");
	if(send_join_request(config.self.socket, &req) < 0){
		printf("Failed to send registration request\n");
		return -1;
	}

	// Retrieve the response
	struct header header;
	memset(&header,0,sizeof(header));

	if(get_header(config.self.socket, &header) < 0){
		printf("Failed to retrieve registration response\n");
		return -1;
	}

	// sanity check
	if(header.type != JOIN_RESP){
		printf("Invalid return type from server\n");
		return -1;
	}

	// receive the full response message
	struct join_response response;
	memset(&response,0,sizeof(struct join_response));

	if(get_data(config.self.socket, &response, (int)header.length) < 0){
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
	exit(1);
}

int main(int argc, char** argv){
	if(argc < 3){
		usage(NULL);
	}

	client(argv[1],argv[2],argv[3]);
}
