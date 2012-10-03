#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "chat.h"
#include "common.h"
#include <curses.h>
#include <signal.h>

extern int user_id;
unsigned char buffer[1024];

int interface(int server_socket);
int init_interface();
int close_interface();
int handle_server(int server_socket);

void signal_handle(void);
#endif
