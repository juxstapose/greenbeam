#include <stdio.h>
#include <string.h>
#include "util.h"
#include "log.h"
#include "database.h"
#include "location.h"
#include "location_table.h" 

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
	Log_log(log_config, LOG_INFO, "Test Insert Find Start\n");
	sqlite3* db = Database_Open("test.db", log_config);
	Database_Drop_Table(db, "location", log_config);
	Location_Create_Table(db, log_config);
	
	

	int user_key_one = 1;
	char* zonename_one = "starting_zone_one";
	int x_one = 10;
	int y_one = 5;
	Location* location_one = Location_Create(0, user_key_one, zonename_one, x_one, y_one);	
	Log_log(log_config, LOG_DEBUG, "INPUT user key:%i zonename:%s x:%i y:%i\n", location_one->user_key, location_one->zonename, location_one->x, location_one->y);
	Location_Insert(db, location_one, log_config);	
	
	int user_key_two = 2;
	char* zonename_two = "starting_zone_two";
	int x_two = 10;
	int y_two = 5;
	Location* location_two = Location_Create(0, user_key_two, zonename_two, x_two, y_two);	
	Log_log(log_config, LOG_DEBUG, "INPUT user key:%i zonename:%s x:%i y:%i\n", location_two->user_key, location_two->zonename, location_two->x, location_two->y);
	Location_Insert(db, location_two, log_config);	
	
	Location* location_one_found = Location_Find_By_Userkey(db, user_key_one, log_config);			
	if(location_one_found->user_key == location_one->user_key) {
		Log_log(log_config, LOG_DEBUG, "test 1 passed\n");	
	}
	
	Location* location_two_found = Location_Find_By_Userkey(db, user_key_two, log_config);			
	if(location_two_found->user_key == location_two->user_key) {
		Log_log(log_config, LOG_DEBUG, "test 2 passed\n");	
	}
	
	int user_key_three = 3;
	Location* location_three_found = Location_Find_By_Userkey(db, user_key_three, log_config);			
	if(location_three_found == NULL) {
		Log_log(log_config, LOG_DEBUG, "test 3 passed\n");	
	}
	
	Database_Drop_Table(db, "location", log_config);
	Location_Destroy(location_one);
	Location_Destroy(location_two);
	Location_Destroy(location_one_found);
	Location_Destroy(location_two_found);
	Location_Destroy(location_three_found);
	Database_Close(db, log_config);
	Log_log(log_config, LOG_INFO, "Test Insert Find End\n");
	LogConfig_Destroy(log_config);
}

void Test_Update_Find() {
	LogConfig* log_config = LogConfig_Create("log.txt", 
						   LOG_ON,
						   LOG_FILE,
						   LOG_DEBUG,
						   1000000, 
						   5);
	Log_log(log_config, LOG_INFO, "Test Update Find Start\n");
	sqlite3* db = Database_Open("test.db", log_config);
	Database_Drop_Table(db, "location", log_config);
	Location_Create_Table(db, log_config);
	
	int user_key_one = 1;
	char* zonename_one = "starting_zone_one";
	int x_one = 10;
	int y_one = 5;
	Location* location_one = Location_Create(0, user_key_one, zonename_one, x_one, y_one);	
	Log_log(log_config, LOG_DEBUG, "INPUT user key:%i zonename:%s x:%i y:%i\n", location_one->user_key, location_one->zonename, location_one->x, location_one->y);
	Location_Insert(db, location_one, log_config);	
	
	int user_key_two = 2;
	char* zonename_two = "starting_zone_two";
	int x_two = 10;
	int y_two = 5;
	Location* location_two = Location_Create(0, user_key_two, zonename_two, x_two, y_two);	
	Log_log(log_config, LOG_DEBUG, "INPUT user key:%i zonename:%s x:%i y:%i\n", location_two->user_key, location_two->zonename, location_two->x, location_two->y);
	Location_Insert(db, location_two, log_config);	
	
	int user_key_three = 3;
	char* zonename_three = "starting_zone_three";
	int x_three = 7;
	int y_three = 2;
	Location* location_update = Location_Create(0, user_key_three, zonename_three, x_three, y_three);	
	Log_log(log_config, LOG_DEBUG, "INPUT user key:%i zonename:%s x:%i y:%i\n", location_update->user_key, location_update->zonename, location_update->x, location_update->y);
	Location_Update_By_Userkey(db, location_update, user_key_two, log_config);
	Location* location_three_found = Location_Find_By_Userkey(db, user_key_three, log_config);			
	if(location_three_found != NULL) {
		if(strcmp(location_three_found->zonename, zonename_three) == 0) {
			Log_log(log_config, LOG_DEBUG, "test 1 passed\n");	
		}
	} else {
		Log_log(log_config, LOG_DEBUG, "test 1 failed\n");
	}
	Location* location_two_found = Location_Find_By_Userkey(db, user_key_two, log_config);			
	if(location_two_found == NULL) {
		Log_log(log_config, LOG_DEBUG, "test 2 passed\n");
	} else {
		Log_log(log_config, LOG_DEBUG, "test 2 failed\n");
	}

	
	Database_Drop_Table(db, "location", log_config);
	Location_Destroy(location_one);
	Location_Destroy(location_two);
	Location_Destroy(location_update);
	Location_Destroy(location_three_found);
	Database_Close(db, log_config);
	Log_log(log_config, LOG_INFO, "Test Update Find End\n");
	LogConfig_Destroy(log_config);
	
}

void Test_Delete_Find() {
	LogConfig* log_config = LogConfig_Create("log.txt", 
						   LOG_ON,
						   LOG_FILE,
						   LOG_DEBUG,
						   1000000, 
						   5);
	Log_log(log_config, LOG_INFO, "Test Delete Find Start\n");
	sqlite3* db = Database_Open("test.db", log_config);
	Database_Drop_Table(db, "location", log_config);
	Location_Create_Table(db, log_config);
	
	int user_key_one = 1;
	char* zonename_one = "starting_zone_one";
	int x_one = 10;
	int y_one = 5;
	Location* location_one = Location_Create(0, user_key_one, zonename_one, x_one, y_one);	
	Log_log(log_config, LOG_DEBUG, "INPUT user key:%i zonename:%s x:%i y:%i\n", location_one->user_key, location_one->zonename, location_one->x, location_one->y);
	Location_Insert(db, location_one, log_config);	

	Location* location_one_found = Location_Find_By_Userkey(db, 1, log_config);			
	if(location_one_found->user_key == location_one->user_key) {
		Log_log(log_config, LOG_DEBUG, "test 1 passed\n");	
	}
	
	Location_Delete_By_Userkey(db, user_key_one, log_config);	
	
	int user_key_two = 2;
	Location* location_deleted = Location_Find_By_Userkey(db, user_key_two, log_config);			
	if(location_deleted == NULL) {
		Log_log(log_config, LOG_DEBUG, "test 2 passed\n");	
	}

	Database_Drop_Table(db, "location", log_config);
	Location_Destroy(location_one);
	Location_Destroy(location_one_found);
	Database_Close(db, log_config);
	Log_log(log_config, LOG_INFO, "Test Delete Find End\n");
	LogConfig_Destroy(log_config);
	

}

int main(int argc, char* argv[]) {
	Test_Insert_Find();
	Test_Update_Find();
	Test_Delete_Find();
}

