#ifndef _CLIENT_H_
#define _CLIENT_H_

#include <pthread.h>
#include "protocol.h"
#include "sock.h"
#include "queue.h"
#include "log.h"

#define CONTEXT_NAME_LENGTH 128

typedef struct ClientContext {
	char name[CONTEXT_NAME_LENGTH+1];	
	Socket* sock;
	Queue* queue;
	LogConfig* log_config;
	int stop_thread;
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_mutex_t stop_mutex;
	pthread_cond_t stop_cond;
} ClientContext;

ClientContext* ClientContext_Create(char name[CONTEXT_NAME_LENGTH+1], Socket* sock, Queue* queue, LogConfig* log_config);	
void ClientContext_Destroy(ClientContext* ctxt);
unsigned int Client_Register_Send(ClientContext* ctxt, char username[USERNAME_LENGTH+1], char password[PASSWORD_LENGTH+1], char email[EMAIL_LENGTH+1]);
unsigned int Client_Login_Send(ClientContext* ctxt, char username[USERNAME_LENGTH+1], char password[PASSWORD_LENGTH+1]);
unsigned int Client_Logout_Send(ClientContext* ctxt, char* session_token);
unsigned int Client_Ping_Send(ClientContext* ctxt, char* session_token, int current_pos_x, int current_pos_y);
unsigned int Client_Movement_Send(ClientContext* ctxt, char* session_token, unsigned short direction, unsigned short speed, unsigned frames);
unsigned int Client_Shutdown_Send(ClientContext* ctxt);
Socket* Client_Connect(char* ip_address, char* port, LogConfig* log_config, unsigned int retry);

#endif
