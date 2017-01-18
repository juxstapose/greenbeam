#ifndef _SESSION_H_
#define _SESSION_H_

#include "sock.h"
#include "protocol.h"

#define SESSION_MAX_STRING_OUTPUT_SIZE 1024

typedef struct Session {
	char username[USERNAME_LENGTH + 1];
	char session_token[SESSION_LENGTH +1];
	char password[PASSWORD_LENGTH + 1];
	char email[EMAIL_LENGTH + 1];
	int current_pos_x;
	int current_pos_y;
	Socket* sock;
} Session;

Session* Session_Create(char* session_token, char* username, char* password, char* email, int current_pos_x, int current_pos_y, Socket* sock);
void Session_Destroy(Session* session);
char* Session_String(Session* session);
void Session_Print(Session* session);

#endif
