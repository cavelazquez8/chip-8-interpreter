# CHIP-8 Interpreter API Documentation

## Overview

This document describes the public API of the CHIP-8 interpreter library. The library provides a modern C++20 interface for emulating CHIP-8 programs with comprehensive error handling and memory safety.

## Core Classes

### `chip8::Chip8`

The main emulator class that provides a complete CHIP-8 virtual machine implementation.

#### Constants

```cpp
static constexpr std::uint16_t MEMORY_SIZE = 4096;
static constexpr std::uint16_t ROM_START_ADDRESS = 0x200;
static constexpr std::uint16_t DISPLAY_WIDTH = 64;
static constexpr std::uint16_t DISPLAY_HEIGHT = 32;
static constexpr std::uint8_t NUM_REGISTERS = 16;
static constexpr std::uint8_t NUM_KEYS = 16;
static constexpr std::uint8_t STACK_SIZE = 16;
```

#### Constructor/Destructor

```cpp
Chip8();                                    // Initialize emulator
~Chip8() = default;                        // Default destructor

// Move semantics supported, copy disabled
Chip8(Chip8&&) = default;
Chip8& operator=(Chip8&&) = default;
Chip8(const Chip8&) = delete;
Chip8& operator=(const Chip8&) = delete;
```

#### Core Operations

```cpp
// Reset emulator to initial state
void reset();

// Load ROM from file
[[nodiscard]] Result<void> loadRom(std::string_view path);

// Execute one instruction cycle
void emulateCycle();
```

#### Display Operations

```cpp
// Get read-only access to frame buffer
[[nodiscard]] std::span<const std::uint8_t> getFrameBuffer() const noexcept;

// Check if screen needs redrawing
[[nodiscard]] bool shouldDraw() const noexcept;

// Clear the draw flag after rendering
void clearDrawFlag() noexcept;
```

#### Input Operations

```cpp
// Check if a key is currently pressed
[[nodiscard]] bool isKeyPressed(std::uint8_t key) const;

// Set key state (pressed/released)
void setKeyState(std::uint8_t key, bool pressed);
```

#### Timer Operations

```cpp
// Get current timer values
[[nodiscard]] std::uint8_t getDelayTimer() const noexcept;
[[nodiscard]] std::uint8_t getSoundTimer() const noexcept;
```

#### Testing Interface

*Note: These methods are primarily intended for testing and debugging.*

```cpp
// Memory operations
[[nodiscard]] Result<void> setMemory(std::uint16_t address, std::uint8_t value);
[[nodiscard]] Result<std::uint8_t> getMemoryAt(std::uint16_t address) const;

// Register operations
[[nodiscard]] Result<void> setRegister(std::uint8_t reg, std::uint8_t value);
[[nodiscard]] Result<std::uint8_t> getRegister(std::uint8_t reg) const;

// Program counter operations
[[nodiscard]] Result<void> setProgramCounter(std::uint16_t address);
[[nodiscard]] std::uint16_t getProgramCounter() const noexcept;

// Index register operations
[[nodiscard]] Result<void> setIndexRegister(std::uint16_t value);
[[nodiscard]] std::uint16_t getIndexRegister() const noexcept;

// Stack operations
[[nodiscard]] Result<void> setStack(std::uint8_t level, std::uint16_t address);
[[nodiscard]] Result<std::uint16_t> getStackAt(std::uint8_t level) const;
[[nodiscard]] Result<void> setStackPointer(std::uint8_t pointer);
[[nodiscard]] std::uint8_t getStackPointer() const noexcept;

// Timer operations
[[nodiscard]] Result<void> setDelayTimer(std::uint8_t value);
void setDrawFlag(bool flag) noexcept;
```

### Error Handling

#### `chip8::EmulatorError`

Represents errors that can occur during emulation.

```cpp
enum class Type {
    InvalidRomSize,     // ROM file is too large or empty
    FileNotFound,       // ROM file cannot be opened
    InvalidAddress,     // Memory address out of bounds
    InvalidRegister,    // Register index out of bounds
    StackOverflow,      // Stack operation would overflow
    StackUnderflow      // Stack operation would underflow
};

EmulatorError(Type type, std::string message);

[[nodiscard]] Type type() const noexcept;
[[nodiscard]] const std::string& message() const noexcept;
```

#### `chip8::Result<T>`

Type alias for `std::expected<T, EmulatorError>` providing monadic error handling.

```cpp
template<typename T>
using Result = std::expected<T, EmulatorError>;
```

## Usage Examples

### Basic Emulation Loop

```cpp
#include "chip8.h"
#include <iostream>

int main() {
    chip8::Chip8 emulator;
    
    // Load ROM
    auto result = emulator.loadRom("game.ch8");
    if (!result) {
        std::cerr << "Failed to load ROM: " << result.error().message() << std::endl;
        return 1;
    }
    
    // Main emulation loop
    while (true) {
        emulator.emulateCycle();
        
        if (emulator.shouldDraw()) {
            auto frameBuffer = emulator.getFrameBuffer();
            // Render frameBuffer to screen
            renderDisplay(frameBuffer);
            emulator.clearDrawFlag();
        }
        
        // Handle input
        updateKeyStates(emulator);
        
        // 60 Hz timing
        std::this_thread::sleep_for(std::chrono::milliseconds(16));
    }
}
```

### Error Handling with Monadic Operations

```cpp
auto setupEmulator = [](const std::string& romPath) -> chip8::Result<chip8::Chip8> {
    chip8::Chip8 emulator;
    
    return emulator.loadRom(romPath)
        .and_then([&](auto) { return emulator.setRegister(0, 42); })
        .and_then([&](auto) { return emulator.setDelayTimer(60); })
        .transform([&](auto) { return std::move(emulator); });
};

auto result = setupEmulator("test.ch8");
if (result) {
    auto emulator = std::move(result.value());
    // Use emulator
} else {
    std::cerr << "Setup failed: " << result.error().message() << std::endl;
}
```

### Key Mapping

```cpp
void handleKeyEvent(chip8::Chip8& emulator, int sdlKey, bool pressed) {
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

- The emulator is designed for high performance with minimal allocations
- Frame buffer access returns a `std::span` for zero-copy access
- Error handling uses `std::expected` to avoid exceptions in hot paths
- Memory operations include bounds checking but are optimized for common cases

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