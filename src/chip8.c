#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEM_SIZE 4096
#define STACK_SIZE 16
#define NUM_REG 16
#define MEM_START 512
#define INSTR_LEN 2
#define CARRY_REG 0xF

#define FONT_IDX_START 0
#define FONT_WIDTH 4
#define FONT_HEIGHT 5
#define FONT_NUM_CHARS 16

#define X(op) ((op & 0x0F00) >> 8)
#define Y(op) ((op & 0x00F0) >> 4)
#define N(op) ((op & 0x000F))
#define NN(op) ((op & 0x00FF))
#define NNN(op) ((op & 0x0FFF))

void set_reg(uint8_t r, uint8_t value);
uint16_t next_instr();
uint8_t get_reg(uint8_t r);
void chip8_exec(uint16_t opcode);

uint8_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];
uint8_t mem[MEM_SIZE];
uint8_t reg[NUM_REG];
uint8_t keys[NUM_INPUT];
uint16_t stack[STACK_SIZE];
uint16_t sp;
uint16_t pc;
uint16_t I;

uint8_t delay_timer;
uint8_t sound_timer;

uint8_t blocking;
uint8_t blocking_reg;

uint8_t font[FONT_NUM_CHARS * FONT_HEIGHT] =
{ 
  0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
  0x20, 0x60, 0x20, 0x20, 0x70, // 1
  0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
  0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
  0x90, 0x90, 0xF0, 0x10, 0x10, // 4
  0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
  0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
  0xF0, 0x10, 0x20, 0x40, 0x40, // 7
  0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
  0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
  0xF0, 0x90, 0xF0, 0x90, 0x90, // A
  0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
  0xF0, 0x80, 0x80, 0x80, 0xF0, // C
  0xE0, 0x90, 0x90, 0x90, 0xE0, // D
  0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
  0xF0, 0x80, 0xF0, 0x80, 0x80  // F
};

void chip8_init(char *file) {
	FILE *fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long flen = ftell(fp);
	rewind(fp);

	fread(mem + MEM_START, flen, 1, fp);
	fclose(fp);

	for (int i = 0; i < FONT_NUM_CHARS * FONT_HEIGHT; i++) {
		mem[FONT_IDX_START + i] = font[i];
	}

	sp = 0;
	I = 0;
	pc = MEM_START;
}

void chip8_next() {
	if (!blocking) {
		chip8_exec(next_instr());
	}
}

void chip8_exec(uint16_t opcode) {
	switch (opcode & 0xF000) {
		case 0x0000:
			switch (opcode) {
				case 0x00EE:
					pc = stack[--sp];
					break;
				case 0x00E0:
					memset(pixels, 0, SCREEN_WIDTH * SCREEN_HEIGHT);
					break;
			}
			break;
		case 0x1000:
			pc = NNN(opcode);
			break;
		case 0x2000:
			stack[sp++] = pc;
			pc = NNN(opcode);
			break;
		case 0x3000:
			if (get_reg(X(opcode)) == (NN(opcode))) {
				next_instr();
			}
			break;
		case 0x4000:
			if (get_reg(X(opcode)) != (NN(opcode))) {
				next_instr();
			}
			break;
		case 0x5000:
			if (get_reg(X(opcode)) == get_reg(Y(opcode))) {
				next_instr();
			}
			break;
		case 0x6000:
			set_reg(X(opcode), NN(opcode));
			break;
		case 0x7000:
			set_reg(X(opcode), get_reg(X(opcode)) + NN(opcode));
			break;
		case 0x8000:;
			uint8_t r1 = X(opcode);
			uint8_t r1_v = get_reg(r1);
			uint8_t r2 = Y(opcode);
			uint8_t r2_v = get_reg(r2);
			switch (N(opcode)) {
				case 0x0000:
					set_reg(r1, r2_v);
					break;
				case 0x0001:
					set_reg(r1, r1_v | r2_v);
					break;
				case 0x0002:
					set_reg(r1, r1_v & r2_v);
					break;
				case 0x0003:
					set_reg(r1, r1_v ^ r2_v);
					break;
				case 0x0004:
					set_reg(r1, r1_v + r2_v);
					set_reg(CARRY_REG, r1_v > (0xFF - r2_v));
					break;
				case 0x0005:
					set_reg(r1, r1_v - r2_v);
					set_reg(CARRY_REG, r1_v > r2_v);
					break;
				case 0x0006:
					set_reg(CARRY_REG, r1_v & 1);
					set_reg(r1, r1_v >> 1);
					break;
				case 0x0007:
					set_reg(r1, r2_v - r1_v);
					set_reg(CARRY_REG, r2_v > r1_v);
					break;
				case 0x000E:
					set_reg(CARRY_REG, (r1_v & 0x80) != 0);
					set_reg(r1, r1_v << 1);
					break;
			}
			break;
		case 0xA000:
			I = NNN(opcode);
			break;
		case 0xB000:
			pc = NNN(opcode) + get_reg(0);
			break;
		case 0xC000:
			set_reg(X(opcode), NN(opcode) & (rand() % 256));
			break;
		case 0xD000:
			set_reg(CARRY_REG, chip8_draw(get_reg(X(opcode)), get_reg(Y(opcode)), mem + I, N(opcode)));
			break;
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x009E:
					if (keys[get_reg(X(opcode))]) {
						next_instr();
					}
				break;
				case 0x00A1:
					if (!keys[get_reg(X(opcode))]) {
						next_instr();
					}
				break;
			}
			break;
		case 0xF000:
			switch (opcode & 0x00FF) {
				case 0x0007:
					set_reg(X(opcode), delay_timer);
					break;
				case 0x000A:
					blocking = 1;
					blocking_reg = X(opcode);
					break;
				case 0x0033:
					mem[I] = get_reg(X(opcode)) / 100;
					mem[I + 1] = (get_reg(X(opcode)) / 10) % 10;
					mem[I + 2] = (get_reg(X(opcode)) % 100) % 10;
					break;
				case 0x0015:
					delay_timer = get_reg(X(opcode));
					break;
				case 0x0018:
					sound_timer = get_reg(X(opcode));
					break;
				case 0x001E:
					I += get_reg(X(opcode));
					break;
				case 0x0029:
					I = get_reg(X(opcode)) * FONT_HEIGHT;
					break;
				case 0x0055:
					for (int i = 0; i <= X(opcode); i++) {
						mem[I + i] = get_reg(i);
					}
					break;
				case 0x0065:
					for (int i = 0; i <= X(opcode); i++) {
						set_reg(i, mem[I + i]);
					}
					break;
			}
			break;
	}
}

void chip8_input(uint8_t key, uint8_t pressed) {
	if (blocking && keys[key] == 0 && pressed) {
		set_reg(blocking_reg, key);
		blocking = 0;
	}
	keys[key] = pressed;
}

uint8_t chip8_draw(uint8_t x, uint8_t y, uint8_t *sprite, uint8_t height) {
	uint8_t pixel_flipped = 0;

	for (int yi = 0; yi < height; yi++) {
		for (int xi = 0; xi < SPRITE_WIDTH; xi++) {
			uint8_t pixel = sprite[yi] & (1 << (SPRITE_WIDTH - xi - 1));
			int idx = x + ((y + yi) * SCREEN_WIDTH) + xi;
			if (pixels[idx]) {
				pixel_flipped = 1;
			}
			pixels[idx] ^= pixel != 0;
		}
	}

	return pixel_flipped;
}

uint8_t *chip8_pixels() {
	return pixels;
}

uint8_t chip8_tick_delay_timer() {
	return delay_timer > 0 ? delay_timer-- : 0;
}

uint8_t chip8_tick_sound_timer() {
	return sound_timer > 0 ? sound_timer-- : 0;
}

uint16_t next_instr() {
	uint16_t curr_instr = mem[pc] << 8 | mem[pc + 1];
	pc += INSTR_LEN;
	return curr_instr;
}

uint8_t get_reg(uint8_t r) {
	return reg[r];
}

void set_reg(uint8_t r, uint8_t value) {
	reg[r] = value;
}