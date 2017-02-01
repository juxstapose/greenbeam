#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <pthread.h>
#include <errno.h>
#include "process.h"
#include "util.h"
#include "sock.h"
#include "log.h"
#include "pack.h"
#include "queue.h"
#include "client.h"
#include "protocol_format.h"
#include "session_hashtable.h"
#include "clientcontext_hashtable.h"
#include "socket_hashtable.h"
#include "string_hashtable.h"
#include "test_client_server.h"

void* receive_handler(void* arg) {
	ClientContext* ctxt = (ClientContext*)(arg);
	Log_log(ctxt->log_config, LOG_DEBUG, "%s: enter read thread\n", ctxt->name);
	
	struct pollfd* fd_set = calloc(1, sizeof(struct pollfd));
		
	fd_set[0].fd = ctxt->sock->id;
	fd_set[0].events = POLLIN;
	
	Log_log(ctxt->log_config, LOG_DEBUG, "%s: enter poll while loop\n", ctxt->name);
	while(1) {
		if(ctxt->stop_thread == 1) {
			Log_log(ctxt->log_config, LOG_DEBUG, "%s: exiting thread\n", ctxt->name);
			break;
		}
		
		Log_log(ctxt->log_config, LOG_DEBUG, "%s: wait before poll\n", ctxt->name);
		int status = poll(fd_set, 1, 2500);
		if(status < 0) {
			Log_log(ctxt->log_config, LOG_ERROR, "%s: error returned on poll: %s\n", ctxt->name, strerror(errno));
		}
		if(status == 0) {
			Log_log(ctxt->log_config, LOG_INFO, "%s: poll timed out\n", ctxt->name);
		}
		if(status > 0) {
			Log_log(ctxt->log_config, LOG_DEBUG, "%s: num fd with events set with stuff: %i\n", ctxt->name, status);	
		}
		if(fd_set[0].revents & POLLIN) {
			
			int sock_errors = 0;
			unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
			unsigned char* header = Socket_Recv(ctxt->sock, header_size, &sock_errors, ctxt->log_config);
			if(sock_errors & SOCK_DISCONNECTED) {
				Log_log(ctxt->log_config, LOG_ERROR, "%s: broken connection\n", ctxt->name);
				fd_set[0].fd = -1;
			}
			if(sock_errors & SOCK_ERROR) {
				Log_log(ctxt->log_config, LOG_ERROR, "%s: error on socket: %s\n", ctxt->name, strerror(errno));
				if(errno & EAGAIN || errno & EWOULDBLOCK) {
					Log_log(ctxt->log_config, LOG_DEBUG, "%s: socket would block\n", ctxt->name);	
				} 
			}
			if(header != NULL) {
				unsigned short cmd = 0;
				unsigned short proto = 0;
				unsigned int payload_size = 0;	
				Protocol_Header_Unpack(header, &cmd, &proto, &payload_size);	

				Log_log(ctxt->log_config, LOG_INFO, "%s: unpacked header cmd:%i and proto:%i...unpacking payload with size:%i\n", ctxt->name, cmd, proto, payload_size);	
				int sock_errors = 0;
				unsigned char* payload = Socket_Recv(ctxt->sock, payload_size, &sock_errors, ctxt->log_config);	
				if(sock_errors & SOCK_DISCONNECTED) {
					Log_log(ctxt->log_config, LOG_ERROR, "%s: broken connection\n", ctxt->name);
					fd_set[0].fd = -1;
				}
				if(sock_errors & SOCK_ERROR) {
					Log_log(ctxt->log_config, LOG_ERROR, "%s: error on socket\n", ctxt->name);
					if(errno & EAGAIN || errno & EWOULDBLOCK) {
						Log_log(ctxt->log_config, LOG_DEBUG, "%s: socket would block\n", ctxt->name);	
					} 
				}
					
				if(payload != NULL) {
					
					//pack header and payload together
					unsigned int size = header_size + payload_size;
					unsigned char* data = (unsigned char*)malloc(size);
					memcpy(data, header, header_size);
					memcpy(data+header_size, payload, payload_size);	

					Log_log(ctxt->log_config, LOG_INFO, "%s: received data put it on queue to be consumed\n", ctxt->name);	
					pthread_mutex_lock(&ctxt->mutex);
					Queue_Enqueue(ctxt->queue, data);		
					pthread_cond_signal(&ctxt->cond);
					pthread_mutex_unlock(&ctxt->mutex);

					free(payload);
				}//end if payload is NULL
				else {
					break;
				}
				free(header);
			}//end if header is NULL
		}//end read event
		if(fd_set[0].revents & POLLERR) {
			Log_log(ctxt->log_config, LOG_ERROR, "%s: error on fd %s\n", ctxt->name, strerror(errno));
		}
		if(fd_set[0].revents & POLLHUP) {
			Log_log(ctxt->log_config, LOG_ERROR, "%s: hang up on fd\n", ctxt->name);
		}
	}
	Log_log(ctxt->log_config, LOG_DEBUG, "%s: signalling calling thread that we have exited\n", ctxt->name);
	pthread_mutex_lock(&ctxt->stop_mutex);
	ctxt->stop_thread = 0;
	pthread_cond_signal(&ctxt->stop_cond);
	pthread_mutex_unlock(&ctxt->stop_mutex);
	return NULL;
}//end handler

void Server_Run(char* ip_address, char* port) { 

	char* args[8] = {'\0'};
	args[0] = "./test_server";
	args[1] = "--ip_address";
	args[2] = ip_address;
	args[3] = "--port";
	args[4] = port;
	args[5] = "--empty_db";
	args[6] = "&";
	args[7] = NULL;
	
	Process_Run(args);

}

unsigned char* Shutdown(Socket* sock, LogConfig* main_log_config, LogConfig* thread_log_config) {
	if(sock != NULL) {	
		Queue* queue = Queue_Create();
		char* name = "SHUTDOWN";
		ClientContext* ctxt = ClientContext_Create(name, sock, queue, thread_log_config);
		
		int bytes_sent = Client_Shutdown_Send(ctxt);

		ClientContext_Destroy(ctxt);
	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}
}

unsigned char* Register(Socket* sock, char* username, char* password, char* email, 
		        LogConfig* main_log_config, LogConfig* thread_log_config) {
	
	Log_log(main_log_config, LOG_DEBUG, "sending initial registration data\n");
	unsigned char* data = NULL;
	if(sock != NULL) {	
		Queue* queue = Queue_Create();
		char* name = "REGISTER";
		ClientContext* ctxt = ClientContext_Create(name, sock, queue, thread_log_config);

		//start receive thread
		pthread_t receive_thread;
		Log_log(main_log_config, LOG_DEBUG, "start receive data thread\n");
		int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
		
		int bytes_sent = Client_Register_Send(ctxt, username, password, email);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&ctxt->mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&ctxt->cond, &ctxt->mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&ctxt->mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Format_Get(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
	
		pthread_mutex_lock(&ctxt->stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&ctxt->stop_cond, &ctxt->stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&ctxt->stop_mutex);
		ClientContext_Destroy(ctxt);
		


	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}
		
	Log_log(main_log_config, LOG_DEBUG, "returning data\n");
	return data;

}

unsigned char* Login(Socket* sock, char* username, char* password, 
		     LogConfig* main_log_config, LogConfig* thread_log_config) {

	
	Log_log(main_log_config, LOG_DEBUG, "sending loging data\n");

	unsigned char* data = NULL;
	if(sock != NULL) {	
		Queue* queue = Queue_Create();
		char* name = "LOGIN";
		ClientContext* ctxt = ClientContext_Create(name, sock, queue, thread_log_config);

		//start receive thread
		pthread_t receive_thread;
		Log_log(main_log_config, LOG_DEBUG, "start receive data thread\n");
		int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
		
		int bytes_sent = Client_Login_Send(ctxt, username, password);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&ctxt->mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&ctxt->cond, &ctxt->mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&ctxt->mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Format_Get(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
		
		pthread_mutex_lock(&ctxt->stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&ctxt->stop_cond, &ctxt->stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&ctxt->stop_mutex);

		ClientContext_Destroy(ctxt);

	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}
	return data;

}

unsigned char* Ping(Socket* sock, char* session_token, int current_pos_x, int current_pos_y, 
		     LogConfig* main_log_config, LogConfig* thread_log_config) {
	
	Log_log(main_log_config, LOG_DEBUG, "sending ping data\n");

	unsigned char* data = NULL;
	if(sock != NULL) {	
		Queue* queue = Queue_Create();
		char* name = "LOGIN";
		ClientContext* ctxt = ClientContext_Create(name, sock, queue, thread_log_config);

		//start receive thread
		pthread_t receive_thread;
		Log_log(main_log_config, LOG_DEBUG, "start receive data thread\n");
		int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
		
		int bytes_sent = Client_Ping_Send(ctxt, session_token, current_pos_x, current_pos_y);

		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&ctxt->mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&ctxt->cond, &ctxt->mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&ctxt->mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Format_Get(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
		
		pthread_mutex_lock(&ctxt->stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&ctxt->stop_cond, &ctxt->stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&ctxt->stop_mutex);

		ClientContext_Destroy(ctxt);

	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}
	return data;

}

void Movement_Broadcast(Socket_Hashtable* socket_hashtable_input, String_Hashtable* string_hashtable_input,
	       		char* sending_username,	
			char* session_token, unsigned short direction, unsigned short speed, unsigned short frames,
			String_Hashtable* string_hashtable_output,
		        LogConfig* main_log_config, LogConfig* thread_log_config) {

	ClientContext_Hashtable* client_context_hashtable = ClientContext_Hashtable_Create(20);
	if(string_hashtable_input != NULL) {
		for(i=0; i<string_hashtable_input->size; i++) {
			if(string_hashtable_input->table[i] != NULL) {
				String_List* list = (String_List*)string_hashtable_input->table[i];
				String_Node* current = list->head->next;
				while(current != NULL) {
					Log_log(main_log_config, LOG_INFO, "username: %s => sock_id: %s\n", current->string_key, current->data);	
					
					Queue* queue = Queue_Create();
					char name[128] = {'\0'};
					int bytes = sprintf(name, "MOVEMENT_%s", current->string_key);
					int fd_id = atoi(current->data);
					Socket* sock = Socket_Hashtable_Get(socket_hashtable_input, fd_id);
					ClientContext* ctxt = ClientContext_Create(name, sock, queue, thread_log_config);
					
					ClientContext_Hashtable_Set(client_context_hashtable, current->string_key, ctxt);	

					//start receive thread
					pthread_t receive_thread;
					Log_log(main_log_config, LOG_DEBUG, "start receive data thread %s\n", name);
					int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
						
					current = current->next;
				}//end while loop
			}//end if not null
		}//end for loop

		ClientContext* sending_ctxt = ClientContext_Session_Get(client_context_hashtable, sending_username);
		int bytes_sent = Client_Movement_Send(sending_ctxt, session_token, direction, speed, frames);
		
		int i = 0;
		for (i=0; i<string_hashtable_input->size; i++) {
			if(string_hashtable_input->table[i] != NULL) {
				String_List* list = (String_List*)string_hashtable_input->table[i];
				String_Node* current = list->head->next;
				unsigned char* data = NULL;
				while(current != NULL) {
					
					ClientContext* context = ClientContext_Hashtable_Get(client_context_hashtable, current->string_key);
						
					//empty receive queue
					Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
					pthread_mutex_lock(&context->mutex);
					while( (data = Queue_Front(context->queue)) == NULL) {
						pthread_cond_wait(&context->cond, &context->mutex);
					}
					Queue_Dequeue(context->queue);
					pthread_mutex_unlock(&context->mutex);
					Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
					
					String_Hashtable_Set(string_hashtable_output, current->string_key, (char*)data);
		
					pthread_mutex_lock(&context->stop_mutex);
					Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
					context->stop_thread = 1;
					while( context->stop_thread == 1) {
						Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", context->stop_thread);
						pthread_cond_wait(&ctxt->stop_cond, &context->stop_mutex);
						Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", context->stop_thread);
					}
					pthread_mutex_unlock(&context->stop_mutex);

					ClientContext_Destroy(context);
					
					current = current->next;
				} //end while
			}//end string hashtable input
		}//end for loop
	}//end if hashtable null	
}//end for loop

unsigned char* Logout(Socket* sock, char* session_token, 
		      LogConfig* main_log_config, LogConfig* thread_log_config) {

	
	Log_log(main_log_config, LOG_DEBUG, "sending logout\n");
	unsigned char* data = NULL;
	if(sock != NULL) {	
		Queue* queue = Queue_Create();
		char* name = "LOGOUT";
		ClientContext* ctxt = ClientContext_Create(name, sock, queue, thread_log_config);

		//start receive thread
		pthread_t receive_thread;
		Log_log(main_log_config, LOG_DEBUG, "start receive data thread\n");
		int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
		
		int bytes_sent = Client_Logout_Send(ctxt, session_token);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&ctxt->mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&ctxt->cond, &ctxt->mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&ctxt->mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Format_Get(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
		
		pthread_mutex_lock(&ctxt->stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&ctxt->stop_cond, &ctxt->stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&ctxt->stop_mutex);

		
		ClientContext_Destroy(ctxt);

	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}
	return data;

}

void Test_Register_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {

	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	Server_Run(ip_address, port);	

	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");

	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock);	

	Log_log(main_log_config, LOG_INFO, "registering\n");
	unsigned char* data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	printf("after\n");
	Log_log(main_log_config, LOG_DEBUG, "after\n");
	
	Log_log(main_log_config, LOG_DEBUG, "data result:%p\n", data);

	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1 register response passed\n");
	}
	
	sleep(3);
		
	Log_log(main_log_config, LOG_INFO, "registering again\n");
	data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	unsigned short error_code = 0;
	Protocol_Error_Response_Payload_Unpack(data+header_size, &error_code);
	Log_log(main_log_config, LOG_INFO, "error code: %i\n", error_code);
	if(cmd == CMD_ERROR && proto == PROTO_RESPONSE && error_code == ERR_REG) {
		Log_log(main_log_config, LOG_INFO, "test 2 register error response passed\n");
	}
	
	sleep(3);
		
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);
	
	free(data);
	Socket_Destroy(sock);

}

void Test_Login_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {
	
	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	Server_Run(ip_address, port);	

	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");

	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock);	
	
	Log_log(main_log_config, LOG_INFO, "logging in before registering\n");
	unsigned char* data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	unsigned short error_code = 0;
	Protocol_Error_Response_Payload_Unpack(data+header_size, &error_code);
	Log_log(main_log_config, LOG_INFO, "error code: %i\n", error_code);
	if(cmd == CMD_ERROR && proto == PROTO_RESPONSE && error_code == ERR_LOGIN_NOT_REG) {
		Log_log(main_log_config, LOG_INFO, "test 1 login error response passed\n");
	}
	
	sleep(3);

	Log_log(main_log_config, LOG_INFO, "registering\n");
	data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 register response passed\n");
	}
	sleep(3);
	
	Log_log(main_log_config, LOG_INFO, "logging in\n");
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 login response passed\n");
	}
	sleep(3);
	
	Log_log(main_log_config, LOG_INFO, "logging in again\n");
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	header_size = Binary_Calcsize(HEADER_FORMAT);
	Protocol_Error_Response_Payload_Unpack(data+header_size, &error_code);
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	Log_log(main_log_config, LOG_INFO, "error code: %i\n", error_code);
	if(cmd == CMD_ERROR && proto == PROTO_RESPONSE && error_code == ERR_LOGIN_AGAIN) {
		Log_log(main_log_config, LOG_INFO, "test 4 login error response passed\n");
	}
	
	sleep(3);
		
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);
	
	free(data);	
	Socket_Destroy(sock);

}

void Test_Multiple_Login_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {
	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	Server_Run(ip_address, port);	

	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");

	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock);	
	
	Log_log(main_log_config, LOG_INFO, "registering %s\n", username);
	unsigned char* data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test register %s response passed\n", username);
	}
	sleep(2);
	
	char username_two[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_two, "bobslob");
	char hidden_password_two[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("2boob2", hidden_password_two);
	char email_two[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_two, "bob@bob.com");
	
	Log_log(main_log_config, LOG_INFO, "registering %s\n", username_two);
	data = Register(sock, username_two, hidden_password_two, email_two, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test register %s response passed\n", username_two);
	}
	sleep(2);

	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username);
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test login response passed\n");
	}
	sleep(2);


	Socket* sock_two = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock_two);
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username_two);
	data = Login(sock_two, username_two, hidden_password_two, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test sock 2 login response passed\n");
	}
	char session_token[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token);
	
	Log_log(main_log_config, LOG_DEBUG, "unpacked session token: %s\n", session_token);	

	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	unsigned char* inrange_data = data + header_size;
		
	Log_log(main_log_config, LOG_DEBUG, "before\n");	
	Session_Hashtable* hash_table = Binary_To_Session_Hashtable(session_token, inrange_data);
	Log_log(main_log_config, LOG_DEBUG, "after\n");	
	Session* session_one = Session_Hashtable_Get(hash_table, username);
	Session* session_two = Session_Hashtable_Get(hash_table, username_two);
	
	if( strcmp(session_one->username, username) == 0 ) {
		Log_log(main_log_config, LOG_DEBUG, "session one username found\n");
	}

	if( session_two == NULL ) {
		Log_log(main_log_config, LOG_DEBUG, "session two is null\n");
	}
	
	Session_Hashtable_Destroy(hash_table);

	sleep(2);
		
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);
	
	free(data);	
	Socket_Destroy(sock);
	Socket_Destroy(sock_two);

}

void Test_Multiple_Ping_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {

	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	Server_Run(ip_address, port);	

	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");

	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock);	

	Log_log(main_log_config, LOG_INFO, "registering %s\n", username);
	unsigned char* data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1 register %s response passed\n", username);
	}
	sleep(2);
	
	char username_two[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_two, "bobslob");
	char hidden_password_two[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("2boob2", hidden_password_two);
	char email_two[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_two, "bob@bob.com");
	
	Log_log(main_log_config, LOG_INFO, "registering %s\n", username_two);
	data = Register(sock, username_two, hidden_password_two, email_two, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 register %s response passed\n", username_two);
	}
	sleep(2);
	
	char username_three[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_three, "jimbean");
	char hidden_password_three[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("licorize", hidden_password_three);
	char email_three[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_three, "jim@dookie.com");
	
	Log_log(main_log_config, LOG_INFO, "registering %s\n", username_three);
	data = Register(sock, username_three, hidden_password_three, email_three, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 register %s response passed\n", username_three);
	}
	sleep(2);
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username);
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1login response passed\n");
	}
	char session_token[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token);	
	sleep(2);


	Socket* sock_two = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock_two);
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username_two);
	data = Login(sock_two, username_two, hidden_password_two, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 sock 2 login response passed\n");
	}
	char session_token_two[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token_two);	
	
	Socket* sock_three = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock_three);
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username_three);
	data = Login(sock_three, username_three, hidden_password_three, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 sock 3 login response passed\n");
	}
	char session_token_three[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token_three);	

	sleep(2);
	
	int x = 100;
	int y = 200;
	data = Ping(sock, session_token, x, y, main_log_config, thread_log_config);  		
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_PING && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1 sock 1 ping response passed\n");
	}
	
	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	unsigned char* inrange_data = data + header_size;
	unsigned int inrange_body_size = 0;
	unsigned short inrange_num_items = 0;
	unsigned int inrange_header_size = sizeof(unsigned int) + sizeof(unsigned short);
	Binary_Unpack("!IH", inrange_data, &inrange_body_size, &inrange_num_items); 
	Session_Hashtable* inrange_session_hashtable = Binary_To_Session_Hashtable(session_token, inrange_data);
	Log_log(main_log_config, LOG_DEBUG, "inrange ping response count: %i\n", inrange_session_hashtable->count);

	unsigned char* outofrange_data = data + header_size + inrange_header_size + inrange_body_size;
	Session_Hashtable* outofrange_session_hashtable = Binary_To_Session_Hashtable(session_token, outofrange_data);
	Log_log(main_log_config, LOG_DEBUG, "outofrange ping response count: %i\n", outofrange_session_hashtable->count);

	sleep(2);
	
	int x_two = 1000;
	int y_two = 250;
	data = Ping(sock_two, session_token_two, x_two, y_two, main_log_config, thread_log_config);  		
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_PING && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 sock 2 ping response passed\n");
	}
	
	unsigned char* inrange_data_two = data + header_size;
	unsigned int inrange_body_size_two = 0;
	unsigned short inrange_num_items_two = 0;
	unsigned int inrange_header_size_two = sizeof(unsigned int) + sizeof(unsigned short);
	Binary_Unpack("!IH", inrange_data_two, &inrange_body_size_two, &inrange_num_items_two); 
	Session_Hashtable* inrange_session_hashtable_two = Binary_To_Session_Hashtable(session_token_two, inrange_data_two);
	Log_log(main_log_config, LOG_DEBUG, "inrange two ping response count: %i\n", inrange_session_hashtable_two->count);

	unsigned char* outofrange_data_two = data + header_size + inrange_header_size_two + inrange_body_size_two;
	Session_Hashtable* outofrange_session_hashtable_two = Binary_To_Session_Hashtable(session_token_two, outofrange_data_two);
	Log_log(main_log_config, LOG_DEBUG, "outofrange two ping response count: %i\n", outofrange_session_hashtable_two->count);

	sleep(2);
	
	int x_three = 250;
	int y_three = 150;
	data = Ping(sock_three, session_token_three, x_three, y_three, main_log_config, thread_log_config);  		
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_PING && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 sock 3 ping response passed\n");
	}
	
	unsigned char* inrange_data_three = data + header_size;
	unsigned int inrange_body_size_three = 0;
	unsigned short inrange_num_items_three = 0;
	unsigned int inrange_header_size_three = sizeof(unsigned int) + sizeof(unsigned short);
	Binary_Unpack("!IH", inrange_data_three, &inrange_body_size_three, &inrange_num_items_three); 
	Session_Hashtable* inrange_session_hashtable_three = Binary_To_Session_Hashtable(session_token_three, inrange_data_three);
	Log_log(main_log_config, LOG_DEBUG, "inrange three ping response count: %i\n", inrange_session_hashtable_three->count);

	unsigned char* outofrange_data_three = data + header_size + inrange_header_size_three + inrange_body_size_three;
	Session_Hashtable* outofrange_session_hashtable_three = Binary_To_Session_Hashtable(session_token_three, outofrange_data_three);
	Log_log(main_log_config, LOG_DEBUG, "outofrange three ping response count: %i\n", outofrange_session_hashtable_three->count);
	
	sleep(2);
	
	int x_four = 220;
	int y_four = 100;
	data = Ping(sock_three, session_token_three, x_four, y_four, main_log_config, thread_log_config);  		
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_PING && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 4 sock 3 ping response passed\n");
	}
	
	unsigned char* inrange_data_four = data + header_size;
	unsigned int inrange_body_size_four = 0;
	unsigned short inrange_num_items_four = 0;
	unsigned int inrange_header_size_four = sizeof(unsigned int) + sizeof(unsigned short);
	Binary_Unpack("!IH", inrange_data_four, &inrange_body_size_four, &inrange_num_items_four); 
	Session_Hashtable* inrange_session_hashtable_four = Binary_To_Session_Hashtable(session_token_three, inrange_data_four);
	Log_log(main_log_config, LOG_DEBUG, "inrange four ping response count: %i\n", inrange_session_hashtable_four->count);

	unsigned char* outofrange_data_four = data + header_size + inrange_header_size_four + inrange_body_size_four;
	Session_Hashtable* outofrange_session_hashtable_four = Binary_To_Session_Hashtable(session_token_three, outofrange_data_four);
	Log_log(main_log_config, LOG_DEBUG, "outofrange four ping response count: %i\n", outofrange_session_hashtable_four->count);
	
	sleep(2);
	
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);
	
	free(data);
	Socket_Destroy(sock);
	Socket_Destroy(sock_two);
	Socket_Destroy(sock_three);
}


void Test_Movement_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {
		
	Socket_Hashtable* socket_hashtable = Socket_Hashtable_Create(20);
	String_Hashtable* string_hashtable_input = String_Hashtable_Create(20);
	String_Hashtable* string_hashtable_output = String_Hashtable_Create(20);	

	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	Server_Run(ip_address, port);	

	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");

	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock);	
	
	unsigned int sock_id_length = Util_Count_Digits(sock->id);
	char* sock_id_str = (char*)malloc(sock_id_length + 1);
	int bytes = sprintf(sock_id_str, "%i", sock->id);
	String_Hashtable_Set(string_hashtable_input, username, sock_id_str);	
	Socket_Hashtable_Set(socket_hashtable, sock->id, sock);	

	Log_log(main_log_config, LOG_INFO, "registering %s\n", username);
	unsigned char* data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1 register %s response passed\n", username);
	}
	sleep(2);
	
	char username_two[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_two, "bobslob");
	char hidden_password_two[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("2boob2", hidden_password_two);
	char email_two[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_two, "bob@bob.com");
	
	Log_log(main_log_config, LOG_INFO, "registering %s\n", username_two);
	data = Register(sock, username_two, hidden_password_two, email_two, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 register %s response passed\n", username_two);
	}
	sleep(2);
	
	char username_three[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_three, "jimbean");
	char hidden_password_three[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("licorize", hidden_password_three);
	char email_three[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_three, "jim@dookie.com");
	
	Log_log(main_log_config, LOG_INFO, "registering %s\n", username_three);
	data = Register(sock, username_three, hidden_password_three, email_three, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 register %s response passed\n", username_three);
	}
	sleep(2);
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username);
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1login response passed\n");
	}
	char session_token[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token);	
	sleep(2);


	Socket* sock_two = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock_two);
	unsigned int sock_id_length_two = Util_Count_Digits(sock_two->id);
	char* sock_id_str_two = (char*)malloc(sock_id_length_two + 1);
	int bytes = sprintf(sock_id_str_two, "%i", sock_two->id);
	String_Hashtable_Set(string_hashtable_input, username_two, sock_id_str_two);	
	Socket_Hashtable_Set(socket_hashtable, sock_two->id, sock_two);	
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username_two);
	data = Login(sock_two, username_two, hidden_password_two, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 sock 2 login response passed\n");
	}
	char session_token_two[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token_two);	
	
	Socket* sock_three = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock_three);
	unsigned int sock_id_length_three = Util_Count_Digits(sock_three->id);
	char* sock_id_str_three = (char*)malloc(sock_id_length_three + 1);
	int bytes = sprintf(sock_id_str_three, "%i", sock_three->id);
	String_Hashtable_Set(string_hashtable_input, username_three, sock_id_str_three);	
	Socket_Hashtable_Set(socket_hashtable, sock_three->id, sock_three);	
	
	Log_log(main_log_config, LOG_INFO, "%s logging in\n", username_three);
	data = Login(sock_three, username_three, hidden_password_three, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 sock 3 login response passed\n");
	}
	char session_token_three[SESSION_LENGTH+1] = {'\0'};
	Protocol_Session_Unpack(data, session_token_three);	

	sleep(2);
			
	unsigned short direction = 7;
	unsigned short speed = 3;
	unsigned short frames = 10;
		
	Movement_Broadcast(socket_hashtable_input, string_hashtable_input,
			   username,	
			   session_token, direction, speed, frames,
			   string_hashtable_output,
			   main_log_config, thread_log_config);
	
	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	unsigned char* data_one = (unsigned char*)String_Hashtable_Get(string_hashtable_output, username);	
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data_one, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_MOVEMENT && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1 movement response passed\n");
	}
	
	unsigned char* data_two = (unsigned char*)String_Hashtable_Get(string_hashtable_output, username_two);	
	Protocol_Header_Unpack(data_two, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_MOVEMENT && proto == PROTO_BROADCAST) {
		Log_log(main_log_config, LOG_INFO, "test 2 movement broadcast passed\n");
		unsigned short direction_output_two = 0;
		unsigned short speed_output_two = 0;
		unsigned short frames_output_two = 0;
		Protocol_Movement_Send_Payload_Unpack(data_two + header_size, &direction_output_two, &speed_output_two, &frames_output_two) {
		if(direction == direction_output_two) {
			Log_log(main_log_config, LOG_DEBUG, "test 2 direction result passed\n");
		}
		if(speed == speed_output_two) {
			Log_log(main_log_config, LOG_DEBUG, "test 2 speed result passed\n");
		}
		if(frames == frames_output_two) {
			Log_log(main_log_config, LOG_DEBUG, "test 2 frames result passed\n");
		}
	}
	
	unsigned char* data_three = (unsigned char*)String_Hashtable_Get(string_hashtable_output, username_three);	
	Protocol_Header_Unpack(data_three, &cmd, &proto, &payload_size);	
	Log_log(main_log_config, LOG_DEBUG, "payload size: %i\n", payload_size);
	if(cmd == CMD_MOVEMENT && proto == PROTO_BROADCAST) {
		Log_log(main_log_config, LOG_INFO, "test 3 movement broadcast passed\n");
		unsigned short direction_output_three = 0;
		unsigned short speed_output_three = 0;
		unsigned short frames_output_three = 0;
		Protocol_Movement_Send_Payload_Unpack(data_three + header_size, &direction_output_three, &speed_output_three, &frames_output_three) {
		if(direction == direction_output_three) {
			Log_log(main_log_config, LOG_DEBUG, "test 3 direction result passed\n");
		}
		if(speed == speed_output_three) {
			Log_log(main_log_config, LOG_DEBUG, "test 3 speed result passed\n");
		}
		if(frames == frames_output_three) {
			Log_log(main_log_config, LOG_DEBUG, "test 3 frames result passed\n");
		}
	}
	
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);

	Socket_Hashtable_Destroy(socket_hashtable);
	String_Hashtable_Destroy(string_hashtable_input);
	String_Hashtable_Destroy(string_hashtable_output);
	
	free(sock_id_str);
	free(sock_id_str_two);
	free(sock_id_str_three);

	Socket_Destroy(sock);
	Socket_Destroy(sock_two);
	Socket_Destroy(sock_three);
	
}

/**

void Test_Logout_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {

	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	Server_Run(ip_address, port);	

	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");

	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	Socket_Make_Nonblocking(sock);	
	
	Log_log(main_log_config, LOG_INFO, "registering\n");
	unsigned char* data = Register(sock, username, hidden_password, email, main_log_config, thread_log_config);
	
	unsigned short cmd = 0;
	unsigned short proto = 0;
	unsigned int payload_size = 0;	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 1 register response passed\n");
	}
	sleep(3);
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////	
	char session_token[SESSION_LENGTH+1] = {'\0'};
	memset(session_token, '0', SESSION_LENGTH+1);	
	Log_log(main_log_config, LOG_INFO, "try logout with no session...not logged in\n");
	data = Logout(sock, session_token, main_log_config, thread_log_config);
		
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	unsigned int header_size = Binary_Calcsize(HEADER_FORMAT);
	unsigned short error_code = 0;
	Protocol_Error_Response_Payload_Unpack(data+header_size, &error_code);
	Log_log(main_log_config, LOG_INFO, "error code: %i\n", error_code);
	if(cmd == CMD_ERROR && proto == PROTO_RESPONSE && error_code == ERR_SESSION_NO_EXIST) {
		Log_log(main_log_config, LOG_INFO, "test 2 logout error response passed\n");
	}

	sleep(3);
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Log_log(main_log_config, LOG_INFO, "logging in\n");
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);
	Protocol_Session_Unpack(data, session_token);	
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 login response passed\n");
	}
	sleep(3);
	
	data = Logout(sock, session_token, main_log_config, thread_log_config);
		
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	if(cmd == CMD_LOGOUT && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 4 logout response passed\n");
	}

	sleep(3);
	
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);
	
	Socket_Destroy(sock);

}
**/

int main(int argc, char* argv[]) {	
	
	LogConfig* main_log_config = LogConfig_Create("log_main.txt", 
							 LOG_ON,
							 LOG_FILE,
							 LOG_DEBUG,
							 1000000, 
							 5);

	LogConfig* thread_log_config = LogConfig_Create("log_thread.txt", 
							 LOG_ON,
							 LOG_FILE,
							 LOG_DEBUG,
							 1000000, 
							 5);
	char* ip_address = "192.168.0.2";
	char* port = "57132";
	//Test_Register_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Login_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Multiple_Login_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Multiple_Ping_Send_Response(ip_address, port, main_log_config, thread_log_config);
	Test_Movement_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Logout_Send_Response(ip_address, port, main_log_config, thread_log_config);
		
	return 0;
}
