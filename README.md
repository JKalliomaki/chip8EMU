# Chip8 emulator
chip8 emulator with basic idea gotten from [here.](http://www.multigesture.net/articles/how-to-write-an-emulator-chip-8-interpreter/) Did this to experiment and learn more about bitwise operations and to learn basics of olc::PGE to maybe make more out of it in future.

Rendering of the game is done with [oneLoneCoder's PixelGameEngine.](https://github.com/OneLoneCoder/olcPixelGameEngine)

Screenshot of the emulator in usage:

![Screenshot of the game](https://github.com/JKalliomaki/chip8EMU/blob/master/ScreenCapOfPong.png)

To play you need to build the game files using Visual Studio
Then run it from terminal with 
```
Debug/chip8EMU.exe *name of the game*
```
for example 
```
Debug/chip8EMU.exe pong2.c8
```

Keypad on the keyboard is 

1 2 3 4

q w e r

a s d f

z x c v

### In progress:
* controls need tweaking
* Audio output
* Cleaning of emulating cycle (maybe function pointers)
* Super Chip-8 integration
