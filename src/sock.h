#ifndef _SOCK_H_
#define _SOCK_H_

#include <sys/socket.h>
#include <netdb.h>
#include "log.h"

#define SOCK_DISCONNECTED 1
#define SOCK_ERROR 2


typedef struct Socket {
	int id;
	char ip_address[NI_MAXHOST];
	char port[NI_MAXSERV];
	LogConfig* log_config;
} Socket;

Socket* Socket_Create(int id, char* ip_address, char* port);
int Socket_Make_Nonblocking(Socket* sock);
unsigned char* Socket_Recv(Socket* sock, unsigned int size, int* sock_errors, LogConfig* log_config);
int Socket_Send(Socket* sock, unsigned char* data, unsigned int size, LogConfig* log_config);
void Socket_Destroy(Socket* sock);

#endif
