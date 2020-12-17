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
		// Handle key presses
		if (GetKey(olc::Key::K1).bPressed) engine->key[0x1] = 1;
		if (GetKey(olc::Key::K2).bPressed) engine->key[0x2] = 1;
		if (GetKey(olc::Key::K3).bPressed) engine->key[0x3] = 1;
		if (GetKey(olc::Key::K4).bPressed) engine->key[0xC] = 1;
		if (GetKey(olc::Key::Q).bPressed) engine->key[0x4] = 1;
		if (GetKey(olc::Key::W).bPressed) engine->key[0x5] = 1;
		if (GetKey(olc::Key::E).bPressed) engine->key[0x6] = 1;
		if (GetKey(olc::Key::R).bPressed) engine->key[0xD] = 1;
		if (GetKey(olc::Key::A).bPressed) engine->key[0x7] = 1;
		if (GetKey(olc::Key::S).bPressed) engine->key[0x8] = 1;
		if (GetKey(olc::Key::D).bPressed) engine->key[0x9] = 1;
		if (GetKey(olc::Key::F).bPressed) engine->key[0xE] = 1;
		if (GetKey(olc::Key::Z).bPressed) engine->key[0xA] = 1;
		if (GetKey(olc::Key::X).bPressed) engine->key[0x0] = 1;
		if (GetKey(olc::Key::C).bPressed) engine->key[0xB] = 1;
		if (GetKey(olc::Key::V).bPressed) engine->key[0xF] = 1;



		// Lock emulator's framerate
		fAccumulatedTime += fElapsedTime;
		if (fAccumulatedTime >= fTargetFrameTime)
		{
			fAccumulatedTime -= fTargetFrameTime;
			fElapsedTime = fTargetFrameTime;
		}
		else {
			return true;
		}

		
		// Do the emulating
		engine->emulateCycle();
		if (engine->drawFlag)
		{
			for (int y = 0; y < 32; y++)
			{
				for (int x = 0; x < 64; x++)
				{
					if (engine->gfx[y * 64 + x])
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