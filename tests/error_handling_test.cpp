#include "../src/chip8.h"

#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>

// Removed namespace usage - Chip8 is not in a namespace

class ErrorHandlingTest : public ::testing::Test {
protected:
    void SetUp() override {
        emulator.init();
    }

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

TEST_F(ErrorHandlingTest, EmulatorErrorTypes) {
    EmulatorError error(EmulatorError::Type::InvalidAddress, "Test error");
    EXPECT_EQ(error.type(), EmulatorError::Type::InvalidAddress);
    EXPECT_EQ(error.message(), "Test error");
}

TEST_F(ErrorHandlingTest, InvalidMemoryAddresses) {
    // Test boundary conditions
    auto result = emulator.setMemory(0, 0xFF);
    EXPECT_TRUE(result.has_value());
    
    result = emulator.setMemory(Chip8::MEMORY_SIZE - 1, 0xFF);
    EXPECT_TRUE(result.has_value());
    
    result = emulator.setMemory(Chip8::MEMORY_SIZE, 0xFF);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidAddress);
    
    result = emulator.setMemory(0xFFFF, 0xFF);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidAddress);
}

TEST_F(ErrorHandlingTest, InvalidRegisterIndices) {
    // Valid registers
    for (std::uint8_t i = 0; i < 16; ++i) {
        auto result = emulator.setRegister(i, 0x42);
        EXPECT_TRUE(result.has_value()) << "Register " << static_cast<int>(i) << " should be valid";
    }
    
    // Invalid registers
    auto result = emulator.setRegister(16, 0x42);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRegister);
    
    result = emulator.setRegister(255, 0x42);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRegister);
}

TEST_F(ErrorHandlingTest, StackOverflowAndUnderflow) {
    // Test stack overflow
    auto result = emulator.setStackPointer(Chip8::STACK_SIZE + 1);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::StackOverflow);
    
    // Test invalid stack level access
    result = emulator.setStack(Chip8::STACK_SIZE, 0x200);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::StackOverflow);
    
    result = emulator.getStackAt(Chip8::STACK_SIZE);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::StackOverflow);
}

TEST_F(ErrorHandlingTest, InvalidProgramCounterAddresses) {
    // Valid addresses
    auto result = emulator.setProgramCounter(0);
    EXPECT_TRUE(result.has_value());
    
    result = emulator.setProgramCounter(Chip8::MEMORY_SIZE - 1);
    EXPECT_TRUE(result.has_value());
    
    // Invalid addresses
    result = emulator.setProgramCounter(Chip8::MEMORY_SIZE);
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidAddress);
}

TEST_F(ErrorHandlingTest, RomLoadingErrors) {
    // Test loading non-existent file
    auto result = emulator.loadRom("does_not_exist.ch8");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::FileNotFound);
    EXPECT_THAT(result.error().message(), ::testing::HasSubstr("does_not_exist.ch8"));
    
    // Test loading empty file
    createTestFile("empty.ch8", {});
    result = emulator.loadRom("empty.ch8");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRomSize);
    
    // Test loading oversized ROM
    std::vector<std::uint8_t> oversizedData(Chip8::MEMORY_SIZE - Chip8::ROM_START_ADDRESS + 1, 0xAA);
    createTestFile("oversized.ch8", oversizedData);
    result = emulator.loadRom("oversized.ch8");
    EXPECT_FALSE(result.has_value());
    EXPECT_EQ(result.error().type(), EmulatorError::Type::InvalidRomSize);
    EXPECT_THAT(result.error().message(), ::testing::HasSubstr("bytes"));
}

TEST_F(ErrorHandlingTest, ErrorMessageQuality) {
    // Test that error messages contain useful information
    auto result = emulator.setMemory(0x1000, 0xFF);
    ASSERT_FALSE(result.has_value());
    EXPECT_THAT(result.error().message(), ::testing::HasSubstr("0x1000"));
    
    result = emulator.setRegister(20, 0xFF);
    ASSERT_FALSE(result.has_value());
    EXPECT_THAT(result.error().message(), ::testing::HasSubstr("V14")); // 20 in hex is 0x14
    
    result = emulator.setStack(20, 0x200);
    ASSERT_FALSE(result.has_value());
    EXPECT_THAT(result.error().message(), ::testing::HasSubstr("20"));
}

TEST_F(ErrorHandlingTest, BoundaryConditions) {
    // Test exact boundary values
    EXPECT_TRUE(emulator.setMemory(0, 0xFF).has_value());
    EXPECT_TRUE(emulator.setMemory(Chip8::MEMORY_SIZE - 1, 0xFF).has_value());
    EXPECT_FALSE(emulator.setMemory(Chip8::MEMORY_SIZE, 0xFF).has_value());
    
    EXPECT_TRUE(emulator.setRegister(0, 0xFF).has_value());
    EXPECT_TRUE(emulator.setRegister(15, 0xFF).has_value());
    EXPECT_FALSE(emulator.setRegister(16, 0xFF).has_value());
    
    EXPECT_TRUE(emulator.setStack(0, 0x200).has_value());
    EXPECT_TRUE(emulator.setStack(Chip8::STACK_SIZE - 1, 0x200).has_value());
    EXPECT_FALSE(emulator.setStack(Chip8::STACK_SIZE, 0x200).has_value());
    
    EXPECT_TRUE(emulator.setStackPointer(0).has_value());
    EXPECT_TRUE(emulator.setStackPointer(Chip8::STACK_SIZE).has_value());
    EXPECT_FALSE(emulator.setStackPointer(Chip8::STACK_SIZE + 1).has_value());
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

TEST_F(ErrorHandlingTest, ResultPatternUsage) {
    // Test that Result pattern is used consistently
    auto memResult = emulator.getMemoryAt(100);
    EXPECT_TRUE(memResult.has_value());
    
    auto regResult = emulator.getRegister(5);
    EXPECT_TRUE(regResult.has_value());
    
    auto stackResult = emulator.getStackAt(3);
    EXPECT_TRUE(stackResult.has_value());
    
    // Test chaining with monadic operations
    auto chainResult = emulator.setRegister(0, 42)
        .and_then([&](auto) { return emulator.setMemory(0x300, 0xFF); })
        .and_then([&](auto) { return emulator.setProgramCounter(0x300); });
    
    EXPECT_TRUE(chainResult.has_value());
}