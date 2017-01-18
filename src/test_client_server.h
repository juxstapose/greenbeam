#ifndef _TEST_CLIENT_SERVER_H_
#define _TEST_CLIENT_SERVER_H_

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t cond = PTHREAD_COND_INITIALIZER;

void* receive_handler(void* arg);
void Server_Run(char* ip_address, char* port); 
unsigned char* Register(Socket* sock, char* username, char* password, char* email, 
		        LogConfig* main_log_config, LogConfig* thread_log_config);
void Test_Register_Send_Response(char* ip_address, char* port, LogConfig* main_log_config, LogConfig* thread_log_config);

#endif
