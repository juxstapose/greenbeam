#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "util.h"
#include "session.h"
#include "session_hashtable.h"

/**
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

**/


void Test_Hashtable_Collision() {
	
	char* filename = "../tests/names.txt";
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		printf("error opening file: %s\n", strerror(errno));
		exit(1);
	}
	
	unsigned int size = 10;	
	Session_Hashtable* session_hashtable = Session_Hashtable_Create(size);

	char line[256];
	while(fgets(line, sizeof(line), fp)) {
		//printf("%s", line);
		Util_Remove_All_Chars(line, '\n');
		Util_Remove_All_Chars(line, '\r');
		Session* session = Session_Create(NULL, line, NULL, NULL, 0, 0, NULL);
		Session_Hashtable_Set(session_hashtable, line, session);	
	}
	
	Session_Hashtable_Print(session_hashtable);	
	Session_Hashtable_Destroy(session_hashtable);	
	fclose(fp);	
}	


void Test_Hashtable_Grow() {
	int initial_size = 3;
	Session_Hashtable* session_hashtable = Session_Hashtable_Create(initial_size);
	char* session_token_one = "ldjkakdjsakldjsdklmsadlkas";
	char* username_one = "juxstapose";
	char* password_one = "aslkjdslakdjslkadjklasd";
	char* email_one = "joe@deep.com";
	int x_one = 10;
	int y_one = 5;
	printf("set one\n");
	Session* session_one = Session_Create(session_token_one, username_one, password_one, email_one, x_one, y_one, NULL);
	Session_Hashtable_Set(session_hashtable, username_one, session_one);  
	
	char* session_token_two = "ldjkakdjsakldjsdklmsadlkas";
	char* username_two = "bob";
	char* password_two = "aslkjdslakdjslkadjklasd";
	char* email_two = "joe@deep.com";
	int x_two = 10;
	int y_two = 5;
	printf("set two\n");
	Session* session_two = Session_Create(session_token_two, username_two, password_two, email_two, x_two, y_two, NULL);
	Session_Hashtable_Set(session_hashtable, username_two, session_two);  
	
	char* session_token_three = "ldjkakdjsakldjsdklmsadlkas";
	char* username_three = "jim";
	char* password_three = "aslkjdslakdjslkadjklasd";
	char* email_three = "jim@poop.com";
	int x_three = 10;
	int y_three = 5;
	printf("set three\n");
	Session* session_three = Session_Create(session_token_three, username_three, password_three, email_three, x_three, y_three, NULL);
	Session_Hashtable_Set(session_hashtable, username_three, session_three); 
	
	char* session_token_four = "ldjkakdjsakldjsdklmsadlkas";
	char* username_four = "blue";
	char* password_four = "aslkjdslakdjslkadjklasd";
	char* email_four = "jim@poop.com";
	int x_four = 10;
	int y_four = 5;
	printf("set four\n");
	Session* session_four = Session_Create(session_token_four, username_four, password_four, email_four, x_four, y_four, NULL);
	Session_Hashtable_Set(session_hashtable, username_four, session_four); 
	
	Session_Hashtable_Print(session_hashtable);	
	
	Session_Hashtable_Remove(session_hashtable, "bob");
	Session_Hashtable_Print(session_hashtable);	
	
	Session_Hashtable_Remove(session_hashtable, "juxstapose");
	Session_Hashtable_Print(session_hashtable);	
	
	char* session_token_five = "ldjkakdjsakldjsdklmsadlkas";
	char* username_five = "lunchtime";
	char* password_five = "aslkjdslakdjslkadjklasd";
	char* email_five = "poop@poop.com";
	int x_five = 10;
	int y_five = 5;
	printf("set five\n");
	Session* session_five = Session_Create(session_token_five, username_five, password_five, email_five, x_five, y_five, NULL);
	Session_Hashtable_Set(session_hashtable, username_five, session_five); 
	Session_Hashtable_Print(session_hashtable);	

	Session_Hashtable_Destroy(session_hashtable);	
}


int main(int argc, char* argv[]) {
	//Test_Hashtable_Add_Get();
	//Test_Hashtable_Add_Remove();
	//Test_Hashtable_Collision();
	Test_Hashtable_Grow();
}
