#ifndef __CHAT_H__
#define __CHAT_H__

#include<sys/socket.h>
#include<sys/types.h>
#include<inttypes.h>
#include<netdb.h>

#define BUFFER_SIZE 140
#define MAX_LEN 140
#define NAME_LEN 20
#define PORT 1337

#define VERSION 457

// MESSAGE TYPES //
#define JOIN		0
#define JOIN_RESP	1
#define MESSAGE		10
#define COMMAND		20

// ROLES
#define USER_G		1
#define ADMIN_G		(1<<16)

struct message_header{
	uint16_t version;
	uint16_t type;
	uint16_t length;
}__attribute__((packed));

struct message{
	uint16_t length;
	unsigned char message[MAX_LEN];
}__attribute__((packed));

struct command{

};

struct user{
	unsigned char name[NAME_LEN];
	uint16_t groups;
};

#endif
