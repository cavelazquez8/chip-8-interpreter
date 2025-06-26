#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <filesystem>
#include <fstream>

#include "../src/chip8.h"

// Removed namespace usage - Chip8 is not in a namespace

class ErrorHandlingTest : public ::testing::Test {
  protected:
    void SetUp() override { emulator.init(); }

    void TearDown() override {
        // Cleanup test files
        for (const auto& file : test_files) {
            if (std::filesystem::exists(file)) {
                std::filesystem::remove(file);
            }
        }
    }

    void createTestFile(const std::string& filename, const std::vector<std::uint8_t>& data) {
        std::ofstream file(filename, std::ios::binary);
        file.write(reinterpret_cast<const char*>(data.data()), data.size());
        test_files.push_back(filename);
    }

    Chip8 emulator;
    std::vector<std::string> test_files;
};

TEST_F(ErrorHandlingTest, ErrorCodeTypes) {
    // Test that error codes are properly set
    emulator.setMemory(Chip8::MEMORY_SIZE, 0xFF);  // Invalid address
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);
    EXPECT_FALSE(emulator.getLastErrorMessage().empty());
}

TEST_F(ErrorHandlingTest, InvalidMemoryAddresses) {
    // Test boundary conditions - valid addresses
    emulator.setMemory(0, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    emulator.setMemory(Chip8::MEMORY_SIZE - 1, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    // Invalid addresses
    emulator.setMemory(Chip8::MEMORY_SIZE, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);

    emulator.setMemory(0xFFFF, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);
}

TEST_F(ErrorHandlingTest, InvalidRegisterIndices) {
    // Valid registers
    for (std::uint8_t i = 0; i < 16; ++i) {
        emulator.setRegisterAt(i, 0x42);
        EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None)
            << "Register " << static_cast<int>(i) << " should be valid";
    }

    // Invalid registers
    emulator.setRegisterAt(16, 0x42);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);

    emulator.setRegisterAt(255, 0x42);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);
}

TEST_F(ErrorHandlingTest, StackOverflowAndUnderflow) {
    // Test stack overflow
    emulator.setStackPointer(Chip8::STACK_SIZE + 1);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);

    // Test invalid stack level access
    emulator.setStack(Chip8::STACK_SIZE, 0x200);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);

    emulator.getStackAt(Chip8::STACK_SIZE);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);
}

TEST_F(ErrorHandlingTest, InvalidProgramCounterAddresses) {
    // Valid addresses
    emulator.setProgramCounter(0);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    emulator.setProgramCounter(Chip8::MEMORY_SIZE - 1);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    // Invalid addresses
    emulator.setProgramCounter(Chip8::MEMORY_SIZE);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);
}

TEST_F(ErrorHandlingTest, RomLoadingErrors) {
    // Test loading non-existent file
    bool loaded = emulator.loadRom("does_not_exist.ch8");
    EXPECT_FALSE(loaded);
    EXPECT_NE(emulator.getLastError(), Chip8::ErrorCode::None);
    EXPECT_THAT(emulator.getLastErrorMessage(), ::testing::HasSubstr("does_not_exist.ch8"));

    // Test loading empty file
    createTestFile("empty.ch8", {});
    loaded = emulator.loadRom("empty.ch8");
    EXPECT_FALSE(loaded);
    EXPECT_NE(emulator.getLastError(), Chip8::ErrorCode::None);

    // Test loading oversized ROM
    std::vector<std::uint8_t> oversizedData(Chip8::MEMORY_SIZE - Chip8::ROM_START_ADDRESS + 1,
                                            0xAA);
    createTestFile("oversized.ch8", oversizedData);
    loaded = emulator.loadRom("oversized.ch8");
    EXPECT_FALSE(loaded);
    EXPECT_NE(emulator.getLastError(), Chip8::ErrorCode::None);
    EXPECT_THAT(emulator.getLastErrorMessage(), ::testing::HasSubstr("bytes"));
}

TEST_F(ErrorHandlingTest, ErrorMessageQuality) {
    // Test that error messages contain useful information
    emulator.setMemory(0x1000, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);
    EXPECT_THAT(emulator.getLastErrorMessage(), ::testing::HasSubstr("0x1000"));

    emulator.setRegisterAt(20, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);
    EXPECT_FALSE(emulator.getLastErrorMessage().empty());

    emulator.setStack(20, 0x200);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);
    EXPECT_FALSE(emulator.getLastErrorMessage().empty());
}

TEST_F(ErrorHandlingTest, BoundaryConditions) {
    // Test exact boundary values
    emulator.setMemory(0, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setMemory(Chip8::MEMORY_SIZE - 1, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setMemory(Chip8::MEMORY_SIZE, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidMemoryAccess);

    emulator.setRegisterAt(0, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setRegisterAt(15, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setRegisterAt(16, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);

    emulator.setStack(0, 0x200);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setStack(Chip8::STACK_SIZE - 1, 0x200);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setStack(Chip8::STACK_SIZE, 0x200);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);

    emulator.setStackPointer(0);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setStackPointer(Chip8::STACK_SIZE);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);
    emulator.setStackPointer(Chip8::STACK_SIZE + 1);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::StackOverflow);
}

TEST_F(ErrorHandlingTest, KeyboardBoundaryConditions) {
    // Valid keys
    EXPECT_FALSE(emulator.isKeyPressed(0));
    EXPECT_FALSE(emulator.isKeyPressed(15));

    // Invalid keys should return false and not crash
    EXPECT_FALSE(emulator.isKeyPressed(16));
    EXPECT_FALSE(emulator.isKeyPressed(255));

    // Setting invalid key states should not crash
    emulator.setKeyState(16, true);
    emulator.setKeyState(255, true);

    // Valid keys should still work
    emulator.setKeyState(5, true);
    EXPECT_TRUE(emulator.isKeyPressed(5));
}

TEST_F(ErrorHandlingTest, ErrorStateManagement) {
    // Test that error states are properly managed
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    // Valid operations should not set errors
    emulator.setRegisterAt(5, 42);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    emulator.setMemory(0x300, 0xFF);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    emulator.setProgramCounter(0x300);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::None);

    // Invalid operation should set error
    emulator.setRegisterAt(20, 42);
    EXPECT_EQ(emulator.getLastError(), Chip8::ErrorCode::InvalidRegisterAccess);
}