#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "user_database.h"
#include "user.h"
#include "log.h"

static int User_Database_Callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

sqlite3* User_Database_Open(char* filename, LogConfig* log_config) {
	sqlite3 *db;
	char* error_msg;
	char* sql;
	int rc;

	rc = sqlite3_open(filename, &db);
	if(rc) { 
		Log_log(log_config, LOG_ERROR, "cant open db %s\n", sqlite3_errmsg(db));
		return NULL;
	} else {
		Log_log(log_config, LOG_DEBUG, "opened %s successfully \n", filename);
	}

	return db;
}

void User_Create_Table(sqlite3* db, LogConfig* log_config) {
	char* sql = "CREATE TABLE IF NOT EXISTS user " \
		    	"(user_username TEXT PRIMARY_KEY UNIQUE NOT NULL, " \
		    	"user_password TEXT, " \
		    	"user_email TEXT);";
	User_Database_Exec_Sql(db, sql, log_config);
}

void User_Drop_Table(sqlite3* db, char* table_name, LogConfig* log_config) {
	
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "DROP TABLE IF EXISTS %s;", table_name);
	User_Database_Exec_Sql(db, buffer, log_config);
	

}

int User_Insert(sqlite3* db, User* user, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "INSERT INTO user (user_username, user_password, user_email) VALUES ('%s', '%s', '%s');", user->username, user->password, user->email);
	return User_Database_Exec_Sql(db, buffer, log_config);
}

void User_Update_By_Username(sqlite3* db, User* user, char* username, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "UPDATE user SET user_username = '%s', user_password = '%s', user_email ='%s' WHERE user_username = '%s';", user->username, user->password, user->email, username);
	User_Database_Exec_Sql(db, buffer, log_config);
}

void User_Delete_By_Username(sqlite3* db, char* username, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "DELETE FROM user WHERE user_username = '%s';", username);
	User_Database_Exec_Sql(db, buffer, log_config);
}

User* User_Find_By_Username(sqlite3* db, char* username, LogConfig* log_config) {
	User* user = NULL;
	sqlite3_stmt* result;
	char* sql = "SELECT user_username, user_password, user_email FROM user WHERE user_username = @username";
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
		const unsigned char* r_username = sqlite3_column_text(result, 0);
		const unsigned char* r_password = sqlite3_column_text(result, 1);
		const unsigned char* r_email = sqlite3_column_text(result, 2);
		user = User_Create((char*)r_username,(char*)r_password,(char*)r_email);
		Log_log(log_config, LOG_DEBUG, "user created with dound data: username: %s password: %s email: %s\n", user->username, user->password, user->email);

	} else if(rc == SQLITE_DONE) {
		Log_log(log_config, LOG_ERROR, "failed to fetch data with username: %s\n", username);
		return NULL;
	} else {
		Log_log(log_config, LOG_ERROR, "failed to fetch data: %s return value: %i\n", sqlite3_errmsg(db), rc);
	}
	sqlite3_finalize(result);
	return user;
		
}

int User_Database_Exec_Sql(sqlite3* db, char* sql, LogConfig* log_config) {
	int rc;
	char* error_msg = '\0';
	rc = sqlite3_exec(db, sql, User_Database_Callback, 0, &error_msg);
	Log_log(log_config, LOG_DEBUG, "%s\n", sql);		
	if(rc != SQLITE_OK) {
		Log_log(log_config, LOG_ERROR, "SQL_ERROR: %s\n", error_msg);
		sqlite3_free(error_msg);
		return -1;
	} else {
		Log_log(log_config, LOG_INFO, "sql executed successfully\n");		
		return 1;
	}
	return 1;
}

void User_Database_Close(sqlite3* db, LogConfig* log_config) {
	Log_log(log_config, LOG_DEBUG, "closing db connection\n");
	sqlite3_close(db);
	Log_log(log_config, LOG_DEBUG, "db connection closed\n");
}

