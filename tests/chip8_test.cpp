#include "../src/chip8.h"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <array>
#include <filesystem>
#include <fstream>

// Removed namespace usage - Chip8 is not in a namespace
using ::testing::_;
using ::testing::Return;

class Chip8Test : public ::testing::Test {
protected:
    void SetUp() override {
        emulator.init();
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

    void loadInstruction(std::uint16_t opcode) {
        emulator.setMemory(Chip8::ROM_START_ADDRESS, (opcode & 0xFF00) >> 8);
        emulator.setMemory(Chip8::ROM_START_ADDRESS + 1, opcode & 0x00FF);
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
    EXPECT_FALSE(emulator.getDrawFlag());
    
    // Test frame buffer is cleared
    auto frameBuffer = emulator.getFrameBuffer();
    EXPECT_TRUE(std::all_of(frameBuffer.begin(), frameBuffer.end(), 
                           [](auto pixel) { return pixel == 0; }));
}

TEST_F(Chip8Test, Reset) {
    // Modify state
    emulator.setRegisterAt(0, 0xFF);
    emulator.setDelayTimer(100);
    emulator.setDrawFlag(true);
    
    // Reset should restore initial state
    emulator.init();
    
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS);
    EXPECT_EQ(emulator.getRegisterAt(0), 0);
    EXPECT_EQ(emulator.getDelayTimer(), 0);
    EXPECT_FALSE(emulator.getDrawFlag());
}

// Error handling tests
TEST_F(Chip8Test, InvalidRegisterAccess) {
    // Set invalid register (should be handled gracefully)
    emulator.setRegisterAt(16, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);
    
    // Clear error and test get
    emulator.getRegisterAt(16);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);
}

TEST_F(Chip8Test, InvalidMemoryAccess) {
    // Set invalid memory address (should be handled gracefully)
    emulator.setMemory(4096, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);
    
    // Clear error and test get
    emulator.getMemoryAt(4096);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);
}

TEST_F(Chip8Test, InvalidStackAccess) {
    // Set invalid stack position (should be handled gracefully) 
    emulator.setStack(16, 0x200);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);
    
    // Clear error and test get
    emulator.getStackAt(16);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);
}

// ROM loading tests
TEST_F(Chip8Test, LoadValidRom) {
    std::vector<std::uint8_t> testData = {0xA2, 0x2A, 0x60, 0x0C, 0x61, 0x08};
    createTestRom(testData);
    
    bool loaded = emulator.loadRom(test_rom_path);
    ASSERT_TRUE(loaded);
    
    // Verify ROM was loaded at correct address
    for (size_t i = 0; i < testData.size(); ++i) {
        EXPECT_EQ(emulator.getMemoryAt(Chip8::ROM_START_ADDRESS + i), testData[i]);
    }
}

TEST_F(Chip8Test, LoadNonexistentRom) {
    bool loaded = emulator.loadRom("nonexistent.ch8");
    ASSERT_FALSE(loaded);
    // Error details can be checked via getLastError if needed
}

TEST_F(Chip8Test, LoadOversizedRom) {
    // Create ROM larger than available memory
    std::vector<std::uint8_t> oversizedData(4000, 0xAA);
    createTestRom(oversizedData);
    
    bool loaded = emulator.loadRom(test_rom_path);
    ASSERT_FALSE(loaded);
    // Error details can be checked via getLastError if needed
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
    // Inherits loadInstruction from Chip8Test
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
    EXPECT_TRUE(emulator.getDrawFlag());
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
    
    EXPECT_EQ(emulator.getRegisterAt(0xA), 0x42);
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
}

TEST_F(Chip8InstructionTest, AddToRegister) {
    // Set initial value
    emulator.setRegisterAt(0x5, 0x10);
    
    // Load add instruction (0x7XNN)
    loadInstruction(0x7505);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getRegisterAt(0x5), 0x15);
}

TEST_F(Chip8InstructionTest, AddWithCarry) {
    // Set values that will overflow
    emulator.setRegisterAt(0x1, 0xFF);
    emulator.setRegisterAt(0x2, 0x01);
    
    // Load add with carry instruction (0x8XY4)
    loadInstruction(0x8124);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getRegisterAt(0x1), 0x00); // Overflow wraps around
    EXPECT_EQ(emulator.getRegisterAt(0xF), 0x01); // Carry flag set
}

TEST_F(Chip8InstructionTest, SubtractWithBorrow) {
    // Test subtraction without borrow
    emulator.setRegisterAt(0x1, 0x10);
    emulator.setRegisterAt(0x2, 0x05);
    
    loadInstruction(0x8125);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getRegisterAt(0x1), 0x0B);
    EXPECT_EQ(emulator.getRegisterAt(0xF), 0x01); // No borrow
}

TEST_F(Chip8InstructionTest, CallAndReturn) {
    // Load call instruction (0x2NNN)
    loadInstruction(0x2300);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getProgramCounter(), 0x300);
    EXPECT_EQ(emulator.getStackPointer(), 1);
    
    EXPECT_EQ(emulator.getStackAt(0), Chip8::ROM_START_ADDRESS);
    
    // Load return instruction (0x00EE) at new location
    emulator.setMemory(0x300, 0x00);
    emulator.setMemory(0x301, 0xEE);
    emulator.emulateCycle();
    
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
    EXPECT_EQ(emulator.getStackPointer(), 0);
}

// Edge case and boundary tests
TEST_F(Chip8Test, StackOverflowProtection) {
    // Fill stack to capacity
    for (std::uint8_t i = 0; i < Chip8::STACK_SIZE; ++i) {
        emulator.setStack(i, 0x200 + i * 2);
    }
    emulator.setStackPointer(Chip8::STACK_SIZE);
    
    // Try to push one more (should be protected)
    loadInstruction(0x2400);
    emulator.emulateCycle();
    
    // Should not have called the subroutine
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 2);
    EXPECT_EQ(emulator.getStackPointer(), Chip8::STACK_SIZE);
}

TEST_F(Chip8Test, ProgramCounterBoundaryProtection) {
    // Set PC near end of memory
    emulator.setProgramCounter(Chip8::MEMORY_SIZE - 1);
    
    // Emulate cycle should not crash
    emulator.emulateCycle();
    
    // PC should not have changed (no instruction executed)
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::MEMORY_SIZE - 1);
}