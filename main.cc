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

class myWindow : public olc::PixelGameEngine
{
public:
	myWindow()
	{
		sAppName = "CHIP8 emulator";
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

private:
	chip8* engine = new chip8;
};

int main()
{

	myWindow window;
	if (window.Construct(64, 32, 4, 4))
	{
		window.Start();
	}

	return 0;
}