#ifndef _SERVER_H_
#define _SERVER_H_

#include <sqlite3.h>
#include "sock.h"
#include "socket_hashtable.h"
#include "session_hashtable.h"
#include "protocol.h"
#include "log.h"
#include "config.h"
#include "zonerange.h"

typedef struct ServerContext {
	
	sqlite3* db;
	Socket* sock;
	ZoneRange* zonerange;
	Config* config;
	Session_Hashtable* session_hashtable_username;
	Session_Hashtable* session_hashtable_token;
	LogConfig* log_config;

} ServerContext;

unsigned int Server_Register_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload);
unsigned int Server_Login_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload);
unsigned int Server_Logout_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload);
unsigned int Server_Movement_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload);
unsigned int Server_Ping_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload);


typedef unsigned int (*cmd_func)(ServerContext*, unsigned char*, unsigned char*);

typedef struct ServerCmd {
	cmd_func cmd_func_table[NUM_CMD_MAX][NUM_PROTO_MAX];
} ServerCmd;

void Server_Poll_Event_Handler(Config* config, 
		               ZoneRange* zonerange, 
			       Socket_Hashtable* sock_hashtable, 
			       Socket* listener, 
			       sqlite3* db, 
		               Session_Hashtable* session_hashtable_username, 
			       Session_Hashtable* session_hashtable_token, 
			       LogConfig* log_config);

ServerContext* ServerContext_Create(sqlite3* db, 
				    Socket* sock, 
				    Config* config,
				    ZoneRange* zonerange,
				    Session_Hashtable* session_hashtable_username, 
				    Session_Hashtable* session_hashtable_token,
				    LogConfig* log_config);

void Server_Populate_Range_Hashtables(ServerContext* ctxt, 
		                      Location* location,
				      Session_Hashtable* session_hashtable_inrange, 
				      Session_Hashtable* session_hashtable_outofrange);

void Server_Broadcast_Movement(ServerContext* ctxt, Session* session, unsigned short direction, unsigned short speed, unsigned short frames);

void ServerContext_Destroy(ServerContext* ctxt);
ServerCmd* ServerCmd_Create();
void ServerCmd_Destroy(ServerCmd* cmd);
unsigned int ServerCmd_Dispatch(ServerContext* ctxt, unsigned char* header, unsigned char* payload, unsigned short cmd, unsigned short proto);
Socket* Server_Connect(char* ip_address, char* port, LogConfig* log_config);
Socket* Server_Bind_Listen(char ip_address[NI_MAXHOST+1], char port[NI_MAXSERV+1], LogConfig* log_Config);
Socket* Server_Accept(Socket* listener, LogConfig* log_config);

#endif
