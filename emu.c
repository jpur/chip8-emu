#include <stdio.h>
#include <inttypes.h>
#include "SDL2/SDL.h"
#include "chip8.h"

#define SCREEN_SCALE_FACTOR 10
#define TICK_INTERVAL (1000/60)

SDL_Keycode input_keys[NUM_INPUT] = { SDLK_0, SDLK_1, SDLK_2, SDLK_3, SDLK_4, SDLK_5, SDLK_6, SDLK_7, SDLK_8, SDLK_9, SDLK_a, SDLK_b, SDLK_c, SDLK_d, SDLK_e, SDLK_f };
uint8_t input_events[NUM_INPUT];

static uint32_t next_tick;

uint32_t tick_time_left() {
	uint32_t now = SDL_GetTicks();
	return next_tick <= now ? 0 : next_tick - now;
}

void update_input(SDL_Keycode key, int pressed) {
	for (int i = 0; i < NUM_INPUT; i++) {
		if (input_keys[i] == key) {
			chip8_input(i, pressed);
			break;
		}
	}
}

int main(int argc, char *args[]) {
	SDL_Window *window = NULL;
	SDL_Renderer *renderer = NULL;
	SDL_Surface *screen = NULL;

	if (SDL_Init(SDL_INIT_VIDEO) < 0) {
		return 1;
	}

	window = SDL_CreateWindow("Chip8", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH * SCREEN_SCALE_FACTOR, SCREEN_HEIGHT * SCREEN_SCALE_FACTOR, SDL_WINDOW_SHOWN);
	renderer = SDL_CreateRenderer(window, -1, 0);

	screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 16, 0xF000, 0x0F00, 0x00F0, 0x000F);
	SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0, 0, 0));

	chip8_init(args[1]);

	int quit = 0;
	next_tick = SDL_GetTicks() + TICK_INTERVAL;
	while (!quit) {
		chip8_update_timers();
		chip8_next(input_events);

		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			} else if (event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) {
				update_input(event.key.keysym.sym, event.type == SDL_KEYDOWN);
			}
		}

		uint8_t *src_pixels = chip8_pixels();
		uint16_t *dst_pixels = screen->pixels;
		for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
			if (src_pixels[i] == 0) {
				dst_pixels[i] = SDL_MapRGB(screen->format, 0, 0, 0);
			} else {
				dst_pixels[i] = SDL_MapRGB(screen->format, 255, 255, 255);
			}
		}

		SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, screen);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, tex, NULL, NULL);
		SDL_RenderPresent(renderer);

		SDL_Delay(tick_time_left());
		next_tick += TICK_INTERVAL;
	}

	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}