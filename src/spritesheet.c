#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <SDL2/SDL.h>
#include "spritesheet.h"

#ifdef main
#undef main
#endif

Spritesheet* Spritesheet_Create(SDL_Renderer* renderer, char* spritesheet_image_filename, char*  spritesheet_meta_filename) {

	Spritesheet* = (Spritesheet*)malloc(sizeof(Spritesheet));
	SDL_Surface *image = IMG_Load(spritesheet_image_filename);
	if(image == NULL) {

		fprintf(stderr, "image load error\n");
		fprintf(stderr, "%s\n", IMG_GetError());
		
		return NULL;
	}
	
	spritesheet->texture = SDL_CreateTextureFromSurface(renderer, image);
	SDL_FreeSurface(image);
	if(texture == NULL) {
		
		fprintf(stderr, "create texture error %s\n", SDL_GetError());
		return NULL;

	}

	spritesheet->meta_file = fopen(spritesheet_meta_filename, "r");
	int errnum = 0;
	if(spritesheet->meta_file == NULL) {
		errnum = errno;
		fprintf(stderr,"Value of error: %d\n", errno);
		fprintf(stderr,"Error opening file: %s\n", strerror( errnum ));
	}
	return spritesheet;
}


SDL_Rect* Sprite_Spritesheet_Load_Animation(int frame, int animation) {
	//printf("spritesheet load\n");
	//parse file by line (y offset) and x offset to find width,height tuple
	//y offset is animation type and x offset is frame
	//add up widths and heights that resulted in this location in textfile
	//widths translated to x in SDL_REct, heights translate to y
	//width is the destination width and height is the destination height	
	int c;
	char number[4096];
	memset(number, '\0', 4096);
	int numbers[128];
	memset(number, 0, 128);
	int x_index = 0;
	int y_index = 0;
	int ni = 0;
	int result_width = 0;
	int result_height = 0;
	int i = 0;
	int count = 0;
	int x_offset = frame;
	int y_offset = animation;
	while ((c = fgetc(spritesheet->meta)) != EOF) {
		char test = c;
		if(y_index == y_offset) {
			//printf("y index: %i", y_index);
			if(test == ' '){
				ni = 0;
				x_index++;
			}
			if(test != ' ' && test != '\n') {
				//printf("%i ", c);
				number[ni] = c;
				numbers[x_index] = atoi(number);
				ni++;
			}
			count++;
		}
		if(test == '\n') {
			y_index++;	
		}
		i++;
	}
	rewind(f);
	
	int result_x_offset = 0;
	int result_y_offset = 0;
	for(int i = 0; i < x_offset+1; i+=2) {
		//printf("numbers[%i] = %i\n", i, numbers[i]);
		//printf("numbers[%i+1] = %i\n", i+1, numbers[i+1]);
		//printf("%i %i\n", x_offset+1, i+1);
		//printf("i: %i y offset: %i\n", i, result_y_offset);
		result_x_offset = result_x_offset + numbers[i];
		result_y_offset = result_y_offset + numbers[i + 1];
	}
	result_width = numbers[x_offset];
	result_height = numbers[x_offset+1];	
	result_x_offset = result_x_offset - result_width;	
	result_y_offset = result_y_offset - result_height;	
	
	//printf("result_width:%i\n", result_width);
	//printf("result_height:%i\n", result_height);
	//printf("result_x_offset:%i\n", result_x_offset);
	//printf("result_y_offset:%i\n", result_y_offset);

	SDL_Rect* result = (SDL_Rect*)malloc(sizeof(SDL_Rect));
	result->x = result_x_offset;
	result->y = result_y_offset;
	result->w = result_width;
	result->h = result_height;
	//printf("end spritesheet load\n");
	return result;
}

void Test_Spritesheet_Load_Animation(FILE * spritesheet_meta, int frame, int animation, SDL_Rect* expected) { 

	Spritesheet* spritesheet = Spritesheet_Create(renderer, spritesheet_image_filename, spritesheet_meta_filename);
	SDL_Rect* result = Spritesheet_Load_Animation(frame, animation);
	Spritesheet_Destroy(spritesheet);

	if(result->x != expected->x) {
		printf("test x failed %i != %i\n",result->x, expected->x);
	} else {
		printf("test x passed\n");
	}
	if(result->y != expected->y) { 
		printf("test y failed %i != %i\n",result->y, expected->y);
	} else {
		printf("test y passed\n");
	}
	if(result->w != expected->w) {
		printf("test w failed %i != %i\n",result->w, expected->w);
	} else {
		printf("test w passed\n");
	}
	if(result->h != expected->h) {
		printf("test h failed %i != %i\n",result->h, expected->h);
	} else {
		printf("test h passed\n");
	}
	printf("\n");

}

void Spritesheet_Destroy(Spritesheet* spritesheet) {
	if(spritesheet->texture != NULL) {
		SDL_DestroyTexture(spritesheet->texture);
	}
	if(spritesheet->meta_file != NULL) {
		fclose(spritesheet->meta_file);
	}
}

