#ifndef __CHAT_COMMON_H__
#define __CHAT_COMMON_H__

#include "chat.h"

int get_header(int client, struct header *header);
int send_message(int client, struct message *msg);
int send_join_request(int client, struct join_request *req);
int send_join_response(int client, struct join_response *req);
int send_user_list_request(int client);

int send_data(int client, void* data, int length, uint16_t type);
int get_data(int client, void *buffer, int length);

int read_line(unsigned char* buffer);

#endif
