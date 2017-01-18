#ifndef _ARGUMENTS_H_
#define _ARGUMENTS_H_

#include <sys/socket.h>
#include <netdb.h>

#define DATABASE_NAME_LEN 128
#define LOGFILE_NAME_LEN 128

void Args_Print_Usage();

void Args_Get_Input(int argc, char* argv[], char ip_address[NI_MAXHOST+1], char port[NI_MAXSERV+1], 
		    char db_name[DATABASE_NAME_LEN+1], char logfile_name[LOGFILE_NAME_LEN+1], int* empty_db);


#endif
