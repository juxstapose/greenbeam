#ifndef _CONFIG_H_
#define _CONFIG_H_

#include "string_hashtable.h"
#include "log.h"

#define MAX_LINE_LENGTH 2048


typedef struct Config {
	String_Hashtable* string_hashtable;
} Config;

Config* Config_Create();
int Config_Read_File(Config* config, char* filename, LogConfig* log_config);
void Config_Destroy(Config* config);

#endif
