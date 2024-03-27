#include "../src/chip8.h"

#include "gtest/gtest.h"

namespace {

TEST(ANNNTest, Valid) {

  Chip8 chip8;

  chip8.setMemory(0x200, 0xA0);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getIndexRegister(), 0x0001);
}

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

} // namespace
