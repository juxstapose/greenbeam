#ifndef _DATABASE_H_
#define _DATABASE_H_

#include <sqlite3.h>
#include "log.h"

static int Database_Callback(void *NotUsed, int argc, char **argv, char **azColName);
sqlite3* Database_Open(char* filename, LogConfig* log_config);
void Database_Drop_Table(sqlite3* db, char* table_name, LogConfig* log_config);
int Database_Exec_Sql(sqlite3* db, char* sql, LogConfig* log_config);
void Database_Close(sqlite3* db, LogConfig* log_config);

#endif
