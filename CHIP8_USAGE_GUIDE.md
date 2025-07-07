# CHIP-8 INTERPRETER PROJECT USAGE GUIDE

This document contains comprehensive information about using and developing
the CHIP-8 interpreter project.

## TABLE OF CONTENTS
1. Basic Project Usage
2. Developer Guide
3. Development Scripts Deep Dive
4. Project Structure Overview

## 1. BASIC PROJECT USAGE

### PREREQUISITES
- CMake 3.14+
- SDL2 development libraries
- OpenGL libraries  
- C++17 compatible compiler

### STEP-BY-STEP USAGE

1. **Build the Project**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

2. **Run the Emulator**
   ```bash
   ./build/src/chip8 <rom_file>
   ```

3. **Try Sample ROMs**
   The project includes 3 sample ROMs:
   
   ```bash
   # Play Maze game
   ./build/src/chip8 roms/maze.ch8

   # Play Connect 4
   ./build/src/chip8 roms/connect4.ch8

   # Run Airplane demo
   ./build/src/chip8 roms/airplane.ch8
   ```

4. **Controls**
   The CHIP-8 uses a 16-key hexadecimal keypad (0-F) mapped to your 
   keyboard for game input.

5. **Optional: Run Tests**
   ```bash
   cd build
   ctest --output-on-failure
   ```

6. **Debug Build (Optional)**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
   cmake --build .
   ```

## 2. DEVELOPER GUIDE

### DEVELOPMENT SETUP

1. **Initial Build Setup**
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

2. **Debug Build with Coverage**
   ```bash
   mkdir build
   cd build
   cmake .. -DCMAKE_BUILD_TYPE=Debug -DENABLE_COVERAGE=ON
   cmake --build .
   ```

### TESTING FRAMEWORK

**Run All Tests:**
```bash
cd build
ctest --output-on-failure
```

**Test Structure:**
- GoogleTest Framework: Automatically fetched during build
- Test Files:
  * opcodes_test.cpp: Core opcode execution tests
  * chip8_test.cpp: General CHIP-8 functionality tests
  * error_handling_test.cpp: Error condition tests
  * integration_test.cpp: End-to-end integration tests
  * performance_test.cpp: Performance benchmarking tests

### CODE QUALITY TOOLS

**Format Code:**
```bash
./scripts/format_code.sh
```

**Static Analysis:**
```bash
./scripts/static_analysis.sh
```

This runs:
- clang-tidy: Advanced static analysis
- cppcheck: Additional static analysis with XML/text reports
- Format checking: Verifies code formatting compliance

**Manual Static Analysis:**
```bash
# Run cppcheck directly
cppcheck --std=c++20 src/

# Run clang-tidy (requires compile_commands.json)
cd build
cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
clang-tidy ../src/chip8.cpp -p .
```

### COVERAGE REPORTING

**Generate Coverage Report:**
```bash
cd build
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/googletest/*' -o coverage.info
genhtml coverage.info --output-directory coverage_html
```

### CONFIGURATION FILES
- .clang-format: Code formatting rules
- .clang-tidy: Static analysis configuration
- CMakeLists.txt: Build system configuration

### CI/CD INTEGRATION
GitHub Actions automatically runs:
- Build verification
- All test suites
- Code coverage reporting
- Static analysis with cppcheck

## 3. DEVELOPMENT SCRIPTS DEEP DIVE

### SCRIPT 1: format_code.sh

**Purpose:** Automatically formats all C++ source code using clang-format

**Location:** scripts/format_code.sh (lines 1-38)

**Workflow:**
1. Setup (line 6): Determines project root directory
2. Tool Check (lines 12-15): Verifies clang-format is installed
3. Source Formatting (lines 23-26): 
   - Finds all .cpp and .h files in src/
   - Applies clang-format in-place (-i flag)
4. Test Formatting (lines 31-34):
   - Finds all .cpp and .h files in tests/
   - Applies same formatting rules

**Key Features:**
- Uses .clang-format configuration file for consistent styling
- Processes files in-place (modifies original files)
- Handles both source and test directories

### SCRIPT 2: static_analysis.sh

**Purpose:** Comprehensive static analysis using multiple tools

**Location:** scripts/static_analysis.sh (lines 1-134)

**Workflow:**

1. **Environment Setup (lines 6-17):**
   - Sets project root and build directory paths
   - Validates build directory exists

2. **Tool Verification (lines 20-32):**
   - Checks for clang-tidy, cppcheck, clang-format availability
   - Exits if required tools missing

3. **clang-tidy Analysis (lines 34-57):**
   - Generates compile_commands.json if missing
   - Runs clang-tidy on all source files using .clang-tidy config
   - Analyzes both src/ and tests/ directories

4. **cppcheck Analysis (lines 59-92):**
   - Runs comprehensive analysis with all checks enabled
   - Generates both XML (cppcheck_report.xml) and human-readable 
     (cppcheck_report.txt) reports
   - Suppresses system includes and certain warnings

5. **Format Validation (lines 94-120):**
   - Checks if files comply with clang-format rules
   - Uses --dry-run --Werror to detect formatting issues
   - Counts and reports formatting violations

**Key Features:**
- Multi-tool analysis (clang-tidy + cppcheck + format checking)
- Generates multiple report formats
- Non-failing execution (continues on errors)
- Comprehensive C++20 standard compliance checking

### TOOL DEPENDENCIES & INTEGRATION

**Required Tools:**
- clang-format: Code formatting (both scripts)
- clang-tidy: Advanced static analysis
- cppcheck: Additional static analysis
- CMake: Build system integration

**Configuration Files:**
- .clang-format: Defines code style rules
- .clang-tidy: Static analysis rules and suppressions
- compile_commands.json: Generated by CMake for clang-tidy

**Integration Points:**
- Scripts require CMake build to be run first
- clang-tidy needs compile_commands.json from CMake
- Both scripts process same file patterns (*.cpp, *.h)
- Reports generated in build/ directory
- Error handling allows scripts to complete even with issues

**Usage Pattern:**
1. Run CMake build first
2. Use format_code.sh to fix formatting
3. Use static_analysis.sh for comprehensive analysis
4. Review generated reports in build/ directory

## 4. PROJECT STRUCTURE OVERVIEW

### DIRECTORY STRUCTURE
```
/
├── CLAUDE.md              # Project instructions for Claude Code
├── CMakeLists.txt          # Main build configuration
├── README.md               # Project documentation
├── build/                  # Generated build files
├── cmake/                  # CMake modules
│   ├── Chip8Config.cmake.in
│   └── FindSDL2.cmake
├── docs/                   # Documentation
│   ├── API.md
│   ├── ARCHITECTURE.md
│   └── BUILDING.md
├── packaging/              # Installation scripts
│   ├── chip8.pc.in
│   ├── install.sh
│   ├── uninstall.cmake.in
│   └── uninstall.sh
├── roms/                   # Sample CHIP-8 ROM files
│   ├── airplane.ch8
│   ├── connect4.ch8
│   └── maze.ch8
├── scripts/                # Development scripts
│   ├── format_code.sh
│   └── static_analysis.sh
├── src/                    # Main source code
│   ├── CMakeLists.txt
│   ├── chip8.cpp           # Core emulator implementation
│   ├── chip8.h
│   ├── imgui/              # ImGui library files
│   ├── main.cpp            # SDL2 frontend application
│   └── random.h            # Random number utilities
└── tests/                  # GoogleTest unit tests
    ├── CMakeLists.txt
    ├── chip8_test.cpp
    ├── error_handling_test.cpp
    ├── integration_test.cpp
    ├── opcodes_test.cpp
    └── performance_test.cpp
```

### CORE COMPONENTS

**Chip8 class (src/chip8.h, src/chip8.cpp):** Main emulator engine containing:
- Memory management (4KB RAM)
- CPU registers (16 8-bit registers V0-VF)
- Stack (16 levels for subroutines)
- Timers (delay and sound)
- Display buffer (64x32 monochrome)
- Keyboard input handling (16-key hexadecimal keypad)
- Opcode execution and cycle emulation

**Main Application (src/main.cpp):** SDL2-based frontend that:
- Initializes SDL2 window and renderer
- Handles main event loop
- Manages keyboard input mapping
- Renders the display buffer to screen

**ImGui Integration (src/imgui/):** Embedded ImGui library for GUI elements

### MEMORY LAYOUT
- 0x000-0x1FF: Reserved for CHIP-8 interpreter
- 0x200-0xFFF: Program ROM and RAM (3584 bytes)
- Display: 64x32 pixel monochrome framebuffer
- Stack: 16 levels, 16-bit addresses

### DEPENDENCIES
- CMake 3.14+
- SDL2 development libraries
- OpenGL libraries
- C++17 compatible compiler
- GoogleTest (automatically fetched during build)

---

This guide was generated on 2025-06-26 for the CHIP-8 interpreter project.
For updates or questions, refer to the project documentation in the docs/ 
directory or the CLAUDE.md file for development guidance.