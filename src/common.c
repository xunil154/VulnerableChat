#include "chat.h"
#include <stdio.h>
#include <string.h>

int get_header(int client, struct message_header* header){
  // read the version and length
  int received = recv(client,header,sizeof(struct message_header),0);
  if(received == 0){
    printf("Client closed\n");
	close(client);
    return -1;
  }
  if(received == -1){
    perror("Failed to read data from client");
    return -1;
  }

  if(ntohs(header->version) != VERSION){
    printf("Invalid version from client");
    return -1;
  }

  header->version = ntohs(header->version);
  header->type = ntohs(header->type);

  return 0;
}

int get_message(int client, int length, struct message *msg){

  // read the message from the client
  // This is a multi part read in case not all of the data was sent
  // in one packet (ie msg->length > network MTU such as 1024)
  int total = 0;
  while(total < length){

    // continue to read into the message  buffer, offset to new pos
    // for each recv
    int received = recv(client,msg+total,length-total,0);
    if(received == 0){
      printf("Client closed\n");
		close(client);
      return -1;
    }
    if(received == -1){
      perror("Failed to read data from client");
      return -1;
    }
    total += received;
  }
  return 0;
}

int send_message(int client, struct message *msg){
  unsigned char buffer[BUFFER_SIZE];

  struct message_header *header = (struct message_header*)buffer;
  msg->length = htons(msg->length);

  header->version 	= htons(457);
  header->type 		= htons(MESSAGE);
  header->length 	= htons(sizeof(struct message));

  memcpy(buffer+sizeof(struct message_header),msg,sizeof(struct message));


  int total = sizeof(struct message); //sizeof(uint16_t)*2 + len;

  int total_sent = 0;
  do{
    int sent = send(client, buffer+total_sent, total,0);
    if(sent == 0){
      perror("Sever closed connection");
      return 0;
    }else if(sent < 0){
      perror("Error sending data");
      return -1;
    }
    total_sent += sent;
//    printf("Sent %d : %d remaining\n",total_sent,total-total_sent);
  }while(total_sent < total);
 // printf("done\n");
}
