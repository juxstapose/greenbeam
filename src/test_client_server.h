#ifndef _TEST_CLIENT_SERVER_H_
#define _TEST_CLIENT_SERVER_H_

//#include <pthread.h>

//static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

//static pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;
//static pthread_cond_t stop_cond = PTHREAD_COND_INITIALIZER;

void* receive_handler(void* arg);
void Server_Run(char* ip_address, char* port); 

unsigned char* Shutdown(Socket* sock, LogConfig* main_log_config, LogConfig* thread_log_config);
unsigned char* Register(Socket* sock, char* username, char* password, char* email, 
		        LogConfig* main_log_config, LogConfig* thread_log_config);
unsigned char* Login(Socket* sock, char* username, char* password, 
		     LogConfig* main_log_config, LogConfig* thread_log_config);
unsigned char* Logout(Socket* sock, char* session_token, 
		      LogConfig* main_log_config, LogConfig* thread_log_config);


void Test_Register_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config);
void Test_Login_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config);
void Test_Logout_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config);

#endif
