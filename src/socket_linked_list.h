#ifndef _SOCKET_LINKED_LIST_H_
#define _SOCKET_LINKED_LIST_H_

#include "sock.h"

#define SOCKET_LIST_STRING_LENGTH 100000 

typedef struct Socket_Node {
	int fd_id;
	Socket* sock;
	struct Socket_Node* next;
} Socket_Node;

typedef struct Socket_List {
	struct Socket_Node* head;
	struct Socket_Node* current;
}Socket_List;

Socket_Node* Socket_Node_Create(int fd_id, Socket* sock);
void Socket_Node_Destroy(Socket_Node* node);
void Socket_List_Destroy(Socket_List* list);
Socket_List* Socket_List_Create();
void Socket_List_Push(Socket_List* list, int fd_id, Socket* session);
int Socket_List_Size(Socket_List* list);
char* Socket_List_String_Keys(Socket_List* list);
void Socket_List_Delete(Socket_List* list, int fd_id);
Socket* Socket_List_Find_By_Key(Socket_List* list, int fd_id);
char* Socket_List_String_Keys(Socket_List* list);
void Socket_List_Print(Socket_List *list);

#endif

