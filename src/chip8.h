#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>

class Chip8 {
public:
  Chip8();

  std::uint8_t frameBuffer[2048];
  std::uint8_t keyboard[16];

  void init();

private:
  std::uint8_t memory[4096];

  std::uint8_t registers[16];
  std::uint16_t indexRegister;

  std::uint16_t stack[16];
  std::uint8_t stackPointer;

  std::uint8_t delayTimer;
  std::uint8_t soundTimer;

  std::uint16_t programCounter;

  std::uint16_t opcode;
};
#endif
