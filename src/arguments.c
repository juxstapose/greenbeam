#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netdb.h>
#include <getopt.h>
#include <string.h>
#include "arguments.h"


void Args_Print_Usage() {
	printf("Usage: server --ip_address --port --db_name --log_file [--empty_db]\n");
}	

void Args_Get_Input(int argc, char* argv[], char ip_address[NI_MAXHOST+1], char port[NI_MAXSERV+1], 
		    char db_name[DATABASE_NAME_LEN+1], char logfile_name[LOGFILE_NAME_LEN+1], int* empty_db) {
	static struct option long_options[] = {
		{"ip_address", required_argument, 0, 'i'},
		{"port",       required_argument, 0, 'p'},
		{"db_name",    required_argument, 0, 'd'},
		{"log_file",   required_argument, 0, 'l'},
		{"empty_db",   no_argument, 	  0, 'e'}
	};
	int opt = 0;
	int long_index = 0;
	*empty_db = 0;
	while( (opt = getopt_long(argc, argv, "i:p:d:l:r", long_options, &long_index)) != -1) {
		//printf("%c: %s\n", opt, optarg);
		switch(opt) {
			case 'i':
				strcpy(ip_address, optarg);
				break;
			case 'p':
				strcpy(port, optarg);
				break;
			case 'd':
				strcpy(db_name, optarg);
				break;
			case 'l':
				strcpy(logfile_name, optarg);
				break;
			case 'e':
				*empty_db = 1;		
				break;
			default:
				Args_Print_Usage();
				exit(1);
		}//end switch
	}//end while
	if(strlen(ip_address) == 0) {
		strcpy(ip_address, "127.0.0.1");
	}
	if(strlen(port) == 0) {
		strcpy(port, "57000");
	}
	if(strlen(db_name) == 0) {
		strcpy(db_name, "test.db");
	}
	if(strlen(logfile_name) == 0) {
		strcpy(logfile_name, "log.txt");
	}
}//end func



