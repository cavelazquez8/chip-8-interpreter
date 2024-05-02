#ifndef CHIP8_H
#define CHIP8_H

#include <cstdint>

class Chip8 {
public:
  Chip8();

  std::uint8_t frameBuffer[64 * 32];
  std::uint8_t keyboard[16];

  void init();
  void emulateCycle();

  // Setters
  void setMemory(std::uint16_t address, std::uint8_t value);
  void setProgramCounter(std::uint16_t address);
  void setStack(std::uint8_t subroutine, std::uint16_t address);
  void setStackPointer(std::uint8_t subroutine);
  void setRegisterAt(std::uint8_t reg, std::uint8_t value);
  void setDelayTimer(std::uint8_t value);
  // Getters
  std::uint16_t getIndexRegister();
  std::uint8_t getStackPointer();
  std::uint16_t getStackAt(std::uint8_t subroutine);
  std::uint16_t getProgramCounter();
  std::uint8_t getRegisterAt(std::uint8_t reg);
  bool getDrawFlag();
  std::uint8_t getDelayTimer();
  std::uint8_t getSoundTimer();

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
  bool drawFlag;
};
#endif
