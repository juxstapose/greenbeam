#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "log.h"
#include "background.h"

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

	unsigned int size = 4;
	char* filenames[4] = {'\0'};	
	filenames[0] = "../res/background_zone_one.png";
	filenames[1] = "../res/background_zone_two.png";
	filenames[2] = "../res/background_zone_three.png";
	filenames[3] = "../res/background_zone_four.png";
	
	Background* background = Background_Create(renderer, size, filenames, log_config);
	if(background != NULL) {
		unsigned int total_width = Background_Get_Total_Width(background);
		Log_log(log_config, LOG_INFO, "total width: %i\n", total_width);

	}
	Background_Destroy(background);
}
