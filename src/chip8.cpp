#include "chip8.h"

#include "random.h"
#include <cstdint>
#include <fstream>
#include <ios>
#include <iostream>
#include <stdio.h>
#include <vector>

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

bool Chip8::loadRom(const std::string &path) {

  std::ifstream file(path, std::ios::binary | std::ios::ate);

  if (!file.is_open()) {
    std::cerr << "Failed to open ROM: " << path << std::endl;
    return false;
  }

  std::streamsize size = file.tellg();

  if (size <= 0 || size > (4096 - 0x200)) {
    std::cerr << "Rom size is invalid or too large: " << size << "bytes"
              << std::endl;
    return false;
  }

  file.seekg(0, std::ios::beg);

  std::vector<std::uint8_t> buffer(size);

  if (!file.read(reinterpret_cast<char *>(buffer.data()), size)) {
    std::cerr << "Failed to read ROM: " << path << std::endl;
    return false;
  }

  std::copy(buffer.begin(), buffer.end(), memory + 0x200);
  return true;
}
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
    case 0x0000: { // 0x8XY0
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
    break;

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
    std::uint8_t Vx = registers[(opcode & 0x0F00) >> 8];
    std::uint8_t Vy = registers[(opcode & 0x00F0) >> 4];
    std::uint8_t N = opcode & 0x000F;
    std::uint8_t pixel;

    registers[0xF] = 0;

    for (int yline = 0; yline < N; ++yline) {
      pixel = memory[indexRegister + yline];
      for (int xline = 0; xline < 8; ++xline) {
        if ((pixel & (0x80 >> xline)) != 0) {
          if (frameBuffer[((Vx + xline) + ((Vy + yline) * 64))] == 1) {
            registers[0xF] = 1;
          }
          frameBuffer[((Vx + xline) + ((Vy + yline) * 64))] ^= 1;
        }
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
    break;
  case 0xF000:
    switch (opcode & 0x00FF) {
    case 0x0007: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      registers[Vx] = delayTimer;
      programCounter += 2;
    } break;
    case 0x000A: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      bool keyPress = false;

      for (int i = 0; i < 16; ++i) {
        if (keyboard[i] != 0) {
          registers[Vx] = i;
          keyPress = true;
        }
      }

      if (!keyPress)
        return;

      programCounter += 2;
    } break;
    case 0x0015: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      delayTimer = registers[Vx];
      programCounter += 2;
    } break;
    case 0x0018: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      soundTimer = registers[Vx];
      programCounter += 2;
    } break;
    case 0x001E: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      indexRegister += registers[Vx];
      programCounter += 2;
    } break;
    case 0x0029: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      indexRegister = registers[Vx] * 0x5;
      programCounter += 2;
    } break;
    case 0x0033: {
      std::uint8_t Vx = registers[(opcode & 0x0F00) >> 8];
      memory[indexRegister] = Vx / 100;
      memory[indexRegister + 1] = (Vx / 10) % 10;
      memory[indexRegister + 2] = Vx % 10;
      programCounter += 2;
    } break;
    case 0x0055: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      for (int i = 0; i <= Vx; ++i) {
        memory[indexRegister + i] = registers[i];
      }
      programCounter += 2;
    } break;
    case 0x0065: {
      std::uint8_t Vx = (opcode & 0x0F00) >> 8;
      for (int i = 0; i <= Vx; ++i) {
        registers[i] = memory[indexRegister + i];
      }
      programCounter += 2;
    } break;
    default:
      printf("Unknown Opcode: %X", opcode);
      break;
    }
    break;

  default:
    printf("Unknown Opcode: %X", opcode);
  }

  if (delayTimer > 0) {
    --delayTimer;
  }
  if (soundTimer > 0) {
    if (soundTimer == 1) {
      printf("Beep!\n");
    }
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
void Chip8::setDelayTimer(std::uint8_t value) { delayTimer = value; }
void Chip8::setDrawFlag(bool condition) { drawFlag = condition; }
void Chip8::setIndexRegister(std::uint16_t value) { indexRegister = value; }

// Getters
std::uint8_t Chip8::getMemoryAt(std::uint8_t address) const {
  return memory[address];
}
std::uint16_t Chip8::getIndexRegister() const { return indexRegister; }
std::uint16_t Chip8::getProgramCounter() const { return programCounter; }
std::uint16_t Chip8::getStackAt(std::uint8_t subroutine) const {
  return stack[subroutine];
}
std::uint8_t Chip8::getStackPointer() const { return stackPointer; }
std::uint8_t Chip8::getRegisterAt(std::uint8_t reg) const {
  return registers[reg];
}
std::uint8_t Chip8::getDelayTimer() const { return delayTimer; }
std::uint8_t Chip8::getSoundTimer() const { return soundTimer; }
bool Chip8::getDrawFlag() const { return drawFlag; }
