#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "chat.h"

int server_connect(char* server, char* port);
int client(char* username, char* server, char* port);
int register_username(char* username);
void usage(const char* message);

struct config{
	uint16_t mode;
	struct user self;
};

struct config config;

#endif
