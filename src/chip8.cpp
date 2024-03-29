#include "chip8.h"

#include <cstdint>
#include <stdio.h>

std::uint8_t fonts[80] = {
    0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
    0x20, 0x60, 0x20, 0x20, 0x70, // 1
    0xF,  0x10, 0xF0, 0x80, 0xF0, // 2
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

Chip8::Chip8() { init(); }

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

void Chip8::emulateCycle() {
  // Fetch Opcode
  opcode = memory[programCounter] << 8 | memory[programCounter + 1];
  // Decode Opcode

  switch (opcode & 0xF000) {

  case 0x0000:
    switch (opcode & 0x000F) {
    case 0x0000: // Clears the screen

      for (int i = 0; i < 64 * 32; ++i) {
        frameBuffer[i] = 0;
      }

      drawFlag = true;

      programCounter += 2;

      break;

    case 0x000E: // Returns from a subroutine
      stackPointer--;
      programCounter = stack[stackPointer];
      programCounter += 2;
      break;

    default:
      printf("Unknown Opcode: %X", opcode);
    }
    break;

  case 0xA000: // ANNN: Sets I to the address NNN
    indexRegister = opcode & 0x0FFF;
    programCounter += 2;
    break;

  default:
    printf("Unknown Opcode: %X", opcode);
  }

  if (delayTimer > 0) {
    --delayTimer;
  }
  if (soundTimer > 0) {
    // Beep
    --soundTimer;
  }
}

// Setters
void Chip8::setMemory(std::uint16_t address, std::uint8_t value) {
  memory[address] = value;
}
void Chip8::setProgramCounter(std::uint16_t address) {
  programCounter = address;
}
void Chip8::setStack(std::uint8_t subroutine, std::uint16_t address) {
  stack[subroutine] = address;
}
void Chip8::setStackPointer(std::uint8_t subroutine) {
  stackPointer = subroutine;
}

// Getters
std::uint16_t Chip8::getIndexRegister() { return indexRegister; }
std::uint8_t Chip8::getStackPointer() { return stackPointer; }
std::uint16_t Chip8::getProgramCounter() { return programCounter; }

bool Chip8::getDrawFlag() { return drawFlag; }
