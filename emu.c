#include <stdio.h>
#include <inttypes.h>
#include <stdlib.h>
#include "SDL2/SDL.h"
#include "chip8.h"

#define SCREEN_SCALE_FACTOR 10
#define CPU_INTERVAL (1000/500)
#define TIMER_INTERVAL (1000/60)

SDL_Keycode input_keys[NUM_INPUT] = { 
	SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, 
	SDLK_8, SDLK_9, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f 
};

void key_event(SDL_Keycode key, int pressed) {
	for (int i = 0; i < NUM_INPUT; i++) {
		if (input_keys[i] == key) {
			chip8_input(i, pressed);
			break;
		}
	}
}

void update(uint32_t delta) {
	static uint32_t next_cpu_tick;
	static uint32_t next_timer_tick;

	uint32_t now = SDL_GetTicks();
	if (next_cpu_tick <= now) {
		next_cpu_tick = now + CPU_INTERVAL;
		chip8_next();
	}
	if (next_timer_tick <= now) {
		next_timer_tick = now + TIMER_INTERVAL;
		chip8_update_timers();
	}
}

void draw(SDL_Surface *screen) {
	// Copy pixels from emulator to screen
	uint8_t *src_pixels = chip8_pixels();
	uint16_t *dst_pixels = screen->pixels;
	for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
		dst_pixels[i] = src_pixels[i] ? SDL_MapRGB(screen->format, 255, 255, 255) : SDL_MapRGB(screen->format, 0, 0, 0);
	}
}

int main(int argc, char *args[]) {
	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
	}

	SDL_Window *window = SDL_CreateWindow("CHIP-8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCREEN_SCALE_FACTOR, SCREEN_HEIGHT * SCREEN_SCALE_FACTOR, SDL_WINDOW_SHOWN);
	SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_Surface *screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0xF000, 0x0F00, 0x00F0, 0x000F);

	// Initialization
	chip8_init(args[1]);

	int quit = 0;
	uint32_t last_update = SDL_GetTicks();
	while (!quit) {
		// Input
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			} else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				key_event(event.key.keysym.sym, event.type == SDL_KEYDOWN);
			}
		}

		// Update
		uint32_t now = SDL_GetTicks();
		update(now - last_update);
		last_update = now;

		// Draw to screen
		draw(screen);

		// Render screen
		SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, screen);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, tex, NULL, NULL);
		SDL_RenderPresent(renderer);
		SDL_DestroyTexture(tex);
	}

	SDL_FreeSurface(screen);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}