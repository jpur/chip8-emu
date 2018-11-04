#include "chip8.h"
#include <stdio.h>
#include <stdlib.h>

#define MEM_SIZE 4096
#define STACK_SIZE 16
#define NUM_REG 16
#define MEM_START 512
#define INSTR_LEN 2
#define CARRY_REG 0xF

#define X(op) ((op & 0x0F00) >> 8)
#define Y(op) ((op & 0x00F0) >> 4)
#define N(op) ((op & 0x000F))
#define NN(op) ((op & 0x00FF))
#define NNN(op) ((op & 0x0FFF))

unsigned short get_instr();
void set_reg(unsigned char r, unsigned char value);
unsigned char get_reg(unsigned char r);
void next_instr();
void set_pc(unsigned short addr);

unsigned char mem[MEM_SIZE];
unsigned char reg[NUM_REG];
unsigned short stack[STACK_SIZE];
unsigned short sp;
unsigned short pc;
unsigned short I;
uint8_t pixels[SCREEN_WIDTH * SCREEN_HEIGHT];

unsigned char delay_timer;

uint8_t *chip8_pixels() {
	return pixels;
}

void chip8_init(char *file) {
	FILE *fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long flen = ftell(fp);
	rewind(fp);

	fread(mem + MEM_START, flen, 1, fp);
	set_pc(MEM_START);
}

int chip8_next() {
	unsigned short opcode = get_instr();
	//printf("0x%04X\n", opcode);

	next_instr();
	
	switch (opcode & 0xF000) {
		case 0x0000:
			if (opcode == 0x00EE) {
				set_pc(stack[--sp]);
			}
			break;
		case 0x1000:
			set_pc(NNN(opcode));
			break;
		case 0x2000:
			stack[sp++] = pc;
			set_pc(NNN(opcode));
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
		case 0x7000:;
			unsigned char rr = X(opcode);
			set_reg(rr, get_reg(rr) + (NN(opcode)));
			break;
		case 0x8000:;
			unsigned char r1 = X(opcode);
			unsigned char r1_v = get_reg(r1);
			unsigned char r2 = Y(opcode);
			unsigned char r2_v = get_reg(r2);
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
			}
		case 0xA000:
			I = NNN(opcode);
			break;
		case 0xE000:
			switch (NN(opcode)) {
				case 0x00A1:
					//Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
					
				break;
				case 0x009E:
					//Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
				break;
			}
			break;
		case 0xC000:;
			unsigned char r = X(opcode);
			set_reg(r, rand() & get_reg(r));
		case 0xD000:;
			unsigned char x = X(opcode);
			unsigned char y = Y(opcode);
			unsigned char height = (N(opcode));
			for (int yi = y; yi < y + height; yi++) {
				for (int xi = x; xi < x + SPRITE_WIDTH; xi++) {
					pixels[yi * SCREEN_WIDTH + xi] = 1;
				}
			}	
			printf("Draw sprite at (%d,%d) of size (%d,%d)\n", x, y, SPRITE_WIDTH, height);
			break;
		case 0xF000:
			set_reg(X(opcode), delay_timer);
			break;
		default:
			printf("^ unimpelemented 0x%04X\n", opcode);
			break;
	}
	
	return 1;
}

void next_instr() {
	set_pc(pc + INSTR_LEN);
}

void set_pc(unsigned short addr) {
	pc = addr;
}

unsigned char get_reg(unsigned char r) {
	return reg[r];
}

void set_reg(unsigned char r, unsigned char value) {
	reg[r] = value;
}

unsigned short get_instr() {
	return mem[pc] << 8 | mem[pc + 1];
}