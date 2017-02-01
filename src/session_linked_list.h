#ifndef _SESSION_LINKED_LIST_H_
#define _SESSION_LINKED_LIST_H_

#include "session.h"

#define SESSION_STRING_MAX_SIZE 4096 
#define SESSION_LIST_STRING_LENGTH 1000000 

typedef struct Session Session;

typedef struct Session_Node {
	char string_key[SESSION_STRING_MAX_SIZE];
	struct Session* session;
	struct Session_Node* next;
} Session_Node;

typedef struct Session_List {
	struct Session_Node* head;
	struct Session_Node* current;
}Session_List;

Session_Node* Session_Node_Create(char* string_key, Session* session);
void Session_Node_Destroy(Session_Node* node);
void Session_List_Destroy(Session_List* list);
Session_List* Session_List_Create();
void Session_List_Push(Session_List* list, char* string_key, Session* session);
int Session_List_Size(Session_List* list);
char* Session_List_String_Keys(Session_List* list);
void Session_List_Delete(Session_List* list, char* string_key);
Session* Session_List_Find_By_Key(Session_List* list, char* string_key);
char* Session_List_String_Keys(Session_List* list);
void Session_List_Print(Session_List *list);

#endif
