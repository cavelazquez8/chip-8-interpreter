#include "../src/chip8.h"

#include "gtest/gtest.h"

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
TEST(ANNNTest, Valid) {

  Chip8 chip8;

  chip8.setMemory(0x200, 0xA0);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getIndexRegister(), 0x0001);
}

} // namespace
