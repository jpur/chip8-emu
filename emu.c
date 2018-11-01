#include <stdio.h>

#define MEM_START 512

unsigned short get_instr();
int next();

unsigned char mem[4096];
unsigned char reg[16];
unsigned short stack[16];
unsigned short sp;
unsigned short pc;

int main() {
	char file[] = "pong.ch8";
	FILE *fp = fopen(file, "rb");
	fseek(fp, 0, SEEK_END);
	long flen = ftell(fp);
	rewind(fp);

	fread(mem + MEM_START, flen, 1, fp);

	pc = MEM_START;
	while (next() == 1);

	fclose(fp);
	return 0;
}

int next() {
	unsigned short opcode = get_instr();
	printf("0x%04X\n", opcode);

	pc += 2;
	
	switch (opcode & 0xF000) {
		case 0x0000:
			if (opcode == 0x00EE) {
				pc = stack[--sp];
			}
			break;
		case 0x2000:
			stack[sp++] = pc + 2;
			pc = opcode & 0x0FFF;
			break;
		case 0x6000:
			reg[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
			break;
	}
	
	return 1;
}

unsigned short get_instr() {
	return mem[pc] << 8 | mem[pc + 1];
}