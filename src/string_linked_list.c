#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_linked_list.h"

String_Node* String_Node_Create(char* string_key, char* data) {
	String_Node* node = (String_Node*)malloc(sizeof(String_Node));
	if(string_key != NULL) {
		strcpy(node->string_key, string_key);
	}
	if(data != NULL) {
		strcpy(node->data, data);
	}
	node->next = NULL;
	return node;
}

void String_Node_Destroy(String_Node* node) {
	free(node);
}

String_List* String_List_Create() {
	String_List* list = (String_List*)malloc(sizeof(String_List));
	list->head = String_Node_Create(NULL, NULL);
	list->current = list->head;
	return list;
}
void String_List_Destroy(String_List* list) {
	if(list != NULL) {
		String_Node* current = list->head->next;
		while(current != NULL) {
			String_Node* temp = current;
			current = temp->next;
			free(temp);
		}
		free(list);
	}
}

void String_List_Push(String_List* list, char* string_key, char* data) {
	//push of new values
	String_Node* node = String_Node_Create(string_key, data);
	list->current->next = node;
	list->current = node;
}

int String_List_Size(String_List* list) {
	int result = 0;
	String_Node* current = list->head->next;
	while(current != NULL) {
		current = current->next;
		result++;
	}	
	return result;
}

void String_List_Delete(String_List* list, char* string_key) {
	String_Node* current = list->head->next;
	String_Node* prev = list->head;
	while(current != NULL) {
		//printf("prev: %s current: %s next: %s\n", prev->string_key, current->string_key, current->next->string_key);
		if(strcmp(string_key, current->string_key) == 0) {
			prev->next = current->next;
			String_Node_Destroy(current);
			break;	
		}
		prev = current;
		current = current->next;
	}

}

char* String_List_Find_By_Key(String_List* list, char* string_key) {
	String_Node* current = list->head->next;
	char* result = NULL;
	while(current != NULL) {
		if(strcmp(string_key, current->string_key) == 0) {
			return current->data;
		}
		current = current->next;
	}
	return result;
}

char* String_List_String_Keys(String_List* list) {
	char opening_char = '[';
	char closing_char = ']';
	char delimiter = ' ';
	int size = String_List_Size(list);
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
	String_Node* current = list->head->next;
	while(current != NULL) {
		bytes = sprintf(result, "%s=%s", current->string_key, current->data);
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

void String_List_Print(String_List *list) {
	char* string = String_List_String_Keys(list);
	printf("%s\n", string);
	free(string);
}
