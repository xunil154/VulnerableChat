#include "chat.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int get_header(int client, struct header* header){
  // read the version and length

  if(get_data(client, header, sizeof(struct header)) < 0){
	  return -1;
  }

  header->version = ntohs(header->version);
  header->type = ntohs(header->type);
  header->length = ntohs(header->length);

  return 0;
}

int send_message(int client, struct message *msg){
	unsigned char buffer[BUFFER_SIZE];

	struct header *header = (struct header*)buffer;

	int total = sizeof(struct header); //sizeof(uint16_t)*2 + len;
	int msg_size = sizeof(uint16_t)*2 + msg->length;
	total += msg_size;

	msg->length = htons(msg->length);

	header->version 	= htons(VERSION);
	header->type 		= htons(MESSAGE);

	header->length 	= htons(sizeof(struct message));

	memcpy(buffer+sizeof(struct header),msg,sizeof(struct message));


	if(send_data(client, buffer, total) < 0){
		return -1;
	}

	return 0;
	 // printf("done\n");
}

int send_join_request(int client, struct join_request *req){
	unsigned char buffer[BUFFER_SIZE];

	struct header *header = (struct header*)buffer;

	int total = sizeof(struct header); //sizeof(uint16_t)*2 + len;
	int msg_size = sizeof(uint16_t) + req->name_length;
	total += msg_size;

	req->name_length = htons(req->name_length);

	header->version 	= htons(VERSION);
	header->type 		= htons(JOIN);

	header->length 	= htons(msg_size);

	memcpy(buffer+sizeof(struct header),req,sizeof(struct join_request));

	if(send_data(client, buffer, total) < 0){
		return -1;
	}

	return 0;
}

int send_join_response(int client, struct join_response *req){
	unsigned char buffer[BUFFER_SIZE];

	struct header *header = (struct header*)buffer;

	int total = sizeof(struct header); //sizeof(uint16_t)*2 + len;
	int msg_size = sizeof(struct join_response);
	total += msg_size;

	header->version 	= htons(VERSION);
	header->type 		= htons(JOIN_RESP);

	header->length 	= ntohs(msg_size);

	memcpy(buffer+sizeof(struct header),req,sizeof(struct join_response));

	if(send_data(client, buffer, total) < 0){
		return -1;
	}
	return 0;

}

int get_data(int client, void *msg, int length){

  // read the message from the client
  // This is a multi part read in case not all of the data was sent
  // in one packet (ie msg->length > network MTU such as 1024)
  int total = 0;
  while(total < length){

    // continue to read into the message  buffer, offset to new pos
    // for each recv
    int received = recv(client, msg+total, length-total,0);
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


int send_data(int client, void* data, int length){
  int total_sent = 0;
  do{
    int sent = send(client, data+total_sent, length-total_sent,0);
    if(sent == 0){
      perror("Sever closed connection");
      return 0;
    }else if(sent < 0){
      perror("Error sending data");
      return -1;
    }
    total_sent += sent;
//    printf("Sent %d : %d remaining\n",total_sent,total-total_sent);
  }while(total_sent < length);
}
