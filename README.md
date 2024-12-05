# Chip-8 Interpreter

## Overview

This project is a Chip-8 emulator written in C++. It supports all standard Chip-8 opcodes and uses SDL2 for rendering and input handling. This emulator is a great way to learn about emulation and low-level programming concepts.

## Features

- Full support for Chip-8 instruction set.
- Keyboard input handling.
- Display rendering using SDL2.
- Unit tests for key opcodes.

## Building the Project

### Prerequisites

- CMake 3.2 or higher
- SDL2 development libraries
- A C++17 compatible compiler

### Building

```sh
git clone https://github.com/cavelazquez8/chip-8-interpreter.git
cd chip-8-interpreter
mkdir build
cd build
cmake ..
make
