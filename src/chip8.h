#ifndef CHIP8_H
#define CHIP8_H

#include <array>
#include <cstdint>
#include <iostream>
#include <string>

class Chip8 {
public:
  static constexpr std::uint16_t MEMORY_SIZE = 4096;
  static constexpr std::uint16_t REGISTER_COUNT = 16;
  static constexpr std::uint16_t STACK_SIZE = 16;
  static constexpr std::uint16_t DISPLAY_WIDTH = 64;
  static constexpr std::uint16_t DISPLAY_HEIGHT = 32;
  static constexpr std::uint16_t DISPLAY_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;
  static constexpr std::uint16_t KEYBOARD_SIZE = 16;
  static constexpr std::uint16_t ROM_START_ADDRESS = 0x200;
  static constexpr std::uint16_t FONT_SET_SIZE = 80;

  Chip8();

  bool loadRom(const std::string &path);
  void init();
  void emulateCycle();
  
  // Frame buffer access
  const std::array<std::uint8_t, DISPLAY_SIZE>& getFrameBuffer() const;
  void setPixel(std::uint16_t x, std::uint16_t y, std::uint8_t value);
  std::uint8_t getPixel(std::uint16_t x, std::uint16_t y) const;
  
  // Keyboard access
  void setKeyState(std::uint8_t key, bool pressed);
  bool isKeyPressed(std::uint8_t key) const;
  
  // Error handling
  enum class ErrorCode {
    None = 0,
    StackOverflow,
    StackUnderflow,
    InvalidMemoryAccess,
    InvalidRegisterAccess,
    UnknownOpcode
  };
  
  ErrorCode getLastError() const;
  const std::string& getLastErrorMessage() const;
  // Setters
  void setMemory(std::uint16_t address, std::uint8_t value);
  void setProgramCounter(std::uint16_t address);
  void setStack(std::uint8_t subroutine, std::uint16_t address);
  void setStackPointer(std::uint8_t subroutine);
  void setRegisterAt(std::uint8_t reg, std::uint8_t value);
  void setDelayTimer(std::uint8_t value);
  void setDrawFlag(bool condition);
  void setIndexRegister(std::uint16_t value);
  // Getters
  std::uint8_t getMemoryAt(std::uint8_t address) const;
  std::uint16_t getIndexRegister() const;
  std::uint16_t getProgramCounter() const;
  std::uint16_t getStackAt(std::uint8_t subroutine) const;
  std::uint8_t getStackPointer() const;
  std::uint8_t getRegisterAt(std::uint8_t reg) const;
  std::uint8_t getDelayTimer() const;
  std::uint8_t getSoundTimer() const;
  bool getDrawFlag() const;

private:
  // Core emulator state
  std::array<std::uint8_t, MEMORY_SIZE> memory_;
  std::array<std::uint8_t, REGISTER_COUNT> registers_;
  std::array<std::uint16_t, STACK_SIZE> stack_;
  std::array<std::uint8_t, DISPLAY_SIZE> frameBuffer_;
  std::array<std::uint8_t, KEYBOARD_SIZE> keyboard_;
  
  std::uint16_t indexRegister_;
  std::uint8_t stackPointer_;
  std::uint8_t delayTimer_;
  std::uint8_t soundTimer_;
  std::uint16_t programCounter_;
  std::uint16_t opcode_;
  bool drawFlag_;
  
  // Error handling
  ErrorCode lastError_;
  std::string lastErrorMessage_;
  
  // Opcode handler methods
  void handleOpcode0xxx();
  void handleOpcode1xxx();
  void handleOpcode2xxx();
  void handleOpcode3xxx();
  void handleOpcode4xxx();
  void handleOpcode5xxx();
  void handleOpcode6xxx();
  void handleOpcode7xxx();
  void handleOpcode8xxx();
  void handleOpcode9xxx();
  void handleOpcodeAxxx();
  void handleOpcodeBxxx();
  void handleOpcodeCxxx();
  void handleOpcodeDxxx();
  void handleOpcodeExxx();
  void handleOpcodeFxxx();
  
  // Utility methods
  void setError(ErrorCode error, const std::string& message);
  void clearError();
  bool isValidMemoryAddress(std::uint16_t address) const;
  bool isValidRegisterIndex(std::uint8_t index) const;
  void logError(const std::string& message) const;
  void logWarning(const std::string& message) const;
  void logInfo(const std::string& message) const;
};
#endif
