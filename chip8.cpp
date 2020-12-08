#include "chip8.h"

void chip8::initialize()
{
	pc = 0x200;	// pc to expected starting location
	// Reset values
	opcode = 0;	
	I = 0;
	sp = 0;

	// Clear display
	disp_clear();

	// Clear stack
	for ( unsigned short i = 0; i < 16; i++)
	{
		stack[i] = 0;
	}

	// Clear registers R0-RF
	for (unsigned short i = 0; i < REGISTER_BANK_SIZE; i++)
	{
		R[i] = 0;
	}

	// Clear memory
	for (unsigned short i = 0; i < MEMORY_SIZE; i++)
	{
		memory[i] = 0;
	}
	// Load fontset

	// Reset timers

}

void chip8::load(const char gameFileName[])
{
	// open file in binary format and move to end of it with ::ate
	std::ifstream gameFile(gameFileName, std::ios::binary | std::ios::ate);
	if (gameFile.is_open())
	{
		// Opening successful, use tellg after ::ate to get position -> size
		std::streampos size = gameFile.tellg();
		long sizeI = long(size);
		char* buffer = new char[size];

		// Read file, 0x200 is default gamefile's storing starting pos
		for (long i = 0; i < sizeI; i++)
		{
			memory[0x200 + i] = buffer[i];
		}
		delete[] buffer;
	}
	else {
		printf("error reading file \n");
	}

	// Close file
	gameFile.close();
}

void chip8::emulateCycle()
{
	// Fetch opcode by merging two bytes together
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Decode opcode
	// Switch for MSB
	switch (opcode & 0xF000)
	{
	case 0x0000:
		// 0NNN (not necessary in most ROMs)
		switch (opcode & 0x000F) {
		// 0x00E0, clear display
		case 0x0000:
			disp_clear();
		// 0x00EE, return from subroutine
		case 0x00e:
			// WIP
		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
		}
		break;

		// 1nnn
	case 0x1000:
		pc = opcode & 0x0FFF;
		break;

		// 2nnn 
	case 0x2000:
		stack[sp] = pc;
		++sp;
		pc = opcode & 0x0FFF;
		break;

		// 3xnn
	case 0x3000:
		if (R[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
		{
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;

		// 4xnn
	case 0x4000:
		if (R[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
		{
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;

		// 5xy0
	case 0x5000:
		if (R[(opcode & 0x0F00) >> 8] == R[(opcode & 0x00F0) >> 4])
		{
			pc += 4;
		}
		else {
			pc += 2;
		}

		// 6xnn
	case 0x6000:
		R[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

		// 7xnn
	case 0x7000:
		R[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F) {
		// 8xy0
		case 0x0000:
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		// 8xy1
		case 0x0001:
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x0F00) >> 8] | R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		// 8xy2
		case 0x0002:
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x0F00) >> 8] & R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		// 8xy3
		case 0x0003:
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x0F00) >> 8] ^ R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		// 8xy4
		case 0x0004:
			R[(opcode & 0x0F00) >> 8] += R[(opcode & 0x00F0) >> 4];
			if (R[(opcode & 0x00F0) >> 4] > R[(opcode & 0x0F00) >> 8]) {
				R[0xF] = 1;
			}
			else {
				R[0xF] = 0;
			}
			pc += 2;
			break;

		// 8xy5
		case 0x0005:
			R[(opcode & 0x0F00) >> 8] -= R[(opcode & 0x00F0) >> 4];
			if (R[(opcode & 0x0F00) >> 8] < R[(opcode & 0x00F0) >> 4])
			{
				R[0xF] = 1;
			}
			else {
				R[0] = 0;
			}
			pc += 2;
			break;

		// 8xy6
		case 0x0006:
			R[0xF] = R[(opcode & 0x0F00) >> 8] & 0x0001;
			R[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		// 8xy7
		case 0x0007:
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x00F0) >> 4] - R[(opcode & 0x0F00) >> 8];
			if (R[(opcode & 0x0F00) >> 8] > R[(opcode & 0x00F0) >> 4])
			{
				R[0xF] = 0;
			}
			else {
				R[0xF] = 1;
			}
			pc += 2;
			break;

		// 8xye
		case 0x000E:
			R[0xF] = R[(opcode & 0x8000) >> 16];
			R[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;


		default:
			printf("opcode not found [0x8000] %X\n", opcode);
		}
		break;

		// 9xy0
	case 0x9000:
		if (R[(opcode & 0x0F00) >> 8] != R[(opcode & 0x00F0) >> 4])
		{
			pc += 4;
		}
		else {
			pc += 2;
		}
		break;

		// annn
	case 0xA000:
		I = memory[opcode & 0x0FFF];
		pc += 2;
		break;

		// bnnn
	case 0xB000:
		pc = R[0] + (opcode & 0x0FFF);
		break;

		// cxnn
	case 0xC000:
		R[(opcode & 0x0F00) >> 8] = rand() % 256;
		pc += 2;
		break;

		// dxyn
	case 0xD000:


		// ex9e
		// exa1
		// fx07
		// fx0a
		// fx15
		// fx18
		// fx1e
		// fx29
		// fx33
		// fx55
		// fx65


	default:
		printf("Unknown opcode 0x%X\n", opcode);
		break;
	}


	// Update timers

}

void chip8::disp_clear()
{
	for (unsigned short i = 0; i < GRAPHICS_SIZE; i++)
	{
		gfx[i] = 0;
	}
}
