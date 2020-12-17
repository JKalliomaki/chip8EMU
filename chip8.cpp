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
	// Fetch opcode
	opcode = memory[pc] << 8 | memory[pc + 1];

	// Process opcode
	switch (opcode & 0xF000)
	{
	case 0x0000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x00E0: Clears the screen
			for (int i = 0; i < 2048; ++i)
				gfx[i] = 0x0;
			drawFlag = true;
			pc += 2;
			break;

		case 0x000E: // 0x00EE: Returns from subroutine
			--sp;
			pc = stack[sp];				
			pc += 2;		
			break;

		default:
			printf("Unknown opcode [0x0000]: 0x%X\n", opcode);
		}
		break;

	case 0x1000: // 0x1NNN: Jumps to address NNN
		pc = opcode & 0x0FFF;
		break;

	case 0x2000: // 0x2NNN: Calls subroutine at NNN.
		stack[sp] = pc;	
		++sp;
		pc = opcode & 0x0FFF;
		break;

	case 0x3000: // 0x3XNN: Skips the next instruction if RX equals NN
		if (R[(opcode & 0x0F00) >> 8] == (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x4000: // 0x4XNN: Skips the next instruction if RX doesn't equal NN
		if (R[(opcode & 0x0F00) >> 8] != (opcode & 0x00FF))
			pc += 4;
		else
			pc += 2;
		break;

	case 0x5000: // 0x5XY0: Skips the next instruction if RX equals RY.
		if (R[(opcode & 0x0F00) >> 8] == R[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0x6000: // 0x6XNN: Sets RX to NN.
		R[(opcode & 0x0F00) >> 8] = opcode & 0x00FF;
		pc += 2;
		break;

	case 0x7000: // 0x7XNN: Adds NN to RX.
		R[(opcode & 0x0F00) >> 8] += opcode & 0x00FF;
		pc += 2;
		break;

	case 0x8000:
		switch (opcode & 0x000F)
		{
		case 0x0000: // 0x8XY0: Sets RX to the value of RY
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0001: // 0x8XY1: Sets RX to "RX OR RY"
			R[(opcode & 0x0F00) >> 8] |= R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0002: // 0x8XY2: Sets RX to "RX AND RY"
			R[(opcode & 0x0F00) >> 8] &= R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0003: // 0x8XY3: Sets RX to "RX XOR RY"
			R[(opcode & 0x0F00) >> 8] ^= R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0004: // 0x8XY4: Adds RY to RX. RF is set to 1 when there's a carry, and to 0 when there isn't					
			if (R[(opcode & 0x00F0) >> 4] > (0xFF - R[(opcode & 0x0F00) >> 8]))
				R[0xF] = 1; //carry
			else
				R[0xF] = 0;
			R[(opcode & 0x0F00) >> 8] += R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0005: // 0x8XY5: RY is subtracted from RX. RF is set to 0 when there's a borrow, and 1 when there isn't
			if (R[(opcode & 0x00F0) >> 4] > R[(opcode & 0x0F00) >> 8])
				R[0xF] = 0; // there is a borrow
			else
				R[0xF] = 1;
			R[(opcode & 0x0F00) >> 8] -= R[(opcode & 0x00F0) >> 4];
			pc += 2;
			break;

		case 0x0006: // 0x8XY6: Shifts RX right by one. RF is set to the value of the least significant bit of RX before the shift
			R[0xF] = R[(opcode & 0x0F00) >> 8] & 0x1;
			R[(opcode & 0x0F00) >> 8] >>= 1;
			pc += 2;
			break;

		case 0x0007: // 0x8XY7: Sets RX to RY minus RX. RF is set to 0 when there's a borrow, and 1 when there isn't
			if (R[(opcode & 0x0F00) >> 8] > R[(opcode & 0x00F0) >> 4])	// RY-RX
				R[0xF] = 0; // there is a borrow
			else
				R[0xF] = 1;
			R[(opcode & 0x0F00) >> 8] = R[(opcode & 0x00F0) >> 4] - R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x000E: // 0x8XYE: Shifts RX left by one. RF is set to the value of the most significant bit of RX before the shift
			R[0xF] = R[(opcode & 0x0F00) >> 8] >> 7;
			R[(opcode & 0x0F00) >> 8] <<= 1;
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0x8000]: 0x%X\n", opcode);
		}
		break;

	case 0x9000: // 0x9XY0: Skips the next instruction if RX doesn't equal RY
		if (R[(opcode & 0x0F00) >> 8] != R[(opcode & 0x00F0) >> 4])
			pc += 4;
		else
			pc += 2;
		break;

	case 0xA000: // ANNN: Sets I to the address NNN
		I = opcode & 0x0FFF;
		pc += 2;
		break;

	case 0xB000: // BNNN: Jumps to the address NNN plus R0
		pc = (opcode & 0x0FFF) + R[0];
		break;

	case 0xC000: // CXNN: Sets RX to a random number and NN
		R[(opcode & 0x0F00) >> 8] = (rand() % 0xFF) & (opcode & 0x00FF);
		pc += 2;
		break;

	case 0xD000: // DXYN: Draws a sprite at coordinate (RX, RY) that has a width of 8 pixels and a height of N pixels. 
	{
		unsigned short x = R[(opcode & 0x0F00) >> 8];
		unsigned short y = R[(opcode & 0x00F0) >> 4];
		unsigned short height = opcode & 0x000F;
		unsigned short pixel;

		R[0xF] = 0;
		for (int yline = 0; yline < height; yline++)
		{
			pixel = memory[I + yline];
			for (int xline = 0; xline < 8; xline++)
			{
				if ((pixel & (0x80 >> xline)) != 0)
				{
					if (gfx[(x + xline + ((y + yline) * 64))] == 1)
					{
						R[0xF] = 1;
					}
					gfx[x + xline + ((y + yline) * 64)] ^= 1;
				}
			}
		}

		drawFlag = true;
		pc += 2;
	}
	break;

	case 0xE000:
		switch (opcode & 0x00FF)
		{
		case 0x009E: // EX9E: Skips the next instruction if the key stored in RX is pressed
			if (key[R[(opcode & 0x0F00) >> 8]] != 0)
				pc += 4;
			else
				pc += 2;
			break;

		case 0x00A1: // EXA1: Skips the next instruction if the key stored in RX isn't pressed
			if (key[R[(opcode & 0x0F00) >> 8]] == 0)
				pc += 4;
			else
				pc += 2;
			break;

		default:
			printf("Unknown opcode [0xE000]: 0x%X\n", opcode);
		}
		break;

	case 0xF000:
		switch (opcode & 0x00FF)
		{
		case 0x0007: // FX07: Sets RX to the value of the delay timer
			R[(opcode & 0x0F00) >> 8] = delay_timer;
			pc += 2;
			break;

		case 0x000A: // FX0A: A key press is awaited, and then stored in RX		
		{
			bool keyPress = false;

			for (int i = 0; i < 16; ++i)
			{
				if (key[i] != 0)
				{
					R[(opcode & 0x0F00) >> 8] = i;
					keyPress = true;
				}
			}

			// If we didn't received a keypress, skip this cycle and try again.
			if (!keyPress)
				return;

			pc += 2;
		}
		break;

		case 0x0015: // FX15: Sets the delay timer to RX
			delay_timer = R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0018: // FX18: Sets the sound timer to RX
			sound_timer = R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x001E: // FX1E: Adds RX to I
			if (I + R[(opcode & 0x0F00) >> 8] > 0xFFF)	// RF is set to 1 when range overflow (I+RX>0xFFF), and 0 when there isn't.
				R[0xF] = 1;
			else
				R[0xF] = 0;
			I += R[(opcode & 0x0F00) >> 8];
			pc += 2;
			break;

		case 0x0029: // FX29: Sets I to the location of the sprite for the character in RX. Characters 0-F (in hexadecimal) are represented by a 4x5 font
			I = R[(opcode & 0x0F00) >> 8] * 0x5;
			pc += 2;
			break;

		case 0x0033: // FX33: Stores the Binary-coded decimal representation of RX at the addresses I, I plus 1, and I plus 2
			memory[I] = R[(opcode & 0x0F00) >> 8] / 100;
			memory[I + 1] = (R[(opcode & 0x0F00) >> 8] / 10) % 10;
			memory[I + 2] = (R[(opcode & 0x0F00) >> 8] % 100) % 10;
			pc += 2;
			break;

		case 0x0055: // FX55: Stores R0 to RX in memory starting at address I					
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				memory[I + i] = R[i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		case 0x0065: // FX65: Fills R0 to RX with values from memory starting at address I					
			for (int i = 0; i <= ((opcode & 0x0F00) >> 8); ++i)
				R[i] = memory[I + i];

			// On the original interpreter, when the operation is done, I = I + X + 1.
			I += ((opcode & 0x0F00) >> 8) + 1;
			pc += 2;
			break;

		default:
			printf("Unknown opcode [0xF000]: 0x%X\n", opcode);
		}
		break;

	default:
		printf("Unknown opcode: 0x%X\n", opcode);
	}

	// Update timers
	if (delay_timer > 0)
		--delay_timer;

	if (sound_timer > 0)
	{
		if (sound_timer == 1)
			printf("BEEP!\n");
		--sound_timer;
	}

}

void chip8::disp_clear()
{
	for (unsigned short i = 0; i < GRAPHICS_SIZE; i++)
	{
		gfx[i] = 0;
	}
}
