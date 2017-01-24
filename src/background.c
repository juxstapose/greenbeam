#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "background.h"

SDL_Texture* Background_Load_Image(SDL_Renderer* renderer, char* filename, LogConfig* log_config) {

	SDL_Texture* tex = NULL;
	tex = IMG_LoadTexture(renderer, filename);
	if(tex == NULL) {
		Log_log(log_config, LOG_ERROR, "failed to load image %s %s\n", filename, IMG_GetError());
	}
	return tex;
}	

Background* Background_Create(SDL_Renderer* renderer, unsigned int size, char** filenames, LogConfig* log_config) {
	Background* background = (Background*)malloc(sizeof(Background));
	background->textures = (SDL_Texture**)calloc(size, sizeof(SDL_Texture*));
	int i = 0;
	for(i = 0; i < size; i++) {
		background->textures[i]	= Background_Load_Image(renderer, filenames[i], log_config);	
	}
	background->size = size;
	return background;
}

void Background_Destroy(Background* background) {
	if(background != NULL) {
		free(background->textures);
		free(background);
	}
}

unsigned int Background_Get_Largest_Height(Background* background) {

	int i = 0;
	unsigned int result = 0;
	for(i=0; i<background->size; i++) {
		if(background->textures[i] != NULL) {
			int width = 0;
			int height = 0;
			SDL_QueryTexture(background->textures[i], NULL, NULL, &width, &height);
			if(height >= result) {
				result = height;
			}
		}
	}
	return result;

}

unsigned int Background_Get_Total_Width(Background* background) {
	
	int i = 0;
	unsigned int result = 0;
	for(i=0; i<background->size; i++) {
		if(background->textures[i] != NULL) {
			int width = 0;
			int height = 0;
			SDL_QueryTexture(background->textures[i], NULL, NULL, &width, &height);
			result += width;
		}
	}
	return result;

}


