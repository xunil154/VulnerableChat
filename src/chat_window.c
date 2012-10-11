
#include "interface.h"
#include "chat_window.h"
#include <stdlib.h>
#include <curses.h>
#include <string.h>

int init_chat_window(int buffer_size){
	head = NULL;
}

int destroy_chat_window(){
	struct chat_line *current = head;
	struct chat_line *next = head;
	while(next != NULL){
		next = current->prev;
		free(current);
		current = next;
	}
	head = NULL;
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

