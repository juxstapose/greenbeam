#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pack.h"
#include "session.h"
#include "session_linked_list.h"
#include "session_hashtable.h"

Session_Hashtable* Session_Hashtable_Create(unsigned int init_size) {	
	Session_Hashtable* session_hashtable = (Session_Hashtable*)malloc(sizeof(Session_Hashtable));
	session_hashtable->size = init_size;
	session_hashtable->count = 0;
	session_hashtable->load_factor = 0.75f;
	session_hashtable->growth_factor = 2;
	session_hashtable->table = (Session_List**)calloc(session_hashtable->size, sizeof(Session_List));	
	return session_hashtable;
}

void Session_Hashtable_Resize(Session_Hashtable* session_hashtable) {
	
	unsigned int old_size = session_hashtable->size;
	unsigned int new_size = session_hashtable->size * session_hashtable->growth_factor;
	float load_factor = session_hashtable->load_factor;
	unsigned int growth_factor = session_hashtable->growth_factor;
	unsigned int new_count = 0;
	//session_hashtable->table = (Session_List**)realloc(session_hashtable->table, session_hashtable->size* sizeof(Session_List)); //this does not work
	
	//printf("old_size: %i\n", old_size);	
	//printf("new_size: %i\n", new_size);	
	Session_List** new_table = (Session_List**)calloc(new_size, sizeof(Session_List));
	//recalc index on each value so remove works
	int i = 0;
	for(i=0; i<old_size; i++) {	
		//printf("i: %i\n", i);
		if(session_hashtable->table[i] != NULL) {
			Session_Node* current = session_hashtable->table[i]->head->next;
			while(current != NULL) {
				//printf("before\n");	
				unsigned int new_index = Session_Hashtable_HashCode(new_size, current->string_key);
				//printf("new index: %i\n", new_index);
				if(new_table[new_index] == NULL) {
					Session_List* list = Session_List_Create();
					new_table[new_index] = list;
					new_count++;
				} 
				Session* existing = Session_List_Find_By_Key(new_table[new_index], current->string_key);
				if(existing == NULL) {
					Session_List_Push(new_table[new_index], current->string_key, current->session);
				}
				current = current->next;
			}
		}
	}
	Session_Hashtable_Destroy(session_hashtable);
	session_hashtable = NULL;
	session_hashtable = (Session_Hashtable*)malloc(sizeof(Session_Hashtable));
	session_hashtable->size = new_size;
	session_hashtable->count = new_count;
	session_hashtable->load_factor = load_factor;
	session_hashtable->growth_factor = growth_factor;
	session_hashtable->table = new_table;

}


void Session_Hashtable_Set(Session_Hashtable* session_hashtable, char* key, Session* session) {
	unsigned int test_size = session_hashtable->load_factor * session_hashtable->size;
	
	if(session_hashtable->count == test_size) {
		Session_Hashtable_Resize(session_hashtable);		
	}	

	int index = Session_Hashtable_HashCode(session_hashtable->size, key);
	if(session_hashtable->table[index] == NULL) {
		Session_List* list = Session_List_Create();
		session_hashtable->table[index] = list;
		session_hashtable->count++;
	}
	//handle collision
	Session* existing = Session_List_Find_By_Key(session_hashtable->table[index], key);
	if(existing == NULL) {
		Session_List_Push(session_hashtable->table[index], key, session);
	}
}

Session* Session_Hashtable_Get(Session_Hashtable* session_hashtable, char* key) {
	Session* result = NULL;
	int index = Session_Hashtable_HashCode(session_hashtable->size, key);
	if(session_hashtable->table[index] != NULL) {
		result = Session_List_Find_By_Key(session_hashtable->table[index], key);
	}
	return result;
}

void Session_Hashtable_Remove(Session_Hashtable* session_hashtable, char* key) {
	int index = Session_Hashtable_HashCode(session_hashtable->size, key);
	//printf("key: %s index: %i\n", key, index);
	if(session_hashtable->table[index] != NULL) {
		int size = Session_List_Size(session_hashtable->table[index]);
		//printf("size: %i\n", size);
		if(size > 0) {
			Session_List_Delete(session_hashtable->table[index], key);
		} 
		int new_size = Session_List_Size(session_hashtable->table[index]);
		//printf("new_size: %i\n", new_size);
		if(new_size == 0) {
			Session_List_Destroy(session_hashtable->table[index]);
			session_hashtable->table[index] = NULL;
			session_hashtable->count--;
		}
	}			
}

Session_Hashtable* Session_Hashtable_Diff_New(Session_Hashtable* old_table, Session_Hashtable* new_table) {
	Session_Hashtable* result = Session_Hashtable_Create(20);
	int x = 0;
	for(x=0; x<new_table->size; x++) {	
		if(new_table->table[x] != NULL) {
			Session_List* list = (Session_List*)new_table->table[x];
			Session_Node* current = list->head->next;
			while(current != NULL) {
				Session* found = Session_Hashtable_Get(old_table, current->string_key);
				if(found == NULL) {
					Session_Hashtable_Set(result, current->string_key, current->session);
				}	
				current = current->next;
			}//end while session list
		}//end if not null
	}//end for loop		
	if(result->count == 0) {
		Session_Hashtable_Destroy(result);
		result = NULL;	
		return result;
	}
	return result;
}

void Session_Hashtable_Calc_Size_Items(Session_Hashtable* session_hashtable, unsigned int *size, unsigned int *num_items) { 
	int x = 0;
	*size = 0;
	*num_items = 0;
	if(session_hashtable != NULL) {
		for(x=0; x<session_hashtable->size; x++) {	
			if(session_hashtable->table[x] != NULL) {
				Session_List* list = (Session_List*)session_hashtable->table[x];
				Session_Node* current = list->head->next;
				while(current != NULL) {
					unsigned int username_size = strlen(current->session->username) + 1;
					char format[128] = {'\0'};
					int bytes = sprintf(format, "!I%isii", username_size);
					*size = *size + Binary_Calcsize(format); 
					*num_items = *num_items + 1;
					current = current->next;
				}//end while session list
			}//end if not null
		}//end for loop	
	}//eend if null
}//end function

unsigned char* Session_Hashtable_To_Binary(Session_Hashtable* session_hashtable, unsigned int payload_body_size, unsigned int num_items) { 
	
	char* header_format = "!IH";	
	unsigned int payload_header_size = Binary_Calcsize(header_format);
	unsigned char* payload_header = Binary_Pack(header_format, payload_body_size, num_items);
	unsigned char* payload_body = (unsigned char*)malloc(payload_body_size);
	unsigned char* payload = (unsigned char*)malloc(payload_header_size + payload_body_size);	
	//printf("payload size: %i\n", payload_header_size+ payload_body_size);	
	//printf("payload_header_size: %i\n", payload_header_size);	
	//printf("payload_body_size: %i\n", payload_body_size);	
	memcpy(payload, payload_header, payload_header_size);	
	int x = 0;
	unsigned int total_size = 0;
	if(session_hashtable != NULL) {
		for(x=0; x<session_hashtable->size; x++) {	
			if(session_hashtable->table[x] != NULL) {
				Session_List* list = (Session_List*)session_hashtable->table[x];
				Session_Node* current = list->head->next;
				while(current != NULL) {
					unsigned int username_size = strlen(current->session->username) + 1;
					//printf("username size: %i\n", username_size);
					//printf("username: %s\n", current->session->username);
					//printf("x: %i y: %i\n", current->session->location->x, current->session->location->y);
					char format[128] = {'\0'};
					int bytes = sprintf(format, "!I%isii", username_size);
					unsigned int size = Binary_Calcsize(format);
					//printf("size:%i %s\n", size, format);
					unsigned char* chunk = Binary_Pack(format, username_size, current->session->username, current->session->location->x, current->session->location->y);
					memcpy(payload_body, chunk, size);
					free(chunk);
					
					payload_body = payload_body + size;
					total_size+=size;
					current = current->next;

				}//end while session list
			}//end if not null
		}//end for loop	
	}	
	//printf("total_size: %i\n", total_size);
	//printf("payload_body_size: %i\n", payload_body_size);
	payload_body-=payload_body_size;
	memcpy(payload+payload_header_size, payload_body, payload_body_size);
	free(payload_header);
	free(payload_body);
	return payload;	
}//end function

Session_Hashtable* Binary_To_Session_Hashtable(char* session_token, unsigned char* data) {
	Session_Hashtable* result = Session_Hashtable_Create(20);
	
	unsigned int size = 0;
	unsigned short num_items = 0;
	char* format = "!IH";
	Binary_Unpack(format, data, &size, &num_items);
	printf("size: %i\n", size);
	printf("num_items: %i\n", num_items);
	unsigned int header_size = Binary_Calcsize(format);

	unsigned char* payload_body = data + header_size; 
	
	int x = 0;
	for(x=0; x<num_items; x++) {
		unsigned int username_size;
		Binary_Unpack("!I", payload_body, &username_size);
		printf("username size:%i\n", username_size);
		char format[128] = {'\0'};
		int bytes = sprintf(format, "!I%isii", username_size);
		char username[USERNAME_LENGTH+1] = {'\0'};
		int x = 0;
		int y = 0;
		printf("format: %s\n", format);
		Binary_Unpack(format, payload_body, &username_size, username, &x, &y);
		printf("username: %s\n", username);
		Location* location = Location_Create(0, 0, '\0', x, y);
	       	Session* session = Session_Create(session_token, username, location, NULL, NULL, NULL);
		Session_Hashtable_Set(result, username, session);
		unsigned int item_size = Binary_Calcsize(format);	
		printf("item_size:%i\n", item_size);
		payload_body += item_size;
	}
	return result;
}


char* Session_Hashtable_String(Session_Hashtable* session_hashtable) {
	if(session_hashtable != NULL) {
		char* result = (char*)malloc(SESSION_HASHTABLE_OUTPUT_SIZE);
		memset(result, '\0', SESSION_HASHTABLE_OUTPUT_SIZE);
		int i = 0;
		int bytes = 0;
		int total_bytes = 0;
		char* list;
		for(i=0; i<session_hashtable->size; i++) {
			if(session_hashtable->table[i] != NULL) {
				list = Session_List_String_Keys(session_hashtable->table[i]);
				bytes = sprintf(result, "%i => %s\n", i, list);
				result+=bytes;
				total_bytes+=bytes;
			}	
		}
		result -= total_bytes;
		return result;
	}
	return NULL;
}

void Session_Hashtable_Print(Session_Hashtable* table) {
	char* result = Session_Hashtable_String(table);
	if(result != NULL) {
		printf("%s\n", result);		
		free(result);
	}
}

void Session_Hashtable_Destroy(Session_Hashtable* session_hashtable) {
	//this is slow if the table is big
	//but should only be used on shutdown
	if(session_hashtable != NULL) {
		int i = 0;
		for(i=0; i<session_hashtable->size; i++) {
			if(session_hashtable->table[i] != NULL) {
				Session_List_Destroy(session_hashtable->table[i]);
			}
		}
		free(session_hashtable);
	}
}	

unsigned long Session_Hashtable_HashCode(unsigned int size, char *str) {
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
