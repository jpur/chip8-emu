OBJS = src/chip8.c src/emu.c
CC = gcc
CFLAGS = -std=c99 -Wall -Wextra -Wno-unused-parameter
LFLAGS = -lmingw32 -lSDL2main -lSDL2
TARGET = chip8emu

all: $(OBJS)
	$(CC) $(OBJS) $(INCLUDE_PATHS) $(CFLAGS) $(LFLAGS) -o $(TARGET)