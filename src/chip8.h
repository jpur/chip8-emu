#include <inttypes.h>

#define SCREEN_WIDTH 64
#define SCREEN_HEIGHT 32
#define SPRITE_WIDTH 8
#define NUM_INPUT 16

void chip8_init(char *file);
uint8_t *chip8_pixels();
void chip8_next();
void chip8_exec(uint16_t opcode);
void chip8_input(uint8_t key, uint8_t pressed);
uint8_t chip8_draw(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t height);

uint8_t chip8_tick_delay_timer();
uint8_t chip8_tick_sound_timer();