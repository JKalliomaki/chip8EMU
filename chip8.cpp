#include "chip8.h"

unsigned char chip8_fontset[80] =
{
	0xF0, 0x90, 0x90, 0x90, 0xF0, //0
	0x20, 0x60, 0x20, 0x20, 0x70, //1
	0xF0, 0x10, 0xF0, 0x80, 0xF0, //2
	0xF0, 0x10, 0xF0, 0x10, 0xF0, //3
	0x90, 0x90, 0xF0, 0x10, 0x10, //4
	0xF0, 0x80, 0xF0, 0x10, 0xF0, //5
	0xF0, 0x80, 0xF0, 0x90, 0xF0, //6
	0xF0, 0x10, 0x20, 0x40, 0x40, //7
	0xF0, 0x90, 0xF0, 0x90, 0xF0, //8
	0xF0, 0x90, 0xF0, 0x10, 0xF0, //9
	0xF0, 0x90, 0xF0, 0x90, 0x90, //A
	0xE0, 0x90, 0xE0, 0x90, 0xE0, //B
	0xF0, 0x80, 0x80, 0x80, 0xF0, //C
	0xE0, 0x90, 0x90, 0x90, 0xE0, //D
	0xF0, 0x80, 0xF0, 0x80, 0xF0, //E
	0xF0, 0x80, 0xF0, 0x80, 0x80  //F
};


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
	drawFlag = true;

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
	for (int i = 0; i < 80; i++) {
		memory[i] = chip8_fontset[i];
	}

	// Reset timers
	delay_timer = 0;
	sound_timer = 0;
}

bool chip8::load(const char gameFileName[])
{
	// open file in binary format and move to end of it with ::ate
	FILE* gameFileP;
	errno_t err = fopen_s(&gameFileP, gameFileName, "rb");
	if (err != 0) {
		printf("file open error \n");
		return false;
	}
	if (gameFileP != nullptr)
	{
		// get size of file
		fseek(gameFileP, 0, SEEK_END);
		long sizeI = ftell(gameFileP);
		rewind(gameFileP);
		printf("filesize of loaded game: %d\n", (int)sizeI);

		// Create buffer
		char* buffer = (char*)malloc(sizeof(char) * sizeI);

		// copy file to buffer
		size_t bufferSize = fread(buffer, 1, sizeI, gameFileP);

		// Read file, 0x200 is default gamefile's storing starting pos
		for (int i = 0; i < sizeI; i++)
		{
			memory[i + 512] = buffer[i];
		}
		free(buffer);
	}
	else {
		printf("error reading file \n");
		return false;
	}

	// Close file
	fclose(gameFileP);
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
		switch (opcode & 0x000F)
		{
		// 0x00E0, clear display
		case 0x0000:
			disp_clear();
			drawFlag = true;
			pc += 2;
			break;

			// 0x00EE, return from subroutine
		case 0x000e:
			--sp;
			pc = stack[sp];
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
			pc += 2;
		}

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
				R[0xF] = 0;
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
			pc += 2;
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
			pc += 2;
		}
	case 0xF000:
		switch (opcode & 0x00FF)
		{
		// fx07
		case 0x0007:
			R[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

			// fx0a
		case 0x000A:
			printf("not made yet %X\n", opcode);
			pc += 2;
			break;

			// fx15
		case 0x0015:
			delay_timer = R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		// fx18
		case 0x0018:
			R[(opcode & 0x0F00) >> 8] = sound_timer;
			pc += 2;
			break;

		// fx1e
		case 0x001E:
			I += R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		// fx29
		case 0x0029:
			printf("not made yet %X\n", opcode);
			pc += 2;
			break;

		// fx33
		case 0x0033:
			tempInt = R[(opcode & 0x0F00) >> 8];
			memory[I] = tempInt / 100;
			memory[I + 1] = (tempInt - (int(memory[I]) * 100)) / 10;
			memory[I + 2] = tempInt % 10;
			pc += 2;
			break;

			// fx55
		case 0x0055:
			tempInt = (opcode & 0x0F00) >> 8;

			for (int j = 0; j <= tempInt; j++) {
				memory[I + j] = R[j];
			}
			pc += 2;
			break;

		// fx65
		case 0x0065:
			tempInt = (opcode & 0x0F00) >> 8;

			for (int j = 0; j <= tempInt; j++) {
				R[j] = memory[I + j];
			}
			pc += 2;
			break;

		default:
			printf("opcode not found [0xF000] %X\n", opcode);
			pc += 2;
		}
		break;


	default:
		printf("Unknown opcode 0x%X\n", opcode);
		pc += 2;
	}


	// Update timers
	if (delay_timer > 0) {
		delay_timer--;
	}
	if (sound_timer > 0) {
		sound_timer--;
	}

}

void chip8::disp_clear()
{
	for (unsigned short i = 0; i < GRAPHICS_SIZE; i++)
	{
		gfx[i] = 0;
	}
}
