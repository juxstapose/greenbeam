#ifndef _LOCATION_H_
#define _LOCATION_H_

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
