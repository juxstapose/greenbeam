#ifndef _SOCKET_HASHTABLE_H_
#define _SOCKET_HASHTABLE_H_

#include "socket_linked_list.h"

#define SOCKET_HASHTABLE_MAX_ITEMS 10000
#define SOCKET_HASHTABLE_OUTPUT_SIZE 250000

typedef struct Socket_Hashtable {
	unsigned int size;
	unsigned int count;
	float load_factor;
	unsigned int growth_factor;
	Socket_List** table;
} Socket_Hashtable;


Socket_Hashtable* Socket_Hashtable_Create();	
void Socket_Hashtable_Set(Socket_Hashtable* sock_hashtable, int fd_id, Socket* sock);
Socket* Socket_Hashtable_Get(Socket_Hashtable* sock_hashtable, int fd_id);
void Socket_Hashtable_Remove(Socket_Hashtable* sock_hashtable, int fd_id);
char* Socket_Hashtable_String(Socket_Hashtable* sock_hashtable);
void Socket_Hashtable_Print(Socket_Hashtable* table);
void Socket_Hashtable_Destroy(Socket_Hashtable* sock_hashtable);
void Socket_Hashtable_Resize(Socket_Hashtable* sock_hashtable);
unsigned long Socket_Hashtable_HashCode(unsigned int size, int fd_id);

#endif
