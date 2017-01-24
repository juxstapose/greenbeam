#ifndef _STRING_HASHTABLE_H_
#define _STRING_HASHTABLE_H_

#include "string_linked_list.h"

#define STRING_HASHTABLE_MAX_ITEMS 10000
#define STRING_HASHTABLE_OUTPUT_SIZE 25000

typedef struct String_Hashtable {
	unsigned int size;
	unsigned int count;
	float load_factor;
	unsigned int growth_factor;
	String_List** table;
} String_Hashtable;

String_Hashtable* String_Hashtable_Create(unsigned int init_size);	
void String_Hashtable_Set(String_Hashtable* string_hashtable, char* key, char* data);
char* String_Hashtable_Get(String_Hashtable* string_hashtable, char* key);
void String_Hashtable_Remove(String_Hashtable* string_hashtable, char* key);
char* String_Hashtable_String(String_Hashtable* string_hashtable);
void String_Hashtable_Print(String_Hashtable* table);
void String_Hashtable_Destroy(String_Hashtable* string_hashtable);
void String_Hashtable_Resize(String_Hashtable* string_hashtable);
unsigned long String_Hashtable_HashCode(unsigned int size, char *str);

#endif
