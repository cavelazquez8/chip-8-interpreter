#include "../src/chip8.h"

#include "gtest/gtest.h"
#include <cstdint>

namespace {

TEST(_00EETest, Valid) {
  Chip8 chip8;

  chip8.setStackPointer(2);
  chip8.setStack(1, 0x300);

  chip8.setMemory(0x200, 0x00);
  chip8.setMemory(0x201, 0xEE);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x302);
  EXPECT_EQ(chip8.getStackPointer(), 1);
}

TEST(_00E0Test, Valid) {
  Chip8 chip8;

  for (int i = 0; i < 64 * 32; ++i) {
    chip8.frameBuffer[i] = 0;
  }

  chip8.setMemory(0x200, 0x00);
  chip8.setMemory(0x201, 0xE0);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getDrawFlag(), true);

  for (int i = 0; i < 64 * 32; ++i) {
    EXPECT_EQ(chip8.frameBuffer[i], 0);
  }

  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}

TEST(_1NNNTest, Valid) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0x10);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x001);
}

TEST(_2NNN, Valid) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0x20);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();
  EXPECT_EQ(chip8.getStackAt(0), 0x200);
  EXPECT_EQ(chip8.getStackPointer(), 1);
  EXPECT_EQ(chip8.getProgramCounter(), 0x001);
}

TEST(_3NNNTest, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);

  chip8.setMemory(0x200, 0x30);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x204);
}

TEST(_4NNNTest, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);

  chip8.setMemory(0x200, 0x40);
  chip8.setMemory(0x201, 0x02);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x204);
}

TEST(_5NNNTest, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x01);

  chip8.setMemory(0x200, 0x50);
  chip8.setMemory(0x201, 0x10);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x204);
}

TEST(_6XNNTest, Valid) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0x60);
  chip8.setMemory(0x201, 0x10);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x10);
}

TEST(_7XNNTest, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x09);
  chip8.setMemory(0x200, 0x70);
  chip8.setMemory(0x201, 0x10);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x19);
}

TEST(_8XY0Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x10);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x02);
}

TEST(_8XY1Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x11);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x03);
}

TEST(_8XY2Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x12);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x00);
}
TEST(_8XY3Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x13);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x03);
}
TEST(_8XY4Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x14);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x03);
}
TEST(_8XY4Test, Overflow) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0xFF);
  chip8.setRegisterAt(1, 0x01);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x14);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XY5Test, NoUnderflow) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x02);
  chip8.setRegisterAt(1, 0x01);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x15);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x01);
  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XY5Test, Underflow) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x00);
  chip8.setRegisterAt(1, 0x03);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x15);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 0);
}
TEST(_8XY6Test, LeastSigBit_1) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);

  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x16);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XY6Test, LeastSigBit_0) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x00);

  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x16);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 0);
}
TEST(_8XY7Test, Underflow) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x00);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x17);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 0);
}
TEST(_8XY7Test, NoUnderflow) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x00);
  chip8.setRegisterAt(1, 0x01);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x17);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XYETest, MostSigBit_1) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0xF0);

  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x1E);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XYETest, MostSigBit_0) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x00);

  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x1E);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 0);
}
TEST(_9XY0Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);

  chip8.setMemory(0x200, 0x90);
  chip8.setMemory(0x201, 0x10);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x204);
}
TEST(ANNNTest, Valid) {

  Chip8 chip8;

  chip8.setMemory(0x200, 0xA0);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getIndexRegister(), 0x0001);
}

TEST(BNNNTest, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);

  chip8.setMemory(0x200, 0xB2);
  chip8.setMemory(0x201, 0x05);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x206);
}
TEST(CXNNTest, Valid) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0xC0);
  chip8.setMemory(0x201, 0xFF);

  chip8.emulateCycle();

  std::uint8_t Vx = chip8.getRegisterAt(0);
  EXPECT_GE(Vx, 0);
  EXPECT_LE(Vx, 0xFF);
}
TEST(DXYN, DrawFont0) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0xD0);
  chip8.setMemory(0x201, 0x05);

  chip8.emulateCycle();

  std::uint8_t expectedFramebuffer[5][8] = {
      {1, 1, 1, 1, 0, 0, 0, 0}, // 0xF0 -> 1111 0000
      {1, 0, 0, 1, 0, 0, 0, 0}, // 0x90 -> 1001 0000
      {1, 0, 0, 1, 0, 0, 0, 0}, // 0x90 -> 1001 0000
      {1, 0, 0, 1, 0, 0, 0, 0}, // 0x90 -> 1001 0000
      {1, 1, 1, 1, 0, 0, 0, 0}  // 0xF0 -> 1111 0000
  };

  for (int i = 0; i < 5; ++i) {
    for (int j = 0; j < 8; ++j) {
      EXPECT_EQ(chip8.frameBuffer[i * 64 + j], expectedFramebuffer[i][j]);
    }
  }
  EXPECT_EQ(chip8.getIndexRegister(), 0);
  EXPECT_EQ(chip8.getRegisterAt(0xF), 0);
  EXPECT_EQ(chip8.getDrawFlag(), true);
}
TEST(DXYN, SettingRegisterFTo1) {
  Chip8 chip8;
  chip8.frameBuffer[0] = 1;
  chip8.setMemory(0x200, 0xD0);
  chip8.setMemory(0x201, 0x05);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.frameBuffer[0], 0);
  EXPECT_EQ(chip8.getIndexRegister(), 0);
  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
  EXPECT_EQ(chip8.getDrawFlag(), true);
}
TEST(EX9E, SkipNextInstruction) {
  Chip8 chip8;

  chip8.keyboard[0] = 1;
  chip8.setMemory(0x200, 0xE0);
  chip8.setMemory(0x201, 0x9E);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x204);
}
TEST(EX9E, DontSkipNextInstruction) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0xE0);
  chip8.setMemory(0x201, 0x9E);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}
TEST(EXA1, SkipNextInstruction) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0xE0);
  chip8.setMemory(0x201, 0xA1);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x204);
}
TEST(EXA1, DontSkipNextInstruction) {
  Chip8 chip8;

  chip8.keyboard[0] = 1;

  chip8.setMemory(0x200, 0xE0);
  chip8.setMemory(0x201, 0xA1);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}
TEST(FX07, setDelayTimer) {
  Chip8 chip8;

  chip8.setDelayTimer(1);
  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x07);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 1);
  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}
TEST(FX0A, setRegisterX) {
  Chip8 chip8;

  chip8.keyboard[8] = 1;

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x0A);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 8);
  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}
TEST(FX0A, DontSetRegisterX) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x0A);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0);
  EXPECT_EQ(chip8.getProgramCounter(), 0x200);
}
TEST(FX15, setDelayTimer) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 2);

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x15);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getDelayTimer(), 1);
  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}
TEST(FX18, setSoundTimer) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 2);

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x18);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getSoundTimer(), 1);
  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}

TEST(FX1E, setSoundTimer) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 2);

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x1E);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getIndexRegister(), 2);
  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}

TEST(FX29, setIndexRegister) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 2);

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x29);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getIndexRegister(), 10);
  EXPECT_EQ(chip8.getProgramCounter(), 0x202);
}

TEST(FX33, storeBCDinMemory_255) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0xFF);

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x33);

  chip8.emulateCycle();

  std::uint8_t indexRegister = chip8.getIndexRegister();
  EXPECT_EQ(chip8.getMemoryAt(indexRegister), 2);
  EXPECT_EQ(chip8.getMemoryAt(indexRegister + 1), 5);
  EXPECT_EQ(chip8.getMemoryAt(indexRegister + 2), 5);
}

TEST(FX33, storeBCDinMemory_001) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);

  chip8.setMemory(0x200, 0xF0);
  chip8.setMemory(0x201, 0x33);

  chip8.emulateCycle();

  std::uint8_t indexRegister = chip8.getIndexRegister();
  EXPECT_EQ(chip8.getMemoryAt(indexRegister), 0);
  EXPECT_EQ(chip8.getMemoryAt(indexRegister + 1), 0);
  EXPECT_EQ(chip8.getMemoryAt(indexRegister + 2), 1);
}

TEST(FX55, StoreFromV0ToV3) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x01);
  chip8.setRegisterAt(1, 0x02);
  chip8.setRegisterAt(2, 0x03);
  chip8.setRegisterAt(3, 0x04);

  chip8.setMemory(0x200, 0xF3);
  chip8.setMemory(0x201, 0x55);

  chip8.emulateCycle();

  std::uint8_t indexRegister = chip8.getIndexRegister();

  for (int i = 0; i <= 3; ++i) {
    EXPECT_EQ(chip8.getMemoryAt(i), i + 1);
  }
}

TEST(FX65, FillFromV0ToV3) {
  Chip8 chip8;

  chip8.setMemory(0x200, 0xF3);
  chip8.setMemory(0x201, 0x65);

  chip8.emulateCycle();

  std::uint8_t indexRegister = chip8.getIndexRegister();

  for (int i = 0; i <= 3; ++i) {
    EXPECT_EQ(chip8.getRegisterAt(i), chip8.getMemoryAt(indexRegister + i));
  }
}
} // namespace
