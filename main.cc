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
		
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		else {
			return true;
		}

		

		engine->emulateCycle();
		if (engine->drawFlag)
		{
			for (int y = 0; y < 32; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (engine->gfx[y * 32 + x])
					{
						Draw(x, y, olc::DARK_GREY);
					}
					else {
						Draw(x, y, olc::WHITE);
					}
				}
			}
			engine->drawFlag = false;
		}
		return true;

	}

	bool setGame(const char gameFileName[])
	{
		return engine->load(gameFileName);
	}

private:
	chip8* engine = new chip8;

	float fTargetFrameTime = 1.0f / 60.0f; // Virtual FPS of 60fps
	float fAccumulatedTime = 0.0f;
};


// Start the olc::PGE window

int main(int argc, char** argv)
{
	if (argc < 2)
	{
		printf("Usage: myChip8.exe chip8application\n");
	}

	myWindow window;
	if (window.Construct(64, 32, 4, 4))
	{
		if (argv[1]) {
			window.setGame(argv[1]);
		}
		else {
			printf("Game initialization failed, trying default game\n");
			window.setGame("../pong2.c8");
		}

		window.Start();
	}

	return 0;
}