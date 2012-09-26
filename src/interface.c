#include "interface.h"
#include "common.h"
#include "chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define STDIN 0

int interface(int server_socket){

	fd_set master;
	fd_set readset;

	FD_ZERO(&readset);
	FD_ZERO(&master);
	FD_SET(server_socket, &master);
	FD_SET(STDIN, &master);

	int exit = 0;
	int max_sock = server_socket;
	while(exit == 0){
		readset = master;

		// don't care about writefds and exceptfds:
		select(max_sock+1, &readset, NULL, NULL, NULL);

		for(int i = 0; i < max_sock+1; ++i){
			if (FD_ISSET(i, &readset)){
				if(i == server_socket){
					handle_server(server_socket);
				}else if(i == STDIN){
					struct message message;
					memset(&message,0,sizeof(struct message));
					gets(message.message);
					message.length = strlen(message.message);
					message.user_id = user_id;
					send_message(server_socket,&message);
				}
			}
		}
	}
	return 0;
}

int handle_server(int server_socket){
	memset(buffer,0,sizeof(buffer));

	struct header header;
	if(get_header(server_socket,&header)){
		printf("Failed to receive header from server, disconnecting\n");
		close(server_socket);
		return -1;
	}

	printf("Getting server data of length: %d\n",header.length);
	if(get_data(server_socket, &buffer, (header.length)) < 0){
		printf("Could not retrieve server's data\n");
		return -1;
	}

	printf("Received packet type: %d and length: %d\n",header.type, header.length);
	switch(header.type){
		case JOIN_RESP:
			printf("We should have already received our join response.... strange hapenings\n");
		break;
		case MESSAGE:
		{
			printf("Receiving message...\n");
			struct message *msg = (struct message*)buffer;
			printf("Server sent: %s\n",msg->message);

		}
				
		break;
		case COMMAND:
		break;
		default:
			printf("Unknown command type: %d\nDisconnecting client\n",header.type);
			close(server_socket);
	}

	/*while(get_message(client, &msg) >= 0){
		printf("Friend: %s\n",msg.message);
		printf("You: ");

		memset(&msg,0, sizeof msg);
		send_message(client, &msg);
		memset(&msg,0,sizeof msg);
	}
	*/
}
