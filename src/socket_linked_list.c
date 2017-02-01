#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "sock.h"
#include "socket_linked_list.h"


Socket_Node* Socket_Node_Create(int fd_id, Socket* sock);
void Socket_Node_Destroy(Socket_Node* node);
void Socket_List_Destroy(Socket_List* list);
Socket_List* Socket_List_Create();
void Socket_List_Push(Socket_List* list, int fd_id, Socket* sock);
int Socket_List_Size(Socket_List* list);
char* Socket_List_String_Keys(Socket_List* list);
void Socket_List_Delete(Socket_List* list, int fd_id);
Socket* Socket_List_Find_By_Key(Socket_List* list, int fd_id);
char* Socket_List_String_Keys(Socket_List* list);
void Socket_List_Print(Socket_List *list);

Socket_Node* Socket_Node_Create(int fd_id, Socket* sock) {
	Socket_Node* node = (Socket_Node*)malloc(sizeof(Socket_Node));
	node->fd_id = fd_id;
	node->sock = sock;
	node->next = NULL;
	return node;
}

void Socket_Node_Destroy(Socket_Node* node) {
	free(node);
}

Socket_List* Socket_List_Create() {
	Socket_List* list = (Socket_List*)malloc(sizeof(Socket_List));
	Socket* sock = Socket_Create(0, NULL, NULL);
	list->head = Socket_Node_Create(0, sock);
	list->current = list->head;
	return list;
}
void Socket_List_Destroy(Socket_List* list) {
	if(list != NULL) {
		Socket_Node* current = list->head->next;
		while(current != NULL) {
			Socket_Node* temp = current;
			current = temp->next;
			free(temp);
		}
		free(list);
	}
}

void Socket_List_Push(Socket_List* list, int fd_id, Socket* sock) {
	//push of new values
	Socket_Node* node = Socket_Node_Create(fd_id, sock);
	list->current->next = node;
	list->current = node;
}

int Socket_List_Size(Socket_List* list) {
	int result = 0;
	Socket_Node* current = list->head->next;
	while(current != NULL) {
		current = current->next;
		result++;
	}	
	return result;
}

void Socket_List_Delete(Socket_List* list, int fd_id) {
	Socket_Node* current = list->head->next;
	Socket_Node* prev = list->head;
	while(current != NULL) {
		//printf("prev: %s current: %s next: %s\n", prev->fd_id, current->fd_id, current->next->fd_id);
		if(fd_id == current->fd_id) {
			prev->next = current->next;
			Socket_Node_Destroy(current);
			break;	
		}
		prev = current;
		current = current->next;
	}

}

Socket* Socket_List_Find_By_Key(Socket_List* list, int fd_id) {
	Socket_Node* current = list->head->next;
	Socket* result = NULL;
	while(current != NULL) {
		if(fd_id == current->fd_id) {
			return current->sock;
		}
		current = current->next;
	}
	return result;
}

char* Socket_List_String_Keys(Socket_List* list) {
	char opening_char = '[';
	char closing_char = ']';
	char delimiter = ' ';
	int size = Socket_List_Size(list);
	if(sizeof(int) * size > SOCKET_LIST_STRING_LENGTH) {
		fprintf(stderr, "output string to large %i > %i\n", sizeof(int)*size, SOCKET_LIST_STRING_LENGTH);
		return NULL;
	}
	//account for spaces
	int space_size = size - 1;
	int brackets_offset = 2;
	char* result = (char*)malloc( (sizeof(int)*size) + space_size + brackets_offset);
	int bytes = 0;
	int total_bytes = 0;
	int i = 0;
	bytes = sprintf(result, "%c", opening_char);
	result += bytes;
	total_bytes += bytes;
	Socket_Node* current = list->head->next;
	while(current != NULL) {
		bytes = sprintf(result, "%i", current->fd_id);
		result += bytes;
		total_bytes += bytes;
		if(i != size-1) {
			bytes = sprintf(result, "%c", delimiter);
			result += bytes;	
			total_bytes += bytes;
		}
		current = current->next;
		i++;
	}
	bytes = sprintf(result, "%c", closing_char);
	result += bytes;
	total_bytes += bytes;
	result -= total_bytes;
	return result;
}

void Socket_List_Print(Socket_List *list) {
	char* string = Socket_List_String_Keys(list);
	printf("%s\n", string);
	free(string);
}

