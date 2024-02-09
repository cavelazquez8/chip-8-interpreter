#include "chip8.h"

#include <cstdint>

std::uint8_t fonts[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
    0x90, 0x90, 0xF0, 0x10, 0x10, // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
    0xF0, 0x10, 0x20, 0x40, 0x40, // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90, // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
    0xF0, 0x80, 0x80, 0x80, 0xF0, // C
    0xE0, 0x90, 0x90, 0x90, 0xE0, // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
    0xF0, 0x80, 0xF0, 0x80, 0x80, // F
};

void Chip8::init() {
  programCounter = 0x200;
  opcode = 0;
  indexRegister = 0;
  stackPointer = 0;
  // Reset framebuffer
  for (int i = 0; i < 2048; ++i) {
    frameBuffer[i] = 0;
  }
  // Reset stack
  for (int i = 0; i < 16; ++i) {
    stack[i] = 0;
    keyboard[i] = 0;
    registers[i] = 0;
  }
  // Reset memory
  for (int i = 0; i < 4096; ++i) {
    memory[i] = 0;
  }

  // Load font set onto memory
  for (int i = 0; i < 80; ++i) {
    memory[i] = fonts[i];
  }

  delayTimer = 0;
  soundTimer = 0;
}
