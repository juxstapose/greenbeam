#include <stdio.h>
#include <string.h>
#include "util.h"
#include "log.h"
#include "database.h"
#include "user.h"
#include "user_table.h" 

#ifdef main
#undef main
#endif



void Test_Insert_Find() {
	LogConfig* log_config = LogConfig_Create("log.txt", 
						   LOG_ON,
						   LOG_FILE,
						   LOG_DEBUG,
						   1000000, 
						   5);
	sqlite3* db = Database_Open("test.db", log_config);
	Database_Drop_Table(db, "user", log_config);
	User_Create_Table(db, log_config);
	
	
	int user_key_one = 1;
	char* username_one = "juxstapose";
	char* password_one = "Inioh2";
	char* email_one = "joe@blow.com";
	char hidden_password_one[SHA_STRING_SIZE] = {'\0'};
	Util_Sha256(password_one, hidden_password_one);	
	User* user_one = User_Create(user_key_one, username_one, hidden_password_one, email_one);	
	Log_log(log_config, LOG_DEBUG, "INPUT username:%s password:%s email:%s\n", user_one->username, user_one->password, user_one->email);
	User_Insert(db, user_one, log_config);	
	
	int user_key_two = 2;
	char* username_two = "bonggong";
	char* password_two = "liquid2";
	char* email_two = "george@bathroom.com";
	char hidden_password_two[SHA_STRING_SIZE] = {'\0'};
	Util_Sha256(password_two, hidden_password_two);	
	User* user_two = User_Create(user_key_two, username_two, hidden_password_two, email_two);	
	Log_log(log_config, LOG_DEBUG, "INPUT username:%s password:%s email:%s\n", user_two->username, user_two->password, user_two->email);
	User_Insert(db, user_two, log_config);	
	
	User* user_one_found = User_Find_By_Username(db, "juxstapose", log_config);			
	if(strcmp(user_one_found->username, user_one->username) == 0) {
		Log_log(log_config, LOG_DEBUG, "test 1 passed\n");	
	}
	
	User* user_two_found = User_Find_By_Username(db, "bonggong", log_config);			
	if(strcmp(user_two_found->username, user_two->username) == 0) {
		Log_log(log_config, LOG_DEBUG, "test 2 passed\n");	
	}
	
	User* user_three_found = User_Find_By_Username(db, "jimbean", log_config);			
	if(user_three_found == NULL) {
		Log_log(log_config, LOG_DEBUG, "test 3 passed\n");	
	}
	
	Database_Drop_Table(db, "user", log_config);
	User_Destroy(user_one);
	User_Destroy(user_two);
	User_Destroy(user_one_found);
	User_Destroy(user_two_found);
	User_Destroy(user_three_found);
	Database_Close(db, log_config);
	LogConfig_Destroy(log_config);
}

void Test_Update_Find() {
	LogConfig* log_config = LogConfig_Create("log.txt", 
						   LOG_ON,
						   LOG_FILE,
						   LOG_DEBUG,
						   1000000, 
						   5);
	sqlite3* db = Database_Open("test.db", log_config);
	Database_Drop_Table(db, "user", log_config);
	User_Create_Table(db, log_config);
	
	int user_key_one = 1;
	char* username_one = "juxstapose";
	char* password_one = "Inioh2";
	char* email_one = "joe@blow.com";
	char hidden_password_one[SHA_STRING_SIZE] = {'\0'};
	Util_Sha256(password_one, hidden_password_one);	
	User* user_one = User_Create(user_key_one, username_one, hidden_password_one, email_one);	
	Log_log(log_config, LOG_DEBUG, "INPUT username:%s password:%s email:%s\n", user_one->username, user_one->password, user_one->email);
	User_Insert(db, user_one, log_config);	
	
	int user_key_two = 2;
	char* username_two = "bonggong";
	char* password_two = "liquid2";
	char* email_two = "george@bathroom.com";
	char hidden_password_two[SHA_STRING_SIZE] = {'\0'};
	Util_Sha256(password_two, hidden_password_two);	
	User* user_two = User_Create(user_key_two, username_two, hidden_password_two, email_two);	
	Log_log(log_config, LOG_DEBUG, "INPUT username:%s password:%s email:%s\n", user_two->username, user_two->password, user_two->email);
	User_Insert(db, user_two, log_config);	
	
	int user_key_three = 3;
	char* username_three = "foobar";
	char* password_three = "password";
	char* email_three = "you@them.com";
	char hidden_password_three[SHA_STRING_SIZE] = {'\0'};
	Util_Sha256(password_two, hidden_password_three);	
	User* update_user = User_Create(user_key_three, username_three, hidden_password_three, email_three);
	User_Update_By_Username(db, update_user, "bonggong", log_config);

	User* user_three_found = User_Find_By_Username(db, "foobar", log_config);			
	if(user_three_found != NULL) {
		if(strcmp(user_three_found->username, update_user->username) == 0) {
			Log_log(log_config, LOG_DEBUG, "test 1 passed\n");	
		}
	} else {
		Log_log(log_config, LOG_DEBUG, "test 1 failed\n");
		

	}
	User* user_two_found = User_Find_By_Username(db, "bonggong", log_config);			
	if(user_two_found == NULL) {
		Log_log(log_config, LOG_DEBUG, "test 2 passed\n");	
	}
	Database_Drop_Table(db, "user", log_config);
	User_Destroy(user_one);
	User_Destroy(user_two);
	User_Destroy(update_user);
	User_Destroy(user_three_found);
	Database_Close(db, log_config);
	LogConfig_Destroy(log_config);
}

void Test_Delete_Find() {
	LogConfig* log_config = LogConfig_Create("log.txt", 
						   LOG_ON,
						   LOG_FILE,
						   LOG_DEBUG,
						   1000000, 
						   5);
	sqlite3* db = Database_Open("test.db", log_config);
	Database_Drop_Table(db, "user", log_config);
	User_Create_Table(db, log_config);

	int user_key_one = 1;	
	char* username_one = "juxstapose";
	char* password_one = "Inioh2";
	char* email_one = "joe@blow.com";
	char hidden_password_one[SHA_STRING_SIZE] = {'\0'};
	Util_Sha256(password_one, hidden_password_one);	
	User* user_one = User_Create(user_key_one, username_one, hidden_password_one, email_one);	
	Log_log(log_config, LOG_DEBUG, "INPUT username:%s password:%s email:%s\n", user_one->username, user_one->password, user_one->email);
	User_Insert(db, user_one, log_config);	
	
	User* user_one_found = User_Find_By_Username(db, "juxstapose", log_config);			
	if(strcmp(user_one_found->username, user_one->username) == 0) {
		Log_log(log_config, LOG_DEBUG, "test 1 passed\n");	
	}
	
	User_Delete_By_Username(db, username_one, log_config);	
	
	User* user_deleted = User_Find_By_Username(db, "juxstapose", log_config);			
	if(user_deleted == NULL) {
		Log_log(log_config, LOG_DEBUG, "test 2 passed\n");	
	}

	Database_Drop_Table(db, "user", log_config);
	User_Destroy(user_one);
	Database_Close(db, log_config);
	LogConfig_Destroy(log_config);

}

int main(int argc, char* argv[]) {
	Test_Insert_Find();
	Test_Update_Find();
	Test_Delete_Find();
}

