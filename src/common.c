#include "chat.h"
#include "common.h"
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>

int get_header(int client, struct header* header){
  // read the version and length

  if(get_data(client, header, sizeof(struct header)) < 0){
	  perror("Failed to read data\n");
	  return -1;
  }

  header->version = ntohs(header->version);
  header->type = ntohs(header->type);
  header->length = ntohs(header->length);

  return 0;
}

int send_message(int client, struct message *msg){
	int msg_size = sizeof(uint16_t)*2 + msg->length;

	printf("Sending message: %s\n",msg->message);

	if(send_data(client, msg, msg_size, MESSAGE) < 0){
		return -1;
	}

	return 0;
	 // printf("done\n");
}

int send_join_request(int client, struct join_request *req){
	int msg_size = sizeof(uint16_t) + req->name_length;

	if(send_data(client, req, msg_size, JOIN) < 0){
		return -1;
	}

	return 0;
}

int send_join_response(int client, struct join_response *req){

	if(send_data(client, req, sizeof(struct join_response), JOIN_RESP) < 0){
		return -1;
	}
	return 0;

}

int get_data(int client, void *msg, int length){
	struct timeval tv;

	tv.tv_sec = 1; tv.tv_usec = 0; 

	setsockopt(client, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval));

//	printf("Reading %d bytes...\n", length);

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
			return -2;
		}
		if(received == -1){
			perror("Failed to read data from client");
			return -1;
		}
		total += received;
	}
//	printf("Read %d bytes\n",total);
	return 0;
}


int send_data(int client, void* data, int length, uint16_t type){
	unsigned char buffer[BUFFER_SIZE];

	struct header *header = (struct header*)buffer;

	int total = sizeof(struct header); //sizeof(uint16_t)*2 + len;
	total += length;

	header->version 	= htons(VERSION);
	header->type 		= htons(type);
	header->length 	= ntohs(length);

	memcpy(buffer+sizeof(struct header), data, length);

//	printf("Sending %d bytes...\n",total);
	int total_sent = 0;
	do{
		int sent = send(client, buffer+total_sent, total-total_sent,0);
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
//	printf("sent %d bytes...\n",total_sent);
}
