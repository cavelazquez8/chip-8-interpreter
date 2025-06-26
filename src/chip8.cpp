#include "chip8.h"

#include <algorithm>
#include <cstdint>
#include <fstream>
#include <iomanip>
#include <ios>
#include <iostream>
#include <sstream>
#include <vector>

#include "random.h"

// Helper function to format hex addresses
std::string formatHex(std::uint16_t value) {
    std::stringstream ss;
    ss << "0x" << std::hex << std::uppercase << value;
    return ss.str();
}

constexpr std::array<std::uint8_t, Chip8::FONT_SET_SIZE> FONT_SET = {
    0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
    0x20, 0x60, 0x20, 0x20, 0x70,  // 1
    0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
    0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
    0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
    0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
    0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
    0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
    0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
    0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
    0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
    0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
    0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
    0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
    0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
    0xF0, 0x80, 0xF0, 0x80, 0x80,  // F
};

Chip8::Chip8() : lastError_(ErrorCode::None) { init(); }

bool Chip8::loadRom(const std::string& path) {
    clearError();

    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        setError(ErrorCode::InvalidMemoryAccess, "Failed to open ROM: " + path);
        return false;
    }

    std::streamsize size = file.tellg();
    const std::streamsize maxRomSize = MEMORY_SIZE - ROM_START_ADDRESS;

    if (size <= 0 || size > maxRomSize) {
        setError(ErrorCode::InvalidMemoryAccess,
                 "ROM size invalid or too large: " + std::to_string(size) + " bytes");
        return false;
    }

    file.seekg(0, std::ios::beg);
    std::vector<std::uint8_t> buffer(size);

    if (!file.read(reinterpret_cast<char*>(buffer.data()), size)) {
        setError(ErrorCode::InvalidMemoryAccess, "Failed to read ROM: " + path);
        return false;
    }

    std::copy(buffer.begin(), buffer.end(), memory_.begin() + ROM_START_ADDRESS);
    logInfo("Successfully loaded ROM: " + path + " (" + std::to_string(size) + " bytes)");
    return true;
}
void Chip8::init() {
    programCounter_ = ROM_START_ADDRESS;
    opcode_ = 0;
    indexRegister_ = 0;
    stackPointer_ = 0;
    drawFlag_ = false;
    delayTimer_ = 0;
    soundTimer_ = 0;

    // Clear all arrays
    frameBuffer_.fill(0);
    stack_.fill(0);
    keyboard_.fill(0);
    registers_.fill(0);
    memory_.fill(0);

    // Load font set into memory
    std::copy(FONT_SET.begin(), FONT_SET.end(), memory_.begin());

    clearError();
    logInfo("CHIP-8 emulator initialized");
}

void Chip8::emulateCycle() {
    clearError();

    // Bounds check for program counter
    if (programCounter_ >= MEMORY_SIZE - 1) {
        setError(ErrorCode::InvalidMemoryAccess,
                 "Program counter out of bounds: " + std::to_string(programCounter_));
        return;
    }

    // Fetch opcode
    opcode_ = (memory_[programCounter_] << 8) | memory_[programCounter_ + 1];
    // Decode and execute opcode
    switch (opcode_ & 0xF000) {
        case 0x0000:
            handleOpcode0xxx();
            break;
        case 0x1000:
            handleOpcode1xxx();
            break;
        case 0x2000:
            handleOpcode2xxx();
            break;
        case 0x3000:
            handleOpcode3xxx();
            break;
        case 0x4000:
            handleOpcode4xxx();
            break;
        case 0x5000:
            handleOpcode5xxx();
            break;
        case 0x6000:
            handleOpcode6xxx();
            break;
        case 0x7000:
            handleOpcode7xxx();
            break;
        case 0x8000:
            handleOpcode8xxx();
            break;
        case 0x9000:
            handleOpcode9xxx();
            break;
        case 0xA000:
            handleOpcodeAxxx();
            break;
        case 0xB000:
            handleOpcodeBxxx();
            break;
        case 0xC000:
            handleOpcodeCxxx();
            break;
        case 0xD000:
            handleOpcodeDxxx();
            break;
        case 0xE000:
            handleOpcodeExxx();
            break;
        case 0xF000:
            handleOpcodeFxxx();
            break;
        default:
            setError(ErrorCode::UnknownOpcode, "Unknown opcode: 0x" + std::to_string(opcode_));
            return;
    }

    // Update timers
    if (delayTimer_ > 0) {
        --delayTimer_;
    }
    if (soundTimer_ > 0) {
        if (soundTimer_ == 1) {
            logInfo("BEEP! Sound timer expired");
        }
        --soundTimer_;
    }
}

// Public accessor methods
const std::array<std::uint8_t, Chip8::DISPLAY_SIZE>& Chip8::getFrameBuffer() const {
    return frameBuffer_;
}

void Chip8::setPixel(std::uint16_t x, std::uint16_t y, std::uint8_t value) {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        setError(ErrorCode::InvalidMemoryAccess, "Pixel coordinates out of bounds: (" +
                                                     std::to_string(x) + ", " + std::to_string(y) +
                                                     ")");
        return;
    }
    frameBuffer_[y * DISPLAY_WIDTH + x] = value;
}

std::uint8_t Chip8::getPixel(std::uint16_t x, std::uint16_t y) const {
    if (x >= DISPLAY_WIDTH || y >= DISPLAY_HEIGHT) {
        return 0;
    }
    return frameBuffer_[y * DISPLAY_WIDTH + x];
}

void Chip8::setKeyState(std::uint8_t key, bool pressed) {
    if (key >= KEYBOARD_SIZE) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid key index: " + std::to_string(key));
        return;
    }
    keyboard_[key] = pressed ? 1 : 0;
}

bool Chip8::isKeyPressed(std::uint8_t key) const {
    if (key >= KEYBOARD_SIZE) {
        return false;
    }
    return keyboard_[key] != 0;
}

// Setters (updated with bounds checking)
void Chip8::setMemory(std::uint16_t address, std::uint8_t value) {
    if (!isValidMemoryAddress(address)) {
        setError(ErrorCode::InvalidMemoryAccess, "Invalid memory address: " + formatHex(address));
        return;
    }
    clearError();  // Clear error on successful operation
    memory_[address] = value;
}

void Chip8::setProgramCounter(std::uint16_t address) {
    if (!isValidMemoryAddress(address)) {
        setError(ErrorCode::InvalidMemoryAccess,
                 "Invalid program counter address: " + std::to_string(address));
        return;
    }
    programCounter_ = address;
}

void Chip8::setStack(std::uint8_t subroutine, std::uint16_t address) {
    if (subroutine >= STACK_SIZE) {
        setError(ErrorCode::StackOverflow,
                 "Stack index out of bounds: " + std::to_string(subroutine));
        return;
    }
    clearError();  // Clear error on successful operation
    stack_[subroutine] = address;
}

void Chip8::setStackPointer(std::uint8_t subroutine) {
    if (subroutine > STACK_SIZE) {
        setError(ErrorCode::StackOverflow,
                 "Stack pointer out of bounds: " + std::to_string(subroutine));
        return;
    }
    clearError();  // Clear error on successful operation
    stackPointer_ = subroutine;
}

void Chip8::setRegisterAt(std::uint8_t reg, std::uint8_t value) {
    if (!isValidRegisterIndex(reg)) {
        setError(ErrorCode::InvalidRegisterAccess,
                 "Invalid register index: " + std::to_string(reg));
        return;
    }
    clearError();  // Clear error on successful operation
    registers_[reg] = value;
}

void Chip8::setDelayTimer(std::uint8_t value) { delayTimer_ = value; }

void Chip8::setDrawFlag(bool condition) { drawFlag_ = condition; }

void Chip8::setIndexRegister(std::uint16_t value) { indexRegister_ = value; }

// Getters (updated with bounds checking)
std::uint8_t Chip8::getMemoryAt(std::uint16_t address) const {
    if (!isValidMemoryAddress(address)) {
        return 0;
    }
    return memory_[address];
}

std::uint16_t Chip8::getIndexRegister() const { return indexRegister_; }

std::uint16_t Chip8::getProgramCounter() const { return programCounter_; }

std::uint16_t Chip8::getStackAt(std::uint8_t subroutine) const {
    if (subroutine >= STACK_SIZE) {
        return 0;
    }
    return stack_[subroutine];
}

std::uint8_t Chip8::getStackPointer() const { return stackPointer_; }

std::uint8_t Chip8::getRegisterAt(std::uint8_t reg) const {
    if (!isValidRegisterIndex(reg)) {
        return 0;
    }
    return registers_[reg];
}

std::uint8_t Chip8::getDelayTimer() const { return delayTimer_; }

std::uint8_t Chip8::getSoundTimer() const { return soundTimer_; }

bool Chip8::getDrawFlag() const { return drawFlag_; }

// Error handling methods
Chip8::ErrorCode Chip8::getLastError() const { return lastError_; }

const std::string& Chip8::getLastErrorMessage() const { return lastErrorMessage_; }

// Opcode handler implementations
void Chip8::handleOpcode0xxx() {
    switch (opcode_ & 0x000F) {
        case 0x0000:  // 0x00E0 - Clear screen
            frameBuffer_.fill(0);
            drawFlag_ = true;
            programCounter_ += 2;
            break;

        case 0x000E:  // 0x00EE - Return from subroutine
            if (stackPointer_ == 0) {
                setError(ErrorCode::StackUnderflow, "Stack underflow on return");
                return;
            }
            stackPointer_--;
            programCounter_ = stack_[stackPointer_];
            programCounter_ += 2;
            break;

        default:
            setError(ErrorCode::UnknownOpcode,
                     "Unknown 0x0xxx opcode: 0x" + std::to_string(opcode_));
    }
}

void Chip8::handleOpcode1xxx() {
    // 0x1NNN - Jump to address NNN
    std::uint16_t address = opcode_ & 0x0FFF;
    if (!isValidMemoryAddress(address)) {
        setError(ErrorCode::InvalidMemoryAccess,
                 "Invalid jump address: " + std::to_string(address));
        return;
    }
    programCounter_ = address;
}

void Chip8::handleOpcode2xxx() {
    // 0x2NNN - Call subroutine at NNN
    if (stackPointer_ >= STACK_SIZE) {
        setError(ErrorCode::StackOverflow, "Stack overflow on subroutine call");
        programCounter_ += 2;  // Advance PC even on error to prevent infinite loop
        return;
    }

    std::uint16_t address = opcode_ & 0x0FFF;
    if (!isValidMemoryAddress(address)) {
        setError(ErrorCode::InvalidMemoryAccess, "Invalid call address: " + formatHex(address));
        programCounter_ += 2;  // Advance PC even on error to prevent infinite loop
        return;
    }

    stack_[stackPointer_] = programCounter_;
    stackPointer_++;
    programCounter_ = address;
}

void Chip8::handleOpcode3xxx() {
    // 0x3XNN - Skip next instruction if VX equals NN
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t nn = opcode_ & 0x00FF;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    if (registers_[x] == nn) {
        programCounter_ += 4;
    } else {
        programCounter_ += 2;
    }
}

void Chip8::handleOpcode4xxx() {
    // 0x4XNN - Skip next instruction if VX doesn't equal NN
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t nn = opcode_ & 0x00FF;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    if (registers_[x] != nn) {
        programCounter_ += 4;
    } else {
        programCounter_ += 2;
    }
}

void Chip8::handleOpcode5xxx() {
    // 0x5XY0 - Skip next instruction if VX equals VY
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t y = (opcode_ & 0x00F0) >> 4;

    if (!isValidRegisterIndex(x) || !isValidRegisterIndex(y)) {
        setError(ErrorCode::InvalidRegisterAccess,
                 "Invalid register indices: " + std::to_string(x) + ", " + std::to_string(y));
        return;
    }

    if (registers_[x] == registers_[y]) {
        programCounter_ += 4;
    } else {
        programCounter_ += 2;
    }
}

void Chip8::handleOpcode6xxx() {
    // 0x6XNN - Set VX to NN
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t nn = opcode_ & 0x00FF;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    registers_[x] = nn;
    programCounter_ += 2;
}

void Chip8::handleOpcode7xxx() {
    // 0x7XNN - Add NN to VX
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t nn = opcode_ & 0x00FF;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    registers_[x] += nn;
    programCounter_ += 2;
}

void Chip8::handleOpcode8xxx() {
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t y = (opcode_ & 0x00F0) >> 4;

    if (!isValidRegisterIndex(x) || !isValidRegisterIndex(y)) {
        setError(ErrorCode::InvalidRegisterAccess,
                 "Invalid register indices: " + std::to_string(x) + ", " + std::to_string(y));
        return;
    }

    switch (opcode_ & 0x000F) {
        case 0x0000:  // 0x8XY0 - Set VX to VY
            registers_[x] = registers_[y];
            break;

        case 0x0001:  // 0x8XY1 - Set VX to VX OR VY
            registers_[x] |= registers_[y];
            break;

        case 0x0002:  // 0x8XY2 - Set VX to VX AND VY
            registers_[x] &= registers_[y];
            break;

        case 0x0003:  // 0x8XY3 - Set VX to VX XOR VY
            registers_[x] ^= registers_[y];
            break;

        case 0x0004:  // 0x8XY4 - Add VY to VX, VF = carry
            if (registers_[y] > 0xFF - registers_[x]) {
                registers_[0xF] = 1;  // Carry
            } else {
                registers_[0xF] = 0;
            }
            registers_[x] += registers_[y];
            break;

        case 0x0005:  // 0x8XY5 - Subtract VY from VX, VF = NOT borrow
            if (registers_[x] >= registers_[y]) {
                registers_[0xF] = 1;  // NOT borrow
            } else {
                registers_[0xF] = 0;
            }
            registers_[x] -= registers_[y];
            break;

        case 0x0006:  // 0x8XY6 - Shift VX right by one, VF = LSB
            registers_[0xF] = registers_[x] & 1;
            registers_[x] >>= 1;
            break;

        case 0x0007:  // 0x8XY7 - Set VX to VY - VX, VF = NOT borrow
            if (registers_[y] >= registers_[x]) {
                registers_[0xF] = 1;  // NOT borrow
            } else {
                registers_[0xF] = 0;
            }
            registers_[x] = registers_[y] - registers_[x];
            break;

        case 0x000E:  // 0x8XYE - Shift VX left by one, VF = MSB
            registers_[0xF] = registers_[x] >> 7;
            registers_[x] <<= 1;
            break;

        default:
            setError(ErrorCode::UnknownOpcode,
                     "Unknown 0x8xxx opcode: 0x" + std::to_string(opcode_));
            return;
    }

    programCounter_ += 2;
}

void Chip8::handleOpcode9xxx() {
    // 0x9XY0 - Skip next instruction if VX doesn't equal VY
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t y = (opcode_ & 0x00F0) >> 4;

    if (!isValidRegisterIndex(x) || !isValidRegisterIndex(y)) {
        setError(ErrorCode::InvalidRegisterAccess,
                 "Invalid register indices: " + std::to_string(x) + ", " + std::to_string(y));
        return;
    }

    if (registers_[x] != registers_[y]) {
        programCounter_ += 4;
    } else {
        programCounter_ += 2;
    }
}

void Chip8::handleOpcodeAxxx() {
    // 0xANNN - Set I to address NNN
    std::uint16_t address = opcode_ & 0x0FFF;
    if (!isValidMemoryAddress(address)) {
        setError(ErrorCode::InvalidMemoryAccess,
                 "Invalid index register address: " + std::to_string(address));
        return;
    }

    indexRegister_ = address;
    programCounter_ += 2;
}

void Chip8::handleOpcodeBxxx() {
    // 0xBNNN - Jump to address NNN + V0
    std::uint16_t address = registers_[0] + (opcode_ & 0x0FFF);
    if (!isValidMemoryAddress(address)) {
        setError(ErrorCode::InvalidMemoryAccess,
                 "Invalid computed jump address: " + std::to_string(address));
        return;
    }

    programCounter_ = address;
}

void Chip8::handleOpcodeCxxx() {
    // 0xCXNN - Set VX to random number AND NN
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t nn = opcode_ & 0x00FF;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    std::uint8_t randomNumber = Random::get<std::uint8_t>(0, 255);
    registers_[x] = randomNumber & nn;
    programCounter_ += 2;
}

void Chip8::handleOpcodeDxxx() {
    // 0xDXYN - Draw sprite at (VX, VY) with height N
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;
    std::uint8_t y = (opcode_ & 0x00F0) >> 4;
    std::uint8_t height = opcode_ & 0x000F;

    if (!isValidRegisterIndex(x) || !isValidRegisterIndex(y)) {
        setError(ErrorCode::InvalidRegisterAccess,
                 "Invalid register indices: " + std::to_string(x) + ", " + std::to_string(y));
        return;
    }

    std::uint8_t xPos = registers_[x];
    std::uint8_t yPos = registers_[y];

    registers_[0xF] = 0;  // Clear collision flag

    for (std::uint8_t row = 0; row < height; ++row) {
        if (indexRegister_ + row >= MEMORY_SIZE) {
            setError(ErrorCode::InvalidMemoryAccess, "Sprite data out of memory bounds");
            return;
        }

        std::uint8_t spriteRow = memory_[indexRegister_ + row];

        for (std::uint8_t col = 0; col < 8; ++col) {
            if ((spriteRow & (0x80 >> col)) != 0) {
                std::uint16_t pixelX = (xPos + col) % DISPLAY_WIDTH;
                std::uint16_t pixelY = (yPos + row) % DISPLAY_HEIGHT;
                std::uint16_t pixelIndex = pixelY * DISPLAY_WIDTH + pixelX;

                if (frameBuffer_[pixelIndex] == 1) {
                    registers_[0xF] = 1;  // Collision detected
                }
                frameBuffer_[pixelIndex] ^= 1;
            }
        }
    }

    drawFlag_ = true;
    programCounter_ += 2;
}

void Chip8::handleOpcodeExxx() {
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    switch (opcode_ & 0x000F) {
        case 0x000E:  // 0xEX9E - Skip if key VX is pressed
            if (registers_[x] < KEYBOARD_SIZE && keyboard_[registers_[x]] != 0) {
                programCounter_ += 4;
            } else {
                programCounter_ += 2;
            }
            break;

        case 0x0001:  // 0xEXA1 - Skip if key VX is not pressed
            if (registers_[x] >= KEYBOARD_SIZE || keyboard_[registers_[x]] == 0) {
                programCounter_ += 4;
            } else {
                programCounter_ += 2;
            }
            break;

        default:
            setError(ErrorCode::UnknownOpcode,
                     "Unknown 0xExxx opcode: 0x" + std::to_string(opcode_));
    }
}

void Chip8::handleOpcodeFxxx() {
    std::uint8_t x = (opcode_ & 0x0F00) >> 8;

    if (!isValidRegisterIndex(x)) {
        setError(ErrorCode::InvalidRegisterAccess, "Invalid register index: " + std::to_string(x));
        return;
    }

    switch (opcode_ & 0x00FF) {
        case 0x0007:  // 0xFX07 - Set VX to delay timer
            registers_[x] = delayTimer_;
            break;

        case 0x000A: {  // 0xFX0A - Wait for key press
            bool keyPressed = false;
            for (std::uint8_t i = 0; i < KEYBOARD_SIZE; ++i) {
                if (keyboard_[i] != 0) {
                    registers_[x] = i;
                    keyPressed = true;
                    break;
                }
            }
            if (!keyPressed) {
                return;  // Don't increment PC, wait for key
            }
            break;
        }

        case 0x0015:  // 0xFX15 - Set delay timer to VX
            delayTimer_ = registers_[x];
            break;

        case 0x0018:  // 0xFX18 - Set sound timer to VX
            soundTimer_ = registers_[x];
            break;

        case 0x001E:  // 0xFX1E - Add VX to I
            indexRegister_ += registers_[x];
            break;

        case 0x0029:  // 0xFX29 - Set I to sprite location for digit VX
            if (registers_[x] > 0xF) {
                setError(ErrorCode::InvalidMemoryAccess,
                         "Invalid sprite digit: " + std::to_string(registers_[x]));
                return;
            }
            indexRegister_ = registers_[x] * 5;  // Each sprite is 5 bytes
            break;

        case 0x0033: {  // 0xFX33 - Store BCD representation of VX
            if (indexRegister_ + 2 >= MEMORY_SIZE) {
                setError(ErrorCode::InvalidMemoryAccess, "BCD storage out of memory bounds");
                return;
            }
            std::uint8_t value = registers_[x];
            memory_[indexRegister_] = value / 100;
            memory_[indexRegister_ + 1] = (value / 10) % 10;
            memory_[indexRegister_ + 2] = value % 10;
            break;
        }

        case 0x0055: {  // 0xFX55 - Store V0 to VX in memory starting at I
            if (indexRegister_ + x >= MEMORY_SIZE) {
                setError(ErrorCode::InvalidMemoryAccess, "Register dump out of memory bounds");
                return;
            }
            for (std::uint8_t i = 0; i <= x; ++i) {
                memory_[indexRegister_ + i] = registers_[i];
            }
            break;
        }

        case 0x0065: {  // 0xFX65 - Load V0 to VX from memory starting at I
            if (indexRegister_ + x >= MEMORY_SIZE) {
                setError(ErrorCode::InvalidMemoryAccess, "Register load out of memory bounds");
                return;
            }
            for (std::uint8_t i = 0; i <= x; ++i) {
                registers_[i] = memory_[indexRegister_ + i];
            }
            break;
        }

        default:
            setError(ErrorCode::UnknownOpcode,
                     "Unknown 0xFxxx opcode: 0x" + std::to_string(opcode_));
            return;
    }

    programCounter_ += 2;
}

// Utility methods
void Chip8::setError(ErrorCode error, const std::string& message) {
    lastError_ = error;
    lastErrorMessage_ = message;
    logError(message);
}

void Chip8::clearError() {
    lastError_ = ErrorCode::None;
    lastErrorMessage_.clear();
}

bool Chip8::isValidMemoryAddress(std::uint16_t address) const { return address < MEMORY_SIZE; }

bool Chip8::isValidRegisterIndex(std::uint8_t index) const { return index < REGISTER_COUNT; }

void Chip8::logError(const std::string& message) const {
    std::cerr << "[ERROR] CHIP-8: " << message << std::endl;
}

void Chip8::logWarning(const std::string& message) const {
    std::cerr << "[WARNING] CHIP-8: " << message << std::endl;
}

void Chip8::logInfo(const std::string& message) const {
    std::cout << "[INFO] CHIP-8: " << message << std::endl;
}
