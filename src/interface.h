#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "chat.h"
#include "common.h"
#include "client.h"
#include <curses.h>
#include <signal.h>

unsigned char buffer[1024];

int interface(struct config *config);
int init_interface();
int close_interface();
int handle_server(int server_socket);
int handle_user();
int process_user();

void signal_handle(void);

struct window{

};
#endif
