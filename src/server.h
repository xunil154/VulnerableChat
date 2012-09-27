#ifndef __SERVER_H__
#define __SERVER_H__

#include "chat.h"
#include "common.h"

int run_server(int port);
int handle_client(int client);

int has_access(uint16_t check, uint16_t perm);

int join(int client, struct join_request* req);

int find_user(char* name);

int is_connected(int socket);

int broadcast(struct message *message);

struct user users[1024];
int next_user_id = 0;

#endif
