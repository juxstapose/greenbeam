#ifndef _SESSION_HASHTABLE_H_
#define _SESSION_HASHTABLE_H_

#include "session_linked_list.h"

#define SESSION_HASHTABLE_MAX_ITEMS 10000
#define SESSION_HASHTABLE_OUTPUT_SIZE 25000

//chaining for collisions
typedef struct Session_Hashtable {	
	Session_List* table[SESSION_HASHTABLE_MAX_ITEMS];
} Session_Hashtable;

Session_Hashtable* Session_Hashtable_Create();	
void Session_Hashtable_Set(Session_Hashtable* session_hashtable, char* key, Session* session);
Session* Session_Hashtable_Get(Session_Hashtable* session_hashtable, char* key);
void Session_Hashtable_Remove(Session_Hashtable* session_hashtable, char* key);
char* Session_Hashtable_String(Session_Hashtable* session_hashtable);
void Session_Hashtable_Print(Session_Hashtable* table);
Session_Hashtable* Session_Hashtable_Destroy(Session_Hashtable* session_hashtable);
unsigned long Session_Hashtable_HashCode(char *str);


#endif
