#ifndef _LOCATION_TABLE_H_
#define _LOCATION_TABLE_H_

#include <sqlite3.h>
#include "location.h"
#include "log.h"


void Location_Create_Table(sqlite3* db, LogConfig* log_config);
int Location_Insert(sqlite3* db, Location* location, LogConfig* log_config);
void Location_Update_By_Userkey(sqlite3* db, Location* location, int user_key, LogConfig* log_Config);
void Location_Delete_By_Userkey(sqlite3* db, int user_key, LogConfig* log_config);
Location* Location_Find_By_Userkey(sqlite3* db, int user_key, LogConfig* log_config);


#endif

