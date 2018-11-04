#include <stdio.h>
#include <inttypes.h>
#include "SDL2/SDL.h"
#include "chip8.h"

#define SCREEN_SCALE_FACTOR 10
#define TICK_MS 1000
#define CPU_TICK_RATE 60

uint32_t update_cpu(uint32_t interval, void *param) {
	chip8_next();
	return interval;
}

uint32_t update_cpu_timers(uint32_t interval, void *param) {
	chip8_update_timers();
	return interval;
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

	chip8_init("pong.ch8");
	SDL_TimerID cpu_timer_id = SDL_AddTimer(TICK_MS / CPU_TICK_RATE, update_cpu, NULL);
	SDL_TimerID cpu_timers_timer_id = SDL_AddTimer(TICK_MS, update_cpu_timers, NULL);

	//draw(0, 0, 5);
	int quit = 0;
	while (!quit) {   
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				quit = 1;
			}
		}

		uint8_t *src_pixels = chip8_pixels();
		uint16_t *dst_pixels = screen->pixels;
		for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
			if (src_pixels[i] == 1) {
				dst_pixels[i] = SDL_MapRGB(screen->format, 0, 0, 0);
			} else {
				dst_pixels[i] = SDL_MapRGB(screen->format, 255, 255, 255);
			}
		}

		SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, screen);
		SDL_RenderClear(renderer);
		SDL_RenderCopy(renderer, tex, NULL, NULL);
		SDL_RenderPresent(renderer);
	}

	SDL_RemoveTimer(cpu_timer_id);
	SDL_RemoveTimer(cpu_timers_timer_id);
	SDL_DestroyWindow(window);
	SDL_Quit();
	return 0;
}