
#define ANIMATION_WALK 0
#define ANIMATION_RUN 1
typedef struct Sprite {
	
	int x;
	int y;

	SDL_Rect* bounding_box;

	int frame;
	int animation;

} Sprite;
