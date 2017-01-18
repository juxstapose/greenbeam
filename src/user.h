#ifndef _USER_H_
#define _USER_H_

#define USERNAME_MAX_SIZE 32
#define PASSWORD_MAX_SIZE 32
#define EMAIL_MAX_SIZE 254

typedef struct User {
	char username[USERNAME_MAX_SIZE];
	char password[PASSWORD_MAX_SIZE];
	char email[EMAIL_MAX_SIZE];
} User;

User* User_Create(char* username, char* password, char* email);
void User_Destroy(User* user);


#endif
