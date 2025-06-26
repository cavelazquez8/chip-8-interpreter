# CHIP-8 Interpreter API Documentation

## Overview

This document describes the public API of the CHIP-8 interpreter. The library provides a straightforward C++ interface for emulating CHIP-8 programs with basic error handling.

## Core Classes

### `Chip8`

The main emulator class that provides a complete CHIP-8 virtual machine implementation.

#### Constants

```cpp
static constexpr std::uint16_t MEMORY_SIZE = 4096;
static constexpr std::uint16_t REGISTER_COUNT = 16;
static constexpr std::uint16_t STACK_SIZE = 16;
static constexpr std::uint16_t DISPLAY_WIDTH = 64;
static constexpr std::uint16_t DISPLAY_HEIGHT = 32;
static constexpr std::uint16_t DISPLAY_SIZE = DISPLAY_WIDTH * DISPLAY_HEIGHT;
static constexpr std::uint16_t KEYBOARD_SIZE = 16;
static constexpr std::uint16_t ROM_START_ADDRESS = 0x200;
static constexpr std::uint16_t FONT_SET_SIZE = 80;
```

#### Constructor

```cpp
Chip8();                                    // Initialize emulator
```

#### Core Operations

```cpp
// Initialize emulator to default state
void init();

// Load ROM from file
bool loadRom(const std::string &path);

// Execute one instruction cycle
void emulateCycle();
```

#### Display Operations

```cpp
// Get read-only access to frame buffer
const std::array<std::uint8_t, DISPLAY_SIZE>& getFrameBuffer() const;

// Set/get individual pixel values
void setPixel(std::uint16_t x, std::uint16_t y, std::uint8_t value);
std::uint8_t getPixel(std::uint16_t x, std::uint16_t y) const;

// Check if screen needs redrawing
bool getDrawFlag() const;

// Set draw flag state
void setDrawFlag(bool condition);
```

#### Input Operations

```cpp
// Check if a key is currently pressed
bool isKeyPressed(std::uint8_t key) const;

// Set key state (pressed/released)
void setKeyState(std::uint8_t key, bool pressed);
```

#### State Access

```cpp
// Memory operations
void setMemory(std::uint16_t address, std::uint8_t value);
std::uint8_t getMemoryAt(std::uint16_t address) const;

// Register operations
void setRegisterAt(std::uint8_t reg, std::uint8_t value);
std::uint8_t getRegisterAt(std::uint8_t reg) const;

// Program counter operations
void setProgramCounter(std::uint16_t address);
std::uint16_t getProgramCounter() const;

// Index register operations
void setIndexRegister(std::uint16_t value);
std::uint16_t getIndexRegister() const;

// Stack operations
void setStack(std::uint8_t subroutine, std::uint16_t address);
std::uint16_t getStackAt(std::uint8_t subroutine) const;
void setStackPointer(std::uint8_t subroutine);
std::uint8_t getStackPointer() const;

// Timer operations
void setDelayTimer(std::uint8_t value);
std::uint8_t getDelayTimer() const;
std::uint8_t getSoundTimer() const;
```

### Error Handling

#### `Chip8::ErrorCode`

Represents errors that can occur during emulation.

```cpp
enum class ErrorCode {
    None = 0,
    StackOverflow,          // Stack operation would overflow
    StackUnderflow,         // Stack operation would underflow
    InvalidMemoryAccess,    // Memory address out of bounds
    InvalidRegisterAccess,  // Register index out of bounds
    UnknownOpcode          // Unrecognized instruction
};
```

#### Error Methods

```cpp
// Get the last error that occurred
ErrorCode getLastError() const;

// Get human-readable error message
const std::string& getLastErrorMessage() const;
```

## Usage Examples

### Basic Emulation Loop

```cpp
#include "chip8.h"
#include <iostream>
#include <thread>
#include <chrono>

int main() {
    Chip8 emulator;
    emulator.init();
    
    // Load ROM
    if (!emulator.loadRom("game.ch8")) {
        std::cerr << "Failed to load ROM: " << emulator.getLastErrorMessage() << std::endl;
        return 1;
    }
    
    // Main emulation loop
    while (true) {
        emulator.emulateCycle();
        
        if (emulator.getDrawFlag()) {
            auto frameBuffer = emulator.getFrameBuffer();
            // Render frameBuffer to screen
            renderDisplay(frameBuffer);
            emulator.setDrawFlag(false);
        }
        
        // Handle input
        updateKeyStates(emulator);
        
        // 60 Hz timing
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
```

### Error Handling

```cpp
void safeEmulationStep(Chip8& emulator) {
    emulator.emulateCycle();
    
    if (emulator.getLastError() != Chip8::ErrorCode::None) {
        std::cerr << "Emulation error: " << emulator.getLastErrorMessage() << std::endl;
        // Handle error appropriately
    }
}
```

### Key Mapping

```cpp
void handleKeyEvent(Chip8& emulator, int sdlKey, bool pressed) {
    static const std::unordered_map<int, std::uint8_t> keyMap = {
        {SDLK_1, 0x1}, {SDLK_2, 0x2}, {SDLK_3, 0x3}, {SDLK_4, 0xC},
        {SDLK_q, 0x4}, {SDLK_w, 0x5}, {SDLK_e, 0x6}, {SDLK_r, 0xD},
        {SDLK_a, 0x7}, {SDLK_s, 0x8}, {SDLK_d, 0x9}, {SDLK_f, 0xE},
        {SDLK_z, 0xA}, {SDLK_x, 0x0}, {SDLK_c, 0xB}, {SDLK_v, 0xF}
    };
    
    auto it = keyMap.find(sdlKey);
    if (it != keyMap.end()) {
        emulator.setKeyState(it->second, pressed);
    }
}
```

## Thread Safety

The `Chip8` class is **not thread-safe**. If you need to access the emulator from multiple threads, you must provide your own synchronization.

## Performance Considerations

- The emulator is designed for straightforward implementation
- Frame buffer access returns a reference to the internal array
- Error handling uses simple error codes to avoid exceptions
- Memory operations include basic bounds checking

## CHIP-8 Instruction Set

The emulator implements the complete original CHIP-8 instruction set:

- **0x0nnn**: System call (ignored)
- **0x00E0**: Clear screen
- **0x00EE**: Return from subroutine
- **0x1nnn**: Jump to address nnn
- **0x2nnn**: Call subroutine at nnn
- **0x3xnn**: Skip if Vx == nn
- **0x4xnn**: Skip if Vx != nn
- **0x5xy0**: Skip if Vx == Vy
- **0x6xnn**: Set Vx = nn
- **0x7xnn**: Add nn to Vx
- **0x8xy0**: Set Vx = Vy
- **0x8xy1**: Set Vx = Vx OR Vy
- **0x8xy2**: Set Vx = Vx AND Vy
- **0x8xy3**: Set Vx = Vx XOR Vy
- **0x8xy4**: Add Vy to Vx (with carry)
- **0x8xy5**: Subtract Vy from Vx (with borrow)
- **0x8xy6**: Shift Vx right by 1
- **0x8xy7**: Set Vx = Vy - Vx (with borrow)
- **0x8xyE**: Shift Vx left by 1
- **0x9xy0**: Skip if Vx != Vy
- **0xAnnn**: Set I = nnn
- **0xBnnn**: Jump to V0 + nnn
- **0xCxnn**: Set Vx = random byte AND nn
- **0xDxyn**: Draw sprite at (Vx, Vy) with height n
- **0xEx9E**: Skip if key Vx is pressed
- **0xExA1**: Skip if key Vx is not pressed
- **0xFx07**: Set Vx = delay timer
- **0xFx0A**: Wait for key press, store in Vx
- **0xFx15**: Set delay timer = Vx
- **0xFx18**: Set sound timer = Vx
- **0xFx1E**: Add Vx to I
- **0xFx29**: Set I = location of sprite for digit Vx
- **0xFx33**: Store BCD representation of Vx
- **0xFx55**: Store V0-Vx in memory starting at I
- **0xFx65**: Load V0-Vx from memory starting at I