#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "user.h"

User* User_Create(char* username, char* password, char* email) {
	User* user = (User*)malloc(sizeof(User));
	if(username != NULL) {
		strcpy(user->username, username);
	} 
	if(password != NULL) {
		strcpy(user->password, password);
	} 
	if(email != NULL) {
		strcpy(user->email, email);
	}
	return user;
}


void User_Destroy(User* user) {
	if(user != NULL) {
		free(user);
	}
}
