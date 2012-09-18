#include "chat.h"
#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int run_server(int port){
  struct message *msg= NULL;

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
    exit(EXIT_FAILURE);
  }

  sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
  if(socket < 0){
    perror("Failed to obtain socket");
    exit(EXIT_FAILURE);
  }

  int yes=1;
  //char yes='1'; // Solaris people use this

  // lose the pesky "Address already in use" error message
  if (setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(int)) == -1) {
        perror("setsockopt");
        return 1;
  } 

  if(bind(sock, res->ai_addr, res->ai_addrlen) == -1){
    perror("Failed to bind");
    exit(EXIT_FAILURE);
  }

  if(listen(sock,2) == -1){
    perror("Failed to listen");
    exit(EXIT_FAILURE);
  }

  char ip[INET6_ADDRSTRLEN];
  struct sockaddr_in * addr = (struct sockaddr_in*)res->ai_addr;
  inet_ntop(res->ai_family, &addr->sin_addr, ip, sizeof ip);
  printf("Waiting for a connection on %s port %d\n",ip,port);
  freeaddrinfo(res);

  while(1){
    struct sockaddr_storage client_addr;
    socklen_t addr_size = sizeof client_addr;

    int client = accept(sock, (struct sockaddr *)&client_addr, &addr_size);
    if(client == -1){
      perror("Failed to accept new client");
      exit(EXIT_FAILURE);
    }
    printf("Found a friend! You receive first.\n");

    struct message msg;
    memset(&msg,0,sizeof msg);
    while(get_message(client, &msg) >= 0){
      printf("Friend: %s\n",msg.message);
      printf("You: ");

      memset(&msg,0, sizeof msg);
      send_message(client, &msg);
      memset(&msg,0,sizeof msg);
    }
  }
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

