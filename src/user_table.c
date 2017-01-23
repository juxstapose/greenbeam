#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "database.h"
#include "user_table.h"
#include "user.h"
#include "log.h"

void User_Create_Table(sqlite3* db, LogConfig* log_config) {
	char* sql = "CREATE TABLE IF NOT EXISTS user " \
		    	"(user_key INTEGER PRIMARY KEY, " \
			"user_username TEXT UNIQUE NOT NULL, " \
		    	"user_password TEXT, " \
		    	"user_email TEXT);";
	Database_Exec_Sql(db, sql, log_config);
}


int User_Insert(sqlite3* db, User* user, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "INSERT INTO user (user_username, user_password, user_email) VALUES ('%s', '%s', '%s');", user->username, user->password, user->email);
	return Database_Exec_Sql(db, buffer, log_config);
}

void User_Update_By_Username(sqlite3* db, User* user, char* username, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "UPDATE user SET user_username = '%s', user_password = '%s', user_email ='%s' WHERE user_username = '%s';", user->username, user->password, user->email, username);
	Database_Exec_Sql(db, buffer, log_config);
}

void User_Delete_By_Username(sqlite3* db, char* username, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "DELETE FROM user WHERE user_username = '%s';", username);
	Database_Exec_Sql(db, buffer, log_config);
}

User* User_Find_By_Username(sqlite3* db, char* username, LogConfig* log_config) {
	User* user = NULL;
	sqlite3_stmt* result;
	char* sql = "SELECT user_key, user_username, user_password, user_email FROM user WHERE user_username = @username";
	int rc;
	rc = sqlite3_prepare_v2(db, sql, strlen(sql)+1, &result, 0);
	if(rc != SQLITE_OK) {
		Log_log(log_config, LOG_ERROR, "failed to fetch data: %s\n", sqlite3_errmsg(db));
		return NULL;
	} else if(rc == SQLITE_OK) {
		int index = sqlite3_bind_parameter_index(result, "@username");
		sqlite3_bind_text(result, index, username, -1, 0);	
	}

	rc = sqlite3_step(result);

	if(rc == SQLITE_ROW) {
		int r_userkey = sqlite3_column_int(result, 0);
		const unsigned char* r_username = sqlite3_column_text(result, 1);
		const unsigned char* r_password = sqlite3_column_text(result, 2);
		const unsigned char* r_email = sqlite3_column_text(result, 3);
		user = User_Create(r_userkey, (char*)r_username,(char*)r_password,(char*)r_email);
		Log_log(log_config, LOG_DEBUG, "user created with bound data: user key:%i username: %s password: %s email: %s\n", user->user_key, user->username, user->password, user->email);

	} else if(rc == SQLITE_DONE) {
		Log_log(log_config, LOG_ERROR, "failed to fetch data with username: %s\n", username);
		return NULL;
	} else {
		Log_log(log_config, LOG_ERROR, "failed to fetch data: %s return value: %i\n", sqlite3_errmsg(db), rc);
	}
	sqlite3_finalize(result);
	return user;
		
}

