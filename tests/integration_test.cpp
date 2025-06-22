#include "../src/chip8.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <thread>
#include <chrono>

using namespace chip8;

class IntegrationTest : public ::testing::Test {
protected:
    void SetUp() override {
        emulator.reset();
    }

    void TearDown() override {
        for (const auto& file : test_files) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }

    void createRom(const std::string& filename, const std::vector<std::uint8_t>& data) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        test_files.push_back(filename);
    }

    void runCycles(int count) {
        for (int i = 0; i < count; ++i) {
            emulator.emulateCycle();
        }
    }

    Chip8 emulator;
    std::vector<std::string> test_files;
};

TEST_F(IntegrationTest, CompleteRomExecution) {
    // Create a simple ROM that:
    // 1. Clears screen
    // 2. Sets some registers
    // 3. Draws a sprite
    // 4. Infinite loop
    std::vector<std::uint8_t> testRom = {
        0x00, 0xE0,  // Clear screen
        0x60, 0x20,  // Set V0 = 32
        0x61, 0x10,  // Set V1 = 16
        0xA2, 0x30,  // Set I = 0x230 (sprite location)
        0xD0, 0x15,  // Draw sprite at (V0, V1) with height 5
        0x12, 0x08   // Jump to 0x208 (infinite loop)
    };
    
    // Add a simple sprite pattern
    std::vector<std::uint8_t> sprite = {0xF0, 0x90, 0x90, 0x90, 0xF0}; // "0" digit
    testRom.insert(testRom.end(), sprite.begin(), sprite.end());
    
    createRom("test_integration.ch8", testRom);
    
    auto loadResult = emulator.loadRom("test_integration.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    // Execute clear screen
    emulator.emulateCycle();
    EXPECT_TRUE(emulator.shouldDraw());
    emulator.clearDrawFlag();
    
    // Execute register sets
    emulator.emulateCycle();
    auto reg0 = emulator.getRegister(0);
    ASSERT_TRUE(reg0.has_value());
    EXPECT_EQ(reg0.value(), 32);
    
    emulator.emulateCycle();
    auto reg1 = emulator.getRegister(1);
    ASSERT_TRUE(reg1.has_value());
    EXPECT_EQ(reg1.value(), 16);
    
    // Execute set index register
    emulator.emulateCycle();
    EXPECT_EQ(emulator.getIndexRegister(), 0x230);
    
    // Execute draw instruction
    emulator.emulateCycle();
    EXPECT_TRUE(emulator.shouldDraw());
    
    // Verify sprite was drawn
    auto frameBuffer = emulator.getFrameBuffer();
    bool spriteDrawn = false;
    for (size_t i = 0; i < frameBuffer.size(); ++i) {
        if (frameBuffer[i] != 0) {
            spriteDrawn = true;
            break;
        }
    }
    EXPECT_TRUE(spriteDrawn);
    
    // Execute jump (should loop back)
    auto pcBefore = emulator.getProgramCounter();
    emulator.emulateCycle();
    EXPECT_EQ(emulator.getProgramCounter(), 0x208);
}

TEST_F(IntegrationTest, TimerDecrement) {
    // Set delay timer and sound timer
    ASSERT_TRUE(emulator.setDelayTimer(10).has_value());
    // Note: Can't directly set sound timer in public interface
    
    // Run several cycles and verify timer decrements
    for (int i = 9; i >= 0; --i) {
        EXPECT_EQ(emulator.getDelayTimer(), i + 1);
        emulator.emulateCycle(); // This should decrement timers
    }
    
    EXPECT_EQ(emulator.getDelayTimer(), 0);
}

TEST_F(IntegrationTest, KeyboardInputIntegration) {
    // Create ROM that waits for key press
    std::vector<std::uint8_t> keyTestRom = {
        0xF0, 0x0A,  // Wait for key press, store in V0
        0x12, 0x04   // Jump to self (infinite loop after key press)
    };
    
    createRom("key_test.ch8", keyTestRom);
    auto loadResult = emulator.loadRom("key_test.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    auto initialPC = emulator.getProgramCounter();
    
    // Execute without key press - should not advance
    emulator.emulateCycle();
    EXPECT_EQ(emulator.getProgramCounter(), initialPC);
    
    // Still no key press
    emulator.emulateCycle();
    EXPECT_EQ(emulator.getProgramCounter(), initialPC);
    
    // Press key 5
    emulator.setKeyState(5, true);
    emulator.emulateCycle();
    
    // Should have advanced and stored key in V0
    EXPECT_EQ(emulator.getProgramCounter(), initialPC + 2);
    auto reg0 = emulator.getRegister(0);
    ASSERT_TRUE(reg0.has_value());
    EXPECT_EQ(reg0.value(), 5);
}

TEST_F(IntegrationTest, SubroutineCallAndReturn) {
    // Create ROM with subroutine call and return
    std::vector<std::uint8_t> subroutineRom = {
        0x22, 0x10,  // Call subroutine at 0x210
        0x60, 0xFF,  // Set V0 = 0xFF (should execute after return)
        0x12, 0x06,  // Jump to self
        // Padding to reach 0x210
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
        // Subroutine at 0x210
        0x61, 0x42,  // Set V1 = 0x42
        0x00, 0xEE   // Return
    };
    
    createRom("subroutine_test.ch8", subroutineRom);
    auto loadResult = emulator.loadRom("subroutine_test.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    // Execute call
    emulator.emulateCycle();
    EXPECT_EQ(emulator.getProgramCounter(), 0x210);
    EXPECT_EQ(emulator.getStackPointer(), 1);
    
    // Execute subroutine instruction
    emulator.emulateCycle();
    auto reg1 = emulator.getRegister(1);
    ASSERT_TRUE(reg1.has_value());
    EXPECT_EQ(reg1.value(), 0x42);
    
    // Execute return
    emulator.emulateCycle();
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS + 4);
    EXPECT_EQ(emulator.getStackPointer(), 0);
    
    // Execute next instruction after return
    emulator.emulateCycle();
    auto reg0 = emulator.getRegister(0);
    ASSERT_TRUE(reg0.has_value());
    EXPECT_EQ(reg0.value(), 0xFF);
}

TEST_F(IntegrationTest, SpriteCollisionDetection) {
    // Create ROM that draws overlapping sprites
    std::vector<std::uint8_t> collisionRom = {
        0xA2, 0x30,  // Set I = 0x230 (sprite location)
        0x60, 0x20,  // Set V0 = 32 (x position)
        0x61, 0x10,  // Set V1 = 16 (y position)
        0xD0, 0x15,  // Draw sprite at (V0, V1) height 5
        0xD0, 0x15,  // Draw same sprite again (should cause collision)
        0x12, 0x0E   // Infinite loop
    };
    
    // Add sprite data
    std::vector<std::uint8_t> sprite = {0xF0, 0x90, 0x90, 0x90, 0xF0};
    collisionRom.insert(collisionRom.end(), sprite.begin(), sprite.end());
    
    createRom("collision_test.ch8", collisionRom);
    auto loadResult = emulator.loadRom("collision_test.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    // Execute setup
    runCycles(3);
    
    // First draw - no collision
    emulator.emulateCycle();
    auto regF = emulator.getRegister(0xF);
    ASSERT_TRUE(regF.has_value());
    EXPECT_EQ(regF.value(), 0);
    
    // Second draw - collision detected
    emulator.emulateCycle();
    regF = emulator.getRegister(0xF);
    ASSERT_TRUE(regF.has_value());
    EXPECT_EQ(regF.value(), 1);
    
    // Verify screen is cleared (XOR operation)
    auto frameBuffer = emulator.getFrameBuffer();
    bool anyPixelSet = false;
    for (size_t i = 0; i < frameBuffer.size(); ++i) {
        if (frameBuffer[i] != 0) {
            anyPixelSet = true;
            break;
        }
    }
    EXPECT_FALSE(anyPixelSet);
}

TEST_F(IntegrationTest, ArithmeticOperations) {
    // Test various arithmetic operations
    std::vector<std::uint8_t> mathRom = {
        0x60, 0x05,  // V0 = 5
        0x61, 0x03,  // V1 = 3
        0x80, 0x14,  // V0 += V1 (V0 = 8)
        0x62, 0x0A,  // V2 = 10
        0x82, 0x05,  // V2 -= V0 (V2 = 2)
        0x63, 0xFF,  // V3 = 255
        0x64, 0x01,  // V4 = 1
        0x83, 0x44,  // V3 += V4 (should overflow, VF = 1)
        0x12, 0x10   // Infinite loop
    };
    
    createRom("math_test.ch8", mathRom);
    auto loadResult = emulator.loadRom("math_test.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    runCycles(2); // Set V0 and V1
    
    emulator.emulateCycle(); // Add operation
    auto reg0 = emulator.getRegister(0);
    ASSERT_TRUE(reg0.has_value());
    EXPECT_EQ(reg0.value(), 8);
    
    runCycles(2); // Set V2 and subtract
    auto reg2 = emulator.getRegister(2);
    ASSERT_TRUE(reg2.has_value());
    EXPECT_EQ(reg2.value(), 2);
    
    runCycles(3); // Overflow test
    auto reg3 = emulator.getRegister(3);
    auto regF = emulator.getRegister(0xF);
    ASSERT_TRUE(reg3.has_value());
    ASSERT_TRUE(regF.has_value());
    EXPECT_EQ(reg3.value(), 0);  // Wrapped around
    EXPECT_EQ(regF.value(), 1);  // Carry flag set
}

TEST_F(IntegrationTest, MemoryOperations) {
    // Test memory store/load operations
    std::vector<std::uint8_t> memoryRom = {
        0x60, 0x11,  // V0 = 0x11
        0x61, 0x22,  // V1 = 0x22
        0x62, 0x33,  // V2 = 0x33
        0xA3, 0x00,  // I = 0x300
        0xF2, 0x55,  // Store V0-V2 at I
        0x63, 0x00,  // V3 = 0 (clear for test)
        0x64, 0x00,  // V4 = 0
        0x65, 0x00,  // V5 = 0
        0xF5, 0x65,  // Load V0-V5 from I
        0x12, 0x12   // Infinite loop
    };
    
    createRom("memory_test.ch8", memoryRom);
    auto loadResult = emulator.loadRom("memory_test.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    runCycles(5); // Setup and store
    
    // Verify memory was written
    auto mem0 = emulator.getMemoryAt(0x300);
    auto mem1 = emulator.getMemoryAt(0x301);
    auto mem2 = emulator.getMemoryAt(0x302);
    ASSERT_TRUE(mem0.has_value() && mem1.has_value() && mem2.has_value());
    EXPECT_EQ(mem0.value(), 0x11);
    EXPECT_EQ(mem1.value(), 0x22);
    EXPECT_EQ(mem2.value(), 0x33);
    
    runCycles(4); // Clear registers and load
    
    // Verify registers were loaded
    auto reg3 = emulator.getRegister(3);
    auto reg4 = emulator.getRegister(4);
    auto reg5 = emulator.getRegister(5);
    ASSERT_TRUE(reg3.has_value() && reg4.has_value() && reg5.has_value());
    EXPECT_EQ(reg3.value(), 0x11);
    EXPECT_EQ(reg4.value(), 0x22);
    EXPECT_EQ(reg5.value(), 0x33);
}

TEST_F(IntegrationTest, LongRunningExecution) {
    // Test that emulator remains stable over many cycles
    std::vector<std::uint8_t> loopRom = {
        0x70, 0x01,  // V0 += 1
        0x12, 0x00   // Jump to start
    };
    
    createRom("loop_test.ch8", loopRom);
    auto loadResult = emulator.loadRom("loop_test.ch8");
    ASSERT_TRUE(loadResult.has_value());
    
    // Run many cycles
    for (int i = 0; i < 1000; ++i) {
        emulator.emulateCycle();
    }
    
    // V0 should have wrapped around multiple times
    auto reg0 = emulator.getRegister(0);
    ASSERT_TRUE(reg0.has_value());
    EXPECT_EQ(reg0.value(), 1000 % 256);
    
    // Emulator should still be in good state
    EXPECT_EQ(emulator.getProgramCounter(), Chip8::ROM_START_ADDRESS);
    EXPECT_EQ(emulator.getStackPointer(), 0);
}