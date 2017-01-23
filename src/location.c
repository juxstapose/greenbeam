#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "location.h"

Location* Location_Create(int location_key, int user_key, char* zonename, int x, int y) {
	Location* location = (Location*)malloc(sizeof(Location));
	location->location_key = location_key;
	location->user_key = user_key;
	if(zonename != NULL) {
		strcpy(location->zonename, zonename);
	} 
	location->x = x;
	location->y = y;
	return location;
}


void Location_Destroy(Location* location) {
	if(location != NULL) {
		free(location);
	}
}

