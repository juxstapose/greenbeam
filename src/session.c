#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "sock.h"
#include "session.h"

Session* Session_Create(char* session_token, char* username, char* password, char* email, int current_pos_x, int current_pos_y, Socket* sock) {
	Session* session = (Session*)malloc(sizeof(Session));
	if(session_token != NULL) {
		strcpy(session->session_token, session_token);
	} else {
		memset(session->session_token, '\0', SESSION_LENGTH+1);
		memset(session->session_token, '0', SESSION_LENGTH);
	}	
	if(username != NULL) {
		strcpy(session->username, username);
	} 
	if(password != NULL) {
		strcpy(session->password, password);
	} 
	if(email != NULL) {
		strcpy(session->email, email);
	}
	session->current_pos_x = current_pos_x;
	session->current_pos_y = current_pos_y;
	session->sock = sock;
	return session;
}

void Session_Destroy(Session* session) {
	if(session != NULL) {
		free(session);
	}
}

char* Session_String(Session* session) {
	char temp[SESSION_MAX_STRING_OUTPUT_SIZE] = {'\0'};
	int bytes = sprintf(temp, "{%s %s %s %s}", session->session_token, session->username, session->password, session->email);	
	char* result = (char*)malloc(bytes);	
	strcpy(result, temp);
	return result;
}

void Session_Print(Session* session) {
	char* string = Session_String(session);
	printf("%s\n", string);		
	free(string);
}

