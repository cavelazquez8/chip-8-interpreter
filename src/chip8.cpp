#include "chip8.h"

#include <cstdint>
#include <stdexcept>
#include <stdio.h>

// Getters
std::uint16_t Chip8::getIndexRegister() { return indexRegister; }

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

void Chip8::setMem(std::uint16_t address, std::uint8_t value) {
  if (address < 0 || address >= 4096) {
    throw std::out_of_range("Address is out of bounds");
  }
  memory[address] = value;
}
