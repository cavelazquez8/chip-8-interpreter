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
TEST(_8XY5Test, Valid) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x02);
  chip8.setRegisterAt(1, 0x01);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x15);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0), 0x01);
  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XY5Test, Overflow) {
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
  chip8.setRegisterAt(1, 0x00);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x16);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 1);
}
TEST(_8XY6Test, LeastSigBit_0) {
  Chip8 chip8;

  chip8.setRegisterAt(0, 0x00);
  chip8.setRegisterAt(1, 0x00);
  chip8.setMemory(0x200, 0x80);
  chip8.setMemory(0x201, 0x16);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getRegisterAt(0xF), 0);
}
TEST(ANNNTest, Valid) {

  Chip8 chip8;

  chip8.setMemory(0x200, 0xA0);
  chip8.setMemory(0x201, 0x01);

  chip8.emulateCycle();

  EXPECT_EQ(chip8.getIndexRegister(), 0x0001);
}

} // namespace
