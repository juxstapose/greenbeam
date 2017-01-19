#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "session.h"
#include "session_linked_list.h"

void Test_Add_Delete() {
	Session_List* list = Session_List_Create();

	char* session_token = "ldjkakdjsakldjsdklmsadlkas";
	char* username = "juxstapose";
	char* password = "aslkjdslakdjslkadjklasd";
	char* email = "joe@deep.com";
	Session* session_one = Session_Create(session_token, username, password, email);
	Session_Print(session_one);
	Session_List_Push(list, username, session_one);
	
	session_token = "ldjkakdjsakldjs323312lmsadlkas";
	username = "spirit128";
	password = "aslkjdslakdjslkadjkl930323942";
	email = "pee@deep.com";
	Session* session_two = Session_Create(session_token, username, password, email);	
	Session_List_Push(list, username, session_two);

	session_token = "dlkjfdsjsndjskcsadfnkldf";
	username = "greenbeam";
	password = "931r9fjewfwqepfj394fi";
	email = "theworm@pablo.com";
	Session* session_three = Session_Create(session_token, username, password, email);
	Session_List_Push(list, username, session_three);
	
	Session_List_Print(list);	
	
	char* list_string_one = Session_List_String_Keys(list);
	char* test_string = "[juxstapose spirit128 greenbeam]";	
	if(strcmp(list_string_one, test_string) == 0) {
		printf("test session add passed\n");
	} else {
		printf("failed session add\n");
	}
	free(list_string_one);
	
	Session_List_Delete(list, "spirit128");
	
	char* list_string_two = Session_List_String_Keys(list);
	test_string = "[juxstapose greenbeam]";	
	if(strcmp(list_string_two, test_string) == 0) {
		printf("test session delete passed\n");
	} else {
		printf("failed session delete\n");
	}
	free(list_string_two);
	
	Session_Destroy(session_one);
	Session_Destroy(session_two);
	Session_Destroy(session_three);
	Session_List_Destroy(list);
}

void Test_Find() {
	Session_List* list = Session_List_Create();

	char* session_token = "ldjkakdjsakldjsdklmsadlkas";
	char* username_one = "juxstapose";
	char* password = "aslkjdslakdjslkadjklasd";
	char* email = "joe@deep.com";
	Session* session_one = Session_Create(session_token, username_one, password, email);
	Session_List_Push(list, username_one, session_one);
	
	session_token = "ldjkakdjsakldjs323312lmsadlkas";
	char* username_two = "spirit128";
	password = "aslkjdslakdjslkadjkl930323942";
	email = "pee@deep.com";
	Session* session_two = Session_Create(session_token, username_two, password, email);	
	Session_List_Push(list, username_two, session_two);

	session_token = "dlkjfdsjsndjskcsadfnkldf";
	char* username_three = "greenbeam";
	password = "931r9fjewfwqepfj394fi";
	email = "theworm@pablo.com";
	Session* session_three = Session_Create(session_token, username_three, password, email);
	Session_List_Push(list, username_three, session_three);
	
	Session* found_session = Session_List_Find_By_Key(list, username_one);
	
	if(strcmp(found_session->username, username_one) == 0) {
		printf("test search passed\n");
	} else {
		printf("failed test search\n");
	}
	
	Session_Destroy(session_one);
	Session_Destroy(session_two);
	Session_Destroy(session_three);
	Session_List_Destroy(list);
}



int main(int argc, char* argv[]) {

	Test_Add_Delete();
	Test_Find();


}
