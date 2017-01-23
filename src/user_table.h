#ifndef _USER_DATABASE_H_
#define _USER_DATABASE_H_

#include <sqlite3.h>
#include "user.h"
#include "log.h"

#define SQL_STRING_MAX_SIZE 8192

sqlite3* User_Database_Open(char* filename, LogConfig* log_config);
static int User_Database_Callback(void *NotUsed, int argc, char **argv, char **azColName);
int User_Database_Exec_Sql(sqlite3* db, char* sql, LogConfig* log_config);
void User_Create_Table(sqlite3* db, LogConfig* log_config);
void User_Drop_Table(sqlite3* db, char* table_name, LogConfig* log_config);
int User_Insert(sqlite3* db, User* user, LogConfig* log_config);
void User_Update_By_Username(sqlite3* db, User* user, char* username, LogConfig* log_Config);
void User_Delete_By_Username(sqlite3* db, char* username, LogConfig* log_config);
User* User_Find_By_Username(sqlite3* db, char* username, LogConfig* log_config);
void User_Database_Close(sqlite3* db, LogConfig* log_config);


#endif
