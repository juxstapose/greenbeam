#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "string_linked_list.h"
#include "string_hashtable.h"

String_Hashtable* String_Hashtable_Create(unsigned int init_size) {
	String_Hashtable* string_hashtable = (String_Hashtable*)malloc(sizeof(String_Hashtable));
	string_hashtable->size = init_size;
	string_hashtable->count = 0;
	string_hashtable->load_factor = 0.75f;
	string_hashtable->growth_factor = 2;
	string_hashtable->table = (String_List**)calloc(string_hashtable->size, sizeof(String_List));	
	return string_hashtable;
}

void String_Hashtable_Resize(String_Hashtable* string_hashtable) {

	unsigned int old_size = string_hashtable->size;
	unsigned int new_size = string_hashtable->size * string_hashtable->growth_factor;
	float load_factor = string_hashtable->load_factor;
	unsigned int growth_factor = string_hashtable->growth_factor;
	unsigned int new_count = 0;
	//string_hashtable->table = (String_List**)realloc(string_hashtable->table, string_hashtable->size* sizeof(String_List)); //this does not work
	
	//printf("old_size: %i\n", old_size);	
	//printf("new_size: %i\n", new_size);	
	String_List** new_table = (String_List**)calloc(new_size, sizeof(String_List));
	//recalc index on each value so remove works
	int i = 0;
	for(i=0; i<old_size; i++) {	
		//printf("i: %i\n", i);
		if(string_hashtable->table[i] != NULL) {
			String_Node* current = string_hashtable->table[i]->head->next;
			while(current != NULL) {
				//printf("before\n");	
				unsigned int new_index = String_Hashtable_HashCode(new_size, current->string_key);
				//printf("new index: %i\n", new_index);
				if(new_table[new_index] == NULL) {
					String_List* list = String_List_Create();
					new_table[new_index] = list;
					new_count++;
				} 
				char* existing = String_List_Find_By_Key(new_table[new_index], current->string_key);
				if(existing == NULL) {
					String_List_Push(new_table[new_index], current->string_key, current->data);
				}
				current = current->next;
			}
		}
	}
	String_Hashtable_Destroy(string_hashtable);
	string_hashtable = (String_Hashtable*)malloc(sizeof(String_Hashtable));
	string_hashtable->size = new_size;
	string_hashtable->count = new_count;
	string_hashtable->load_factor = load_factor;
	string_hashtable->growth_factor = growth_factor;
	string_hashtable->table = new_table;
}

void String_Hashtable_Set(String_Hashtable* string_hashtable, char* key, char* data) {

	unsigned int test_size = string_hashtable->load_factor * string_hashtable->size;
	
	if(string_hashtable->count == test_size) {
		String_Hashtable_Resize(string_hashtable);		
	}	

	int index = String_Hashtable_HashCode(string_hashtable->size, key);
	if(string_hashtable->table[index] == NULL) {
		String_List* list = String_List_Create();
		string_hashtable->table[index] = list;
		string_hashtable->count++;
	}
	//handle collision
	char* existing = String_List_Find_By_Key(string_hashtable->table[index], key);
	if(existing == NULL) {
		String_List_Push(string_hashtable->table[index], key, data);
	}


}
char* String_Hashtable_Get(String_Hashtable* string_hashtable, char* key) {
	char* result = NULL;
	int index = String_Hashtable_HashCode(string_hashtable->size, key);
	if(string_hashtable->table[index] != NULL) {
		result = String_List_Find_By_Key(string_hashtable->table[index], key);
	}
	return result;
}

void String_Hashtable_Remove(String_Hashtable* string_hashtable, char* key) {
	int index = String_Hashtable_HashCode(string_hashtable->size, key);
	//printf("key: %s index: %i\n", key, index);
	if(string_hashtable->table[index] != NULL) {
		int size = String_List_Size(string_hashtable->table[index]);
		//printf("size: %i\n", size);
		if(size > 0) {
			String_List_Delete(string_hashtable->table[index], key);
		} 
		int new_size = String_List_Size(string_hashtable->table[index]);
		//printf("new_size: %i\n", new_size);
		if(new_size == 0) {
			String_List_Destroy(string_hashtable->table[index]);
			string_hashtable->table[index] = NULL;
			string_hashtable->count--;
		}
	}			
}

char* String_Hashtable_String(String_Hashtable* string_hashtable) {
	char* result = (char*)malloc(STRING_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', STRING_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<string_hashtable->size; i++) {
		if(string_hashtable->table[i] != NULL) {
			list = String_List_String_Keys(string_hashtable->table[i]);
			bytes = sprintf(result, "%i => %s\n", i, list);
			result+=bytes;
			total_bytes+=bytes;
		}	
	}
	result -= total_bytes;
	return result;
}

void String_Hashtable_Print(String_Hashtable* table) {
	char* result = String_Hashtable_String(table);
	printf("%s\n", result);		
	free(result);
}

void String_Hashtable_Destroy(String_Hashtable* string_hashtable) {
	//this is slow if the table is big
	//but should only be used on shutdown
	int i = 0;
	for(i=0; i<string_hashtable->size; i++) {
		if(string_hashtable->table[i] != NULL) {
			String_List_Destroy(string_hashtable->table[i]);
		}
	}
	free(string_hashtable);
}	

unsigned long String_Hashtable_HashCode(unsigned int size, char *str) {
	if(str != NULL) {
		str = (unsigned char*)str;
		unsigned long hash = 5381;
		unsigned int c = 0;
		while(c = *str++) {
			hash = ((hash << 5) + hash) + c;
		}
		hash = hash % size;
		hash = (hash < 0) ? hash * -1 : hash;
		return hash;
	}
	return -1;
}

