#include <stdio.h>
#include <sqlite3.h>
#include <string.h>
#include <stdlib.h>
#include "database.h"
#include "location_table.h"

void Location_Create_Table(sqlite3* db, LogConfig* log_config) {
	char* sql = "CREATE TABLE IF NOT EXISTS location " \
		    	"(location_key INTEGER PRIMARY KEY, " \
		    	"location_user_key INTEGER, " \
			"location_zonename TEXT NOT NULL, " \
			"location_x INTEGER, " \
			"location_y INTEGER);";
	Database_Exec_Sql(db, sql, log_config);
}

int Location_Insert(sqlite3* db, Location* location, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "INSERT INTO location (location_user_key, location_zonename, location_x, location_y) VALUES ('%i', '%s', '%i', '%i');", location->user_key, location->zonename, location->x, location->y);
	return Database_Exec_Sql(db, buffer, log_config);
}

void Location_Update_By_Userkey(sqlite3* db, Location* location, int user_key, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "UPDATE location SET location_user_key = '%i', location_zonename = '%s', location_x = '%i', location_y = '%i' WHERE location_user_key = '%i';", location->user_key, location->zonename, location->x, location->y, user_key);
	Database_Exec_Sql(db, buffer, log_config);
}

void Location_Delete_By_Userkey(sqlite3* db, int user_key, LogConfig* log_config) {
	char buffer[SQL_STRING_MAX_SIZE] = {'\0'};
	int bytes = sprintf(buffer, "DELETE FROM location WHERE location_user_key = '%i';", user_key);
	Database_Exec_Sql(db, buffer, log_config);
}

Location* Location_Find_By_Userkey(sqlite3* db, int user_key, LogConfig* log_config) {
	Location* location = NULL;
	sqlite3_stmt* result;
	char* sql = "SELECT location_key, location_user_key, location_zonename, location_x, location_y FROM location WHERE location_user_key = @user_key";
	int rc;
	rc = sqlite3_prepare_v2(db, sql, strlen(sql)+1, &result, 0);
	if(rc != SQLITE_OK) {
		Log_log(log_config, LOG_ERROR, "failed to fetch data: %s\n", sqlite3_errmsg(db));
		sqlite3_finalize(result);
		return NULL;
	} else if(rc == SQLITE_OK) {
		int index = sqlite3_bind_parameter_index(result, "@user_key");
		sqlite3_bind_int(result, index, user_key);	
	}

	rc = sqlite3_step(result);
	if(rc == SQLITE_ROW) {
		int r_location_key = sqlite3_column_int(result, 0);
		int r_userkey = sqlite3_column_int(result, 1);
		const unsigned char* r_zonename = sqlite3_column_text(result, 2);
		int r_location_x = sqlite3_column_int(result, 3);
		int r_location_y = sqlite3_column_int(result, 4);
		location = Location_Create(r_location_key, r_userkey, (char*)r_zonename, r_location_x, r_location_y);
		Log_log(log_config, LOG_DEBUG, "location created with bound data: location key:%i user_key: %i zonename: %s x: %i y: %i\n", location->location_key, location->user_key, location->zonename, location->x, location->y);

	} else if(rc == SQLITE_DONE) {
		Log_log(log_config, LOG_ERROR, "failed to fetch data with user key: %i\n", user_key);
	} else {
		Log_log(log_config, LOG_ERROR, "failed to fetch data: %s return value: %i\n", sqlite3_errmsg(db), rc);
	}
	sqlite3_finalize(result);
	return location;
		
}


