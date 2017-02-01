#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "client.h"
#include "clientcontext_linked_list.h"
#include "clientcontext_hashtable.h"

ClientContext_Hashtable* ClientContext_Hashtable_Create(unsigned int init_size) {
	ClientContext_Hashtable* clientcontext_hashtable = (ClientContext_Hashtable*)malloc(sizeof(ClientContext_Hashtable));
	clientcontext_hashtable->size = init_size;
	clientcontext_hashtable->count = 0;
	clientcontext_hashtable->load_factor = 0.75f;
	clientcontext_hashtable->growth_factor = 2;
	clientcontext_hashtable->table = (ClientContext_List**)calloc(clientcontext_hashtable->size, sizeof(ClientContext_List));	
	return clientcontext_hashtable;
}

void ClientContext_Hashtable_Resize(ClientContext_Hashtable* clientcontext_hashtable) {

	unsigned int old_size = clientcontext_hashtable->size;
	unsigned int new_size = clientcontext_hashtable->size * clientcontext_hashtable->growth_factor;
	float load_factor = clientcontext_hashtable->load_factor;
	unsigned int growth_factor = clientcontext_hashtable->growth_factor;
	unsigned int new_count = 0;
	//clientcontext_hashtable->table = (ClientContext_List**)realloc(clientcontext_hashtable->table, clientcontext_hashtable->size* sizeof(ClientContext_List)); //this does not work
	
	//printf("old_size: %i\n", old_size);	
	//printf("new_size: %i\n", new_size);	
	ClientContext_List** new_table = (ClientContext_List**)calloc(new_size, sizeof(ClientContext_List));
	//recalc index on each value so remove works
	int i = 0;
	for(i=0; i<old_size; i++) {	
		//printf("i: %i\n", i);
		if(clientcontext_hashtable->table[i] != NULL) {
			ClientContext_Node* current = clientcontext_hashtable->table[i]->head->next;
			while(current != NULL) {
				//printf("before\n");	
				unsigned int new_index = ClientContext_Hashtable_HashCode(new_size, current->string_key);
				//printf("new index: %i\n", new_index);
				if(new_table[new_index] == NULL) {
					ClientContext_List* list = ClientContext_List_Create();
					new_table[new_index] = list;
					new_count++;
				} 
				ClientContext* existing = ClientContext_List_Find_By_Key(new_table[new_index], current->string_key);
				if(existing == NULL) {
					ClientContext_List_Push(new_table[new_index], current->string_key, current->ctxt);
				}
				current = current->next;
			}
		}
	}
	ClientContext_Hashtable_Destroy(clientcontext_hashtable);
	clientcontext_hashtable = (ClientContext_Hashtable*)malloc(sizeof(ClientContext_Hashtable));
	clientcontext_hashtable->size = new_size;
	clientcontext_hashtable->count = new_count;
	clientcontext_hashtable->load_factor = load_factor;
	clientcontext_hashtable->growth_factor = growth_factor;
	clientcontext_hashtable->table = new_table;
}

void ClientContext_Hashtable_Set(ClientContext_Hashtable* clientcontext_hashtable, char* key, ClientContext* ctxt) {

	unsigned int test_size = clientcontext_hashtable->load_factor * clientcontext_hashtable->size;
	
	if(clientcontext_hashtable->count == test_size) {
		ClientContext_Hashtable_Resize(clientcontext_hashtable);		
	}	

	int index = ClientContext_Hashtable_HashCode(clientcontext_hashtable->size, key);
	if(clientcontext_hashtable->table[index] == NULL) {
		ClientContext_List* list = ClientContext_List_Create();
		clientcontext_hashtable->table[index] = list;
		clientcontext_hashtable->count++;
	}
	//handle collision
	ClientContext* existing = ClientContext_List_Find_By_Key(clientcontext_hashtable->table[index], key);
	if(existing == NULL) {
		ClientContext_List_Push(clientcontext_hashtable->table[index], key, ctxt);
	}


}
ClientContext* ClientContext_Hashtable_Get(ClientContext_Hashtable* clientcontext_hashtable, char* key) {
	ClientContext* result = NULL;
	int index = ClientContext_Hashtable_HashCode(clientcontext_hashtable->size, key);
	if(clientcontext_hashtable->table[index] != NULL) {
		result = ClientContext_List_Find_By_Key(clientcontext_hashtable->table[index], key);
	}
	return result;
}

void ClientContext_Hashtable_Remove(ClientContext_Hashtable* clientcontext_hashtable, char* key) {
	int index = ClientContext_Hashtable_HashCode(clientcontext_hashtable->size, key);
	//printf("key: %s index: %i\n", key, index);
	if(clientcontext_hashtable->table[index] != NULL) {
		int size = ClientContext_List_Size(clientcontext_hashtable->table[index]);
		//printf("size: %i\n", size);
		if(size > 0) {
			ClientContext_List_Delete(clientcontext_hashtable->table[index], key);
		} 
		int new_size = ClientContext_List_Size(clientcontext_hashtable->table[index]);
		//printf("new_size: %i\n", new_size);
		if(new_size == 0) {
			ClientContext_List_Destroy(clientcontext_hashtable->table[index]);
			clientcontext_hashtable->table[index] = NULL;
			clientcontext_hashtable->count--;
		}
	}			
}

char* ClientContext_Hashtable_String(ClientContext_Hashtable* clientcontext_hashtable) {
	char* result = (char*)malloc(CLIENTCONTEXT_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', CLIENTCONTEXT_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<clientcontext_hashtable->size; i++) {
		if(clientcontext_hashtable->table[i] != NULL) {
			list = ClientContext_List_String_Keys(clientcontext_hashtable->table[i]);
			bytes = sprintf(result, "%i => %s\n", i, list);
			result+=bytes;
			total_bytes+=bytes;
		}	
	}
	result -= total_bytes;
	return result;
}

void ClientContext_Hashtable_Print(ClientContext_Hashtable* table) {
	char* result = ClientContext_Hashtable_String(table);
	printf("%s\n", result);		
	free(result);
}

void ClientContext_Hashtable_Destroy(ClientContext_Hashtable* clientcontext_hashtable) {
	//this is slow if the table is big
	//but should only be used on shutdown
	if(clientcontext_hashtable != NULL) {
		int i = 0;
		for(i=0; i<clientcontext_hashtable->size; i++) {
			if(clientcontext_hashtable->table[i] != NULL) {
				ClientContext_List_Destroy(clientcontext_hashtable->table[i]);
			}
		}
		free(clientcontext_hashtable);
	}
}	

unsigned long ClientContext_Hashtable_HashCode(unsigned int size, char *str) {
	if(str != NULL) {
		str = (unsigned char*)str;
		unsigned long hash = 5381;
		unsigned int c = 0;
		while(c = *str++) {
			hash = ((hash << 5) + hash) + c;
		}
		hash = hash % size;
		hash = (hash < 0) ? hash * -1 : hash;
		return hash;
	}
	return -1;
}


