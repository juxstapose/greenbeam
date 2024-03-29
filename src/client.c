#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include "client.h"
#include "protocol.h"
#include "protocol_format.h"
#include "pack.h"
#include "queue.h"

ClientContext* ClientContext_Create(char name[CONTEXT_NAME_LENGTH+1], Socket* sock, Queue* queue, LogConfig* log_config) {		
	ClientContext* ctxt = (ClientContext*)malloc(sizeof(ClientContext));
	
	if(ctxt->name != NULL) {
		strcpy(ctxt->name, name);		
	}	
	ctxt->sock = sock;
	ctxt->queue = queue;
	ctxt->log_config = log_config;
	
	ctxt->stop_thread = 0;	
	
	ctxt->mutex = PTHREAD_MUTEX_INITIALIZER; 
	ctxt->cond = PTHREAD_COND_INITIALIZER;
	
	ctxt->stop_mutex = PTHREAD_MUTEX_INITIALIZER;	
	ctxt->stop_cond = PTHREAD_COND_INITIALIZER;

	return ctxt;
}

void ClientContext_Destroy(ClientContext* ctxt) {
	if(ctxt != NULL) {
		free(ctxt);
	}
}

unsigned int Client_Register_Send(ClientContext* ctxt, char username[USERNAME_LENGTH+1], char password[PASSWORD_LENGTH+1], char email[EMAIL_LENGTH+1]) {
	unsigned char* data = Protocol_Register_Send(username, password, email);
	char* format = Protocol_Format_Get(data);
	unsigned int size_to_send = Binary_Calcsize(format);
	int bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	if(bytes_sent != size_to_send) {
		Log_log(ctxt->log_config, LOG_ERROR, "%i != %i did not send all of the data", bytes_sent, size_to_send);
		return -1;
	}
	return bytes_sent;
}

unsigned int Client_Logout_Send(ClientContext* ctxt, char* session_token) {
	unsigned char* data = Protocol_Logout_Send(session_token);
	char* format = Protocol_Format_Get(data);
	unsigned int size_to_send = Binary_Calcsize(format);
	int bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	if(bytes_sent != size_to_send) {
		Log_log(ctxt->log_config, LOG_ERROR, "%i != %i did not send all of the data", bytes_sent, size_to_send);
		return -1;
	}
	return bytes_sent;
}

unsigned int Client_Login_Send(ClientContext* ctxt, char username[USERNAME_LENGTH+1], char password[PASSWORD_LENGTH+1]) {
	unsigned char* data = Protocol_Login_Send(username, password);
	char* format = Protocol_Format_Get(data);
	unsigned int size_to_send = Binary_Calcsize(format);
	int bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	if(bytes_sent != size_to_send) {
		Log_log(ctxt->log_config, LOG_ERROR, "%i != %i did not send all of the data", bytes_sent, size_to_send);
		return -1;
	}
	return bytes_sent;
}

unsigned int Client_Ping_Send(ClientContext* ctxt, char* session_token, int current_pos_x, int current_pos_y) {
	unsigned char* data = Protocol_Ping_Send(session_token, current_pos_x, current_pos_y);
	char* format = Protocol_Format_Get(data);
	unsigned int size_to_send = Binary_Calcsize(format);
	int bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	if(bytes_sent != size_to_send) {
		Log_log(ctxt->log_config, LOG_ERROR, "%i != %i did not send all of the data", bytes_sent, size_to_send);
		return -1;
	}
	return bytes_sent;
}

unsigned int Client_Movement_Send(ClientContext* ctxt, char* session_token, unsigned short direction, unsigned short speed, unsigned frames) {
	unsigned char* data = Protocol_Movement_Send(session_token, direction, speed, frames);
	char* format = Protocol_Format_Get(data);
	unsigned int size_to_send = Binary_Calcsize(format);
	int bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	if(bytes_sent != size_to_send) {
		Log_log(ctxt->log_config, LOG_ERROR, "%i != %i did not send all of the data", bytes_sent, size_to_send);
		return -1;
	}
	return bytes_sent;
}

unsigned int Client_Shutdown_Send(ClientContext* ctxt) {
	unsigned char* data = Protocol_Shutdown_Send();
	char* format = Protocol_Format_Get(data);
	unsigned int size_to_send = Binary_Calcsize(format);
	int bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	if(bytes_sent != size_to_send) {
		Log_log(ctxt->log_config, LOG_ERROR, "%i != %i did not send all of the data", bytes_sent, size_to_send);
		return -1;
	}
	return bytes_sent;
}

Socket* Client_Connect(char* ip_address, char* port, LogConfig* log_config, unsigned int retry) {
	
	Socket* sock = NULL;
	
	int status;
	struct addrinfo config;
	struct addrinfo* servinfo;

	memset(&config, 0, sizeof(config));	
	config.ai_family = AF_UNSPEC;
	config.ai_socktype = SOCK_STREAM;
	
	Log_log(log_config, LOG_DEBUG, "get information about this ip address and port (%s,%s)\n", ip_address, port);
	status = getaddrinfo(ip_address, port, &config, &servinfo);
	if(status != 0) {
		Log_log(log_config, LOG_ERROR, "getaddrinfo error %s\n", gai_strerror(status));
	}
	
	Log_log(log_config, LOG_DEBUG, "create socket\n");
	int id = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(id == -1) {
		Log_log(log_config, LOG_ERROR, "socket creation error %s\n", strerror(errno));
	}
	sock = Socket_Create(id, ip_address, port);	
	
	int i = 0;
	int is_connected = 1;	
	for(i=0; i<retry; i++) {
		Log_log(log_config, LOG_DEBUG, "connect socket %i\n", id);
		status = connect(id, servinfo->ai_addr, servinfo->ai_addrlen);
		Log_log(log_config, LOG_DEBUG, "connect status %i\n", status);
		if(status < 0) {
			Log_log(log_config, LOG_ERROR, "connect error %s\n", strerror(errno));
			is_connected = 0;
		} else if(status == 0) {
			break;
			is_connected = 1;
		}
		sleep(1);
	}

	if(is_connected == 1) {
		if(sock !=NULL) {
			Log_log(log_config, LOG_DEBUG, "make socket nonblocking\n");
			status = Socket_Make_Nonblocking(sock);
			Log_log(log_config, LOG_DEBUG, "nonblocking status %i\n", status);
			if(status == -1) {
				Log_log(log_config, LOG_ERROR, "make socket non blocking failed\n");
			}
		}
	}
	
	return sock;	
}

