#include "server.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

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

int run_server(int port){
	struct message *msg= NULL;
	fd_set readfds;
	fd_set master;

	int sock = start_listening(port);
	int max_sock = sock;

	printf("Waiting for a connection on port %d\n",port);

	FD_ZERO(&readfds);
	FD_ZERO(&master);
	FD_SET(sock, &master);

	while(1){
		struct timeval tv;
		tv.tv_sec = 2;
		tv.tv_usec = 500000;

		readfds = master;

		// don't care about writefds and exceptfds:
		select(max_sock+1, &readfds, NULL, NULL, &tv);

		if(tv.tv_sec == 0){ continue; }

		for(int i = 0; i < max_sock+1; ++i){
			if (FD_ISSET(i, &readfds)){
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
				}else{
					handle_client(i);
				}
			}
		}
	}
}

int is_connected(int socket){
	if(socket <= 0) return 0;
	return 1;
}

int handle_client(int client){
	unsigned char buffer[BUFFER_SIZE];
	memset(buffer,0,sizeof(buffer));

	struct header header;
	if(get_header(client,&header)){
		printf("Failed to receive header from client, disconnecting\n");
		close(client);
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
			join(client,(struct join_request*)buffer);
		break;
		case MESSAGE:
		{
//			printf("Receiving message...\n");
			struct message *msg = (struct message*)buffer;
			printf("Client sent: %s\n",msg->message);

			struct message new_msg;
			memset(&new_msg,0,sizeof(struct message));

			struct user user = users[msg->user_id];

			strncat(new_msg.message, user.name, NAME_LEN);
			strcat(new_msg.message, ": ");
			strncat(new_msg.message, msg->message, MAX_LEN);

			for(int i = 0; i < next_user_id; i++){
				struct user other_user = users[i];
				if(other_user.id == user.id){
					continue;
				}
				if(is_connected(other_user.socket)){
					printf("Sending message to %s\n",other_user.name);
					send_message(other_user.socket,&new_msg);
				}

			}
		}
				
		break;
		case COMMAND:
		break;
		default:
			printf("Unknown command type: %d\nDisconnecting client\n",header.type);
			close(client);
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

/*struct user{
	uint16_t id;
	uint16_t groups;
	int socket;
	int name_length;
	unsigned char name[NAME_LEN];
}__attribute__((packed));
*/
	struct user *user = &(users[id]);

	user->id = id;
	user->groups = DEFAULT_GROUP;
	user->socket = client;
	user->name_length = req->name_length;
	strcpy(user->name, req->name);

	printf("User %s joined with the id %d\n",user->name,user->id);

}


// Checks if the bit value is set in to_test
// eg if(has_access(client_user->permissions, ADMIN_G);
int has_access(uint16_t to_test, uint16_t value){
	return to_test & value;
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

