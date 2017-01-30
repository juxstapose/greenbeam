#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netdb.h>
#include <poll.h>
#include <sqlite3.h>
#include "log.h"
#include "server.h"
#include "socket_hashtable.h"
#include "session_hashtable.h"
#include "database.h"
#include "user_table.h"
#include "location_table.h"
#include "arguments.h"

#ifdef main
#undef main
#endif



int main(int argc, char* argv[]) {
	char ip_address[NI_MAXHOST+1] = {'\0'};
	char port[NI_MAXSERV+1] = {'\0'};
	char db_name[DATABASE_NAME_LEN+1] = {'\0'};
	char logfile_name[LOGFILE_NAME_LEN +1] = {'\0'};
	int empty_db = 0;

	Args_Get_Input(argc, argv, ip_address, port, db_name, logfile_name, &empty_db);
	
	printf("ip_address: %s\n", ip_address);
	printf("port: %s\n", port);
	printf("db_name: %s\n", db_name);
	printf("logfile_name: %s\n", logfile_name);
	printf("empty_db:%i\n", empty_db);
		
	LogConfig* log_config = LogConfig_Create(logfile_name, 
						 LOG_ON,
						 LOG_FILE,
						 LOG_DEBUG,
						 100000000, 
						 5);

	sqlite3* db = Database_Open(db_name, log_config);
	if(empty_db == 1) {
		Database_Drop_Table(db, "user", log_config);
		Database_Drop_Table(db, "location", log_config);
	}
	User_Create_Table(db, log_config);
	Location_Create_Table(db, log_config);
	
	Config* config = Config_Create();
	Config_Read_File(config, "config.ini", log_config);
	
	float rect_percentage = 0.1f;
	ZoneRange* zonerange = ZoneRange_Create(rect_percentage);
	
	Socket_Hashtable* sock_hashtable = Socket_Hashtable_Create();
	Session_Hashtable* session_hashtable_username = Session_Hashtable_Create(20);
	Session_Hashtable* session_hashtable_token = Session_Hashtable_Create(20);
	
	Log_log(log_config, LOG_INFO, "create socket and bind an listen on %s with port %s\n", ip_address, port);
	Socket* listener = Server_Bind_Listen(ip_address, port, log_config);
	
	if(listener != NULL) {	
		Server_Poll_Event_Handler(config, zonerange, sock_hashtable, listener, db, session_hashtable_username, session_hashtable_token, log_config);
	} else {
		Log_log(log_config, LOG_ERROR, "bind an listen failed...listener socket is null\n");
	}

	Config_Destroy(config);
	Socket_Hashtable_Destroy(sock_hashtable);	
	Session_Hashtable_Destroy(session_hashtable_username);	
	Session_Hashtable_Destroy(session_hashtable_token);	
	Socket_Destroy(listener);	
	Database_Close(db, log_config);
	LogConfig_Destroy(log_config);
	return 0;	
}
