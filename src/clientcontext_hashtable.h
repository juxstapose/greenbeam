#ifndef _CLIENTCONTEXT_HASHTABLE_H_
#define _CLIENTCONTEXT_HASHTABLE_H_

#include "client.h"
#include "clientcontext_linked_list.h"

#define CLIENTCONTEXT_HASHTABLE_MAX_ITEMS 10000
#define CLIENTCONTEXT_HASHTABLE_OUTPUT_SIZE 250000

typedef struct ClientContext_Hashtable {
	unsigned int size;
	unsigned int count;
	float load_factor;
	unsigned int growth_factor;
	ClientContext_List** table;
} ClientContext_Hashtable;

ClientContext_Hashtable* ClientContext_Hashtable_Create(unsigned int init_size);	
void ClientContext_Hashtable_Set(ClientContext_Hashtable* clientcontext_hashtable, char* key, ClientContext* ctxt);
char* ClientContext_Hashtable_Get(ClientContext_Hashtable* clientcontext_hashtable, char* key);
void ClientContext_Hashtable_Remove(ClientContext_Hashtable* clientcontext_hashtable, char* key);
char* ClientContext_Hashtable_String(ClientContext_Hashtable* clientcontext_hashtable);
void ClientContext_Hashtable_Print(ClientContext_Hashtable* table);
void ClientContext_Hashtable_Destroy(ClientContext_Hashtable* clientcontext_hashtable);
void ClientContext_Hashtable_Resize(ClientContext_Hashtable* clientcontext_hashtable);
unsigned long ClientContext_Hashtable_HashCode(unsigned int size, char *str);

#endif
