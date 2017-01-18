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
#include "sock.h"
#include "user.h"
#include "user_database.h"
#include "pollfd_dynamic_array.h"

ServerContext* ServerContext_Create(sqlite3* db, Socket* sock, Session_Hashtable* session_hashtable, LogConfig* log_config) {		
	
	ServerContext* ctxt = (ServerContext*)malloc(sizeof(ServerContext));
	
	ctxt->db = db;
	ctxt->sock = sock;
	ctxt->session_hashtable = session_hashtable;
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


void Server_Poll_Event_Handler(Socket_Hashtable* sock_hashtable, Socket* listener, sqlite3* db, Session_Hashtable* session_hashtable, LogConfig* log_config) {
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
			
			char* string_hashtable = Session_Hashtable_String(session_hashtable);
			Log_log(log_config, LOG_DEBUG, "session table:\n %s", string_hashtable);
			free(string_hashtable);

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
							ServerContext* ctxt = ServerContext_Create(db, sock, session_hashtable, log_config); 
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
								ServerContext* ctxt = ServerContext_Create(db, sock, session_hashtable, log_config); 
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
		User* new_user = User_Create(username, password, email);
		User_Insert(ctxt->db, new_user, ctxt->log_config);
		//send response back that user has been registered
		unsigned char* data = Protocol_Register_Response();
	       	char* format = Protocol_Get_Format(data);	
		unsigned int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
	} else {
		Log_log(ctxt->log_config, LOG_DEBUG, "already registered send error response\n");
		unsigned char* data = Protocol_Error_Response(NULL, ERR_REG);
		Log_log(ctxt->log_config, LOG_DEBUG, "after error response\n");
		char* format = Protocol_Get_Format(data);	
		unsigned int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
	}			
	return bytes_sent;	
}

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
	       	char* format = Protocol_Get_Format(data);	
		unsigned int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
	} else {
		//is this user logged in already
		Session* logged_in_session = Session_Hashtable_Get(ctxt->session_hashtable, username);
		if(logged_in_session == NULL) {
			//create session toke
			
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

			Session* session = Session_Create(session_token, user->username, user->password, user->email, 0, 0, ctxt->sock);
			Session_Hashtable_Set(ctxt->session_hashtable, username, session);

			//send in logged in response back to client
			unsigned char* data = Protocol_Login_Response(session_token);
			char* format = Protocol_Get_Format(data);	
			unsigned int size_to_send = Binary_Calcsize(format);
			bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	

		} else {
			//already logged in send error code back to client
			unsigned char* data = Protocol_Error_Response(NULL, ERR_LOGIN_AGAIN);
	       		char* format = Protocol_Get_Format(data);	
			unsigned int size_to_send = Binary_Calcsize(format);
			bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config); 	
		}			
	}
	return bytes_sent;


}

unsigned int Server_Logout_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {
	char* session_token = (char*)malloc(SESSION_LENGTH + 1);
	Protocol_Session_Unpack(header, session_token);
	Session* session = Session_Hashtable_Get(ctxt->session_hashtable, session_token); 
	int bytes_sent = 0;
	if(session != NULL) {
		unsigned char* data = Protocol_Logout_Response(session_token);
		char* format = Protocol_Get_Format(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	} else {	
		//send error response
		unsigned char* data = Protocol_Error_Response(session_token, ERR_SESSION_NO_EXIST);
		char* format = Protocol_Get_Format(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);	
	}	
	free(session_token);
	return bytes_sent;

}
unsigned int Server_Movement_Response_Send(ServerContext* ctxt, unsigned char* header, unsigned char* payload) {
	unsigned short direction = 0;
	unsigned short speed = 0;
	unsigned short previous_frames = 0;
	Protocol_Movement_Send_Payload_Unpack(payload, &direction, &speed, &previous_frames);
	Log_log(ctxt->log_config, LOG_INFO, "direction: %i speed: %i\n", direction, speed, previous_frames);	
	
	Log_log(ctxt->log_config, LOG_DEBUG, "broadcasting direction and speed to all other connected clients within range\n");
	//lookup session for this session....find coordinates
	//broadcast direction and speed to close by players
	//close by is on screen and a bit off screen...camera +- 20%
	//go through all logged in sessions and see if position is close by
	
	unsigned int bytes_sent = 0;
	Log_log(ctxt->log_config, LOG_DEBUG, "sending movement response\n");
	char* session_token = (char*)malloc(SESSION_LENGTH + 1);
	Protocol_Session_Unpack(header, session_token);
	Session* session = Session_Hashtable_Get(ctxt->session_hashtable, session_token); 
	if(session != NULL) {
		unsigned char* data = Protocol_Movement_Response(session_token);
		char* format = Protocol_Get_Format(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);
	} else {	
		//send error response
		unsigned char* data = Protocol_Error_Response(session_token, ERR_SESSION_NO_EXIST);
		char* format = Protocol_Get_Format(data);
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
	Session* session = Session_Hashtable_Get(ctxt->session_hashtable, session_token); 
	if(session != NULL) {
		int current_pos_x = 0;
		int current_pos_y = 0;	
		Protocol_Ping_Send_Payload_Unpack(payload, &current_pos_x, &current_pos_y);
		session->current_pos_x = current_pos_x;
		session->current_pos_y = current_pos_y;
		unsigned char* data = Protocol_Ping_Response(session_token);
		char* format = Protocol_Get_Format(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);	
	} else {	
		//send error response
		unsigned char* data = Protocol_Error_Response(session_token, ERR_SESSION_NO_EXIST);
		char* format = Protocol_Get_Format(data);
		int size_to_send = Binary_Calcsize(format);
		bytes_sent = Socket_Send(ctxt->sock, data, size_to_send, ctxt->log_config);	
	}	
	free(session_token);	
	return bytes_sent;
}