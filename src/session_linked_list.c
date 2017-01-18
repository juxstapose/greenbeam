#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "session_linked_list.h"

Session_Node* Session_Node_Create(char* string_key, Session* session) {
	Session_Node* node = (Session_Node*)malloc(sizeof(Session_Node));
	if(string_key != NULL) {
		strcpy(node->string_key, string_key);
	}
	node->session = session;
	node->next = NULL;
	return node;
}

void Session_Node_Destroy(Session_Node* node) {
	free(node);
}

Session_List* Session_List_Create() {
	Session_List* list = (Session_List*)malloc(sizeof(Session_List));
	Session* session = Session_Create(NULL, NULL, NULL, NULL, 0, 0, NULL);
	list->head = Session_Node_Create(NULL, session);
	list->current = list->head;
	return list;
}
void Session_List_Destroy(Session_List* list) {
	if(list != NULL) {
		Session_Node* current = list->head->next;
		while(current != NULL) {
			Session_Node* temp = current;
			current = temp->next;
			free(temp);
		}
		free(list);
	}
}

void Session_List_Push(Session_List* list, char* string_key, Session* session) {
	//push of new values
	Session_Node* node = Session_Node_Create(string_key, session);
	list->current->next = node;
	list->current = node;
}

int Session_List_Size(Session_List* list) {
	int result = 0;
	Session_Node* current = list->head->next;
	while(current != NULL) {
		current = current->next;
		result++;
	}	
	return result;
}

void Session_List_Delete(Session_List* list, char* string_key) {
	Session_Node* current = list->head->next;
	Session_Node* prev = list->head;
	while(current != NULL) {
		//printf("prev: %s current: %s next: %s\n", prev->string_key, current->string_key, current->next->string_key);
		if(strcmp(string_key, current->string_key) == 0) {
			prev->next = current->next;
			Session_Node_Destroy(current);
			break;	
		}
		prev = current;
		current = current->next;
	}

}

Session* Session_List_Find_By_Key(Session_List* list, char* string_key) {
	Session_Node* current = list->head->next;
	Session* result = NULL;
	while(current != NULL) {
		if(strcmp(string_key, current->string_key) == 0) {
			return current->session;
		}
		current = current->next;
	}
	return result;
}

char* Session_List_String_Keys(Session_List* list) {
	char opening_char = '[';
	char closing_char = ']';
	char delimiter = ' ';
	int size = Session_List_Size(list);
	if(STRING_MAX_SIZE * size > LIST_STRING_LENGTH) {
		fprintf(stderr, "output string to large %i > %i\n", STRING_MAX_SIZE*size, LIST_STRING_LENGTH);
		return NULL;
	}
	//account for spaces
	int space_size = size - 1;
	int brackets_offset = 2;
	char* result = (char*)malloc( (STRING_MAX_SIZE*size) + space_size + brackets_offset);
	int bytes = 0;
	int total_bytes = 0;
	int i = 0;
	bytes = sprintf(result, "%c", opening_char);
	result += bytes;
	total_bytes += bytes;
	Session_Node* current = list->head->next;
	while(current != NULL) {
		bytes = sprintf(result, "%s", current->string_key);
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

void Session_List_Print(Session_List *list) {
	char* string = Session_List_String_Keys(list);
	printf("%s\n", string);
	free(string);
}
