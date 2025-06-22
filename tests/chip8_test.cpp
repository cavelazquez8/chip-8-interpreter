#include "../src/chip8.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <filesystem>
#include <fstream>

using namespace chip8;
using ::testing::_;
using ::testing::Return;

class Chip8Test : public ::testing::Test {
protected:
    void SetUp() override {
        emulator.reset();
    }

    void TearDown() override {
        // Cleanup any test files
        if (std::filesystem::exists(test_rom_path)) {
            std::filesystem::remove(test_rom_path);
        }
    }

    void createTestRom(const std::vector<std::uint8_t>& data) {
        std::ofstream file(test_rom_path, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
    }

    Chip8 emulator;
    const std::string test_rom_path = "test_rom.ch8";
};

// Basic functionality tests
TEST_F(Chip8Test, InitialState) {
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS);
    EXPECT_EQ(emulator.getIndexRegister(), 0);
    EXPECT_EQ(emulator.getStackPointer(), 0);
    EXPECT_EQ(emulator.getDelayTimer(), 0);
    EXPECT_EQ(emulator.getSoundTimer(), 0);
    EXPECT_FALSE(emulator.shouldDraw());
    
    // Test frame buffer is cleared
    auto frameBuffer = emulator.getFrameBuffer();
    EXPECT_TRUE(std::all_of(frameBuffer.begin(), frameBuffer.end(), 
                           [](auto pixel) { return pixel == 0; }));
}

TEST_F(Chip8Test, Reset) {
    // Modify state
    ASSERT_TRUE(emulator.setRegister(0, 0xFF).has_value());
    ASSERT_TRUE(emulator.setDelayTimer(100).has_value());
    emulator.setDrawFlag(true);
    
    // Reset should restore initial state
    emulator.reset();
    
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS);
    auto result = emulator.getRegister(0);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0);
    EXPECT_EQ(emulator.getDelayTimer(), 0);
    EXPECT_FALSE(emulator.shouldDraw());
}

// Error handling tests
TEST_F(Chip8Test, InvalidRegisterAccess) {
    auto result = emulator.setRegister(16, 0xFF);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRegister);
    
    result = emulator.getRegister(16);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRegister);
}

TEST_F(Chip8Test, InvalidMemoryAccess) {
    auto result = emulator.setMemory(4096, 0xFF);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidAddress);
    
    result = emulator.getMemoryAt(4096);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidAddress);
}

TEST_F(Chip8Test, InvalidStackAccess) {
    auto result = emulator.setStack(16, 0x200);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::StackOverflow);
    
    result = emulator.getStackAt(16);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::StackOverflow);
}

// ROM loading tests
TEST_F(Chip8Test, LoadValidRom) {
    std::vector<std::uint8_t> testData = {0xA2, 0x2A, 0x60, 0x0C, 0x61, 0x08};
    createTestRom(testData);
    
    auto result = emulator.loadRom(test_rom_path);
    ASSERT_TRUE(result.has_value());
    
    // Verify ROM was loaded at correct address
    for (size_t i = 0; i < testData.size(); ++i) {
        auto memResult = emulator.getMemoryAt(Chip8::ROM_START_ADDRESS + i);
        ASSERT_TRUE(memResult.has_value());
        EXPECT_EQ(memResult.value(), testData[i]);
    }
}

TEST_F(Chip8Test, LoadNonexistentRom) {
    auto result = emulator.loadRom("nonexistent.ch8");
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::FileNotFound);
}

TEST_F(Chip8Test, LoadOversizedRom) {
    // Create ROM larger than available memory
    std::vector<std::uint8_t> oversizedData(4000, 0xAA);
    createTestRom(oversizedData);
    
    auto result = emulator.loadRom(test_rom_path);
    ASSERT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRomSize);
}

// Keyboard tests
TEST_F(Chip8Test, KeyboardInput) {
    // Initially no keys pressed
    for (std::uint8_t i = 0; i < 16; ++i) {
        EXPECT_FALSE(emulator.isKeyPressed(i));
    }
    
    // Press and release keys
    emulator.setKeyState(0x5, true);
    EXPECT_TRUE(emulator.isKeyPressed(0x5));
    EXPECT_FALSE(emulator.isKeyPressed(0x6));
    
    emulator.setKeyState(0x5, false);
    EXPECT_FALSE(emulator.isKeyPressed(0x5));
}

TEST_F(Chip8Test, InvalidKeyAccess) {
    // Invalid key indices should be handled gracefully
    EXPECT_FALSE(emulator.isKeyPressed(16));
    emulator.setKeyState(16, true); // Should not crash
    EXPECT_FALSE(emulator.isKeyPressed(16));
}

// Instruction tests
class Chip8InstructionTest : public Chip8Test {
protected:
    void loadInstruction(std::uint16_t opcode) {
        ASSERT_TRUE(emulator.setMemory(Chip8::ROM_START_ADDRESS, 
                                      (opcode & 0xFF00) >> 8).has_value());
        ASSERT_TRUE(emulator.setMemory(Chip8::ROM_START_ADDRESS + 1, 
                                      opcode & 0x00FF).has_value());
    }
};

TEST_F(Chip8InstructionTest, ClearScreen) {
    // Set some pixels
    auto frameBuffer = emulator.getFrameBuffer();
    emulator.setDrawFlag(true);
    
    // Load and execute clear screen instruction (0x00E0)
    loadInstruction(0x00E0);
    emulator.emulateCycle();
    
    // Verify screen is cleared and draw flag is set
    frameBuffer = emulator.getFrameBuffer();
    EXPECT_TRUE(std::all_of(frameBuffer.begin(), frameBuffer.end(), 
                           [](auto pixel) { return pixel == 0; }));
    EXPECT_TRUE(emulator.shouldDraw());
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
}

TEST_F(Chip8InstructionTest, Jump) {
    // Load jump instruction (0x1NNN)
    loadInstruction(0x1234);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getProgramCounter(), 0x234);
}

TEST_F(Chip8InstructionTest, SetRegister) {
    // Load set register instruction (0x6XNN)
    loadInstruction(0x6A42);
    emulator.emulateCycle();
    
    auto result = emulator.getRegister(0xA);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0x42);
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
}

TEST_F(Chip8InstructionTest, AddToRegister) {
    // Set initial value
    ASSERT_TRUE(emulator.setRegister(0x5, 0x10).has_value());
    
    // Load add instruction (0x7XNN)
    loadInstruction(0x7505);
    emulator.emulateCycle();
    
    auto result = emulator.getRegister(0x5);
    ASSERT_TRUE(result.has_value());
    EXPECT_EQ(result.value(), 0x15);
}

TEST_F(Chip8InstructionTest, AddWithCarry) {
    // Set values that will overflow
    ASSERT_TRUE(emulator.setRegister(0x1, 0xFF).has_value());
    ASSERT_TRUE(emulator.setRegister(0x2, 0x01).has_value());
    
    // Load add with carry instruction (0x8XY4)
    loadInstruction(0x8124);
    emulator.emulateCycle();
    
    auto result1 = emulator.getRegister(0x1);
    auto resultF = emulator.getRegister(0xF);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(resultF.has_value());
    
    EXPECT_EQ(result1.value(), 0x00); // Overflow wraps around
    EXPECT_EQ(resultF.value(), 0x01); // Carry flag set
}

TEST_F(Chip8InstructionTest, SubtractWithBorrow) {
    // Test subtraction without borrow
    ASSERT_TRUE(emulator.setRegister(0x1, 0x10).has_value());
    ASSERT_TRUE(emulator.setRegister(0x2, 0x05).has_value());
    
    loadInstruction(0x8125);
    emulator.emulateCycle();
    
    auto result1 = emulator.getRegister(0x1);
    auto resultF = emulator.getRegister(0xF);
    ASSERT_TRUE(result1.has_value());
    ASSERT_TRUE(resultF.has_value());
    
    EXPECT_EQ(result1.value(), 0x0B);
    EXPECT_EQ(resultF.value(), 0x01); // No borrow
}

TEST_F(Chip8InstructionTest, CallAndReturn) {
    // Load call instruction (0x2NNN)
    loadInstruction(0x2300);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getProgramCounter(), 0x300);
    EXPECT_EQ(emulator.getStackPointer(), 1);
    
    auto stackResult = emulator.getStackAt(0);
    ASSERT_TRUE(stackResult.has_value());
    EXPECT_EQ(stackResult.value(), Chip8::ROM_START_ADDRESS);
    
    // Load return instruction (0x00EE) at new location
    ASSERT_TRUE(emulator.setMemory(0x300, 0x00).has_value());
    ASSERT_TRUE(emulator.setMemory(0x301, 0xEE).has_value());
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
    EXPECT_EQ(emulator.getStackPointer(), 0);
}

// Edge case and boundary tests
TEST_F(Chip8Test, StackOverflowProtection) {
    // Fill stack to capacity
    for (std::uint8_t i = 0; i < Chip8::STACK_SIZE; ++i) {
        ASSERT_TRUE(emulator.setStack(i, 0x200 + i * 2).has_value());
    }
    ASSERT_TRUE(emulator.setStackPointer(Chip8::STACK_SIZE).has_value());
    
    // Try to push one more (should be protected)
    loadInstruction(0x2400);
    emulator.emulateCycle();
    
    // Should not have called the subroutine
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
    EXPECT_EQ(emulator.getStackPointer(), Chip8::STACK_SIZE);
}

TEST_F(Chip8Test, ProgramCounterBoundaryProtection) {
    // Set PC near end of memory
    ASSERT_TRUE(emulator.setProgramCounter(Chip8::MEMORY_SIZE - 1).has_value());
    
    // Emulate cycle should not crash
    emulator.emulateCycle();
    
    // PC should not have changed (no instruction executed)
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::MEMORY_SIZE - 1);
}