#include <stdlib.h>
#include <stdio.h>
#include "SDL2/SDL.h"
#include "config.h"
#include "background.h"
#include "zonerange.h"
#include "string_hashtable.h"
#include "util.h"
#include "log.h"
#include "location.h"

ZoneRange* ZoneRange_Create(float rect_percentage) {
	ZoneRange* zr = (ZoneRange*)malloc(sizeof(ZoneRange));
	zr->rect_percentage = rect_percentage;
	return zr;
}

char** ZoneRange_Get_Background_Image_Filenames(Config* config, unsigned int *size) {
	//printf("before string hashtable\n");
	char* resource_directory = String_Hashtable_Get(config->string_hashtable, "resource_directory");
	char* comma_delimited_image_filenames = String_Hashtable_Get(config->string_hashtable, "background_images");
	char image_filenames[4096];
	strcpy(image_filenames, comma_delimited_image_filenames);
	//printf("deliiter count\n");
	*size = Util_Comma_Delimited_Count(image_filenames);	
	//printf("size: %i\n", *size);
	Util_Remove_All_Chars(image_filenames, '\r');	
	Util_Remove_All_Chars(image_filenames, '\n');	
	Util_Remove_All_Chars(image_filenames, ' ');
	char* base_filename = '\0';
	
	char** filenames = (char**)calloc(*size, sizeof(char*));
	int i = 0;
	//printf("i: %i\n", i);
	base_filename = strtok(image_filenames, ",");
	if(base_filename != NULL) {
		filenames[i] = calloc(1, sizeof(char) * MAX_LINE_LENGTH);
		int bytes = sprintf(filenames[i], "%s/%s", resource_directory, base_filename);
		i++;
	}
	while(base_filename != NULL) {
		base_filename = strtok(NULL, ",");
		if(base_filename != NULL) {
			filenames[i] = calloc(1, sizeof(char) * MAX_LINE_LENGTH);
			int bytes = sprintf(filenames[i], "%s/%s", resource_directory, base_filename);
			//printf("i: %i\n", i);
			i++;
		}
	}	
	return filenames;
}

SDL_Rect* ZoneRange_Get_Rect(ZoneRange* zr, Config* config, Location* location, LogConfig* log_config) {
	SDL_Rect* rect = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	rect->x = 0;
	rect->y = 0;
	rect->w = 0;
	rect->h = 0;
	
	SDL_Window* window = SDL_CreateWindow("test", 
			                       SDL_WINDOWPOS_CENTERED,
					       SDL_WINDOWPOS_CENTERED,
					       640, 480, SDL_WINDOW_HIDDEN);
	if(window == NULL) {
		Log_log(log_config, LOG_ERROR, "window creation error: %s\n", SDL_GetError());
	}
	
	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	
	if(renderer == NULL) {
		Log_log(log_config, LOG_ERROR, "renderer creation error: %s\n", SDL_GetError());
	}
	
	unsigned int size = 0;
	char** filenames = ZoneRange_Get_Background_Image_Filenames(config, &size);
	
	/**	
	int i = 0;
	printf("%p\n", filenames);
	for(i=0; i<size; i++) {
		printf("%s\n", filenames[i]);	
	}
	**/

	Background* background = Background_Create(renderer, size, filenames, log_config);
	if(background != NULL) {
		unsigned int total_width = Background_Get_Total_Width(background);
		//printf("total_width:%i\n", total_width);
		Log_log(log_config, LOG_INFO, "total width: %i\n", total_width);
		
		unsigned int largest_height = Background_Get_Largest_Height(background);
		
			
		int delta_pixels = zr->rect_percentage * total_width;
		Log_log(log_config, LOG_INFO, "delta pixels: %i\n", delta_pixels);
		int x_post = location->x + delta_pixels;
		int x_pre = location->x - delta_pixels;
		int width = abs(x_post - x_pre);

		rect->x = x_pre;
		rect->y = 0;
		rect->w = width;
		rect->h = largest_height;
	}
	int i = 0;
	for(i=0; i<size; i++) {
		free(filenames[i]);
	}
	free(filenames);
	return rect;	
}

int ZoneRange_Is_Location_InRange(ZoneRange* zr, Config* config, Location* src_loc, Location* test_loc, LogConfig* log_config) {
	SDL_Rect* rect = ZoneRange_Get_Rect(zr, config, src_loc, log_config);
	if(test_loc->x > rect->x && test_loc->x < (rect->x + rect->w)) {
		return 1;
	}
	return 0;
}

void ZoneRange_Destroy(ZoneRange* zr) {
	if(zr != NULL) {
		free(zr);
	}
}
