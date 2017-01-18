#ifndef _LOG_H_
#define _LOG_H_

#define LOG_MSG_BUFFER_SIZE 4096

#define LOG_DEBUG 0
#define LOG_INFO 1
#define LOG_WARN 2
#define LOG_ERROR 3

#define LOG_CONSOLE 0x01
#define LOG_FILE 0x02

#define LOG_ON 0
#define LOG_OFF 1

#include <stdio.h>

#define LOG_FILENAME_MAX_LEN 4096

typedef struct LogConfig {
		
	char filename[LOG_FILENAME_MAX_LEN];
	
	unsigned short log_switch;
	
	FILE* current_file;
	/**
	 * set file or console output type
	 **/
	int output_type;

	/**
	 * file or console type options
	 **/
	unsigned char output_options;
	/**
	 * this log level shows all messages and above so debug will show all msgs
	 * info will show info,warn and error
	 * warn will show warn and error
	 * error will just show error
	 **/
	int show_log_level;	
	/**
	 * if output type is file
	 * it will create a new file when this file size is reached
	 **/
	int max_file_size;
	/**
	 * current file number in rotation
	 * */
	int current_file_no;
	/**
	 * this determines how many new files it creates before it starts over
	 **/
	int file_backlog;	
} LogConfig;


LogConfig* LogConfig_Create(char* filename, unsigned short log_switch, unsigned char output_options, int show_log_level, int max_file_size, int file_backlog);
void Log_log(LogConfig* config, int log_level, const char* format, ...);
void LogConfig_Destroy(LogConfig* config);
char* Log_Level_String(int log_level);
char* Log_Timestamp();

#endif
