#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_

#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "log.h"

typedef struct Background {
	SDL_Texture** textures;
	unsigned int size;
} Background;

SDL_Texture* Background_Load_Image(SDL_Renderer* renderer, char* filename, LogConfig* log_config); 
Background* Background_Create(SDL_Renderer* renderer, unsigned int size, char* filenames[], LogConfig* log_config);
void Background_Destroy(Background* background);
unsigned int Background_Get_Total_Width(Background* background);

#endif
