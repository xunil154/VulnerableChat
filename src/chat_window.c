
#include "interface.h"
#include "chat_window.h"
#include <stdlib.h>
#include <curses.h>
#include <strings.h>

int init_chat_window(int buffer_size){
	head = NULL;
}

int destroy_chat_window(){
	for(struct chat_line *current = head; head != NULL;){
		head = current->prev;
		free(current);
	}
}

int add_message(struct message *new_message){
	struct chat_line *line = malloc(sizeof(struct chat_line));
	memcpy(&(line->line), new_message, sizeof(struct message));
	line->prev= head;
	head = line;
}

int show_messages(WINDOW* window){
	int rows,cols;
	getmaxyx(window,rows,cols);
	struct chat_line *current = head;

	wclear(window);
	box(window,0,0);

	rows -= 3;
	while(rows != 0 && current != NULL){
		int len = strlen(current->line.message);
		if( len > cols - 3){
			rows -= len / (cols-1);
		}

		wmove(window,rows--,3);
		wprintw(window,current->line.message);
		current = current->prev;
	}
	wrefresh(window);
}

