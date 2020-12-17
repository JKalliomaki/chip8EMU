#include "chip8.h"

chip8::~chip8()
{
	
}

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

bool chip8::load(const char gameFileName[])
{
	// open file in binary format and move to end of it with ::ate
	std::ifstream gameFile(gameFileName, std::ios::binary | std::ios::ate);
	if (gameFile.is_open())
	{
		// Opening successful, use tellg after ::ate to get position -> size
		std::streampos size = gameFile.tellg();
		long sizeI = long(size);
		char* buffer = new char[sizeI];

		// Read file, 0x200 is default gamefile's storing starting pos
		for (long i = 0; i < sizeI; i++)
		{
			memory[512 + i] = buffer[i];
		}
		delete[] buffer;
	}
	else {
		printf("error reading file \n");
		return false;
	}

	// Close file
	gameFile.close();
	return true;
}

void chip8::emulateCycle()
{
	// Fetch opcode by merging two bytes together
	opcode = memory[pc] << 8 | memory[pc + 1];


	int tempInt;
	unsigned short tempShort1;
	unsigned short tempShort2;
	unsigned short tempShort3;
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
		tempShort1 = (opcode & 0x000F);
		tempShort2 = R[(opcode & 0x0F00) >> 8];
		tempShort3 = R[(opcode & 0x00F0) >> 4];
		R[0xF] = 0;

		for (int row = 0; row < tempShort1; row++) {
			char rowbyte = memory[I + row];
			for (int line = 0; line < 8; line++) {
				// If bit to be drawn is not zero
				if ((rowbyte & (0x80 >> line)) != 0) {
					if (gfx[tempShort2 + line + ((tempShort3 + row) * 64)] == 1){
						R[0xF] = 1;

					gfx[tempShort2 + line + ((tempShort3 + row) * 64)] ^= 1;
					}
				}
			}
		}
		pc += 2;
		drawFlag = true;
		break;

		// ex9e
	case 0xE000:
		switch (opcode & 0x000F) {
		case 0x000E:
			if (key[R[(opcode & 0x0F00) >> 8]] == 1) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;

		// exa1
		case 0x0001:
			if (key[R[(opcode & 0x0F00) >> 8]] == 0) {
				pc += 4;
			}
			else {
				pc += 2;
			}
			break;

		default:
			printf("opcode not found [0xE000] %X\n", opcode);
			break;
		}
	case 0xF000:
		switch (opcode & 0x000F)
		{
		// fx07
		case 0x0007:
			R[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		// fx18
		case 0x0008:
			R[(opcode & 0x0F00) >> 8] = sound_timer;
			pc += 2;
			break;

		// fx0a
		case 0x000A:
			printf("not made yet %X\n", opcode);
			pc += 2;
			break;

		// fx1e
		case 0x000E:
			I += R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		// fx29
		case 0x0009:
			printf("not made yet %X\n", opcode);
			pc += 2;
			break;

		// fx33
		case 0x0003:
			tempInt = R[(opcode & 0x0F00) >> 8];
			memory[I] = tempInt / 100;
			memory[I + 1] = (tempInt - (int(memory[I]) * 100)) / 10;
			memory[I + 2] = tempInt % 10;
			pc += 2;
			break;

		case 0x0005:
			switch (opcode & 0x00F0) {
			// fx15
			case 0x0010:
				delay_timer = R[(opcode & 0x0F00) >> 8];
				pc += 2;
				break;

			// fx55
			case 0x0050:
				tempInt = (opcode & 0x0F00) >> 8;

				for (int j = 0; j <= tempInt; j++) {
					memory[I + j] = R[j];
				}
				pc += 2;
				break;

			// fx65
			case 0x0060:
				tempInt = (opcode & 0x0F00) >> 8;

				for (int j = 0; j <= tempInt; j++) {
					R[j] = memory[I + j];
				}
				pc += 2;
				break;

			default:
				printf("opcode not found [0xF005] %X\n", opcode);
			}

			break;
		}


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
