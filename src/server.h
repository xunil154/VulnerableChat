#ifndef __SERVER_H__
#define __SERVER_H__

#include "chat.h"
#include "common.h"

/**
 * Start a socket listening on a given port
 * Arguments:
 *	port: the port to listen on
 * Returns:
 *	the socket on success
 *  0 on failure
 */
int start_listening(int port);

/**
 * Start the server on a given port. This function never exits
 * Arguments:
 *	port: the port to listen on
 * Return:
 *	nothing
 */
int run_server(int port);
/**
 * Process data from the client
 * Arguments:
 *	client: The client socket to read data from
 */
int handle_client(int client);

// Checks if the bit value is set in to_test
// eg if(has_access(client_user->permissions, ADMIN_G);
int has_access(uint16_t check, uint16_t perm);

/**
 * Process a join request from a client.
 * It will setup the user information in the users table
 * as well as send the response codes to the client
 */
int join(int client, struct join_request* req);

int find_user(char* name);
int find_user_by_socket(int socket);

int remove_user(int socket);

int is_connected(int socket);

/**
 * This will broadcast a message to all users connected to the server
 * Arguments:
 *	msg: the message to send
 */
int broadcast(struct message *message);

/**
 * Process the join request
 * If the join is succesful then it will broadcast
 *	[<username> has joined the chat room]
 */
int process_join(int client, unsigned char* buffer);
/**
 * Process a general message
 * It will read the message from a user, reformat the string as
 *  <usernam>: <message
 * and broadcast to all connected users
 */
int process_message(int client, unsigned char* buffer);
/**
 * Processes a PM message from a user
 * Receives the PM message, looks up the username of the sender
 * and build a message with the format
 *	[PM from <username>: <message>]
 * and then sends this message to the intended recepiant
 */
int process_pm(int client, unsigned char* buffer);
/**
 * Process a WHO request
 * It will find all of the users who are connected and send thir user information
 * to the requesting client
 */
int process_who(int client, unsigned char* buffer);
/**
 * Process WHOIS request
 * It will lookup the username of the requested user and
 * send back the user data to the requesting client.
 * If a user does not exist it will send NO_USER as the
 * status.
 */
int process_whois(int client, unsigned char* buffer);
//int process_join(int client, unsigned char* buffer);

// array to store user informatoin
struct user users[1024];
int next_user_id = 0;

#endif
