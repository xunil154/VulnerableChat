#ifndef __CHAT_COMMON_H__
#define __CHAT_COMMON_H__

#include "chat.h"

int get_message(int client, int length, struct message *msg);
int send_message(int client, struct message *msg);

#endif
