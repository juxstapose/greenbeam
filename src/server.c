#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <poll.h>
#include "server.h"
#include "socket_hashtable.h"
#include "session_hashtable.h"
#include "util.h"
#include "log.h"
#include "pack.h"
#include "protocol.h"
#include "protocol_format.h"
#include "sock.h"
#include "database.h"
#include "user.h"
#include "user_table.h"
#include "location.h"
#include "location_table.h"
#include "pollfd_dynamic_array.h"
#include "config.h"
#include "zonerange.h"

ServerContext* ServerContext_Create(sqlite3* db, 
		                    Socket* sock, 
		                    Config* config,
                                    ZoneRange* zonerange,
				    Session_Hashtable* session_hashtable_username, 
				    Session_Hashtable* session_hashtable_token, LogConfig* log_config) {		
	
	ServerContext* ctxt = (ServerContext*)malloc(sizeof(ServerContext));
	
	ctxt->db = db;
	ctxt->sock = sock;
	ctxt->config = config;
	ctxt->zonerange = zonerange;
	ctxt->session_hashtable_username = session_hashtable_username;
	ctxt->session_hashtable_token = session_hashtable_token;
	ctxt->log_config = log_config;

	return ctxt;
}

void ServerContext_Destroy(ServerContext* ctxt) {
	if(ctxt != NULL) {
		free(ctxt);
	}
}

ServerCmd* ServerCmd_Create() {
	ServerCmd* server_cmd = (ServerCmd*)malloc(sizeof(ServerCmd));
	int x = 0;
	int y = 0;
	for(x=0; x< NUM_CMD_MAX; x++) {
		for(y=0; y<NUM_PROTO_MAX; y++) {
			server_cmd->cmd_func_table[x][y] = NULL;
		}	
	}
	server_cmd->cmd_func_table[CMD_REGISTER][PROTO_SEND] = Server_Register_Response_Send;
	server_cmd->cmd_func_table[CMD_LOGIN][PROTO_SEND] = Server_Login_Response_Send;
	server_cmd->cmd_func_table[CMD_LOGOUT][PROTO_SEND] = Server_Logout_Response_Send;
	server_cmd->cmd_func_table[CMD_MOVEMENT][PROTO_SEND] = Server_Movement_Response_Send;
	server_cmd->cmd_func_table[CMD_PING][PROTO_SEND] = Server_Ping_Response_Send;
	return server_cmd;
}

void ServerCmd_Destroy(ServerCmd* server_cmd) {
	if(server_cmd != NULL) {
		free(server_cmd);
	}
}



Socket* Server_Bind_Listen(char ip_address[NI_MAXHOST+1], char port[NI_MAXSERV+1], LogConfig* log_config) {
	Socket* sock = NULL;

	int status;
	struct addrinfo config;
	struct addrinfo* servinfo;

	memset(&config, 0, sizeof(config));	
	config.ai_family = AF_UNSPEC;
	config.ai_socktype = SOCK_STREAM;
	
	status = getaddrinfo(ip_address, port, &config, &servinfo);
	if(status != 0) {
		Log_log(log_config, LOG_ERROR, "getaddrinfo error %s\n", gai_strerror(status));
		return NULL;
	}
	Log_log(log_config, LOG_DEBUG, "list sock should be null: %p\n", sock);	
	int listener_socket = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
	if(listener_socket == -1) {
		Log_log(log_config, LOG_ERROR, "socket creation error %s", strerror(errno));
		return NULL;
	} else {
		sock = Socket_Create(listener_socket, ip_address, port);
	}
	Log_log(log_config, LOG_DEBUG, "list sock should be allocated: %p\n", sock);	
	status = bind(listener_socket, servinfo->ai_addr, servinfo->ai_addrlen);
	if(status == -1) {
		Log_log(log_config, LOG_ERROR, "bind error %s\n", strerror(errno));
		return NULL;
	}	
	Log_log(log_config, LOG_DEBUG, "list sock should be: %p\n", sock);	
	status = listen(listener_socket, 10);	
	if(status == -1) {
		Log_log(log_config, LOG_ERROR, "listen error %s\n", strerror(errno));
		return NULL;
	}
	return sock;
}

Socket* Server_Accept(Socket* listener, LogConfig* log_config) {

	struct sockaddr_storage client_address;
	socklen_t client_len = sizeof(client_address);

	int connected_socket_id = accept(listener->id, (struct sockaddr *)&client_address, &client_len);
	
	Log_log(log_config, LOG_DEBUG, "max host len: %i\n", NI_MAXHOST+1);	
	Log_log(log_config, LOG_DEBUG, "max serv len: %i\n", NI_MAXSERV+1);	
	char hostname[NI_MAXHOST+1] = {'\0'};
	char port[NI_MAXSERV+1] = {'\0'};
	int gni_status = getnameinfo((struct sockaddr*)&client_address, client_len, hostname, NI_MAXHOST+1, port, NI_MAXSERV+1, 0);
	if(gni_status > 0) {
		Log_log(log_config, LOG_ERROR, "getnameinfo error: %s\n", gai_strerror(gni_status));
	}
	Log_log(log_config, LOG_INFO, "socket connected %s on %s\n", hostname, port);
	Socket* sock = Socket_Create(connected_socket_id, hostname, port);
	return sock;
}


void Server_Poll_Event_Handler(Config* config, 
		               ZoneRange* zonerange, 
			       Socket_Hashtable* sock_hashtable, 
			       Socket* listener, 
			       sqlite3* db, 
		               Session_Hashtable* session_hashtable_username, 
			       Session_Hashtable* session_hashtable_token, 
			       LogConfig* log_config) {
	//struct pollfd* fd_set = calloc(MAX_FD, sizeof(struct pollfd));
		
	int connected_socket_id = 0;
	int fd_index = 0;
		
	
	int num_fd = 10;
	PollFD_Dynamic_Array* fd_array = (PollFD_Dynamic_Array*)PollFD_Dynamic_Array_Create(num_fd);
	struct pollfd fd_listener_item;
	fd_listener_item.fd = listener->id;
	fd_listener_item.events = POLLIN;
	PollFD_Dynamic_Array_Insert(fd_array, &fd_listener_item);

	int shutdown = 0;
	while(1) {
		Log_log(log_config, LOG_DEBUG, "before poll\n");
		int status = poll(fd_array->array, fd_array->count, 2500);
		if(status < 0) {
			Log_log(log_config, LOG_ERROR, "error returned on poll: %s\n", strerror(errno));
		}
		if(status == 0) {
			Log_log(log_config, LOG_INFO, "poll timed out\n");
		}
		if(status > 0) {
			Log_log(log_config, LOG_DEBUG, "num fd with events set with stuff: %i\n", status);	
		}
		for(fd_index = 0; fd_index<fd_array->count; fd_index++) {
			Log_log(log_config, LOG_DEBUG, "check for read event on index:%i and fd:%i\n", fd_index, fd_array->array[fd_index].fd);
			
			char* string_d_array = PollFD_Dynamic_Array_String(fd_array);
			Log_log(log_config, LOG_DEBUG, "socket array:\n %s\n", string_d_array);
			free(string_d_array);
			
			char* string_username_hashtable = Session_Hashtable_String(session_hashtable_username);
			Log_log(log_config, LOG_DEBUG, "session table:\n %s", string_username_hashtable);
			free(string_username_hashtable);
			
			char* string_token_hashtable = Session_Hashtable_String(session_hashtable_token);
			Log_log(log_config, LOG_DEBUG, "session table:\n %s", string_token_hashtable);
			free(string_token_hashtable);

			if(fd_array->array[fd_index].revents & POLLIN) {
				Log_log(log_config, LOG_DEBUG, "read event found!\n");
				if(fd_array->array[fd_index].fd == listener->id) {
					Log_log(log_config, LOG_DEBUG, "incoming connection...\n");
					Socket* sock = Server_Accept(listener, log_config);
					struct pollfd fd_item;
					fd_item.fd = sock->id;
					fd_item.events = POLLIN;	
					PollFD_Dynamic_Array_Insert(fd_array, &fd_item);
					Socket_Hashtable_Set(sock_hashtable, sock->id, sock);
						
				} else {
					//get connected socket in set
					Socket* sock = Socket_Hashtable_Get(sock_hashtable, fd_array->array[fd_index].fd);
					Log_log(log_config, LOG_INFO, "read event on fd id:%i connected from: (%s,%s)\n", sock->id, sock->ip_address, sock->port);
						
					Log_log(log_config, LOG_DEBUG, "inspect header to dispatch approriate response\n");
					unsigned int size_to_read = Binary_Calcsize(HEADER_FORMAT);
					int sock_errors = 0;
					unsigned char* header = Socket_Recv(sock, size_to_read, &sock_errors, log_config);
					if(sock_errors & SOCK_DISCONNECTED) {
						Log_log(log_config, LOG_ERROR, "broken connection\n");
						//fd_set[fd_index].fd = -1;
						PollFD_Dynamic_Array_Remove(fd_array, fd_index);
					}
					if(sock_errors & SOCK_ERROR) {
						Log_log(log_config, LOG_ERROR, "error on socket\n");
						if(errno & EAGAIN || errno & EWOULDBLOCK) {
							Log_log(log_config, LOG_DEBUG, "socket would block\n");	
						} 
					}
					if(header != NULL) {
						unsigned short cmd = 0;
						unsigned short proto = 0;
						unsigned int payload_size = 0;	
						Protocol_Header_Unpack(header, &cmd, &proto, &payload_size);	
						Log_log(log_config, LOG_INFO, "unpacked header cmd:%i and proto:%i...unpacking payload with size:%i\n", cmd, proto, payload_size);	
						if(cmd == CMD_SHUTDOWN && proto == PROTO_SEND) {
							Log_log(log_config, LOG_INFO, "received shut down cmd...shutting down server\n");
							shutdown = 1;
							free(header);
							break;
						}
						//NULL payload
						if(payload_size == 0) {
							ServerContext* ctxt = ServerContext_Create(db, sock, config, zonerange, session_hashtable_username, 
												   session_hashtable_token, log_config); 
							Log_log(log_config, LOG_INFO, "cmd dispatched (%i %i)\n", cmd, proto);
							unsigned int bytes_sent = ServerCmd_Dispatch(ctxt, header, NULL, cmd, proto);
							Log_log(log_config, LOG_INFO, "%i bytes sent\n", bytes_sent);
							ServerContext_Destroy(ctxt);
						} else {
							int sock_errors = 0;
							unsigned char* payload = Socket_Recv(sock, payload_size, &sock_errors, log_config);	
							if(sock_errors & SOCK_DISCONNECTED) {
								Log_log(log_config, LOG_ERROR, "broken connection\n");
								//fd_set[fd_index].fd = -1;
								PollFD_Dynamic_Array_Remove(fd_array, fd_index);
							}
							if(sock_errors & SOCK_ERROR) {
								Log_log(log_config, LOG_ERROR, "error on socket\n");
								if(errno & EAGAIN || errno & EWOULDBLOCK) {
									Log_log(log_config, LOG_DEBUG, "socket would block\n");	
								} 
							}
							if(payload != NULL) {
								ServerContext* ctxt = ServerContext_Create(db, sock, config, zonerange, session_hashtable_username, 
													   session_hashtable_token, log_config); 
								Log_log(log_config, LOG_INFO, "cmd dispatched (%i %i)\n", cmd, proto);
								unsigned int bytes_sent = ServerCmd_Dispatch(ctxt, header, payload, cmd, proto);
								Log_log(log_config, LOG_INFO, "%i bytes sent\n", bytes_sent);
								ServerContext_Destroy(ctxt);
								free(payload);
							} else {
								Log_log(log_config, LOG_ERROR, "no payload received\n");

							}	
						}
						free(header);
					} else { //end if no header
						Log_log(log_config, LOG_ERROR, "no header received\n");
						break;
					}

				}//end else if listener
			}//end if read event

		}//end loop through FDs
		if(shutdown == 1) {
			PollFD_Dynamic_Array_Destroy(fd_array);
			break;
		}
	}//end while 1
	PollFD_Dynamic_Array_Destroy(fd_array);
}//end event handler

unsigned int ServerCmd_Dispatch(ServerContext* ctxt, unsigned char* header, unsigned char* payload, unsigned short cmd, unsigned short proto) {
	Log_log(ctxt->log_config, LOG_DEBUG, "ServerCmd_Dispatch\n");	
	int cmd_valid = Protocol_Is_Cmd_Valid(cmd, proto);
	Log_log(ctxt->log_config, LOG_DEBUG, "cmd valid: %i\n", cmd_valid);	
	ServerCmd* server_cmd = ServerCmd_Create();
	if(cmd_valid) { 
		Log_log(ctxt->log_config, LOG_DEBUG, "execute function pointer cmd:%i proto:%i\n", cmd, proto);
		Log_log(ctxt->log_config, LOG_DEBUG, "function pointer %p\n", server_cmd->cmd_func_table[cmd][proto]);	
		if(server_cmd->cmd_func_table[cmd][proto] != NULL) {
			return (*server_cmd->cmd_func_table[cmd][proto])(ctxt, header, payload);
		}
	} else {
		Log_log(ctxt->log_config, LOG_INFO, "cmd and proto not valid: cmd:%i proto:%i\n", cmd, proto);
	}
	ServerCmd_Destroy(server_cmd);
	return 0;
		
}


unsigned int Server_Register_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {
	Log_log(ctxt->log_config, LOG_DEBUG, "Server Register Response_Send\n");

	char username[USERNAME_LENGTH+1] = {'\0'};
	char password[PASSWORD_LENGTH+1] = {'\0'};
	char email[EMAIL_LENGTH+1] = {'\0'};
	Protocol_Register_Send_Payload_Unpack(payload, username, password, email);
	Log_log(ctxt->log_config, LOG_DEBUG, "username, password, and email (%s %s %s)\n", username, password, email);
	
	//see if this user has been created before	
	User* user = User_Find_By_Username(ctxt->db, username, ctxt->log_config);
	
	Log_log(ctxt->log_config, LOG_DEBUG, "user: %p\n", user);
	
	unsigned int bytes_sent = 0;
	if(user == NULL) {
		User* new_user = User_Create(0, username, password, email);
		User_Insert(ctxt->db, new_user, ctxt->log_config);
		User* user_inserted = User_Find_By_Username(ctxt->db, username, ctxt->log_config);
		if(user_inserted != NULL) {
			Location* location = Location_Create(0, user_inserted->user_key, LOC_STARTING_ZONE, LOC_STARTING_X, LOC_STARTING_Y);
			Log_log(ctxt->log_config, LOG_DEBUG, "location: %p\n", location);
			Location_Insert(ctxt->db, location, ctxt->log_config);
		}
		//send response back that user has been registered
		unsigned char* data = Protocol_Register_Response();
	       	char* format = Protocol_Format_Get(data);	
		unsigned int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
	} else {
		Log_log(ctxt->log_config, LOG_DEBUG, "already registered send error response\n");
		unsigned char* data = Protocol_Error_Response(NULL, ERR_REG);
		Log_log(ctxt->log_config, LOG_DEBUG, "after error response\n");
		char* format = Protocol_Format_Get(data);	
		unsigned int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
	}			
	return bytes_sent;	
}

void Server_Populate_Range_Hashtables(ServerContext* ctxt, 
		                      Location* location,
				      Session_Hashtable* session_hashtable_inrange, 
				      Session_Hashtable* session_hashtable_outofrange) {
	int x = 0;
	for(x=0; x<ctxt->session_hashtable_username->size; x++) {	
		if(ctxt->session_hashtable_username->table[x] != NULL) {
			Session_List* list = (Session_List*)ctxt->session_hashtable_username->table[x];
			Session_Node* current = list->head->next;
			while(current != NULL) {
				int r = ZoneRange_Is_Location_InRange(ctxt->zonerange, ctxt->config, location, current->session->location, ctxt->log_config);
				Log_log(ctxt->log_config, LOG_DEBUG, "is in zone range: %i\n", r);
				if(r == 1) {
					Log_log(ctxt->log_config, LOG_DEBUG, "populate inrange with session\n");
					Session_Hashtable_Remove(session_hashtable_outofrange, current->string_key); 
					Session_Hashtable_Set(session_hashtable_inrange, current->string_key, current->session); 
				} else {
					Log_log(ctxt->log_config, LOG_DEBUG, "populate outofrange with session\n");
					Session_Hashtable_Remove(session_hashtable_inrange, current->string_key); 
					Session_Hashtable_Set(session_hashtable_outofrange, current->string_key, current->session); 
				}
				current = current->next;
			}//end while session list
		}//end if not null
	}//end for loop		
}//end server function

unsigned int Server_Login_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {

	Log_log(ctxt->log_config, LOG_DEBUG, "received login packet\n");
	char username[USERNAME_LENGTH+1] = {'\0'};
	char password[PASSWORD_LENGTH+1] = {'\0'};
	Protocol_Login_Send_Payload_Unpack(payload, username, password);
	Log_log(ctxt->log_config, LOG_DEBUG, "username and password login (%s %s)\n", username, password);
	

	unsigned int bytes_sent = 0;	
	//has this user registered	
	User* user = User_Find_By_Username(ctxt->db, username, ctxt->log_config);
	if(user == NULL) {
		//send response back that the user logging in has not registered
		unsigned char* data = Protocol_Error_Response(NULL, ERR_LOGIN_NOT_REG);
	       	char* format = Protocol_Format_Get(data);	
		unsigned int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
	} else {
		//is this user logged in already
		Session* logged_in_session = Session_Hashtable_Get(ctxt->session_hashtable_username, username);
		if(logged_in_session == NULL) {
			Log_log(ctxt->log_config, LOG_DEBUG, "create session token\n");
			//create session token	
			char session_token[SESSION_LENGTH+1];
			uint64_t microseconds = Util_Microsecond_Timestamp();
			unsigned int microsecond_length = Util_Count_Digits64(microseconds);
			unsigned int size = USERNAME_LENGTH+1+PASSWORD_LENGTH+1+microsecond_length;
			char* seed = (char*)malloc(size);
			int bytes = sprintf(seed, "%s%s%llu", username, password, microseconds);
			Util_Sha256(seed, session_token);		
			free(seed);
			
			//lookup last position by username
			//last position is last position recorded when client broken connection or logged out
			//session is updated by pings to keep position updated
			//when clients connection breaks or client logs out the session position is saved to DB 
			
			Location* location = Location_Find_By_Userkey(ctxt->db, user->user_key, ctxt->log_config);
			Log_log(ctxt->log_config, LOG_DEBUG, "find associated location: %p\n", location);
			unsigned int initial_size = 20;
			
			//in range and out of range sessions that are logged in
			Session_Hashtable* session_hashtable_inrange = Session_Hashtable_Create(initial_size);
			Session_Hashtable* session_hashtable_outofrange = Session_Hashtable_Create(initial_size);
			
			Log_log(ctxt->log_config, LOG_DEBUG, "populate inrange and out of range hashtables\n");
			//determine which sessions are in range and which are out of range	
			Server_Populate_Range_Hashtables(ctxt, 
		        			         location,
				      			 session_hashtable_inrange, 
				      			 session_hashtable_outofrange);
			
			char* inrange_str = Session_Hashtable_String(session_hashtable_inrange);	
			Log_log(ctxt->log_config, LOG_DEBUG, "inrange: %s\n", inrange_str);
			free(inrange_str);
			
			char* outofrange_str = Session_Hashtable_String(session_hashtable_outofrange);	
			Log_log(ctxt->log_config, LOG_DEBUG, "outofrange: %s\n", outofrange_str);
			free(outofrange_str);
			
			//send load cmds with locations back to client for each session that is in range

			Log_log(ctxt->log_config, LOG_DEBUG, "create session\n");
			Session* session = Session_Create(session_token, user->username, location, 
							  session_hashtable_inrange, session_hashtable_outofrange, ctxt->sock);
			
			Session_Hashtable_Set(ctxt->session_hashtable_username, username, session);
			Session_Hashtable_Set(ctxt->session_hashtable_token, session_token, session);

			Log_log(ctxt->log_config, LOG_DEBUG, "get binary form of in range hashtable\n");
			//get difference data	
			unsigned int inrange_payload_body_size = 0;
			unsigned int inrange_num_items = 0;		
			Session_Hashtable_Calc_Size_Items(session_hashtable_inrange, &inrange_payload_body_size, &inrange_num_items);
			Log_log(ctxt->log_config, LOG_DEBUG, "in range payload body size: %i inrange num_items:%i\n", inrange_payload_body_size, inrange_num_items);
			unsigned char* inrange_data = Session_Hashtable_To_Binary(session_hashtable_inrange, inrange_payload_body_size, inrange_num_items); 
			
			Log_log(ctxt->log_config, LOG_DEBUG, "get binary form of out of range hashtable\n");
			unsigned int outofrange_payload_body_size = 0;
			unsigned int outofrange_num_items = 0;		
			Session_Hashtable_Calc_Size_Items(session_hashtable_outofrange, &outofrange_payload_body_size, &outofrange_num_items);
			Log_log(ctxt->log_config, LOG_DEBUG, "out of range payload body size: %i out of range num_items:%i\n", outofrange_payload_body_size, outofrange_num_items);
			unsigned char* outofrange_data = Session_Hashtable_To_Binary(session_hashtable_outofrange, outofrange_payload_body_size, outofrange_num_items); 
			//calc data sizes	
			unsigned int outofrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
			unsigned int outofrange_data_size = outofrange_header_size + outofrange_payload_body_size;
			unsigned int inrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
			unsigned int inrange_data_size = inrange_header_size + inrange_payload_body_size;
				
			Log_log(ctxt->log_config, LOG_DEBUG, "sending data with session token: %s - inrange size: %i outofrange size: %i\n", session_token, inrange_data_size, outofrange_data_size);
			
			//send in logged in response back to client
			unsigned char* data = Protocol_Login_Response(session_token, inrange_data_size, inrange_data, outofrange_data_size, outofrange_data);
			char* format = Protocol_Format_Get(data);	
			Log_log(ctxt->log_config, LOG_DEBUG, "outgoing format: %s\n", format);
			unsigned int size_to_send = Binary_Calcsize(format);
			//printf("size to send:%i == size: %i\n", size_to_send, (Binary_Calcsize(HEADER_FORMAT) + inrange_data_size + outofrange_data_size));
			Log_log(ctxt->log_config, LOG_DEBUG, "size to send:%i == size: %i\n", size_to_send, (Binary_Calcsize(HEADER_FORMAT) + inrange_data_size + outofrange_data_size));
			bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	

		} else {
			//already logged in send error code back to client
			unsigned char* data = Protocol_Error_Response(NULL, ERR_LOGIN_AGAIN);
	       		char* format = Protocol_Format_Get(data);	
			unsigned int size_to_send = Binary_Calcsize(format);
			bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
		}			
	}
	return bytes_sent;
}

unsigned int Server_Logout_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {
	char* session_token = (char*)malloc(SESSION_LENGTH + 1);
	Protocol_Session_Unpack(header, session_token);
	Session* session = Session_Hashtable_Get(ctxt->session_hashtable_token, session_token); 
	int bytes_sent = 0;
	if(session != NULL) {
		Session_Hashtable_Remove(ctxt->session_hashtable_username, session->username);	
		Session_Hashtable_Remove(ctxt->session_hashtable_token, session->session_token);	
		unsigned char* data = Protocol_Logout_Response(session_token);
		char* format = Protocol_Format_Get(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	} else {	
		//send error response
		unsigned char* data = Protocol_Error_Response(session_token, ERR_SESSION_NO_EXIST);
		char* format = Protocol_Format_Get(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);	
	}	
	free(session_token);
	return bytes_sent;
}

void Server_Broadcast_Movement(ServerContext* ctxt, Session* session, unsigned short direction, unsigned short speed, unsigned short frames) {
	int x = 0;
	for(x=0; x<session->session_table_inrange->size; x++) {	
		if(session->session_table_inrange->table[x] != NULL) {
			Session_List* list = (Session_List*)session->session_table_inrange->table[x];
			Session_Node* current = list->head->next;
			while(current != NULL) {
				Log_log(ctxt->log_config, LOG_DEBUG, "sending %s dir:%i and speed:%i frames:%i on sock id: %i", 
					current->string_key, direction, speed, frames, current->session->sock->id);
				
				unsigned char* data = Protocol_Movement_Broadcast(current->session->session_token, direction, speed, frames);	
				char* format = Protocol_Format_Get(data);
				int size_to_send = Binary_Calcsize(format);
				int bytes_sent = Socket_Send(current->session->sock, data, size_to_send, ctxt->log_config);
				
				Log_log(ctxt->log_config, LOG_DEBUG, "bytes sent:%i \n", bytes_sent);
					
				current = current->next;
			}//end while session list
		}//end if not null
	}//end for loop		
}//Server Movement Broadcast


unsigned int Server_Movement_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {
	unsigned short direction = 0;
	unsigned short speed = 0;
	unsigned short frames = 0;
	Protocol_Movement_Send_Payload_Unpack(payload, &direction, &speed, &frames);
	Log_log(ctxt->log_config, LOG_INFO, "direction: %i speed: %i\n", direction, speed, frames);	
	
	//lookup session for this session....find coordinates
	//broadcast direction and speed to close by players
	//close by is on screen and a bit off screen...camera +- 20%
	//go through all logged in sessions and see if position is close by
	
	unsigned int bytes_sent = 0;
	char* session_token = (char*)malloc(SESSION_LENGTH + 1);
	Protocol_Session_Unpack(header, session_token);
	Session* session = Session_Hashtable_Get(ctxt->session_hashtable_token, session_token); 
	if(session != NULL) {
		
		//broadcast movement changes to in range players
		Log_log(ctxt->log_config, LOG_DEBUG, "broadcasting direction and speed to all other connected clients within range\n");
		Server_Broadcast_Movement(ctxt, session, direction, speed, frames);

		Log_log(ctxt->log_config, LOG_DEBUG, "sending movement response\n");
		unsigned char* data = Protocol_Movement_Response(session_token);
		char* format = Protocol_Format_Get(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	} else {	
		//send error response
		unsigned char* data = Protocol_Error_Response(session_token, ERR_SESSION_NO_EXIST);
		char* format = Protocol_Format_Get(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);	
	}	
	free(session_token);	
	return bytes_sent;
}





/**
 * ping sends a x,y position of a client player
 **/
unsigned int Server_Ping_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {
	//check if session exists
	int bytes_sent = 0;
	char* session_token = (char*)malloc(SESSION_LENGTH+1);	
	Protocol_Session_Unpack(header, session_token);
	Session* session_from_token = Session_Hashtable_Get(ctxt->session_hashtable_token, session_token); 
	if(session_from_token != NULL) {
		Log_log(ctxt->log_config, LOG_DEBUG, "ping get x and y pos update\n");	
		int current_pos_x = 0;
		int current_pos_y = 0;
		
		Protocol_Ping_Send_Payload_Unpack(payload, &current_pos_x, &current_pos_y);	
		
		session_from_token->location->x = current_pos_x;
		session_from_token->location->y = current_pos_y;
		
		Log_log(ctxt->log_config, LOG_DEBUG, "(%i,%i)\n", session_from_token->location->x, session_from_token->location->y);	
		
		//populate incoming differences	
		Session_Hashtable* session_hashtable_inrange = Session_Hashtable_Create(20);
		Session_Hashtable* session_hashtable_outofrange = Session_Hashtable_Create(20);

		Log_log(ctxt->log_config, LOG_DEBUG, "populate inrange and outofrange hash tables\n");	
		Server_Populate_Range_Hashtables(ctxt, 
						 session_from_token->location,
						 session_hashtable_inrange, 
						 session_hashtable_outofrange);
		
		
		//take the difference of the previous ping with the incoming ping...compare keys of the new with the old...add item from the new if its not in the old 
		Log_log(ctxt->log_config, LOG_DEBUG, "take the differences to see if anyone moved\n");	
		//old = [item1 item2 item4 item7] new = [item1 item2 item3]  new diff = item3 | old diff = item4, item7
		Session_Hashtable* session_hashtable_inrange_diff_new = Session_Hashtable_Diff_New(session_from_token->session_table_inrange, session_hashtable_inrange);
		Session_Hashtable* session_hashtable_outofrange_diff_new = Session_Hashtable_Diff_New(session_from_token->session_table_outofrange, session_hashtable_outofrange);
		
		Log_log(ctxt->log_config, LOG_DEBUG, "inrange diff new:%p\n", session_hashtable_inrange_diff_new);	
		Log_log(ctxt->log_config, LOG_DEBUG, "outofrange diff new:%p\n", session_hashtable_outofrange_diff_new);	
		
		Log_log(ctxt->log_config, LOG_DEBUG, "reassign inrange and outofrange hashtables\n");	
		Session_Hashtable_Destroy(session_from_token->session_table_inrange);
		session_from_token->session_table_inrange = session_hashtable_inrange;
		Session_Hashtable_Destroy(session_from_token->session_table_outofrange);
		session_from_token->session_table_outofrange = session_hashtable_outofrange;

		//send load cmds with locations back to client for each session that is in range
		//send unload cmds with locations back to client for each session that is in range
		//inrange [total_size][num_items][username1_size username1 x1 y1 username2_size username2 x2 y2 username3_size username3 x3 y3] 
		//outofrange [total_size][num_items][username1_size username1 username2_size username2 username3_size username3] 
		
		Log_log(ctxt->log_config, LOG_DEBUG, "get the binary output\n");	
		//get difference data	
		unsigned int inrange_payload_body_size = 0;
		unsigned int inrange_num_items = 0;		
		Session_Hashtable_Calc_Size_Items(session_hashtable_inrange_diff_new, &inrange_payload_body_size, &inrange_num_items);
		Log_log(ctxt->log_config, LOG_DEBUG, "inrange payload body size:%i\n", inrange_payload_body_size);
		Log_log(ctxt->log_config, LOG_DEBUG, "inrange num items:%i\n", inrange_num_items);
		unsigned char* inrange_data = Session_Hashtable_To_Binary(session_hashtable_inrange_diff_new, inrange_payload_body_size, inrange_num_items); 
		Log_log(ctxt->log_config, LOG_DEBUG, "inrange data:%p\n", inrange_data);
		
		char* inrange_diff_new_str = Session_Hashtable_String(session_hashtable_inrange_diff_new);
		Log_log(ctxt->log_config, LOG_DEBUG, "inrange: %s\n", inrange_diff_new_str);
		free(inrange_diff_new_str);
		
		unsigned int outofrange_payload_body_size = 0;
		unsigned int outofrange_num_items = 0;		
		Session_Hashtable_Calc_Size_Items(session_hashtable_outofrange_diff_new, &outofrange_payload_body_size, &outofrange_num_items);
		Log_log(ctxt->log_config, LOG_DEBUG, "outofrange payload body size:%i\n", outofrange_payload_body_size);
		Log_log(ctxt->log_config, LOG_DEBUG, "outofrange num items:%i\n", outofrange_num_items);
		unsigned char* outofrange_data = Session_Hashtable_To_Binary(session_hashtable_outofrange_diff_new, outofrange_payload_body_size, outofrange_num_items); 
		Log_log(ctxt->log_config, LOG_DEBUG, "outofrange data:%p\n", outofrange_data);
		
		char* outofrange_diff_new_str = Session_Hashtable_String(session_hashtable_outofrange_diff_new);
		Log_log(ctxt->log_config, LOG_DEBUG, "outofrange: %s\n", outofrange_diff_new_str);
		free(outofrange_diff_new_str);

		//calc data sizes	
		unsigned int outofrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
		unsigned int outofrange_data_size = outofrange_header_size + outofrange_payload_body_size;
		unsigned int inrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);	
		unsigned int inrange_data_size = inrange_header_size + inrange_payload_body_size;
		
		//send data
		Log_log(ctxt->log_config, LOG_DEBUG, "create and send ping response\n");	
		unsigned char* data = Protocol_Ping_Response(session_token, inrange_data_size, inrange_data, outofrange_data_size, outofrange_data);
		char* format = Protocol_Format_Get(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
		
		Log_log(ctxt->log_config, LOG_DEBUG, "session hashtable cleanup\n");	
		
		//cleanup	
		Session_Hashtable_Destroy(session_hashtable_inrange_diff_new);
		session_hashtable_inrange_diff_new = NULL;
		Session_Hashtable_Destroy(session_hashtable_outofrange_diff_new);
		session_hashtable_outofrange_diff_new = NULL;
		
		if(inrange_data != NULL) {
			free(inrange_data);
		}
		if(outofrange_data != NULL) {
			free(outofrange_data);
		}

	} else {	
		//send error response
		unsigned char* data = Protocol_Error_Response(session_token, ERR_SESSION_NO_EXIST);
		char* format = Protocol_Format_Get(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);	
	}	
	free(session_token);	
	return bytes_sent;
}
