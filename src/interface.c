#include "interface.h"
#include "common.h"
#include "chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <curses.h>
//#include <signal.h>

#define STDIN 0

int EXIT = 0;
int buffer_pos = 0;
struct config *CONFIG;

void signal_handler(int sig){
	signal(SIGINT, signal_handler);
	printf("You pressed ^C\n");
	close_interface();
	EXIT = 1;
}

int close_interface(){
	endwin();
}

int init_interface(){
	signal(SIGINT, signal_handler);
	//signal(SIGQUIT, quitproc);

	initscr();			/* Start curses mode 		  */
	//printw("Hello World !!!");	/* Print Hello World		  */
	refresh();			/* Print it on to the real screen */
	//getch();			/* Wait for user input */
}

int interface(struct config* config){

	init_interface();

	fd_set master;
	fd_set readset;

	FD_ZERO(&readset);
	FD_ZERO(&master);
	FD_SET(config->self.socket, &master);
	FD_SET(STDIN, &master);

	CONFIG = config;

	int max_sock = config->self.socket;
	struct timeval tv;
	while(EXIT == 0){
		readset = master;

		tv.tv_sec = 1;
		tv.tv_usec = 0;
		// don't care about writefds and exceptfds:
		select(max_sock+1, &readset, NULL, NULL, &tv);
		if(tv.tv_sec == 0){
			continue;
		}

		for(int i = 0; i < max_sock+1; ++i){
			if (FD_ISSET(i, &readset)){
				if(i == config->self.socket){
					handle_server(config->self.socket);
				}else if(i == STDIN){
					handle_user();
				}
			}
		}
	}
	return 0;
}

int handle_user(){
	int c = getchar();
	switch(c){
		case '\n':
			process_user(buffer);
			break;
		default:
			buffer[buffer_pos++] = (char)c;
	}

}

int process_user(){
	int buffer_len = strlen(buffer);
	int command = 0;

	// TODO: add command processing
	//if(buffer[0] == '/'){
	//
	//}

	switch(command){
		case 0:
		{
			struct message message;
			memset(&message,0,sizeof(struct message));
			strcpy(message.message,buffer);

			message.length = strlen(message.message);
			message.user_id = CONFIG->self.id;
			send_message(CONFIG->self.socket,&message);
		}
	}

}

int handle_server(int server_socket){
	memset(buffer,0,sizeof(buffer));

	struct header header;
	if(get_header(server_socket,&header)){
		printf("Failed to receive header from server, disconnecting\n");
		close(server_socket);
		return -1;
	}

#ifdef DEBUG
	printf("Getting server data of length: %d\n",header.length);
#endif
	if(get_data(server_socket, &buffer, (header.length)) < 0){
		printf("Could not retrieve server's data\n");
		return -1;
	}

#ifdef DEBUG
	printf("Received packet type: %d and length: %d\n",header.type, header.length);
#endif
	switch(header.type){
		case JOIN_RESP:
			printf("We should have already received our join response.... strange hapenings\n");
		break;
		case MESSAGE:
		{
			struct message *msg = (struct message*)buffer;
			printf("%s\n",msg->message);

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
