#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "session.h"
#include "session_hashtable.h"

void Test_Hashtable_Add_Get() {
	Session_Hashtable* session_hashtable = Session_Hashtable_Create();
	
	char* session_token = "ldjkakdjsakldjsdklmsadlkas";
	char* username_one = "juxstapose";
	char* password = "aslkjdslakdjslkadjklasd";
	char* email = "joe@deep.com";
	Session* session_one = Session_Create(session_token, username_one, password, email);
	Session_Hashtable_Set(session_hashtable, username_one, session_one);  
	
	session_token = "ldjkakdjsakldjs323312lmsadlkas";
	char* username_two = "spirit128";
	password = "aslkjdslakdjslkadjkl930323942";
	email = "pee@deep.com";
	Session* session_two = Session_Create(session_token, username_two, password, email);	
	Session_Hashtable_Set(session_hashtable, username_two, session_two);

	session_token = "dlkjfdsjsndjskcsadfnkldf";
	char* username_three = "greenbeam";
	password = "931r9fjewfwqepfj394fi";
	email = "theworm@pablo.com";
	Session* session_three = Session_Create(session_token, username_three, password, email);
	Session_Hashtable_Set(session_hashtable, username_three, session_three);
	
	Session* found_session = Session_Hashtable_Get(session_hashtable, username_two);
	if(strcmp(session_two->username, found_session->username) == 0) {
		printf("test hashtable get passed\n");
	} else {
		printf("test hashtable get failed\n");
	}
	Session_Hashtable_Destroy(session_hashtable);	
}

void Test_Hashtable_Add_Remove() {

	Session_Hashtable* session_hashtable = Session_Hashtable_Create();
	
	char* session_token = "ldjkakdjsakldjsdklmsadlkas";
	char* username_one = "juxstapose";
	char* password = "aslkjdslakdjslkadjklasd";
	char* email = "joe@deep.com";
	Session* session_one = Session_Create(session_token, username_one, password, email);
	Session_Hashtable_Set(session_hashtable, username_one, session_one);  
	
	session_token = "ldjkakdjsakldjs323312lmsadlkas";
	char* username_two = "spirit128";
	password = "aslkjdslakdjslkadjkl930323942";
	email = "pee@deep.com";
	Session* session_two = Session_Create(session_token, username_two, password, email);	
	Session_Hashtable_Set(session_hashtable, username_two, session_two);

	session_token = "dlkjfdsjsndjskcsadfnkldf";
	char* username_three = "greenbeam";
	password = "931r9fjewfwqepfj394fi";
	email = "theworm@pablo.com";
	Session* session_three = Session_Create(session_token, username_three, password, email);
	Session_Hashtable_Set(session_hashtable, username_three, session_three);
	
	Session_Hashtable_Print(session_hashtable);
	Session_Hashtable_Remove(session_hashtable, username_one);
	Session_Hashtable_Print(session_hashtable);
	
	Session_Hashtable_Destroy(session_hashtable);	

}



void Test_Hashtable_Collision() {
	
	char* filename = "../tests/names.txt";
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("error opening file: %s\n", strerror(errno));
		exit(1);
	}
	
	Session_Hashtable* session_hashtable = Session_Hashtable_Create();

	char line[256];
	while(fgets(line, sizeof(line), fp)) {
		//printf("%s", line);
		Util_Remove_All_Chars(line, '\n');
		Util_Remove_All_Chars(line, '\r');
		Session* session = Session_Create(NULL, line, NULL, NULL);
		Session_Hashtable_Set(session_hashtable, line, session);	
	}
	
	Session_Hashtable_Print(session_hashtable);	
	Session_Hashtable_Destroy(session_hashtable);	
	fclose(fp);	

}	

int main(int argc, char* argv[]) {
	Test_Hashtable_Add_Get();
	Test_Hashtable_Add_Remove();
	Test_Hashtable_Collision();
}
