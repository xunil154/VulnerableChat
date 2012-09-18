#ifndef __CHAT_H__
#define __CHAT_H__

#include<sys/socket.h>
#include<sys/types.h>
#include<inttypes.h>
#include<netdb.h>

#define BUFFER_SIZE 140
#define MAX_LEN 140
#define PORT 1337
#define VERSION 457

struct message{
  uint16_t version;
	uint16_t length;
	unsigned char message[BUFFER_SIZE];
}__attribute__((packed));

#endif
