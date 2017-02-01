#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "binary_linked_list.h"
#include "binary_hashtable.h"

Binary_Hashtable* Binary_Hashtable_Create(unsigned int init_size) {
	Binary_Hashtable* binary_hashtable = (Binary_Hashtable*)malloc(sizeof(Binary_Hashtable));
	binary_hashtable->size = init_size;
	binary_hashtable->count = 0;
	binary_hashtable->load_factor = 0.75f;
	binary_hashtable->growth_factor = 2;
	binary_hashtable->table = (Binary_List**)calloc(binary_hashtable->size, sizeof(Binary_List));	
	return binary_hashtable;
}

void Binary_Hashtable_Resize(Binary_Hashtable* binary_hashtable) {

	unsigned int old_size = binary_hashtable->size;
	unsigned int new_size = binary_hashtable->size * binary_hashtable->growth_factor;
	float load_factor = binary_hashtable->load_factor;
	unsigned int growth_factor = binary_hashtable->growth_factor;
	unsigned int new_count = 0;
	//binary_hashtable->table = (Binary_List**)realloc(binary_hashtable->table, binary_hashtable->size* sizeof(Binary_List)); //this does not work
	
	//printf("old_size: %i\n", old_size);	
	//printf("new_size: %i\n", new_size);	
	Binary_List** new_table = (Binary_List**)calloc(new_size, sizeof(Binary_List));
	//recalc index on each value so remove works
	int i = 0;
	for(i=0; i<old_size; i++) {	
		//printf("i: %i\n", i);
		if(binary_hashtable->table[i] != NULL) {
			Binary_Node* current = binary_hashtable->table[i]->head->next;
			while(current != NULL) {
				//printf("before\n");	
				unsigned int new_index = Binary_Hashtable_HashCode(new_size, current->string_key);
				//printf("new index: %i\n", new_index);
				if(new_table[new_index] == NULL) {
					Binary_List* list = Binary_List_Create();
					new_table[new_index] = list;
					new_count++;
				} 
				unsigned char* existing = Binary_List_Find_By_Key(new_table[new_index], current->string_key);
				if(existing == NULL) {
					Binary_List_Push(new_table[new_index], current->string_key, current->size, current->data);
				}
				current = current->next;
			}
		}
	}
	Binary_Hashtable_Destroy(binary_hashtable);
	binary_hashtable = (Binary_Hashtable*)malloc(sizeof(Binary_Hashtable));
	binary_hashtable->size = new_size;
	binary_hashtable->count = new_count;
	binary_hashtable->load_factor = load_factor;
	binary_hashtable->growth_factor = growth_factor;
	binary_hashtable->table = new_table;
}

void Binary_Hashtable_Set(Binary_Hashtable* binary_hashtable, char* key, unsigned int size, unsigned char* data) {

	unsigned int test_size = binary_hashtable->load_factor * binary_hashtable->size;
	
	if(binary_hashtable->count == test_size) {
		Binary_Hashtable_Resize(binary_hashtable);		
	}	

	int index = Binary_Hashtable_HashCode(binary_hashtable->size, key);
	if(binary_hashtable->table[index] == NULL) {
		Binary_List* list = Binary_List_Create();
		binary_hashtable->table[index] = list;
		binary_hashtable->count++;
	}
	//handle collision
	unsigned char* existing = Binary_List_Find_By_Key(binary_hashtable->table[index], key);
	if(existing == NULL) {
		Binary_List_Push(binary_hashtable->table[index], key, size, data);
	}


}
unsigned char* Binary_Hashtable_Get(Binary_Hashtable* binary_hashtable, char* key) {
	unsigned char* result = NULL;
	int index = Binary_Hashtable_HashCode(binary_hashtable->size, key);
	if(binary_hashtable->table[index] != NULL) {
		result = Binary_List_Find_By_Key(binary_hashtable->table[index], key);
	}
	return result;
}

unsigned int Binary_Hashtable_Get_Size(Binary_Hashtable* binary_hashtable, char* key) {
	unsigned int result = 0;
	int index = Binary_Hashtable_HashCode(binary_hashtable->size, key);
	if(binary_hashtable->table[index] != NULL) {
		result = Binary_List_Find_Size_By_Key(binary_hashtable->table[index], key);
	}
	return result;
}

void Binary_Hashtable_Remove(Binary_Hashtable* binary_hashtable, char* key) {
	int index = Binary_Hashtable_HashCode(binary_hashtable->size, key);
	//printf("key: %s index: %i\n", key, index);
	if(binary_hashtable->table[index] != NULL) {
		int size = Binary_List_Size(binary_hashtable->table[index]);
		//printf("size: %i\n", size);
		if(size > 0) {
			Binary_List_Delete(binary_hashtable->table[index], key);
		} 
		int new_size = Binary_List_Size(binary_hashtable->table[index]);
		//printf("new_size: %i\n", new_size);
		if(new_size == 0) {
			Binary_List_Destroy(binary_hashtable->table[index]);
			binary_hashtable->table[index] = NULL;
			binary_hashtable->count--;
		}
	}			
}

char* Binary_Hashtable_String(Binary_Hashtable* binary_hashtable) {
	char* result = (char*)malloc(BINARY_HASHTABLE_OUTPUT_SIZE);
	memset(result, '\0', BINARY_HASHTABLE_OUTPUT_SIZE);
	int i = 0;
	int bytes = 0;
	int total_bytes = 0;
	char* list;
	for(i=0; i<binary_hashtable->size; i++) {
		if(binary_hashtable->table[i] != NULL) {
			list = Binary_List_String_Keys(binary_hashtable->table[i]);
			bytes = sprintf(result, "%i => %s\n", i, list);
			result+=bytes;
			total_bytes+=bytes;
		}	
	}
	result -= total_bytes;
	return result;
}

void Binary_Hashtable_Print(Binary_Hashtable* table) {
	char* result = Binary_Hashtable_String(table);
	printf("%s\n", result);		
	free(result);
}

void Binary_Hashtable_Destroy(Binary_Hashtable* binary_hashtable) {
	//this is slow if the table is big
	//but should only be used on shutdown
	if(binary_hashtable != NULL) {
		int i = 0;
		for(i=0; i<binary_hashtable->size; i++) {
			if(binary_hashtable->table[i] != NULL) {
				Binary_List_Destroy(binary_hashtable->table[i]);
			}
		}
		free(binary_hashtable);
	}
}	

unsigned long Binary_Hashtable_HashCode(unsigned int size, char *str) {
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


