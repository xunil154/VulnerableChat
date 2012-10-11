#include "interface.h"
#include "chat_window.h"
#include "common.h"
#include "chat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>
#include <curses.h>
//#include <signal.h>

#define STDIN 0

int EXIT = 0;
int buffer_pos = 0;
struct config *CONFIG;

void signal_handler(int sig){
	signal(SIGINT, signal_handler);
	printf("You pressed ^C\n");
	close_interface();
	EXIT = 1;
}

int close_interface(){
	endwin();
	close(CONFIG->self.socket);
}

int init_interface(){
	signal(SIGINT, signal_handler);
	//signal(SIGQUIT, quitproc);

	initscr();			/* Start curses mode 		  */
	raw();
	nodelay(stdscr,TRUE);
	keypad(stdscr, TRUE);           /* We get F1, F2 etc..          */
	noecho();

	refresh();

	windows[CHAT_WIN] = create_newwin(LINES-4, COLS, 1, 0);
	init_chat_window(100);

	windows[INPUT_WIN] = create_newwin(4, COLS, LINES-4, 0);

	wmove(windows[CHAT_WIN], 1, 2);
	wmove(windows[INPUT_WIN], 1, 2);
	//wprintw(windows[INPUT_WIN], CONFIG->self.name);
	wprintw(windows[INPUT_WIN], " >");
	wrefresh(windows[INPUT_WIN]);

	printw("Press F4 to exit");
	refresh();			/* Print it on to the real screen */

	//destroy_win(my_win);

	//getch();			/* Wait for user input */
}

int interface(struct config* config){

	init_interface();

	fd_set master;
	fd_set readset;

	FD_ZERO(&readset);
	FD_ZERO(&master);
	FD_SET(config->self.socket, &master);
	FD_SET(STDIN, &master);

	CONFIG = config;

	int max_sock = config->self.socket;
	struct timeval tv;
	while(EXIT == 0){
		readset = master;

		tv.tv_sec = 0;
		tv.tv_usec = 100;

		select(max_sock+1, &readset, NULL, NULL, &tv);
		if(tv.tv_sec == 0 && tv.tv_usec == 0){
			handle_user();
			continue;
		}

		for(int i = 0; i < max_sock+1; ++i){
			if (FD_ISSET(i, &readset)){
				if(i == config->self.socket){
					if(handle_server(config->self.socket) < 0){
						close_interface();
						perror("Connection closed to server");
						return -1;
					} }else if(i == STDIN){
					handle_user();
				}
			}
		}
		refresh();
	}
	return 0;
}

int handle_user(){
	refresh();
	int c = getch();
	if(c == ERR){
		return 1;
	}
	switch(c){
		case '\n':
			process_user(buffer);
			buffer_pos = 0;
			break;
		case KEY_F(4):
			close_interface();
			exit(0);
		default:
			wprintw(windows[INPUT_WIN],"%c",(char)c);
			buffer[buffer_pos++] = (char)c;
			wrefresh(windows[INPUT_WIN]);
	}

}

int process_user(){
	int buffer_len = strlen(buffer);
	int command = 0;

	wclear(windows[INPUT_WIN]);
	box(windows[INPUT_WIN], 0 , 0);
	wmove(windows[INPUT_WIN], 1, 1);
	//wprintw(windows[INPUT_WIN], CONFIG->self.name);
	wprintw(windows[INPUT_WIN]," >");
	wrefresh(windows[INPUT_WIN]);

	if(buffer_len == 0){
		return -1;
	}

	// TODO: add command processing
	//if(buffer[0] == '/'){
	//
	//}

	switch(command){
		case 0:
		{
			struct message message;
			memset(&message,0,sizeof(struct message));
			strcpy(message.message,buffer);

			message.length = strlen(message.message);
			message.user_id = CONFIG->self.id;
			send_message(CONFIG->self.socket,&message);
		}
	}

	memset(buffer,0,sizeof(buffer));
}


int handle_server(int server_socket){
	char buffer[BUFFER_SIZE];
	memset(buffer,0,sizeof(buffer));

	struct header header;
	if(get_header(server_socket,&header)){
		printf("Failed to receive header from server, disconnecting\n");
		close(server_socket);
		return -1;
	}

#ifdef DEBUG
	printf("Getting server data of length: %d\n",header.length);
#endif
	if(get_data(server_socket, &buffer, (header.length)) < 0){
		printf("Could not retrieve server's data\n");
		close(server_socket);
		return -1;
	}

#ifdef DEBUG
	printf("Received packet type: %d and length: %d\n",header.type, header.length);
#endif
	switch(header.type){
		case JOIN_RESP:
			printf("We should have already received our join response.... strange hapenings\n");
		break;
		case MESSAGE:
		{
			struct message *msg = (struct message*)buffer;
			add_message(msg);
			show_messages(windows[CHAT_WIN]);
			//wprintw(windows[CHAT_WIN],"%s\n",msg->message);
			wrefresh(windows[CHAT_WIN]);
		}
				
		break;
		case COMMAND:
		break;
		default:
			printf("Unknown command type: %d\nDisconnecting client\n",header.type);
			close(server_socket);
			return -1;
	}

	/*while(get_message(client, &msg) >= 0){
		printf("Friend: %s\n",msg.message);
		printf("You: ");

		memset(&msg,0, sizeof msg);
		send_message(client, &msg);
		memset(&msg,0,sizeof msg);
	}
	*/
}

WINDOW *create_newwin(int height, int width, int starty, int startx)
{	WINDOW *local_win;

	local_win = newwin(height, width, starty, startx);
	box(local_win, 0 , 0);		/* 0, 0 gives default characters 
					 * for the vertical and horizontal
					 * lines			*/
	wrefresh(local_win);		/* Show that box 		*/

	return local_win;
}

void destroy_win(WINDOW *local_win)
{	
	/* box(local_win, ' ', ' '); : This won't produce the desired
	 * result of erasing the window. It will leave it's four corners 
	 * and so an ugly remnant of window. 
	 */
	wborder(local_win, ' ', ' ', ' ',' ',' ',' ',' ',' ');
	/* The parameters taken are 
	 * 1. win: the window on which to operate
	 * 2. ls: character to be used for the left side of the window 
	 * 3. rs: character to be used for the right side of the window 
	 * 4. ts: character to be used for the top side of the window 
	 * 5. bs: character to be used for the bottom side of the window 
	 * 6. tl: character to be used for the top left corner of the window 
	 * 7. tr: character to be used for the top right corner of the window 
	 * 8. bl: character to be used for the bottom left corner of the window 
	 * 9. br: character to be used for the bottom right corner of the window
	 */
	wrefresh(local_win);
	delwin(local_win);
}
