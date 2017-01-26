#include "session.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "location.h"
#include "sock.h"
#include "session_hashtable.h"

Session* Session_Create(char* session_token, char* username, Location* location, 
			Session_Hashtable* session_table_inrange, Session_Hashtable* session_table_outofrange, Socket* sock) {
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
	session->location = location;
	session->session_table_inrange = session_table_inrange;
	session->session_table_outofrange = session_table_outofrange;
	session->sock = sock;
	return session;
}

void Session_Destroy(Session* session) {
	if(session != NULL) {
		Location_Destroy(session->location);
		Session_Hashtable_Destroy(session->session_table_inrange);
		Session_Hashtable_Destroy(session->session_table_outofrange);
		Socket_Destroy(session->sock);
		free(session);
	}
}

char* Session_String(Session* session) {
	char temp[SESSION_MAX_STRING_OUTPUT_SIZE] = {'\0'};
	int bytes = sprintf(temp, "{%s %s (%i,%i)}", session->session_token, session->username, session->location->x, session->location->y);	
	char* result = (char*)malloc(bytes);	
	strcpy(result, temp);
	return result;
}

void Session_Print(Session* session) {
	char* string = Session_String(session);
	printf("%s\n", string);		
	free(string);
}

