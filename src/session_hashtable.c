#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "session.h"
#include "session_linked_list.h"
#include "session_hashtable.h"

Session_Hashtable* Session_Hashtable_Create(unsigned int init_size) {	
	Session_Hashtable* session_hashtable = (Session_Hashtable*)malloc(sizeof(Session_Hashtable));
	session_hashtable->size = init_size;
	session_hashtable->count = 0;
	session_hashtable->load_factor = 0.75f;
	session_hashtable->growth_factor = 2;
	session_hashtable->table = (Session_List**)calloc(session_hashtable->size, sizeof(Session_List));	
	return session_hashtable;
}

void Session_Hashtable_Resize(Session_Hashtable* session_hashtable) {
	
	unsigned int old_size = session_hashtable->size;
	unsigned int new_size = session_hashtable->size * session_hashtable->growth_factor;
	float load_factor = session_hashtable->load_factor;
	unsigned int growth_factor = session_hashtable->growth_factor;
	unsigned int new_count = 0;
	//session_hashtable->table = (Session_List**)realloc(session_hashtable->table, session_hashtable->size* sizeof(Session_List)); //this does not work
	
	//printf("old_size: %i\n", old_size);	
	//printf("new_size: %i\n", new_size);	
	Session_List** new_table = (Session_List**)calloc(new_size, sizeof(Session_List));
	//recalc index on each value so remove works
	int i = 0;
	for(i=0; i<old_size; i++) {	
		//printf("i: %i\n", i);
		if(session_hashtable->table[i] != NULL) {
			Session_Node* current = session_hashtable->table[i]->head->next;
			while(current != NULL) {
				//printf("before\n");	
				unsigned int new_index = Session_Hashtable_HashCode(new_size, current->string_key);
				//printf("new index: %i\n", new_index);
				if(new_table[new_index] == NULL) {
					Session_List* list = Session_List_Create();
					new_table[new_index] = list;
					new_count++;
				} 
				Session* existing = Session_List_Find_By_Key(new_table[new_index], current->string_key);
				if(existing == NULL) {
					Session_List_Push(new_table[new_index], current->string_key, current->session);
				}
				current = current->next;
			}
		}
	}
	Session_Hashtable_Destroy(session_hashtable);
	session_hashtable = (Session_Hashtable*)malloc(sizeof(Session_Hashtable));
	session_hashtable->size = new_size;
	session_hashtable->count = new_count;
	session_hashtable->load_factor = load_factor;
	session_hashtable->growth_factor = growth_factor;
	session_hashtable->table = new_table;

}


void Session_Hashtable_Set(Session_Hashtable* session_hashtable, char* key, Session* session) {
	unsigned int test_size = session_hashtable->load_factor * session_hashtable->size;
	
	if(session_hashtable->count == test_size) {
		Session_Hashtable_Resize(session_hashtable);		
	}	

	int index = Session_Hashtable_HashCode(session_hashtable->size, key);
	if(session_hashtable->table[index] == NULL) {
		Session_List* list = Session_List_Create();
		session_hashtable->table[index] = list;
		session_hashtable->count++;
	}
	//handle collision
	Session* existing = Session_List_Find_By_Key(session_hashtable->table[index], key);
	if(existing == NULL) {
		Session_List_Push(session_hashtable->table[index], key, session);
	}
}

Session* Session_Hashtable_Get(Session_Hashtable* session_hashtable, char* key) {
	Session* result = NULL;
	int index = Session_Hashtable_HashCode(session_hashtable->size, key);
	if(session_hashtable->table[index] != NULL) {
		result = Session_List_Find_By_Key(session_hashtable->table[index], key);
	}
	return result;
}

void Session_Hashtable_Remove(Session_Hashtable* session_hashtable, char* key) {
	int index = Session_Hashtable_HashCode(session_hashtable->size, key);
	//printf("key: %s index: %i\n", key, index);
	if(session_hashtable->table[index] != NULL) {
		int size = Session_List_Size(session_hashtable->table[index]);
		//printf("size: %i\n", size);
		if(size > 0) {
			Session_List_Delete(session_hashtable->table[index], key);
		} 
		int new_size = Session_List_Size(session_hashtable->table[index]);
		//printf("new_size: %i\n", new_size);
		if(new_size == 0) {
			Session_List_Destroy(session_hashtable->table[index]);
			session_hashtable->table[index] = NULL;
			session_hashtable->count--;
		}
	}			
}

char* Session_Hashtable_String(Session_Hashtable* session_hashtable) {
	char* result = (char*)malloc(SESSION_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', SESSION_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<session_hashtable->size; i++) {
		if(session_hashtable->table[i] != NULL) {
			list = Session_List_String_Keys(session_hashtable->table[i]);
			bytes = sprintf(result, "%i => %s\n", i, list);
			result+=bytes;
			total_bytes+=bytes;
		}	
	}
	result -= total_bytes;
	return result;
}

void Session_Hashtable_Print(Session_Hashtable* table) {
	char* result = Session_Hashtable_String(table);
	printf("%s\n", result);		
	free(result);
}

Session_Hashtable* Session_Hashtable_Destroy(Session_Hashtable* session_hashtable) {
	//this is slow if the table is big
	//but should only be used on shutdown
	int i = 0;
	for(i=0; i<session_hashtable->size; i++) {
		if(session_hashtable->table[i] != NULL) {
			Session_List_Destroy(session_hashtable->table[i]);
		}
	}
	free(session_hashtable);
}	

unsigned long Session_Hashtable_HashCode(unsigned int size, char *str) {
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
