#ifndef __CLIENT_H__
#define __CLIENT_H__

#include "chat.h"

/**
 * Connect to the server on a given address and port
 * 	server: The server to connect to
 * 	port: the service to connect on
 * Returns a socket connected to the server, -1 on failure
 */
int server_connect(char* server, char* port);
/**
 * Start the client chat program
 * It connects to the server and starts up the curses interface
 * Arguments:
 *	username: The desired username
 *	server: The server to connect to
 *	port: The port to connect on
 */
int client(char* username, char* server, char* port);
/**
 * Register our username with the server. This will grant us a slot
 * on the server as well as validate our username
 * Arguments:
 *	username: our desired username
 * Returns:
 *  Our new user id on success
 * 	-1 on network error
 *  -2 if our username is already used
 *  -3 if our username is invalid
 *  -4 on all other errors
 */
int register_username(char* username);
void usage(const char* message);

struct config{
	uint16_t mode;
	struct user self;
};

struct config config;

#endif
