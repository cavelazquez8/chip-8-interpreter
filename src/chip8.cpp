#include "chip8.h"

#include "random.h"
#include <cstdint>
#include <stdio.h>

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

  case 0x1000:
    programCounter = opcode & 0x0FFF;
    break;
  case 0x2000:
    stack[stackPointer] = programCounter;
    stackPointer++;
    programCounter = opcode & 0x0FFF;
    break;
  case 0x3000: {
    std::uint8_t x = (opcode & 0x0F00) >> 8;
    std::uint8_t nn = opcode & 0x00FF;
    if (registers[x] == nn) {
      programCounter += 4;
    } else {
      programCounter += 2;
    }
  } break;
  case 0x4000: {
    std::uint8_t x = (opcode & 0x0F00) >> 8;
    std::uint8_t nn = opcode & 0x00FF;
    if (registers[x] != nn) {
      programCounter += 4;
    } else {
      programCounter += 2;
    }
  } break;
  case 0x5000: {
    std::uint8_t x = (opcode & 0x0F00) >> 8;
    std::uint8_t y = (opcode & 0x00F0) >> 4;
    if (registers[x] == registers[y]) {
      programCounter += 4;
    } else {
      programCounter += 2;
    }
  } break;

  case 0x6000: {
    std::uint8_t Vx = (opcode & 0x0F00) >> 8;
    std::uint8_t NN = (opcode & 0x00FF);

    registers[Vx] = NN;

    programCounter += 2;
  } break;
  case 0x7000: {
    std::uint8_t Vx = (opcode & 0x0F00) >> 8;
    std::uint8_t NN = (opcode & 0x00FF);

    registers[Vx] += NN;

    programCounter += 2;

  } break;

  case 0x8000:
    switch (opcode & 0x000F) {
    // 0x8XY0
    case 0x0000: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      registers[Vx] = registers[Vy];

      programCounter += 2;
    } break;

    case 0x0001: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      registers[Vx] |= registers[Vy];

      programCounter += 2;
    } break;
    case 0x0002: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      registers[Vx] &= registers[Vy];

      programCounter += 2;
    } break;
    case 0x0003: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      registers[Vx] ^= registers[Vy];

      programCounter += 2;
    } break;
    case 0x0004: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      if (registers[Vy] > 0xFF - registers[Vx]) {
        registers[0xF] = 1;
      } else {
        registers[0xF] = 0;
      }

      registers[Vx] += registers[Vy];

      programCounter += 2;
    } break;
    case 0x0005: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      if (registers[Vx] < registers[Vy]) {
        registers[0xF] = 0;
      } else {
        registers[0xF] = 1;
      }

      registers[Vx] -= registers[Vy];

      programCounter += 2;
    } break;
    case 0x0006: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      registers[0xF] = registers[Vx] & 1;

      registers[Vx] >>= 1;

      programCounter += 2;
    } break;
    case 0x0007: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      if (registers[Vx] > registers[Vy]) {
        registers[0xF] = 0;
      } else {
        registers[0xF] = 1;
      }

      registers[Vx] = registers[Vy] - registers[Vx];

      programCounter += 2;
    } break;

    case 0x000E: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      std::uint8_t Vy = (opcode & 0x00F0) >> 4;

      registers[0xF] = registers[Vx] >> 7;

      registers[Vx] <<= 1;

      programCounter += 2;
    } break;
    default:
      printf("Unknown Opcode: %X", opcode);
    }
  case 0x9000: {
    std::uint8_t Vx = (opcode & 0x0F00) >> 8;
    std::uint8_t Vy = (opcode & 0x00F0) >> 4;

    if (registers[Vx] != registers[Vy]) {
      programCounter += 4;
    } else {
      programCounter += 2;
    }
  } break;

  case 0xA000: // ANNN: Sets I to the address NNN
    indexRegister = opcode & 0x0FFF;
    programCounter += 2;
    break;

  case 0xB000:
    programCounter = registers[0] + (opcode & 0x0FFF);
    break;
  case 0xC000: {
    std::uint8_t Vx = (opcode & 0x0F00) >> 8;
    std::uint8_t NN = opcode & 0x00FF;
    std::uint8_t randNum = Random::get<uint8_t>(0, 255);

    registers[Vx] = randNum & NN;

    programCounter += 2;
  } break;

  case 0xD000: {
    std::uint8_t Vx = (opcode & 0x0F00) >> 8;
    std::uint8_t Vy = (opcode & 0x00F0) >> 4;
    std::uint8_t N = opcode & 0x000F;
    std::uint8_t pixel{};

    registers[0xF] = 0;

    for (int yline = 0; yline < N; ++yline) {
      pixel = memory[indexRegister + yline];

      for (int xline = 0; xline < 8; ++xline) {
        if ((pixel & (0x80 >> xline)) != 0) {

          if (frameBuffer[((Vx + xline) + ((Vy + yline) * 64))] == 1) {

            registers[0xF] = 1;
          }
        }
        frameBuffer[(yline + Vy) * 64 + (xline + Vx)] ^= 1;
      }
    }

    drawFlag = true;
    programCounter += 2;

  } break;

  case 0xE000:
    switch (opcode & 0x000F) {
    case 0x000E: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;

      if (keyboard[Vx] != 0) {
        programCounter += 4;
      } else {
        programCounter += 2;
      }
    } break;

    case 0x0001: {

      std::uint8_t Vx = (opcode & 0x0F00) >> 8;

      if (keyboard[Vx] == 0) {
        programCounter += 4;
      } else {
        programCounter += 2;
      }
    } break;
    default:
      printf("Unknown Opcode: %X", opcode);
      break;
    }
    default:
      printf("Unknown Opcode: %X", opcode);
      break;
    }
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
void Chip8::setRegisterAt(std::uint8_t reg, std::uint8_t value) {
  registers[reg] = value;
}

// Getters
std::uint16_t Chip8::getIndexRegister() { return indexRegister; }
std::uint8_t Chip8::getStackPointer() { return stackPointer; }
std::uint16_t Chip8::getStackAt(std::uint8_t subroutine) {
  return stack[subroutine];
}
std::uint16_t Chip8::getProgramCounter() { return programCounter; }
std::uint8_t Chip8::getRegisterAt(std::uint8_t reg) { return registers[reg]; }
bool Chip8::getDrawFlag() { return drawFlag; }
