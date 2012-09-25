#ifndef __CLIENT_H__
#define __CLIENT_H__


int server_connect(char* server, char* port);
int client(char* username, char* server, char* port);
int register_username(int server, char* username);
void usage(const char* message);

int user_id = -1;

#endif
