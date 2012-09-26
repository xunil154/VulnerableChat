#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "chat.h"
#include "common.h"

extern int user_id;
unsigned char buffer[1024];

int interface(int server_socket);
int handle_server(int server_socket);

#endif
