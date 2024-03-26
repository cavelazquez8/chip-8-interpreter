#include "../src/chip8.h"

#include "gtest/gtest.h"

namespace {

TEST(OpcodesTest, ValidANNN) {

  Chip8 chip8;

  chip8.setMem(0x200, 0xA0);
  chip8.setMem(0x201, 0x01);

  chip8.emulateCycle();

  ASSERT_EQ(chip8.getIndexRegister(), 0x0001);
}

} // namespace
