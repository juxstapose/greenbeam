#include "config.h"
#include "string_hashtable.h"

int main(int argc, char* argv[]) {
	
	LogConfig* log_config = LogConfig_Create("log.txt", 
						 LOG_ON,
						 LOG_FILE,
						 LOG_DEBUG,
						 100000000, 
						 5);
	
	Config* config = Config_Create();
	int r = Config_Read_File(config, "config.ini", log_config);
	String_Hashtable_Print(config->string_hashtable);
	Config_Destroy(config);

	return 0;
}
