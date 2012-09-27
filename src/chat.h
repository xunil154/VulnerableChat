#ifndef __CHAT_H__
#define __CHAT_H__

//#define DEBUG

#include<sys/socket.h>
#include<sys/types.h>
#include<inttypes.h>
#include<netdb.h>

#define MAX_LEN 256
#define BUFFER_SIZE MAX_LEN*2

#define NAME_LEN 20
#define PORT 1337

#define VERSION 1

// MESSAGE TYPES //
#define JOIN		0
#define JOIN_RESP	1

#define MESSAGE		10
#define PM 			11	

#define COMMAND		20
#define KICK		21
#define PROMOTE		22

// ROLES
#define USER_G		1
#define ADMIN_G		(1<<16)

#define DEFAULT_GROUP USER_G

// JOIN STATUS
#define OK		0
#define USED 	1
#define INVALID 2

struct header{
	uint16_t version;
	uint16_t type;
	uint16_t length;
}__attribute__((packed));

struct message{
	uint16_t user_id;
	uint16_t length;
	unsigned char message[MAX_LEN];
}__attribute__((packed));

struct command{

}__attribute__((packed));

struct join_request{
	uint16_t name_length;
	unsigned char name[NAME_LEN];
}__attribute__((packed));

struct join_response{
	uint8_t status;
	uint16_t user_id;
}__attribute__((packed));




struct user{
	uint16_t id;
	uint16_t groups;
	int socket;
	int name_length;
	unsigned char name[NAME_LEN];
}__attribute__((packed));

#endif
