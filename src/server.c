#include "server.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

/**
 * Start a socket listening on a given port
 * Arguments:
 *	port: the port to listen on
 * Returns:
 *	the socket on success
 *  0 on failure
 */
int start_listening(int port){
	struct addrinfo hints, *res;
	int sock;

	memset(&hints,0,sizeof hints);
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;

	int result = 0;
	char text[20];
	snprintf(text,20,"%d",port);

	if((result = getaddrinfo(NULL,text, &hints, &res)) != 0){
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(result));
		return -1;
	}

	sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
	if(socket < 0){
		perror("Failed to obtain socket");
		return -1;
	}

	int yes=1;
	//char yes='1'; // Solaris people use this

	// lose the pesky "Address already in use" error message
	if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	} 

	if(bind(sock, res->ai_addr, res->ai_addrlen) == -1){
		perror("Failed to bind");
		return -1;
	}

	if(listen(sock,2) == -1){
		perror("Failed to listen");
		return -1;
	}
	freeaddrinfo(res);
	return sock;
}

/**
 * Start the server on a given port. This function never exits
 * Arguments:
 *	port: the port to listen on
 * Return:
 *	nothing
 */
int run_server(int port){
	struct message *msg= NULL;
	// use select for server processing
	fd_set readfds;
	fd_set master;

	// Get our socket
	int sock = start_listening(port);
	int max_sock = sock;

	printf("Waiting for a connection on port %d\n",port);

	// setup select
	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(sock, &master);

	// The main server
	while("false"){
		struct timeval tv;
		// 2 second timeout
		tv.tv_sec = 2;
		tv.tv_usec = 500000;

		// copy master since select modifies our list
		readfds = master;

		// don't care about writefds and exceptfds:
		select(max_sock+1, &readfds, NULL, NULL, &tv);

		// if we timed out do it again
		if(tv.tv_sec == 0){ continue; }

		// Something happened! iterate over sockets and find which one has data
		for(int i = 0; i < max_sock+1; ++i){
			if (FD_ISSET(i, &readfds)){
				// if something happend on the listen socket, accept the client
				if(i == sock){
					struct sockaddr_storage client_addr;
					socklen_t addr_size = sizeof client_addr;

					int client = accept(sock, (struct sockaddr *)&client_addr, &addr_size);
					if(client == -1){
						perror("Failed to accept new client");
						continue;
					}

					printf("Client connected\n");
					if(client > max_sock) max_sock = client;
					FD_SET(client,&master);
				}else{ // otherwise handle the client
					if(handle_client(i) < 0){
						FD_CLR(i, &master);
					}
				}
			}
		}
	}
	return 0;
}

int is_connected(int socket){
	if(socket <= 0) return 0;
	return 1;
}

/**
 * Process data from the client
 * Arguments:
 *	client: The client socket to read data from
 */
int handle_client(int client){
	unsigned char buffer[BUFFER_SIZE];
	memset(buffer,0,sizeof(buffer));
#ifdef DEBUG
	printf("Getting data from client %d\n",client);
#endif

	struct header header;
	if(get_header(client,&header)){
		printf("Failed to receive header from client, disconnecting\n");
		int user_id = find_user_by_socket(client);
		remove_user(user_id);
		return -1;
	}

	printf("Getting client data of length: %d\n",header.length);
	if(get_data(client, &buffer, (header.length)) < 0){
		printf("Could not retrieve client's data\n");
		return -1;
	}

	printf("Received packet type: %d and length: %d\n",header.type, header.length);
	switch(header.type){
		case JOIN:
			process_join(client,buffer);	
		break;
		case MESSAGE:
			process_message(client,buffer);	
		break;
		case PM:
			process_pm(client,buffer);
		break;
		case USER_LIST:
			process_who(client,buffer);
		break;
		case WHOIS:
			process_whois(client,buffer);
		break;
		case COMMAND:
		break;
		default:
			printf("Unknown command type: %d\nDisconnecting client\n",header.type);
			close(client);
	}

}

/**
 * Process the join request
 * If the join is succesful then it will broadcast
 *	[<username> has joined the chat room]
 */
int process_join(int client, unsigned char* buffer){
	struct join_request *req = (struct join_request*)buffer ;
	if(join(client,req) == OK){
		struct message msg;
		memset(&msg,0,sizeof(struct message));
		msg.user_id = 65535;
		msg.message[0] = '[';
		strncat(msg.message+1,req->name,req->name_length);
		strcat(msg.message," has joined the chat room]");
		msg.length=(strlen(msg.message));
		broadcast(&msg);
	}

}

/**
 * Process a general message
 * It will read the message from a user, reformat the string as
 *  <usernam>: <message
 * and broadcast to all connected users
 */
int process_message(int client, unsigned char* buffer){
	struct message *msg = (struct message*)buffer;
	printf("Client sent: %s\n",msg->message);

	struct message new_msg;
	memset(&new_msg,0,sizeof(struct message));

	struct user user = users[msg->user_id];

	strncat(new_msg.message, user.name, NAME_LEN);
	strcat(new_msg.message, ": ");
	strncat(new_msg.message, msg->message, MAX_LEN);
	new_msg.length = strlen(new_msg.message);
	new_msg.user_id = msg->user_id;

	broadcast(&new_msg);
	return 0;
}

/**
 * Processes a PM message from a user
 * Receives the PM message, looks up the username of the sender
 * and build a message with the format
 *	[PM from <username>: <message>]
 * and then sends this message to the intended recepiant
 */
int process_pm(int client, unsigned char* buffer){
//			printf("Receiving message...\n");
	struct private_message *msg = (struct private_message*)buffer;

	struct message new_msg;
	memset(&new_msg,0,sizeof(struct message));

	struct user user = users[msg->from];

	strcpy(new_msg.message,"[PM from ");
	strncat(new_msg.message, user.name, NAME_LEN);
	strcat(new_msg.message, ": ");
	strncat(new_msg.message, msg->message.message, MAX_LEN);
	strcat(new_msg.message, " ]");
	new_msg.length = strlen(new_msg.message);
	new_msg.user_id = msg->from;

	int to = find_user(msg->to);
	if(to >= 0){
		send_message(users[to].socket, &new_msg);
	}

	memset(&new_msg,0,sizeof(struct message));

	strcpy(new_msg.message,"[PM to ");
	strncat(new_msg.message, user.name, NAME_LEN);
	strcat(new_msg.message, ": ");
	strncat(new_msg.message, msg->message.message, MAX_LEN);
	strcat(new_msg.message, " ]");
	new_msg.length = strlen(new_msg.message);
	new_msg.user_id = msg->from;

	if(to >= 0){
		send_message(users[msg->from].socket, &new_msg);
	}
	return 0;
}

/**
 * Process a WHO request
 * It will find all of the users who are connected and send thir user information
 * to the requesting client
 */
int process_who(int client, unsigned char* buffer){
	printf("WHO command ran from client %d\n",client);
	memset(buffer, 0, sizeof(buffer));
	struct user_list *list = (struct user_list*)buffer;
	int count = 0;
	int index = sizeof(struct user_list);
	for(int i = 0; i < next_user_id; i++){
		if(is_connected(users[i].socket)){
			memcpy(buffer+index, &users[i],sizeof(struct user));
			count++;
			index += sizeof(struct user);
		}
	}

	list->user_count = htons(count);
	printf("WHO response is %d bytes\n",index);
	if(send_data(client,buffer,index, USER_LIST_RESP) < 0){
		perror("Failed to send user list to client");
		return -1;
	}
	return 0;
}
/**
 * Process WHOIS request
 * It will lookup the username of the requested user and
 * send back the user data to the requesting client.
 * If a user does not exist it will send NO_USER as the
 * status.
 */
int process_whois(int client, unsigned char* buffer){
	//uint16_t name_len;
	//unsigned char name[NAME_LEN];
	struct whois *whois = (struct whois*)buffer;
	whois->name_len = ntohs(whois->name_len);
	int id = find_user(whois->name);
	int status = OK;
	struct user *found = NULL;
	if(id < 0){
		status = NO_USER;
	}else{
		status = OK;
		found = &users[id];
	}
	if(send_whois_response(client,status,found) < 0){
		perror("Failed to send whois response");
		return -1;
	}
	return 0;
}

/**
 * This will broadcast a message to all users connected to the server
 * Arguments:
 *	msg: the message to send
 */
int broadcast(struct message *msg){
	for(int i = 0; i < next_user_id; i++){
		struct user other_user = users[i];
	/*	if(other_user.id == msg->user_id){
			continue;
		} */
		if(is_connected(other_user.socket)){
			printf("Sending message to %s\n",other_user.name);
			send_message(other_user.socket,msg);
		}

	}
}

/**
 * Process a join request from a client.
 * It will setup the user information in the users table
 * as well as send the response codes to the client
 */
int join(int client, struct join_request *req){
	printf("Trying to register new user %s\n",req->name);

	int response = OK;
	int id = find_user(req->name);
	if(id >= 0){
		response = USED;
		printf("Username %s already exists in the system\n");
	}else{
		id = next_user_id++;
	}

	struct join_response resp;
	resp.user_id = id;
	resp.status = response;

	if(send_join_response(client, &resp) < 0){
		printf("Failed to send registration response\n");
		return -1;
	}

	struct user *user = &(users[id]);

	user->id = id;
	user->groups = DEFAULT_GROUP;
	user->socket = client;
	user->name_length = req->name_length;
	strcpy(user->name, req->name);

	printf("User %s joined with the id %d\n",user->name,user->id);
	return response;

}


// Checks if the bit value is set in to_test
// eg if(has_access(client_user->permissions, ADMIN_G);
int has_access(uint16_t to_test, uint16_t value){
	return to_test & value;
}

int find_user_by_socket(int socket){
	for(int i = 0; i < next_user_id; i++){
		struct user user = users[i];
		if(user.socket == socket){
			return i;
		}
	}
	return -1;
}

int find_user(char* username){
	printf("Searching for user %s\n",username);
	for(int i = 0; i < next_user_id; i++){
		struct user user = users[i];
		printf(" trying: %s\n",user.name);
		if(strcmp(user.name, username) == 0){
			return i;
		}
	}
	return -1;
}

int remove_user(int id){
	struct message msg;
	printf("Removing user id: %d\n",id);

	memset(&msg,0,sizeof(struct message));
	msg.user_id = 65535;
	msg.message[0] = '[';
	strncat(msg.message+1,users[id].name,users[id].name_length);
	strcat(msg.message," has left]");
	msg.length=(strlen(msg.message));

	//memset(users+id),0,sizeof(struct user));
	struct user *user = &users[id];
	user->id = 0;
	user->groups = 0;
	close(user->socket);
	user->socket=0;
	user->name_length = 0;
	memset(user->name,0,NAME_LEN);

	broadcast(&msg);
	printf("User removed\n");
}

void usage(const char *name){
  printf("Usage: %s <args>\n",name);
  printf("\tIf no arguments are give, this program will run as a server\n");
  printf("\t-h\tPrint this message\n");
  printf("\t-s <name> \tThe server to connect to\n");
  printf("\t-p <port> \tThe server port to connect to\n\n");
}

int main (int argc, char** argv){
  char *server = NULL;
  int port = 0;

  int c = 0;
  while((c = getopt(argc,argv,"hs:p:")) != -1){
    switch (c){
      case 'h': usage(argv[0]); exit(0); break;
      case 's': server = optarg; break;
      case 'p': port = atoi(optarg); break;
      default: printf("Unknown option %c\n",(char)c); usage(argv[0]); exit(1);
    }
  }
  if(server == NULL && port == 0){
    run_server(PORT);
    exit(0);
  }
  if(server != NULL && port == 0){
    printf("A port number must be specified!\n");
    usage(argv[0]);
    exit(1);
  }
  if(server == NULL && port != 0){
    printf("A server must be specified!\n");
    usage(argv[0]);
    exit(1);
  }
  if(port <= 0){
    printf("Port numbers must be greater than 0\n");
    exit(1);
  }
//  client(server,port);
  return 0;
}

