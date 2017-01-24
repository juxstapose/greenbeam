#ifndef _LOCATION_H_
#define _LOCATION_H_

#define LOC_STARTING_ZONE "starting_zone"
#define LOC_STARTING_X 0
#define LOC_STARTING_Y 0

#define ZONENAME_MAX_SIZE 64

typedef struct Location {
	int location_key;
	int user_key;
	char zonename[ZONENAME_MAX_SIZE];
	int x;
	int y;
} Location;

Location* Location_Create(int location_key, int user_key, char* zonename, int x, int y);
void Location_Destroy(Location* location);


#endif
