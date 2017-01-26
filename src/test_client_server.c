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
					pthread_mutex_lock(&mutex);
					Queue_Enqueue(ctxt->queue, data);		
					pthread_cond_signal(&cond);
					pthread_mutex_unlock(&mutex);

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
	pthread_mutex_lock(&stop_mutex);
	ctxt->stop_thread = 0;
	pthread_cond_signal(&stop_cond);
	pthread_mutex_unlock(&stop_mutex);
	return NULL;
}//end handler

void Server_Run(char* ip_address, char* port) { 

	char* args[8] = {'\0'};
	args[0] = "./server";
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
		pthread_mutex_lock(&mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
	
		pthread_mutex_lock(&stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&stop_cond, &stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&stop_mutex);
		
		
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
		pthread_mutex_lock(&mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
		
		pthread_mutex_lock(&stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&stop_cond, &stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&stop_mutex);

		ClientContext_Destroy(ctxt);

	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}
	return data;

}

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
		pthread_mutex_lock(&mutex);
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
				
			free(format);
			free(string_data);	
		}//end if data is null
		
		pthread_mutex_lock(&stop_mutex);
		Log_log(main_log_config, LOG_DEBUG, "stopping thread....\n");
		ctxt->stop_thread = 1;
		while( ctxt->stop_thread == 1) {
			Log_log(main_log_config, LOG_DEBUG, "before cond wait: %i\n", ctxt->stop_thread);
			pthread_cond_wait(&stop_cond, &stop_mutex);
			Log_log(main_log_config, LOG_DEBUG, "after cond wait: %i\n", ctxt->stop_thread);
		}
		pthread_mutex_unlock(&stop_mutex);

		
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
	if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 2 register response passed\n");
	}
	sleep(3);
	
	Log_log(main_log_config, LOG_INFO, "logging in\n");
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
		Log_log(main_log_config, LOG_INFO, "test 3 login response passed\n");
	}
	sleep(3);
	
	Log_log(main_log_config, LOG_INFO, "logging in again\n");
	data = Login(sock, username, hidden_password, main_log_config, thread_log_config);
	
	Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
	header_size = Binary_Calcsize(HEADER_FORMAT);
	Protocol_Error_Response_Payload_Unpack(data+header_size, &error_code);
	Log_log(main_log_config, LOG_INFO, "error code: %i\n", error_code);
	if(cmd == CMD_ERROR && proto == PROTO_RESPONSE && error_code == ERR_LOGIN_AGAIN) {
		Log_log(main_log_config, LOG_INFO, "test 4 login error response passed\n");
	}
	
	sleep(3);
		
	Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
	Shutdown(sock, main_log_config, thread_log_config);
	
	Socket_Destroy(sock);

}

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

/**
void Test_Logout_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {

	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	char* args[8] = {'\0'};
	args[0] = "./server";
	args[1] = "--ip_address";
	args[2] = ip_address;
	args[3] = "--port";
	args[4] = port;
	args[5] = "--empty_db";
	args[6] = "&";
	args[7] = NULL;
	
	Process_Run(args);
	
	sleep(1);	
	
	char username[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username, "juxstapose");
	char hidden_password[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password);
	char email[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email, "jimmy@thisisjimmy.com");
	
	Socket* sock = Client_Connect(ip_address, port, main_log_config);
	int status = Socket_Make_Nonblocking(sock);	
	
	if(sock != NULL) {	
		Queue* queue = Queue_Create();
		ClientContext* ctxt = ClientContext_Create(sock, queue, thread_log_config);

		//start receive thread
		pthread_t receive_thread;
		Log_log(main_log_config, LOG_DEBUG, "start receive data thread\n");
		int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
		
		int bytes_sent = Client_Register_Send(ctxt, username, hidden_password, email);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&mutex);
		unsigned char* data = NULL;
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
			
			unsigned short cmd = 0;
			unsigned short proto = 0;
			unsigned int payload_size = 0;	
			Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
			if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
				Log_log(main_log_config, LOG_INFO, "test 1 register response passed\n");
			}
			
			free(format);
			free(string_data);	
			free(data);
		}
		
		sleep(1);
	
		int login_bytes_sent = Client_Login_Send(ctxt, username, hidden_password);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&mutex);
		data = NULL;
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		char* session_token = NULL;

		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
			
			unsigned short cmd = 0;
			unsigned short proto = 0;
			unsigned int payload_size = 0;	
			Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
			if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
				Log_log(main_log_config, LOG_INFO, "test 2 login response passed\n");
			}
			
			session_token = (char*)malloc(SESSION_LENGTH+1);	
			Protocol_Session_Unpack(data, session_token);	
					
			free(format);
			free(string_data);	
			free(data);
		}
		
		sleep(1);
			
		if(session_token != NULL) {
			Log_log(main_log_config, LOG_DEBUG, "logging out with session %s\n", session_token);
			
			int logout_bytes_sent = Client_Logout_Send(ctxt, session_token);
			
			//empty receive queue
			Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
			pthread_mutex_lock(&mutex);
			data = NULL;
			while( (data = Queue_Front(ctxt->queue)) == NULL) {
				pthread_cond_wait(&cond, &mutex);
			}
			Queue_Dequeue(ctxt->queue);
			pthread_mutex_unlock(&mutex);
			Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
			
			if(data != NULL) {
				char* format = Protocol_Get_Format(data);
				Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
				unsigned char* string_data = Binary_String(format, data);
				Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
				//passes test if successfully registered
				
				unsigned short cmd = 0;
				unsigned short proto = 0;
				unsigned int payload_size = 0;	
				Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);
				char* unpacked_session = (char*)malloc(SESSION_LENGTH+1);	
				Protocol_Session_Unpack(data, unpacked_session);
				if(cmd == CMD_LOGOUT && proto == PROTO_RESPONSE && strcmp(session_token, unpacked_session) == 0) {
					Log_log(main_log_config, LOG_INFO, "test 3 logout response passed\n");
				}
				
				free(unpacked_session);
				free(format);
				free(string_data);	
				free(data);
			}
			
			free(session_token);
		}//end if session token
		
		sleep(1);

		Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
		int sd_bytes_sent = Client_Shutdown_Send(ctxt);
		
		sleep(2);

		Socket_Destroy(sock);
		ClientContext_Destroy(ctxt);
		pthread_cancel(receive_thread);
	} else { //end if client connected
		Log_log(main_log_config, LOG_ERROR, "client couldn't connect to server\n");
	}

}

void Test_Ping_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config) {
	
	Log_log(main_log_config, LOG_DEBUG, "starting server in the background\n");
	char* args[8] = {'\0'};
	args[0] = "./server";
	args[1] = "--ip_address";
	args[2] = ip_address;
	args[3] = "--port";
	args[4] = port;
	args[5] = "--empty_db";
	args[6] = "&";
	args[7] = NULL;
	
	Process_Run(args);
	
	sleep(1);	
	
	char username_one[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_one, "juxstapose");
	char hidden_password_one[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("INioh2", hidden_password_one);
	char email_one[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_one, "jimmy@thisisjimmy.com");

	//connect socket 1
	//register
	//login and get session
	//send ping
	//
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//socket one test register
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	Socket* sock_one = Client_Connect(ip_address, port, main_log_config);
	int status = Socket_Make_Nonblocking(sock_one);	
	if(sock_one != NULL) {
		Log_log(main_log_config, LOG_DEBUG, "sock one connected with id %i\n", sock_one->id);
		Queue* queue = Queue_Create();
		ClientContext* ctxt = ClientContext_Create(sock_one, queue, thread_log_config);
		
		//start receive thread
		pthread_t receive_thread;
		Log_log(main_log_config, LOG_DEBUG, "start receive data thread\n");
		int receive_thread_ret = pthread_create(&receive_thread, NULL, receive_handler, (void*)ctxt);
		
		int bytes_sent = Client_Register_Send(ctxt, username_one, hidden_password_one, email_one);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&mutex);
		unsigned char* data = NULL;
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
			
			unsigned short cmd = 0;
			unsigned short proto = 0;
			unsigned int payload_size = 0;	
			Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
			if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
				Log_log(main_log_config, LOG_INFO, "test 1 register response passed\n");
			}
			
			free(format);
			free(string_data);	
			free(data);
		}
		
		sleep(1);
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//socket one test login
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		int login_bytes_sent = Client_Login_Send(ctxt, username_one, hidden_password_one);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&mutex);
		data = NULL;
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		char* session_token = NULL;

		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
			
			unsigned short cmd = 0;
			unsigned short proto = 0;
			unsigned int payload_size = 0;	
			Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
			if(cmd == CMD_LOGIN && proto == PROTO_RESPONSE) {
				Log_log(main_log_config, LOG_INFO, "test 2 login response passed\n");
			}
			
			session_token = (char*)malloc(SESSION_LENGTH+1);	
			Protocol_Session_Unpack(data, session_token);	
					
			free(format);
			free(string_data);	
			free(data);
		}
		
		//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//socket one test ping
		/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		
		if(session_token != NULL) {
			int current_pos_x = 4;
			int current_pos_y = 3;	
			int ping_bytes_sent = Client_Ping_Send(ctxt, session_token, current_pos_x, current_pos_y);
		
			
			//empty receive queue
			Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
			pthread_mutex_lock(&mutex);
			data = NULL;
			while( (data = Queue_Front(ctxt->queue)) == NULL) {
				pthread_cond_wait(&cond, &mutex);
			}
			
			Queue_Dequeue(ctxt->queue);
			pthread_mutex_unlock(&mutex);
			Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
			
			char* session_token = NULL;

			if(data != NULL) {
				char* format = Protocol_Get_Format(data);
				Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
				unsigned char* string_data = Binary_String(format, data);
				Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
				//passes test if successfully registered
				
				unsigned short cmd = 0;
				unsigned short proto = 0;
				unsigned int payload_size = 0;	
				Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
				if(cmd == CMD_PING && proto == PROTO_RESPONSE) {
					Log_log(main_log_config, LOG_INFO, "test 3 ping response passed\n");
				}
				
				session_token = (char*)malloc(SESSION_LENGTH+1);	
				Protocol_Session_Unpack(data, session_token);	
						
				free(format);
				free(string_data);	
				free(data);
			}
		
		
		}

	}	
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	//socket two test register
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	
	sleep(2);	
	
	char username_two[USERNAME_LENGTH+1] = {'\0'};
	strcpy(username_two, "bob");
	char hidden_password_two[PASSWORD_LENGTH+1] = {'\0'};
	Util_Sha256("mypassword123", hidden_password_two);
	char email_two[EMAIL_LENGTH+1] = {'\0'};
	strcpy(email_two, "bob@bobs.com");

	Socket* sock_two = Client_Connect(ip_address, port, main_log_config);
	status = Socket_Make_Nonblocking(sock_two);	
	if(sock_two != NULL) {
		Log_log(main_log_config, LOG_DEBUG, "sock two connected with id %i\n", sock_two->id);
		
		Queue* queue = Queue_Create();
		ClientContext* ctxt = ClientContext_Create(sock_two, queue, thread_log_config);
		
		int bytes_sent = Client_Register_Send(ctxt, username_two, hidden_password_two, email_two);
		
		//empty receive queue
		Log_log(main_log_config, LOG_DEBUG, "peek at queue for data\n");
		pthread_mutex_lock(&mutex);
		unsigned char* data = NULL;
		while( (data = Queue_Front(ctxt->queue)) == NULL) {
			pthread_cond_wait(&cond, &mutex);
		}
		Queue_Dequeue(ctxt->queue);
		pthread_mutex_unlock(&mutex);
		Log_log(main_log_config, LOG_DEBUG, "dequeued data from queue\n");
		
		if(data != NULL) {
			char* format = Protocol_Get_Format(data);
			Log_log(main_log_config, LOG_DEBUG, "format: %s\n", format);
			unsigned char* string_data = Binary_String(format, data);
			Log_log(main_log_config, LOG_INFO, "data received: %s\n", string_data); 		
			//passes test if successfully registered
			
			unsigned short cmd = 0;
			unsigned short proto = 0;
			unsigned int payload_size = 0;	
			Protocol_Header_Unpack(data, &cmd, &proto, &payload_size);	
			if(cmd == CMD_REGISTER && proto == PROTO_RESPONSE) {
				Log_log(main_log_config, LOG_INFO, "socket 2 test 1 register response passed\n");
			}
			
			free(format);
			free(string_data);	
			free(data);
		}
		
		sleep(1);
		
		Log_log(main_log_config, LOG_INFO, "sending shutdown cmd\n");
		int sd_bytes_sent = Client_Shutdown_Send(ctxt);
		
		ClientContext_Destroy(ctxt);
	}	
		
		
	sleep(2);

	Socket_Destroy(sock_one);
	Socket_Destroy(sock_two);

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
	Test_Register_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Login_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Logout_Send_Response(ip_address, port, main_log_config, thread_log_config);
	//Test_Ping_Send_Response(ip_address, port, main_log_config, thread_log_config);
		
	return 0;
}
