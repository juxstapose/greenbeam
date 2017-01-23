#ifndef _USER_TABLE_H_
#define _USER_TABLE_H_

#include <sqlite3.h>
#include "user.h"
#include "log.h"


void User_Create_Table(sqlite3* db, LogConfig* log_config);
int User_Insert(sqlite3* db, User* user, LogConfig* log_config);
void User_Update_By_Username(sqlite3* db, User* user, char* username, LogConfig* log_Config);
void User_Delete_By_Username(sqlite3* db, char* username, LogConfig* log_config);
User* User_Find_By_Username(sqlite3* db, char* username, LogConfig* log_config);


#endif
