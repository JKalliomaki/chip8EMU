#pragma once

#include <fstream>
#include <iostream>
#include <cstdlib>

const unsigned short GRAPHICS_SIZE = 64 * 32;
const unsigned short MEMORY_SIZE = 4096;
const unsigned short REGISTER_BANK_SIZE = 16;

class chip8
{
public:
	chip8() { initialize(); }
	~chip8();

	void initialize();

	bool load(const char gameFileName[] = "pong");

	void emulateCycle();

	bool drawFlag = false;

	// Graphics: 64x32 px with pixel on or off (1 or 0)
	unsigned char gfx[GRAPHICS_SIZE] = { 0 };

	// HEX based keypad (0x0-0xf)
	unsigned char key[16] = { 0 };

private:

	unsigned short opcode = 0;
	
	unsigned char memory[MEMORY_SIZE] = { 0 };

	// Register bank
	unsigned char R[REGISTER_BANK_SIZE] = { 0 };

	// Index register and program counter
	unsigned short I = 0;
	unsigned short pc = 0;

	// Stack for jumps and stackpointer for them
	unsigned short stack[16] = { 0 };
	unsigned short sp = 0;

	// Timers count at 60 Hz. When set above zero they count towards zero
	unsigned char delay_timer = 0;
	unsigned char sound_timer = 0;


	void disp_clear();

};

