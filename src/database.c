#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include "database.h"
#include "log.h"

static int Database_Callback(void *NotUsed, int argc, char **argv, char **azColName){
   int i;
   for(i=0; i<argc; i++){
      printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
   }
   printf("\n");
   return 0;
}

sqlite3* Database_Open(char* filename, LogConfig* log_config) {
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

void Database_Drop_Table(sqlite3* db, char* table_name, LogConfig* log_config) {
	
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "DROP TABLE IF EXISTS %s;", table_name);
	User_Database_Exec_Sql(db, buffer, log_config);
	

}
int Database_Exec_Sql(sqlite3* db, char* sql, LogConfig* log_config) {
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

void Database_Close(sqlite3* db, LogConfig* log_config) {
	Log_log(log_config, LOG_DEBUG, "closing db connection\n");
	sqlite3_close(db);
	Log_log(log_config, LOG_DEBUG, "db connection closed\n");
}


