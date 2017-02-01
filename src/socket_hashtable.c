#include "socket_hashtable.h"
#include "server.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Socket_Hashtable* Socket_Hashtable_Create(unsigned int init_size) {	
	Socket_Hashtable* sock_hashtable = (Socket_Hashtable*)malloc(sizeof(Socket_Hashtable));
	sock_hashtable->size = init_size;
       	sock_hashtable->count = 0;
	sock_hashtable->load_factor = 0.75f;
	sock_hashtable->growth_factor = 2;
	sock_hashtable->table = (Socket_List**)calloc(sock_hashtable->size, sizeof(Socket_List));	
	return sock_hashtable;
}

void Socket_Hashtable_Resize(Socket_Hashtable* sock_hashtable) {

	unsigned int old_size = sock_hashtable->size;
	unsigned int new_size = sock_hashtable->size * sock_hashtable->growth_factor;
	float load_factor = sock_hashtable->load_factor;
	unsigned int growth_factor = sock_hashtable->growth_factor;
	unsigned int new_count = 0;
	//sock_hashtable->table = (Socket_List**)realloc(sock_hashtable->table, sock_hashtable->size* sizeof(Socket_List)); //this does not work
	
	//printf("old_size: %i\n", old_size);	
	//printf("new_size: %i\n", new_size);	
	Socket_List** new_table = (Socket_List**)calloc(new_size, sizeof(Socket_List));
	//recalc index on each value so remove works
	int i = 0;
	for(i=0; i<old_size; i++) {	
		//printf("i: %i\n", i);
		if(sock_hashtable->table[i] != NULL) {
			Socket_Node* current = sock_hashtable->table[i]->head->next;
			while(current != NULL) {
				//printf("before\n");	
				unsigned int new_index = Socket_Hashtable_HashCode(new_size, current->fd_id);
				//printf("new index: %i\n", new_index);
				if(new_table[new_index] == NULL) {
					Socket_List* list = Socket_List_Create();
					new_table[new_index] = list;
					new_count++;
				} 
				Socket* existing = Socket_List_Find_By_Key(new_table[new_index], current->fd_id);
				if(existing == NULL) {
					Socket_List_Push(new_table[new_index], current->fd_id, current->sock);
				}
				current = current->next;
			}
		}
	}
	Socket_Hashtable_Destroy(sock_hashtable);
	sock_hashtable = (Socket_Hashtable*)malloc(sizeof(Socket_Hashtable));
	sock_hashtable->size = new_size;
	sock_hashtable->count = new_count;
	sock_hashtable->load_factor = load_factor;
	sock_hashtable->growth_factor = growth_factor;
	sock_hashtable->table = new_table;
}

void Socket_Hashtable_Set(Socket_Hashtable* sock_hashtable, int fd_id, Socket* sock) {
	unsigned int test_size = sock_hashtable->load_factor * sock_hashtable->size;
	
	if(sock_hashtable->count == test_size) {
		Socket_Hashtable_Resize(sock_hashtable);		
	}	

	int index = Socket_Hashtable_HashCode(sock_hashtable->size, fd_id);
	if(sock_hashtable->table[index] == NULL) {
		Socket_List* list = Socket_List_Create();
		sock_hashtable->table[index] = list;
		sock_hashtable->count++;
	}
	Socket* existing = Socket_List_Find_By_Key(sock_hashtable->table[index], fd_id);
	if(existing == NULL) {
		Socket_List_Push(sock_hashtable->table[index], fd_id, sock);
	}
}

Socket* Socket_Hashtable_Get(Socket_Hashtable* sock_hashtable, int fd_id) {
	Socket* result = NULL;
	int index = Socket_Hashtable_HashCode(sock_hashtable->size, fd_id);
	if(sock_hashtable->table[index] != NULL) {
		result = Socket_List_Find_By_Key(sock_hashtable->table[index], fd_id);
	}
	return result;
}

void Socket_Hashtable_Remove(Socket_Hashtable* sock_hashtable, int fd_id) {
	int index = Socket_Hashtable_HashCode(sock_hashtable->size, fd_id);
	if(sock_hashtable->table[index] != NULL) {
		int size = Socket_List_Size(sock_hashtable->table[index]);
		//printf("size: %i\n", size);
		if(size > 0) {
			Socket_List_Delete(sock_hashtable->table[index], fd_id);
		} 
		int new_size = Socket_List_Size(sock_hashtable->table[index]);
		//printf("new_size: %i\n", new_size);
		if(new_size == 0) {
			Socket_List_Destroy(sock_hashtable->table[index]);
			sock_hashtable->table[index] = NULL;
			sock_hashtable->count--;
		}
	}			
}

char* Socket_Hashtable_String(Socket_Hashtable* sock_hashtable) {
	char* result = (char*)malloc(SOCKET_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', SOCKET_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<sock_hashtable->size; i++) {
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
	if(sock_hashtable != NULL) {
		int i = 0;
		for(i=0; i<sock_hashtable->size; i++) {
			if(sock_hashtable->table[i] != NULL) {
				Socket_List_Destroy(sock_hashtable->table[i]);
			}
		}
		free(sock_hashtable);
	}
}	

unsigned long Socket_Hashtable_HashCode(unsigned int size, int fd_id) {
	unsigned long hash = (unsigned long)fd_id;
	hash = hash % size;
	hash = (hash < 0) ? hash * -1 : hash;
	return hash;
}
