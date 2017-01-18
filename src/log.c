#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include "util.h"
#include "log.h"

LogConfig* LogConfig_Create(char* filename, unsigned short log_switch, unsigned char output_options, int show_log_level, int max_file_size, int file_backlog) {
	//printf("filename: %s\n", filename);
	LogConfig* config = (LogConfig*)malloc(sizeof(LogConfig));
	config->log_switch = log_switch;
	if(filename == NULL && output_options & LOG_FILE) {
		output_options = LOG_CONSOLE;
	}
	else if(filename != NULL && output_options & LOG_FILE) {
		config->current_file = fopen(filename, "a");
		if(config->current_file == NULL) {
			fprintf(stderr,"Error opening file: %s\n", strerror(errno));
			exit(1);
		}
	} else {
		config->current_file = NULL;
	}
	
	if(filename != NULL) {
		strcpy(config->filename, filename);
	}
	/**
	 * this log level shows all messages and above so debug will show all msgs
	 * info will show info,warn and error
	 * warn will show warn and error
	 * error will just show error
	 **/
	config->show_log_level = show_log_level;
	/**
	 * if output type is file
	 * it will create a new file when this file size is reached
	 **/
	config->max_file_size = max_file_size;
	/**
	 * current file number in rotation
	 * */
	config->current_file_no = 0;
	/**
	 * this determines how many new files it creates before it starts over
	 **/
	config->file_backlog = file_backlog;
	
	config->output_options = output_options;

	return config;

}

void Log_log(LogConfig* config, int log_level, const char* format, ...) {
	if(config->log_switch == LOG_ON) {
		va_list list;
		va_start(list, format);
		//determine text for log level
		//get time stamp
		//combine log level time stamp and message to output
		char* ll = Log_Level_String(log_level);
		char* ts = Util_Formatted_Timestamp();	
		char* log_msg = (char*)malloc(LOG_MSG_BUFFER_SIZE);
		int bytes = sprintf(log_msg, "%s %s %s\n", ll, ts, format);
		//printf("filename: %s bytes: %i\n", config->filename, bytes);	
		if(config->output_options & LOG_CONSOLE) {
			if(log_level >= config->show_log_level) {
				//fprintf(stdout, "%s",log_msg);
				vfprintf(stdout, format, list);
			}
		}	
		if(config->output_options & LOG_FILE) {
			if(log_level >= config->show_log_level) {
				//check to see if size of file exceeds max
				fseek(config->current_file, 0L, SEEK_END);
				int size = ftell(config->current_file);
				fseek(config->current_file, 0L, SEEK_SET);
				//printf("%i >= %i\n", size, config->max_file_size);
				if(size >= config->max_file_size) {
					config->current_file_no++;	
					config->current_file_no = config->current_file_no % config->file_backlog;
					//printf("current no:%i\n", config->current_file_no);
					//printf("config->filename %s\n", config->filename);
					char* config_filename = (char*)malloc(sizeof(config->filename));
					memcpy(config_filename, config->filename, sizeof(config->filename));
					char* basename = strtok(config_filename, ".");
					//printf("basename: %s\n", basename);
					char* ext = strtok(NULL, ".");
					//printf("ext: %s\n", ext);
					int current_file_no_length = Util_Count_Digits(config->current_file_no);
					//printf("current_file_no_length:%i\n", current_file_no_length);
					int filename_length = strlen(basename) + strlen(ext) + 1 + current_file_no_length; 
					//printf("filename_length:%i\n", filename_length);	
					char* filename = (char*)malloc(filename_length);
					if(config->current_file_no > 0) {
						sprintf(filename, "%s%i.%s", basename, config->current_file_no, ext);
					}else {
						sprintf(filename, "%s.%s", basename, ext);
					}

					//printf("new filename %s", filename);
					fclose(config->current_file);	
					//wipe file if it already exists
					FILE* temp = fopen(filename, "w");
					if(temp == NULL) {
						fprintf(stderr,"Error opening file: %s\n", strerror(errno));
						exit(1);
					}
					fclose(temp);
					config->current_file = fopen(filename, "a");
					if(config->current_file == NULL) {
						fprintf(stderr,"Error opening file: %s\n", strerror(errno));
						exit(1);
					}
					free(config_filename);
					free(filename);	
				}//end max file size
				
				char buffer[LOG_MSG_BUFFER_SIZE] = {'\0'};
				char* ts = Util_Formatted_Timestamp();
				char* ll = Log_Level_String(log_level);
				char* modified_format = (char*)malloc(strlen(format) + 1 + strlen(ts) + 1 + strlen(ll) + 1 + 4);
				int bytes = sprintf(modified_format, "[%s] %s %s", ts, ll, format);
				vsprintf(buffer, modified_format, list);
				//printf("%i\n", strlen(buffer));
				int num = fwrite(buffer, sizeof(char), strlen(buffer), config->current_file);
				free(modified_format);
				free(ts);
				fflush(config->current_file);
				//printf("num: %i\n", num);
			
			}//end show log level	
		}//end if log file
		va_end(list);
		free(log_msg);	
		free(ts);
		free(ll);
	}//end log on
	
}

char* Log_Level_String(int log_level) {
	char* result;	
	if(log_level == LOG_DEBUG) {
		result = (char*)malloc(6);
		sprintf(result, "%s", "DEBUG");
	}
	if(log_level == LOG_INFO) {
		result = (char*)malloc(5);
		sprintf(result, "%s", "INFO");
	}
	if(log_level == LOG_WARN) {
		result = (char*)malloc(5);
		sprintf(result, "%s", "WARN");
	}
	if(log_level == LOG_ERROR) {
		result = (char*)malloc(6);
		sprintf(result, "%s", "ERROR");
	}
	return result;
}


void LogConfig_Destroy(LogConfig* config) {
	if(config->current_file != NULL) {
		fclose(config->current_file);
		config->current_file = NULL;
	}
	free(config);
}

