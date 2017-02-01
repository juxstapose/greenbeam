#ifndef _CLIENTCONTEXT_LINKED_LIST_H_
#define _CLIENTCONTEXT_LINKED_LIST_H_

#define STRING_MAX_SIZE 8192 
#define LIST_STRING_LENGTH 1000000 


typedef struct ClientContext_Node {
	char string_key[STRING_MAX_SIZE];
	ClientContext* ctxt;
	struct ClientContext_Node* next;
} ClientContext_Node;

typedef struct ClientContext_List {
	struct ClientContext_Node* head;
	struct ClientContext_Node* current;
}ClientContext_List;

ClientContext_Node* ClientContext_Node_Create(char* string_key, ClientContext* ctxt);
void ClientContext_Node_Destroy(ClientContext_Node* node);
void ClientContext_List_Destroy(ClientContext_List* list);
ClientContext_List* ClientContext_List_Create();
void ClientContext_List_Push(ClientContext_List* list, char* string_key, ClientContext* ctxt);
int ClientContext_List_Size(ClientContext_List* list);
char* ClientContext_List_String_Keys(ClientContext_List* list);
void ClientContext_List_Delete(ClientContext_List* list, char* string_key);
char* ClientContext_List_Find_By_Key(ClientContext_List* list, char* string_key);
void ClientContext_List_Print(ClientContext_List *list);

#endif

