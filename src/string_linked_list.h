#ifndef _STRING_LINKED_LIST_H_
#define _STRING_LINKED_LIST_H_

#define STRING_MAX_SIZE 8192 
#define LIST_STRING_LENGTH 1000000 


typedef struct String_Node {
	char string_key[STRING_MAX_SIZE];
	char data[STRING_MAX_SIZE];
	struct String_Node* next;
} String_Node;

typedef struct String_List {
	struct String_Node* head;
	struct String_Node* current;
}String_List;

String_Node* String_Node_Create(char* string_key, char* data);
void String_Node_Destroy(String_Node* node);
void String_List_Destroy(String_List* list);
String_List* String_List_Create();
void String_List_Push(String_List* list, char* string_key, char* data);
int String_List_Size(String_List* list);
char* String_List_String_Keys(String_List* list);
void String_List_Delete(String_List* list, char* string_key);
char* String_List_Find_By_Key(String_List* list, char* string_key);
char* String_List_String_Keys(String_List* list);
void String_List_Print(String_List *list);

#endif
