#ifndef _BINARY_LINKED_LIST_H_
#define _BINARY_LINKED_LIST_H_

#define BINARY_STRING_MAX_SIZE 8192 
#define BINARY_LIST_STRING_LENGTH 1000000 

typedef struct Binary_Node {
	char string_key[BINARY_STRING_MAX_SIZE];
	unsigned char* data;
	unsigned int size;
	struct Binary_Node* next;
} Binary_Node;

typedef struct Binary_List {
	struct Binary_Node* head;
	struct Binary_Node* current;
}Binary_List;

Binary_Node* Binary_Node_Create(char* string_key, unsigned int size, unsigned char* data);
void Binary_Node_Destroy(Binary_Node* node);
void Binary_List_Destroy(Binary_List* list);
Binary_List* Binary_List_Create();
void Binary_List_Push(Binary_List* list, char* string_key, unsigned int size, unsigned char* data);
int Binary_List_Size(Binary_List* list);
void Binary_List_Delete(Binary_List* list, char* string_key);
unsigned int Binary_List_Find_Size_By_Key(Binary_List* list, char* string_key);
unsigned char* Binary_List_Find_By_Key(Binary_List* list, char* string_key);
char* Binary_List_String_Keys(Binary_List* list);
void Binary_List_Print(Binary_List *list);

#endif

