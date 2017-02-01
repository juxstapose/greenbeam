#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "binary_linked_list.h"

Binary_Node* Binary_Node_Create(char* string_key, unsigned int size, unsigned char* data) {
	Binary_Node* node = (Binary_Node*)malloc(sizeof(Binary_Node));
	if(string_key != NULL) {
		strcpy(node->string_key, string_key);
	}
	node->size = size;
	if(data != NULL) {
		node->data = data;
	}
	node->next = NULL;
	return node;
}

void Binary_Node_Destroy(Binary_Node* node) {
	free(node);
}

Binary_List* Binary_List_Create() {
	Binary_List* list = (Binary_List*)malloc(sizeof(Binary_List));
	list->head = Binary_Node_Create(NULL, 0, NULL);
	list->current = list->head;
	return list;
}
void Binary_List_Destroy(Binary_List* list) {
	if(list != NULL) {
		Binary_Node* current = list->head->next;
		while(current != NULL) {
			Binary_Node* temp = current;
			current = temp->next;
			free(temp);
		}
		free(list);
	}
}

void Binary_List_Push(Binary_List* list, char* string_key, unsigned int size, unsigned char* data) {
	//push of new values
	Binary_Node* node = Binary_Node_Create(string_key, size, data);
	list->current->next = node;
	list->current = node;
}

int Binary_List_Size(Binary_List* list) {
	int result = 0;
	Binary_Node* current = list->head->next;
	while(current != NULL) {
		current = current->next;
		result++;
	}	
	return result;
}

void Binary_List_Delete(Binary_List* list, char* string_key) {
	Binary_Node* current = list->head->next;
	Binary_Node* prev = list->head;
	while(current != NULL) {
		//printf("prev: %s current: %s next: %s\n", prev->string_key, current->string_key, current->next->string_key);
		if(strcmp(string_key, current->string_key) == 0) {
			prev->next = current->next;
			Binary_Node_Destroy(current);
			break;	
		}
		prev = current;
		current = current->next;
	}

}

unsigned int Binary_List_Find_Size_By_Key(Binary_List* list, char* string_key) {
	Binary_Node* current = list->head->next;
	unsigned int result = 0;
	while(current != NULL) {
		if(strcmp(string_key, current->string_key) == 0) {
			return current->size;
		}
		current = current->next;
	}
	return result;
}

unsigned char* Binary_List_Find_By_Key(Binary_List* list, char* string_key) {
	Binary_Node* current = list->head->next;
	char* result = NULL;
	while(current != NULL) {
		if(strcmp(string_key, current->string_key) == 0) {
			return current->data;
		}
		current = current->next;
	}
	return result;
}

char* Binary_List_String_Keys(Binary_List* list) {
	char opening_char = '[';
	char closing_char = ']';
	char delimiter = ' ';
	int size = Binary_List_Size(list);
	if(BINARY_STRING_MAX_SIZE * size > BINARY_LIST_STRING_LENGTH) {
		fprintf(stderr, "output string to large %i > %i\n", BINARY_STRING_MAX_SIZE*size, BINARY_LIST_STRING_LENGTH);
		return NULL;
	}
	//account for spaces
	int space_size = size - 1;
	int brackets_offset = 2;
	char* result = (char*)malloc( (BINARY_STRING_MAX_SIZE*size) + space_size + brackets_offset);
	int bytes = 0;
	int total_bytes = 0;
	int i = 0;
	bytes = sprintf(result, "%c", opening_char);
	result += bytes;
	total_bytes += bytes;
	Binary_Node* current = list->head->next;
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

void Binary_List_Print(Binary_List *list) {
	char* string = Binary_List_String_Keys(list);
	printf("%s\n", string);
	free(string);
}
