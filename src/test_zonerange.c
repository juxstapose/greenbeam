#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "config.h"
#include "location.h"
#include "log.h"
#include "zonerange.h"

#ifdef main
#undef main
#endif



int main(int argc, char* argv[]) {
	

	LogConfig* log_config = LogConfig_Create("log.txt", 
						 LOG_ON,
						 LOG_FILE,
						 LOG_DEBUG,
						 100000000, 
						 5);
	
	Config* config = Config_Create();
	Config_Read_File(config, "config.ini", log_config);
	
	Location* location = Location_Create(0, 0, "starting_zone", 0, 0); 

	float rect_percentage = 0.1f;
	ZoneRange* zr = ZoneRange_Create(rect_percentage);
	
	SDL_Rect* rect = ZoneRange_Get_Rect(zr, config, location, log_config);
	
	Log_log(log_config, LOG_INFO, "zone rect x:%i y:%i width:%i height:%i\n", rect->x, rect->y, rect->w, rect->h);
	
	Location* src_loc = Location_Create(0, 0, "starting_zone", 0, 0); 
	Location* test_loc = Location_Create(0, 0, "starting_zone", 200, 100); 
	int r = ZoneRange_Is_Location_InRange(zr, config, src_loc, test_loc, log_config);
	Log_log(log_config, LOG_DEBUG, "src x:%i src y:%i test x:%i test y:%i in range:%i\n", src_loc->x, src_loc->y, test_loc->x, test_loc->y, r);
	
	Location* test_loc_two = Location_Create(0, 0, "starting_zone", 500, 100); 
	r = ZoneRange_Is_Location_InRange(zr, config, src_loc, test_loc_two, log_config);
	Log_log(log_config, LOG_DEBUG, "src x:%i src y:%i test x:%i test y:%i in range:%i\n", src_loc->x, src_loc->y, test_loc_two->x, test_loc_two->y, r);
	
	Location* test_loc_three = Location_Create(0, 0, "starting_zone", 0, 0); 
	r = ZoneRange_Is_Location_InRange(zr, config, src_loc, test_loc_three, log_config);
	Log_log(log_config, LOG_DEBUG, "src x:%i src y:%i test x:%i test y:%i in range:%i\n", src_loc->x, src_loc->y, test_loc_three->x, test_loc_three->y, r);
	

	Location_Destroy(location);
	Config_Destroy(config);	
	ZoneRange_Destroy(zr);
}
