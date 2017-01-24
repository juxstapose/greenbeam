#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "log.h"
#include "util.h"
#include "string_hashtable.h"
#include "config.h"

Config* Config_Create() {
	Config* config = (Config*)malloc(sizeof(Config));
	config->string_hashtable = String_Hashtable_Create(10);
	return config;
}

int Config_Read_File(Config* config, char* filename, LogConfig* log_config) {
	
	FILE* fp = fopen(filename, "r");
	if(fp == NULL) {
		Log_log(log_config, LOG_ERROR, "error opening file: %s\n", strerror(errno));
		return -1;	
	}
	
	char line[MAX_LINE_LENGTH] = {'\0'};
	while(fgets(line, sizeof(line), fp)) {
		Util_Remove_All_Chars(line, '\n');
		Util_Remove_All_Chars(line, '\r');
		Util_Remove_All_Chars(line, ' ');
		Log_log(log_config, LOG_DEBUG, "line: %s\n", line);
		char* key = strtok(line, "=");
		char* value = strtok(NULL, "=");
		//printf("key: %s\n", key);
		//printf("value: %s\n", value);
		String_Hashtable_Set(config->string_hashtable, key, value);
	}

	return 1;
}	

void Config_Destroy(Config* config) {
	if(config != NULL) {
		free(config);
	}
}
