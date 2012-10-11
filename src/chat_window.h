#ifndef __CHAT_WINDOW_H__
#define __CHAT_WINDOW_H__

#include "chat.h"
#include <curses.h>


int init_chat_window(int buffer_size);
int destroy_chat_window();
int add_message();
int show_messages(WINDOW *win);


struct chat_line{
	struct chat_line* prev;
	struct message line;
};


struct chat_line *head;

#endif
