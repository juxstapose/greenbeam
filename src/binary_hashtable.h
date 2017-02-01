#ifndef _BINARY_HASHTABLE_H_
#define _BINARY_HASHTABLE_H_

#include "binary_linked_list.h"

#define BINARY_HASHTABLE_MAX_ITEMS 10000
#define BINARY_HASHTABLE_OUTPUT_SIZE 250000

typedef struct Binary_Hashtable {
	unsigned int size;
	unsigned int count;
	float load_factor;
	unsigned int growth_factor;
	Binary_List** table;
} Binary_Hashtable;

Binary_Hashtable* Binary_Hashtable_Create(unsigned int init_size);	
void Binary_Hashtable_Set(Binary_Hashtable* binary_hashtable, char* key, unsigned int size, unsigned char* data);
unsigned int Binary_Hashtable_Get_Size(Binary_Hashtable* binary_hashtable, char* key);
unsigned char* Binary_Hashtable_Get(Binary_Hashtable* binary_hashtable, char* key);
void Binary_Hashtable_Remove(Binary_Hashtable* binary_hashtable, char* key);
char* Binary_Hashtable_String(Binary_Hashtable* binary_hashtable);
void Binary_Hashtable_Print(Binary_Hashtable* table);
void Binary_Hashtable_Destroy(Binary_Hashtable* binary_hashtable);
void Binary_Hashtable_Resize(Binary_Hashtable* binary_hashtable);
unsigned long Binary_Hashtable_HashCode(unsigned int size, char *str);

#endif


