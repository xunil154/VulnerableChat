#ifndef __INTERFACE_H__
#define __INTERFACE_H__

#include "chat.h"
#include "common.h"
#include "client.h"
#include <curses.h>
#include <signal.h>

#define CHAT_WIN 0
#define INPUT_WIN 1

int interface(struct config *config);
int init_interface();
int close_interface();
int handle_server(int server_socket);
int handle_user();
int process_user();

void signal_handle(void);


WINDOW *create_newwin(int height, int width, int starty, int startx);
void destroy_win(WINDOW *local_win);



unsigned char buffer[1024];
WINDOW* windows[3];

#endif
