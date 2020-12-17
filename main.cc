/*
* CHIP-8 programming language emulator
* 
* Author: JKalliomaki (github)
* 
* This code uses OneLoneCoder's PixelGameEngine for rendering the game. 
* https://github.com/OneLoneCoder/olcPixelGameEngine
*/
#pragma once
#include "olcPixelGameEngine.h"
#include "chip8.h"

// olc::PGE window consisting the game and controls
class myWindow : public olc::PixelGameEngine
{
public:
	myWindow()
	{
		sAppName = "CHIP8 emulator";
	}
	~myWindow()
	{
		delete engine;
	}

public:
	bool OnUserCreate() override
	{
		// Called once at the start, so create things here

		return true;
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		// called once per frame
		for (int x = 0; x < ScreenWidth(); x++)
			for (int y = 0; y < ScreenHeight(); y++)
				Draw(x, y, olc::Pixel(rand() % 255, rand() % 255, rand() % 255));
		return true;
	}

	bool setGame(const char gameFileName[])
	{
		return engine->load(gameFileName);
	}

private:
	chip8* engine = new chip8;
};


// Start the olc::PGE window

int main(int argc, char** argv)
{

	if (argc < 2)
	{
		printf("Usage: myChip8.exe chip8application\n\n");
		return 1;
	}



	myWindow window;
	if (window.Construct(64, 32, 4, 4))
	{
		if (!window.setGame(argv[1])) {
			printf("Game initialization failed");
			return 1;
		}

		window.Start();
	}

	return 0;
}