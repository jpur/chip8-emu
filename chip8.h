#include <inttypes.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SPRITE_WIDTH 8

int chip8_next();
void chip8_init(char *file);
uint8_t *chip8_pixels();