#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "clientcontext_linked_list.h"

ClientContext_Node* ClientContext_Node_Create(char* string_key, ClientContext* ctxt) {
	ClientContext_Node* node = (ClientContext_Node*)malloc(sizeof(ClientContext_Node));
	if(string_key != NULL) {
		strcpy(node->string_key, string_key);
	}
	if(data != NULL) {
		strcpy(node->data, data);
	}
	node->next = NULL;
	return node;
}

void ClientContext_Node_Destroy(ClientContext_Node* node) {
	free(node);
}

ClientContext_List* ClientContext_List_Create() {
	ClientContext_List* list = (ClientContext_List*)malloc(sizeof(ClientContext_List));
	list->head = ClientContext_Node_Create(NULL, NULL);
	list->current = list->head;
	return list;
}
void ClientContext_List_Destroy(ClientContext_List* list) {
	if(list != NULL) {
		ClientContext_Node* current = list->head->next;
		while(current != NULL) {
			ClientContext_Node* temp = current;
			current = temp->next;
			free(temp);
		}
		free(list);
	}
}

void ClientContext_List_Push(ClientContext_List* list, char* string_key, ClientContext* ctxt) {
	//push of new values
	ClientContext_Node* node = ClientContext_Node_Create(string_key, ctxt);
	list->current->next = node;
	list->current = node;
}

int ClientContext_List_Size(ClientContext_List* list) {
	int result = 0;
	ClientContext_Node* current = list->head->next;
	while(current != NULL) {
		current = current->next;
		result++;
	}	
	return result;
}

void ClientContext_List_Delete(ClientContext_List* list, char* string_key) {
	ClientContext_Node* current = list->head->next;
	ClientContext_Node* prev = list->head;
	while(current != NULL) {
		//printf("prev: %s current: %s next: %s\n", prev->string_key, current->string_key, current->next->string_key);
		if(strcmp(string_key, current->string_key) == 0) {
			prev->next = current->next;
			ClientContext_Node_Destroy(current);
			break;	
		}
		prev = current;
		current = current->next;
	}

}

char* ClientContext_List_Find_By_Key(ClientContext_List* list, char* string_key) {
	ClientContext_Node* current = list->head->next;
	char* result = NULL;
	while(current != NULL) {
		if(strcmp(string_key, current->string_key) == 0) {
			return current->data;
		}
		current = current->next;
	}
	return result;
}

char* ClientContext_List_String_Keys(ClientContext_List* list) {
	char opening_char = '[';
	char closing_char = ']';
	char delimiter = ' ';
	int size = ClientContext_List_Size(list);
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
	ClientContext_Node* current = list->head->next;
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

void ClientContext_List_Print(ClientContext_List *list) {
	char* string = ClientContext_List_String_Keys(list);
	printf("%s\n", string);
	free(string);
}

