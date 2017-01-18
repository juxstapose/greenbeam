#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "session.h"
#include "session_linked_list.h"
#include "session_hashtable.h"

Session_Hashtable* Session_Hashtable_Create() {	
	Session_Hashtable* session_hashtable = (Session_Hashtable*)malloc(sizeof(Session_Hashtable));
	int i = 0;
	for(i=0; i<SESSION_HASHTABLE_MAX_ITEMS; i++) {
		session_hashtable->table[i] = NULL;
	}
	return session_hashtable;
}

void Session_Hashtable_Set(Session_Hashtable* session_hashtable, char* key, Session* session) {
	int index = Session_Hashtable_HashCode(key);
	if(session_hashtable->table[index] == NULL) {
		Session_List* list = Session_List_Create();
		session_hashtable->table[index] = list;
	}
	Session* existing = Session_List_Find_By_Key(session_hashtable->table[index], key);
	if(existing == NULL) {
		Session_List_Push(session_hashtable->table[index], key, session);
	}
}

Session* Session_Hashtable_Get(Session_Hashtable* session_hashtable, char* key) {
	Session* result = NULL;
	int index = Session_Hashtable_HashCode(key);
	if(session_hashtable->table[index] != NULL) {
		result = Session_List_Find_By_Key(session_hashtable->table[index], key);
	}
	return result;
}

void Session_Hashtable_Remove(Session_Hashtable* session_hashtable, char* key) {
	int index = Session_Hashtable_HashCode(key);
	if(session_hashtable->table[index] != NULL) {
		Session_List_Delete(session_hashtable->table[index], key);
	}			
}

char* Session_Hashtable_String(Session_Hashtable* session_hashtable) {
	char* result = (char*)malloc(SESSION_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', SESSION_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<SESSION_HASHTABLE_MAX_ITEMS; i++) {
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
	for(i=0; i<SESSION_HASHTABLE_MAX_ITEMS; i++) {
		if(session_hashtable->table[i] != NULL) {
			Session_List_Destroy(session_hashtable->table[i]);
		}
	}
	free(session_hashtable);
}	

unsigned long Session_Hashtable_HashCode(char *str) {
	if(str != NULL) {
		str = (unsigned char*)str;
		unsigned long hash = 5381;
		unsigned int c = 0;
		while(c = *str++) {
			hash = ((hash << 5) + hash) + c;
		}
		hash = hash % SESSION_HASHTABLE_MAX_ITEMS;
		hash = (hash < 0) ? hash * -1 : hash;
		return hash;
	}
	return -1;
}
