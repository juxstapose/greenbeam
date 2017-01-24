#ifndef _ZONERANGE_H_
#define _ZONERANGE_H_

#include "config.h"
#include "location.h"
#include "log.h"

typedef struct ZoneRange {
	float rect_percentage;
} ZoneRange;

ZoneRange* ZoneRange_Create(float rect_percentage);
int ZoneRange_Is_Location_InRange(ZoneRange* zr, Config* config, Location* src_loc, Location* test_loc, LogConfig* log_config);
SDL_Rect* ZoneRange_Get_Rect(ZoneRange* zr, Config* config, Location* location, LogConfig* log_config);
char** ZoneRange_Get_Background_Image_Filenames(Config* config, unsigned int *size);
void ZoneRange_Destroy(ZoneRange* zr);

#endif
