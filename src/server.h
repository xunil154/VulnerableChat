#ifndef __SERVER_H__
#define __SERVER_H__

#include "chat.h"
#include "common.h"

int run_server(int port);
int handle_client(int client);

int has_access(uint16_t check, uint16_t perm);

#endif
