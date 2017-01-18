#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include "sock.h" 
#include "log.h"


Socket* Socket_Create(int id, char* ip_address, char* port) {
	Socket* sock = (Socket*)malloc(sizeof(Socket));
	sock->id = id;
	if(ip_address != NULL) {
		strcpy(sock->ip_address,ip_address);
	}
	if(port != NULL) {
		strcpy(sock->port, port);
	}
	return sock;
}

int Socket_Make_Nonblocking(Socket* sock) {
	
	if(sock != NULL) {
		int flags;
		int s;

		flags = fcntl(sock->id, F_GETFL, 0);
		if(flags == -1) {
			return -1;
		}
		flags |= O_NONBLOCK;
		s = fcntl(sock->id, F_SETFL, flags);
		if(s == -1) {
			return -1;
		}
	} else if(sock == NULL) {
		return -1;	
	}
	return 0;
}


int Socket_Send(Socket* sock, unsigned char* data, unsigned int size, LogConfig* log_config) {
	int size_to_write = size;
	int bytes_sent = 0;
	int current_size = 0;
	while(size_to_write > 0) {
		bytes_sent = send(sock->id, data+bytes_sent, size_to_write, 0);
		Log_log(log_config, LOG_DEBUG, "bytes sent:%i current_size:%i size_to_write:%i\n", bytes_sent, current_size, size_to_write);
		if(bytes_sent < 0) {
			Log_log(log_config, LOG_ERROR, "send error %s\n", strerror(errno));
			break;
		}
		if(bytes_sent == 0) {
			Log_log(log_config, LOG_DEBUG, "disconnected socket\n");
			break;
		}
		current_size += bytes_sent;
		size_to_write -= bytes_sent;
	}
	return current_size;
}

unsigned char* Socket_Recv(Socket* sock, unsigned int size, int* sock_errors, LogConfig* log_config) {
	int bytes_read = 0;
	int current_size = 0;
	int size_to_read = size;
	unsigned char* data = (unsigned char*)malloc(size_to_read);
	memset(data, '\0', size_to_read);
	*sock_errors = 0;
	while(size_to_read > 0) {
		bytes_read = recv(sock->id, data + current_size, size_to_read, 0);
		Log_log(log_config, LOG_DEBUG, "bytes read:%i current_size:%i size_to_read:%i\n", bytes_read, current_size, size_to_read);
		if(bytes_read < 0) {
			Log_log(log_config, LOG_ERROR, "recv error %s\n", strerror(errno));
			if(errno & ECONNABORTED) {
				Log_log(log_config, LOG_INFO, "sock err: disconnected socket\n");
				*sock_errors |= SOCK_DISCONNECTED;
			} else {
				*sock_errors |= SOCK_ERROR;
			}
			free(data);
			return NULL;
		}
		if(bytes_read == 0) {
			Log_log(log_config, LOG_INFO, "read 0: disconnected socket\n");
			*sock_errors |= SOCK_DISCONNECTED;
			free(data);
			return NULL;
		}
		size_to_read -= bytes_read;
		current_size += bytes_read;
	}

	return data;
}


void Socket_Destroy(Socket* sock) {
	if(sock != NULL) {
		free(sock);
	}
}
