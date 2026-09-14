# CHIP-8 Emulator

CHIP-8 Emulator written in C.\
Fully functional including graphics, input and sound using SDL.

## Building
### Requirements
- C Compiler
- CMake 3.10+
- SDL3

### Build Instructions

Clone the repository:
```
git clone https://github.com/ihsanfz/chip8-emu.git
cd chip8-emu
```

Build the project:
```
cmake -S . -B build
cmake --build build
cd build
```

Run the emulator (provide your own rom files):
```
./chip8 /path_to_rom/rom.ch8
```

## Screenshots

![Example](screenshots/pong.png "Example")


## References
https://tobiasvl.github.io/blog/write-a-chip-8-emulator/

https://en.wikipedia.org/wiki/CHIP-8
