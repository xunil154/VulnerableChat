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
	destroy_chat_window();
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

	print_prompt(windows[INPUT_WIN]);

	printw("Press F4 to exit");
	printw("\tYou are: ");
	printw(CONFIG->self.name);
	refresh();			/* Print it on to the real screen */

}

void print_prompt(WINDOW *win){
	wclear(win);
	box(win, 0 , 0);
	wmove(win, 1, 2);
	wmove(win, 1, 2);
	wprintw(win, " > ");
	wrefresh(win);
}

int interface(struct config* config){
	CONFIG = config;

	init_interface();

	fd_set master;
	fd_set readset;

	FD_ZERO(&readset);
	FD_ZERO(&master);
	FD_SET(config->self.socket, &master);
	FD_SET(STDIN, &master);


	int max_sock = config->self.socket;
	struct timeval tv;
	while(EXIT == 0){
		readset = master;

		tv.tv_sec = 0;
		tv.tv_usec = 100;

		select(max_sock+1, &readset, NULL, NULL, &tv);
		if(tv.tv_sec == 0 && tv.tv_usec == 0){
			if(handle_user() != 0){
				perror("BAD THINGS HAPPENED!");
				EXIT = 1;
			}
			continue;
		}

		for(int i = 0; i < max_sock+1; ++i){
			if (FD_ISSET(i, &readset)){
				if(i == config->self.socket){
					if(handle_server(config->self.socket) < 0){
						close_interface();
						perror("Connection closed to server");
						return -1;
					} 
				}else if(i == STDIN){
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
		return 0;
	}
	switch(c){
		case '\n':
			if(process_user(buffer) != 0){
				return 1;
			}
			buffer_pos = 0;
			break;
		case KEY_F(4):
			close_interface();
			exit(0);
		case KEY_F(1):
		case KEY_F(2):
		case KEY_F(3):
		case KEY_F(5):
		case KEY_F(6):
		case KEY_F(7):
		case KEY_F(8):
		case KEY_F(9):
		case KEY_F(10):
		case KEY_F(11):
		case KEY_F(12):
			break;
		case KEY_UP:
		case KEY_DOWN:
			break;
		case KEY_LEFT:
			break;
		case KEY_RIGHT:
			break;
		case KEY_BACKSPACE:
			break;
		default:
			wprintw(windows[INPUT_WIN],"%c",(char)c);
			buffer[buffer_pos++] = (char)c;
			wrefresh(windows[INPUT_WIN]);
	}
	return 0;

}

int process_user(){
	int buffer_len = strlen(buffer);
	int command = 0;
	unsigned char* next_arg = buffer;

	print_prompt(windows[INPUT_WIN]);

	if(buffer_len == 0){
		return 0;
	}

	// TODO: add command processing
	if(buffer[0] == '/'){
		next_arg = strchr(buffer,' ');
		if(next_arg){
			*next_arg = 0; // null terminate the command
			next_arg++;
		}

		if(strcmp(buffer+1,"who") == 0){
			command = USER_LIST;
		}
		else if(strcmp(buffer+1,"pm") == 0){
			command = PM;
		}
		else if(strcmp(buffer+1,"whois") == 0){
			command = WHOIS;
		}
		else if(strcmp(buffer+1,"q") == 0){
			command = -1;
		}
		else if(strcmp(buffer+1,"quit") == 0){
			command = -1;
		}
		else if(strcmp(buffer+1,"help") == 0){
			command = HELP;
		}
	}

	switch(command){
		case -1:
			close_interface();
			return -1;
		break;
		case 0:
		{
			struct message message;
			memset(&message,0,sizeof(struct message));
			strcpy(message.message,buffer);

			message.length = strlen(message.message);
			message.user_id = CONFIG->self.id;
			send_message(CONFIG->self.socket,&message);
		}
		break;
		case PM:
		{
			unsigned char* to = next_arg;
			next_arg = strchr(next_arg,' ');
			if(next_arg){
				*next_arg = 0; // null terminate the command
				next_arg++;
			}

			struct private_message message;
			memset(&message,0,sizeof(struct private_message));
			strcpy(message.to, to);
			strcpy(message.message.message,next_arg);

			message.message.length = strlen(message.message.message);
			message.message.user_id = CONFIG->self.id;
			message.from = CONFIG->self.id;
			send_pm(CONFIG->self.socket,&message);
		}
		break;
		case WHOIS:
		{
			send_whois_request(CONFIG->self.socket, next_arg);
		}
		break;
		case USER_LIST:
			send_user_list_request(CONFIG->self.socket);
		break;
		case HELP:
		{
			struct message msg;
			msg.user_id = 65535;
			memset(msg.message,0,sizeof(msg.message));
			strcpy(msg.message,"[Commands: ]");
			msg.length = strlen(msg.message);
			add_message(&msg);
			strcpy(msg.message,"[who	Show who is on the server]");
			msg.length = strlen(msg.message);
			add_message(&msg);
			strcpy(msg.message,"[whois <usernaem>	Get details about a user]");
			msg.length = strlen(msg.message);
			add_message(&msg);
			strcpy(msg.message,"[pm <username>	Send a private message to a user]");
			msg.length = strlen(msg.message);
			add_message(&msg);
			/*strcpy(msg.message,"[Commands: ]");
			msg.length = strlen(msg.message);
			add_message(&msg);
			strcpy(msg.message,"[Commands: ]");
			msg.length = strlen(msg.message);
			add_message(&msg);
			*/
			show_messages(windows[CHAT_WIN]);
			wrefresh(windows[CHAT_WIN]);
		}
		break;
	}

	memset(buffer,0,sizeof(buffer));
	return 0;
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
			perror("We should have already received our join response.... strange hapenings\n");
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
		case USER_LIST_RESP:
		{
			struct message msg;
			msg.user_id = 65535;
			memset(msg.message,0,sizeof(msg.message));
			strcpy(msg.message,"[server: WHO]");
			msg.length = strlen(msg.message);

			add_message(&msg);

			struct user_list *list = (struct user_list*)buffer;
			list->user_count = ntohs(list->user_count);

			int index = sizeof(struct user_list);
			while(list->user_count --> 0){

				struct user *user = (struct user*)(buffer+index);
				index += sizeof(struct user);

				msg.user_id = 65535;
				memset(msg.message,0,sizeof(msg.message));
				strcpy(msg.message,"[who:  ");

				strcat(msg.message,user->name);

				strcat(msg.message,"]");
				msg.length = strlen(msg.message);
				add_message(&msg);
			}
			show_messages(windows[CHAT_WIN]);
			wrefresh(windows[CHAT_WIN]);
		}
		break;
		case WHOIS:
		{
			struct message msg;
			msg.user_id = 65535;
			memset(msg.message,0,sizeof(msg.message));
			strcpy(msg.message,"[server: WHOIS]");
			msg.length = strlen(msg.message);

			add_message(&msg);

			struct whois_response *who = (struct whois_response*)buffer;
			who->status = ntohs(who->status);
			switch(who->status){
				case OK:
				{
					strcpy(msg.message,"[name: ");
					strcat(msg.message,who->user.name);
					strcat(msg.message,"]");
					msg.length = strlen(msg.message);
					add_message(&msg);
					strcpy(msg.message,"[id: ");
					sprintf(msg.message+strlen(msg.message),"%d",ntohs(who->user.id));
					//strcat(msg.message,ntohs(who->user.id));
					strcat(msg.message,"]");
					msg.length = strlen(msg.message);
					add_message(&msg);
				}
				break;
				case NO_USER:
				{
					strcpy(msg.message,"[server: User not found]");
					msg.length = strlen(msg.message);
					add_message(&msg);
				}
				break;

				default:
				break;
			}

			show_messages(windows[CHAT_WIN]);
			wrefresh(windows[CHAT_WIN]);
		}
		break;
		case PM:
		{
			struct private_message *pm = (struct private_message*)buffer;
		}
		break;
		default:
			printf("Unknown command type: %d\nDisconnecting client\n",header.type);
			close(server_socket);
			return -1;
	}

	return 0;

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
