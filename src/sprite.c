#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "sprite.h"

Sprite* Sprite_Create() {
	
	Sprite* sprite = (Sprite*)malloc(sizeof(Sprite));

	SDL_Rect* bounding_box = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	bounding_box->x = 0;
	bounding_box->y = 0;
	bounding_box->width = 0;
	bounding_box->height = 0;
	sprite->bounding_box = bounding_box;

	sprite->animation = ANIMATION_WALK;
	sprite->frame = 0;

	return sprite;
}

int Sprite_Render(Sprite* sprite) {

	//source rectangle is the slice of the spritesheet
	//destination rectangle is the width and height of the screen
	//the x and y is the current position of the sprite
	
	SDL_Rect dest;
	dest.x = sprite->bounding_box->x; 
	dest.y = sprite->bounding_box->y;
	dest.w = sprite->bounding_box->width;
	dest.h = sprite->bounding_box->height;
	
	SDL_RenderCopy(renderer, spritesheet, src, &dest);

	return 0;
}

void Sprite_Destroy(Sprite* sprite) {
	free(sprite);
}
