#include "chip8.h"
#include <stdio.h>

#define MEM_SIZE 4096
#define STACK_SIZE 16
#define NUM_REG 16
#define MEM_START 512
#define INSTR_LEN 2
#define CARRY_REG 0xF

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
	printf("0x%04X\n", opcode);

	next_instr();
	
	switch (opcode & 0xF000) {
		case 0x0000:
			if (opcode == 0x00EE) {
				set_pc(stack[--sp]);
			}
			break;
		case 0x1000:
			set_pc(opcode & 0x0FFF);
			break;
		case 0x2000:
			stack[sp++] = pc;
			set_pc(opcode & 0x0FFF);
			break;
		case 0x3000:
			if (get_reg((opcode & 0x0F00) >> 8) == (opcode & 0x00FF)) {
				next_instr();
			}
			break;
		case 0x4000:
			if (get_reg((opcode & 0x0F00) >> 8) != (opcode & 0x00FF)) {
				next_instr();
			}
			break;
		case 0x5000:
			if (get_reg((opcode & 0x0F00) >> 8) == get_reg((opcode & 0x00F0) >> 4)) {
				next_instr();
			}
			break;
		case 0x6000:
			set_reg((opcode & 0x0F00) >> 8, opcode & 0x00FF);
			break;
		case 0x7000:;
			unsigned char r = (opcode & 0x0F00) >> 8;
			set_reg(r, get_reg(r) + (opcode & 0x00FF));
			break;
		case 0x8000:;
			unsigned char r1 = (opcode & 0x0F00) >> 8;
			unsigned char r1_v = get_reg(r1);
			unsigned char r2 = (opcode & 0x00F0) >> 4;
			unsigned char r2_v = get_reg(r2);
			switch (opcode & 0x000F) {
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
			I = opcode & 0x0FFF;
			break;
		case 0xE000:
			switch (opcode & 0x00FF) {
				case 0x00A1:
					//Skips the next instruction if the key stored in VX isn't pressed. (Usually the next instruction is a jump to skip a code block)
				break;
				case 0x009E:
					//Skips the next instruction if the key stored in VX is pressed. (Usually the next instruction is a jump to skip a code block)
				break;
			}
			break;
		case 0xD000:;
			unsigned char x = (opcode & 0x0F00) >> 8;
			unsigned char y = (opcode & 0x00F0) >> 4;
			unsigned char height = (opcode & 0x000F);
			for (int yi = y; yi < y + height; yi++) {
				for (int xi = x; xi < x + SPRITE_WIDTH; xi++) {
					pixels[yi * SCREEN_WIDTH + xi] = 1;
				}
			}	
			printf("Draw sprite at (%d,%d) of size (%d,%d)\n", x, y, SPRITE_WIDTH, height);
			break;
		case 0xF000:
			set_reg((opcode & 0x0F00) >> 8, delay_timer);
			break;
		default:
			printf("^ unimpelemented\n");
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