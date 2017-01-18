#include "socket_hashtable.h"
#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Socket_Hashtable* Socket_Hashtable_Create() {	
	Socket_Hashtable* sock_hashtable = (Socket_Hashtable*)malloc(sizeof(Socket_Hashtable));
	int i = 0;
	for(i=0; i<SESSION_HASHTABLE_MAX_ITEMS; i++) {
		sock_hashtable->table[i] = NULL;
	}
	return sock_hashtable;
}

void Socket_Hashtable_Set(Socket_Hashtable* sock_hashtable, int fd_id, Socket* sock) {
	int index = fd_id;
	if(sock_hashtable->table[index] == NULL) {
		Socket_List* list = Socket_List_Create();
		sock_hashtable->table[index] = list;
	}
	Socket* existing = Socket_List_Find_By_Key(sock_hashtable->table[index], fd_id);
	if(existing == NULL) {
		Socket_List_Push(sock_hashtable->table[index], fd_id, sock);
	}
}

Socket* Socket_Hashtable_Get(Socket_Hashtable* sock_hashtable, int fd_id) {
	Socket* result = NULL;
	int index = fd_id;
	if(sock_hashtable->table[index] != NULL) {
		result = Socket_List_Find_By_Key(sock_hashtable->table[index], fd_id);
	}
	return result;
}

void Socket_Hashtable_Remove(Socket_Hashtable* sock_hashtable, int fd_id) {
	int index = fd_id;
	if(sock_hashtable->table[index] != NULL) {
		Socket_List_Delete(sock_hashtable->table[index], fd_id);
	}			
}

char* Socket_Hashtable_String(Socket_Hashtable* sock_hashtable) {
	char* result = (char*)malloc(SESSION_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', SESSION_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<SESSION_HASHTABLE_MAX_ITEMS; i++) {
		if(sock_hashtable->table[i] != NULL) {
			list = Socket_List_String_Keys(sock_hashtable->table[i]);
			bytes = sprintf(result, "%i => %s\n", i, list);
			result+=bytes;
			total_bytes+=bytes;
		}	
	}
	result -= total_bytes;
	return result;
}

void Socket_Hashtable_Print(Socket_Hashtable* table) {
	char* result = Socket_Hashtable_String(table);
	printf("%s\n", result);		
	free(result);
}

void Socket_Hashtable_Destroy(Socket_Hashtable* sock_hashtable) {
	//this is slow if the table is big
	//but should only be used on shutdown
	int i = 0;
	for(i=0; i<SESSION_HASHTABLE_MAX_ITEMS; i++) {
		if(sock_hashtable->table[i] != NULL) {
			Socket_List_Destroy(sock_hashtable->table[i]);
		}
	}
	free(sock_hashtable);
}	

