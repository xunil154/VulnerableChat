#include "chat.h"
#include <stdio.h>
#include <string.h>

int get_message(int client, struct message *msg){

  // read the version and length
  int received = recv(client,(unsigned char *)msg,2*sizeof(uint16_t),0);
  if(received == 0){
    printf("Client closed\n");
    return -1;
  }
  if(received == -1){
    perror("Failed to read data from client");
    return -1;
  }

  if(ntohs(msg->version) != 457){
    printf("Invalid version from client");
    return -1;
  }

  msg->length = ntohs(msg->length);
  //printf("Reading %d bytes\n",msg->length);

  // read the message from the client
  // This is a multi part read in case not all of the data was sent
  // in one packet (ie msg->length > network MTU such as 1024)
  int total = 0;
  while(total < msg->length){

    // continue to read into the message  buffer, offset to new pos
    // for each recv
    int received = recv(client,msg->message+total,msg->length-total,0);
    if(received == 0){
      printf("Client closed\n");
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

  fgets((char *)(msg->message),MAX_LEN,stdin);
  int len = strlen(msg->message);
  msg->message[len-1] = 0;
  len -= 1;

  msg->version = htons(457);
  msg->length = htons((uint16_t)len);

  int total = sizeof(struct message); //sizeof(uint16_t)*2 + len;
  int total_sent = 0;
  do{
    int sent = send(client, msg+total_sent, total,0);
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
