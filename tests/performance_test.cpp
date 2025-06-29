#include <gtest/gtest.h>

#include <chrono>
#include <filesystem>
#include <fstream>

#include "../src/chip8.h"

// Removed namespace usage - Chip8 is not in a namespace

class PerformanceTest : public ::testing::Test {
  protected:
    void SetUp() override { emulator.init(); }

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

    template <typename Func>
    std::chrono::nanoseconds measureExecutionTime(Func&& func) {
        auto start = std::chrono::high_resolution_clock::now();
        func();
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::nanoseconds>(end - start);
    }

    Chip8 emulator;
    std::vector<std::string> test_files;
};

TEST_F(PerformanceTest, CycleExecutionSpeed) {
    // Create a simple ROM with various instructions
    std::vector<std::uint8_t> testRom = {
        0x60, 0x20,  // V0 = 32
        0x61, 0x10,  // V1 = 16
        0x80, 0x14,  // V0 += V1
        0xA2, 0x30,  // I = 0x230
        0xD0, 0x15,  // Draw sprite
        0x12, 0x00   // Jump to start
    };

    createRom("perf_test.ch8", testRom);
    bool loadResult = emulator.loadRom("perf_test.ch8");
    ASSERT_TRUE(loadResult);

    const int numCycles = 100000;

    auto duration = measureExecutionTime([&]() {
        for (int i = 0; i < numCycles; ++i) {
            emulator.emulateCycle();
        }
    });

    // Calculate cycles per second
    double cyclesPerSecond =
        static_cast<double>(numCycles) / (static_cast<double>(duration.count()) / 1e9);

    // CHIP-8 typically runs at 500-1000 Hz, but our emulator should be much faster
    // Expect at least 100,000 cycles per second in debug mode
    EXPECT_GT(cyclesPerSecond, 100000.0)
        << "Emulator too slow: " << cyclesPerSecond << " cycles/sec";

    std::cout << "Performance: " << cyclesPerSecond << " cycles/second" << std::endl;
    std::cout << "Average cycle time: " << duration.count() / numCycles << " ns" << std::endl;
}

TEST_F(PerformanceTest, MemoryAccessSpeed) {
    const int numAccesses = 100000;

    // Test sequential memory access
    auto writeTime = measureExecutionTime([&]() {
        for (int i = 0; i < numAccesses; ++i) {
            emulator.setMemory(0x300 + (i % 1000), static_cast<std::uint8_t>(i));
            // Note: errors handled via getLastError() if needed
        }
    });

    auto readTime = measureExecutionTime([&]() {
        for (int i = 0; i < numAccesses; ++i) {
            volatile auto value = emulator.getMemoryAt(0x300 + (i % 1000));
            (void)value;  // Prevent optimization
        }
    });

    double writesPerSecond =
        static_cast<double>(numAccesses) / (static_cast<double>(writeTime.count()) / 1e9);
    double readsPerSecond =
        static_cast<double>(numAccesses) / (static_cast<double>(readTime.count()) / 1e9);

    std::cout << "Memory writes: " << writesPerSecond << " operations/second" << std::endl;
    std::cout << "Memory reads: " << readsPerSecond << " operations/second" << std::endl;

    // Expect very fast memory operations (millions per second)
    EXPECT_GT(writesPerSecond, 1000000.0);
    EXPECT_GT(readsPerSecond, 1000000.0);
}

TEST_F(PerformanceTest, DisplayUpdateSpeed) {
    // Create ROM that continuously draws sprites
    std::vector<std::uint8_t> displayRom = {0xA2, 0x30,  // I = 0x230
                                            0x60, 0x00,  // V0 = 0
                                            0x61, 0x00,  // V1 = 0
                                            0xD0, 0x15,  // Draw sprite
                                            0x70, 0x08,  // V0 += 8 (move right)
                                            0x40, 0x38,  // Skip if V0 != 56 (8*7)
                                            0x60, 0x00,  // Reset V0
                                            0x71, 0x08,  // V1 += 8 (move down)
                                            0x41, 0x18,  // Skip if V1 != 24 (8*3)
                                            0x61, 0x00,  // Reset V1
                                            0x12, 0x06,  // Jump to draw
                                            // Sprite data at 0x230
                                            0xF0, 0x90, 0x90, 0x90, 0xF0};

    createRom("display_test.ch8", displayRom);
    bool loadResult = emulator.loadRom("display_test.ch8");
    ASSERT_TRUE(loadResult);

    const int numDraws = 10000;
    int drawCount = 0;

    auto duration = measureExecutionTime([&]() {
        for (int i = 0; i < 100000 && drawCount < numDraws; ++i) {
            bool wasDrawing = emulator.getDrawFlag();
            emulator.emulateCycle();
            if (!wasDrawing && emulator.getDrawFlag()) {
                drawCount++;
                emulator.setDrawFlag(false);
            }
        }
    });

    if (drawCount > 0) {
        double drawsPerSecond =
            static_cast<double>(drawCount) / (static_cast<double>(duration.count()) / 1e9);

        std::cout << "Display updates: " << drawsPerSecond << " draws/second" << std::endl;

        // Expect reasonable drawing performance
        EXPECT_GT(drawsPerSecond, 1000.0);
    }
}

TEST_F(PerformanceTest, KeyboardResponseTime) {
    // Create ROM that waits for key press
    std::vector<std::uint8_t> keyRom = {
        0xF0, 0x0A,  // Wait for key press
        0x12, 0x02   // Jump to self
    };

    createRom("key_test.ch8", keyRom);
    bool loadResult = emulator.loadRom("key_test.ch8");
    ASSERT_TRUE(loadResult);

    const int numTests = 1000;

    auto duration = measureExecutionTime([&]() {
        for (int i = 0; i < numTests; ++i) {
            // Reset to waiting state
            emulator.setProgramCounter(Chip8::ROM_START_ADDRESS);

            // Execute wait instruction (should not advance)
            emulator.emulateCycle();

            // Press key
            emulator.setKeyState(5, true);

            // Execute with key pressed (should advance)
            emulator.emulateCycle();

            // Release key
            emulator.setKeyState(5, false);
        }
    });

    double testsPerSecond =
        static_cast<double>(numTests) / (static_cast<double>(duration.count()) / 1e9);

    std::cout << "Keyboard tests: " << testsPerSecond << " tests/second" << std::endl;

    EXPECT_GT(testsPerSecond, 10000.0);
}

TEST_F(PerformanceTest, MemoryIntensiveOperations) {
    // Test memory store/load operations performance
    std::vector<std::uint8_t> memRom = {
        0x60, 0x01,  // V0 = 1
        0x61, 0x02,  // V1 = 2
        0x62, 0x03,  // V2 = 3
        0x63, 0x04,  // V3 = 4
        0x64, 0x05,  // V4 = 5
        0x65, 0x06,  // V5 = 6
        0x66, 0x07,  // V6 = 7
        0x67, 0x08,  // V7 = 8
        0xA3, 0x00,  // I = 0x300
        0xF7, 0x55,  // Store V0-V7
        0xF7, 0x65,  // Load V0-V7
        0x12, 0x12   // Jump to store/load loop
    };

    createRom("memory_intensive.ch8", memRom);
    bool loadResult = emulator.loadRom("memory_intensive.ch8");
    ASSERT_TRUE(loadResult);

    // Skip initial setup
    for (int i = 0; i < 9; ++i) {
        emulator.emulateCycle();
    }

    const int numOperations = 10000;

    auto duration = measureExecutionTime([&]() {
        for (int i = 0; i < numOperations; ++i) {
            emulator.emulateCycle();  // Store operation
            emulator.emulateCycle();  // Load operation
            emulator.emulateCycle();  // Jump back
        }
    });

    double operationsPerSecond = static_cast<double>(numOperations * 2) /  // *2 for store+load
                                 (static_cast<double>(duration.count()) / 1e9);

    std::cout << "Memory operations: " << operationsPerSecond << " ops/second" << std::endl;

    EXPECT_GT(operationsPerSecond, 50000.0);
}

TEST_F(PerformanceTest, ErrorHandlingOverhead) {
    // Compare performance with and without error conditions
    const int numOperations = 100000;

    // Test valid operations
    auto validTime = measureExecutionTime([&]() {
        for (int i = 0; i < numOperations; ++i) {
            emulator.setRegisterAt(i % 16, static_cast<std::uint8_t>(i));
            // Note: errors handled via getLastError() if needed
        }
    });

    // Test invalid operations
    auto invalidTime = measureExecutionTime([&]() {
        for (int i = 0; i < numOperations; ++i) {
            emulator.setRegisterAt(16, static_cast<std::uint8_t>(i));
            // This should generate an error internally
        }
    });

    double validOpsPerSecond =
        static_cast<double>(numOperations) / (static_cast<double>(validTime.count()) / 1e9);
    double invalidOpsPerSecond =
        static_cast<double>(numOperations) / (static_cast<double>(invalidTime.count()) / 1e9);

    std::cout << "Valid operations: " << validOpsPerSecond << " ops/second" << std::endl;
    std::cout << "Invalid operations: " << invalidOpsPerSecond << " ops/second" << std::endl;

    // Error handling shouldn't be more than 1000x slower (relaxed due to stderr logging)
    double overhead = static_cast<double>(invalidTime.count()) / validTime.count();
    EXPECT_LT(overhead, 1000.0) << "Error handling overhead too high: " << overhead << "x";
}

TEST_F(PerformanceTest, FrameBufferAccess) {
    const int numAccesses = 100000;

    auto duration = measureExecutionTime([&]() {
        for (int i = 0; i < numAccesses; ++i) {
            auto frameBuffer = emulator.getFrameBuffer();
            // Access random pixel
            volatile auto pixel = frameBuffer[i % (64 * 32)];
            (void)pixel;  // Prevent optimization
        }
    });

    double accessesPerSecond =
        static_cast<double>(numAccesses) / (static_cast<double>(duration.count()) / 1e9);

    std::cout << "Frame buffer accesses: " << accessesPerSecond << " accesses/second" << std::endl;

    EXPECT_GT(accessesPerSecond, 1000000.0);
}

TEST_F(PerformanceTest, WorstCaseInstructionPerformance) {
    // Test the most complex instruction (draw sprite)
    std::vector<std::uint8_t> drawRom = {0xA2, 0x10,  // I = 0x210
                                         0x60, 0x1F,  // V0 = 31 (near edge)
                                         0x61, 0x1F,  // V1 = 31 (near edge)
                                         0xD0, 0x1F,  // Draw maximum height sprite
                                         0x12, 0x06,  // Jump back to draw
                                         // Large sprite data
                                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                         0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
                                         0xFF, 0xFF};

    createRom("draw_intensive.ch8", drawRom);
    bool loadResult = emulator.loadRom("draw_intensive.ch8");
    ASSERT_TRUE(loadResult);

    // Skip setup
    for (int i = 0; i < 3; ++i) {
        emulator.emulateCycle();
    }

    const int numDraws = 1000;

    auto duration = measureExecutionTime([&]() {
        for (int i = 0; i < numDraws; ++i) {
            emulator.emulateCycle();  // Draw instruction
            emulator.emulateCycle();  // Jump back
        }
    });

    double drawsPerSecond =
        static_cast<double>(numDraws) / (static_cast<double>(duration.count()) / 1e9);

    std::cout << "Complex draws: " << drawsPerSecond << " draws/second" << std::endl;

    // Even complex draws should be reasonably fast
    EXPECT_GT(drawsPerSecond, 1000.0);
}