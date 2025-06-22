# Building the CHIP-8 Interpreter

This guide covers building, testing, and installing the CHIP-8 interpreter from source.

## Prerequisites

### Required Dependencies

- **CMake 3.20+**: Build system
- **C++20 Compiler**: GCC 10+, Clang 11+, or MSVC 2019+
- **SDL2**: Graphics and input handling
- **OpenGL**: Hardware-accelerated rendering

### Optional Dependencies

- **lcov**: Code coverage reports
- **clang-tidy**: Static analysis
- **clang-format**: Code formatting
- **pkg-config**: Dependency management

## Platform-Specific Setup

### Ubuntu/Debian

```bash
# Install required packages
sudo apt update
sudo apt install -y \
    cmake \
    build-essential \
    libsdl2-dev \
    libgl1-mesa-dev \
    pkg-config

# Optional packages for development
sudo apt install -y \
    lcov \
    clang-tidy \
    clang-format \
    doxygen
```

### macOS

```bash
# Install Homebrew if not already installed
/bin/bash -c "$(curl -fsSL https://raw.githubusercontent.com/Homebrew/install/HEAD/install.sh)"

# Install dependencies
brew install cmake sdl2 pkg-config

# Optional development tools
brew install lcov llvm doxygen
```

### Windows

#### Using vcpkg

```powershell
# Install vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# Install dependencies
.\vcpkg install sdl2:x64-windows

# Set environment variable
$env:CMAKE_TOOLCHAIN_FILE = "C:\path\to\vcpkg\scripts\buildsystems\vcpkg.cmake"
```

#### Using MSYS2

```bash
# Install MSYS2 dependencies
pacman -S mingw-w64-x86_64-cmake \
          mingw-w64-x86_64-gcc \
          mingw-w64-x86_64-SDL2 \
          mingw-w64-x86_64-pkg-config
```

## Basic Build

### Quick Start

```bash
# Clone the repository
git clone <repository-url>
cd chip-8-interpreter

# Create build directory
mkdir build
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build .

# Run tests
ctest --output-on-failure

# Run the emulator
./src/chip8 ../roms/maze.ch8
```

### Build Configuration Options

```bash
# Release build (optimized)
cmake .. -DCMAKE_BUILD_TYPE=Release

# Debug build with symbols
cmake .. -DCMAKE_BUILD_TYPE=Debug

# Disable tests
cmake .. -DCHIP8_BUILD_TESTS=OFF

# Enable coverage reporting
cmake .. -DCHIP8_ENABLE_COVERAGE=ON

# Enable sanitizers
cmake .. -DCHIP8_ENABLE_SANITIZERS=ON

# Enable static analysis
cmake .. -DCHIP8_ENABLE_STATIC_ANALYSIS=ON
```

## Advanced Build Options

### Coverage Build

```bash
mkdir build-coverage
cd build-coverage

cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCHIP8_ENABLE_COVERAGE=ON

cmake --build .
ctest --output-on-failure

# Generate coverage report
make coverage
# or
ninja coverage

# View coverage report
open coverage_html/index.html
```

### Sanitizer Build

```bash
mkdir build-sanitized
cd build-sanitized

cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCHIP8_ENABLE_SANITIZERS=ON

cmake --build .

# Run with sanitizers
./src/chip8 ../roms/maze.ch8
```

### Static Analysis

```bash
# Configure with analysis tools
cmake .. \
    -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
    -DCHIP8_ENABLE_STATIC_ANALYSIS=ON

# Run analysis script
../scripts/static_analysis.sh

# Format code
../scripts/format_code.sh
```

## Testing

### Running Tests

```bash
# Run all tests
ctest

# Run with verbose output
ctest --output-on-failure

# Run specific test
ctest -R chip8_test

# Run tests in parallel
ctest -j $(nproc)

# Run performance tests
ctest -R performance_test
```

### Test Categories

- **Unit Tests**: Core functionality
- **Integration Tests**: Full emulation scenarios
- **Error Handling Tests**: Error condition coverage
- **Performance Tests**: Benchmark critical paths

### Coverage Analysis

```bash
# Generate coverage report
lcov --capture --directory . --output-file coverage.info
lcov --remove coverage.info '/usr/*' '*/googletest/*' -o coverage.info
genhtml coverage.info --output-directory coverage_html
```

## Installation

### System Installation

```bash
# Configure for installation
cmake .. -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
cmake --build .

# Install (may require sudo)
sudo cmake --install .

# Verify installation
chip8 --help
pkg-config --cflags --libs chip8
```

### User Installation

```bash
# Install to user directory
cmake .. -DCMAKE_INSTALL_PREFIX=$HOME/.local
cmake --build .
cmake --install .

# Add to PATH if needed
echo 'export PATH=$HOME/.local/bin:$PATH' >> ~/.bashrc
```

### Custom Installation

```bash
# Custom prefix
cmake .. -DCMAKE_INSTALL_PREFIX=/opt/chip8
cmake --build .
sudo cmake --install .
```

### Uninstallation

```bash
# Using CMake
sudo cmake --build . --target uninstall

# Using packaging script
sudo ../packaging/uninstall.sh /usr/local
```

## Cross-Compilation

### Linux ARM64

```bash
# Install cross-compiler
sudo apt install gcc-aarch64-linux-gnu g++-aarch64-linux-gnu

# Configure for ARM64
cmake .. \
    -DCMAKE_SYSTEM_NAME=Linux \
    -DCMAKE_SYSTEM_PROCESSOR=aarch64 \
    -DCMAKE_C_COMPILER=aarch64-linux-gnu-gcc \
    -DCMAKE_CXX_COMPILER=aarch64-linux-gnu-g++

cmake --build .
```

### Windows from Linux

```bash
# Install MinGW
sudo apt install mingw-w64

# Configure for Windows
cmake .. \
    -DCMAKE_SYSTEM_NAME=Windows \
    -DCMAKE_C_COMPILER=x86_64-w64-mingw32-gcc \
    -DCMAKE_CXX_COMPILER=x86_64-w64-mingw32-g++ \
    -DCMAKE_FIND_ROOT_PATH=/usr/x86_64-w64-mingw32

cmake --build .
```

## Development Workflow

### Code Quality Checks

```bash
# Format code
./scripts/format_code.sh

# Run static analysis
./scripts/static_analysis.sh

# Full quality check
cmake --build . --target all
ctest --output-on-failure
./scripts/static_analysis.sh
```

### Debugging Build

```bash
cmake .. \
    -DCMAKE_BUILD_TYPE=Debug \
    -DCMAKE_CXX_FLAGS="-g -O0 -DDEBUG"

cmake --build .

# Debug with GDB
gdb ./src/chip8
(gdb) run ../roms/maze.ch8
```

### Performance Profiling

```bash
# Build with profiling
cmake .. -DCMAKE_CXX_FLAGS="-pg -O2"
cmake --build .

# Run with profiling
./src/chip8 ../roms/maze.ch8

# Analyze profile
gprof ./src/chip8 gmon.out > profile.txt
```

## IDE Integration

### Visual Studio Code

Create `.vscode/settings.json`:

```json
{
    "cmake.configureSettings": {
        "CMAKE_BUILD_TYPE": "Debug",
        "CHIP8_BUILD_TESTS": "ON"
    },
    "C_Cpp.default.configurationProvider": "ms-vscode.cmake-tools"
}
```

### CLion

1. Open project directory
2. Configure CMake profiles in Settings
3. Set build configurations as needed

### Visual Studio

1. Open folder in Visual Studio
2. Configure CMake settings in CMakeSettings.json
3. Build and debug as normal

## Troubleshooting

### Common Issues

#### SDL2 Not Found

```bash
# Ubuntu/Debian
sudo apt install libsdl2-dev

# macOS
brew install sdl2

# Set PKG_CONFIG_PATH if needed
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH
```

#### CMake Version Too Old

```bash
# Ubuntu: Install from Kitware APT repository
wget -O - https://apt.kitware.com/keys/kitware-archive-latest.asc | sudo apt-key add -
sudo apt-add-repository 'deb https://apt.kitware.com/ubuntu/ focal main'
sudo apt update
sudo apt install cmake

# macOS
brew install cmake

# From source
wget https://github.com/Kitware/CMake/releases/download/v3.25.0/cmake-3.25.0.tar.gz
tar -xzf cmake-3.25.0.tar.gz
cd cmake-3.25.0
./bootstrap && make && sudo make install
```

#### Compiler Errors

```bash
# Ensure C++20 support
g++ --version  # Should be 10.0+
clang++ --version  # Should be 11.0+

# Update compiler if needed
sudo apt install gcc-11 g++-11
sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-11 100
sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-11 100
```

#### Test Failures

```bash
# Run tests individually for debugging
ctest -R specific_test_name -V

# Check test output
cat Testing/Temporary/LastTest.log

# Run with memory debugging
valgrind --leak-check=full ./tests/chip8_tests
```

### Performance Issues

```bash
# Ensure release build
cmake .. -DCMAKE_BUILD_TYPE=Release

# Check optimization flags
cmake .. -DCMAKE_CXX_FLAGS="-O3 -DNDEBUG"

# Profile performance
perf record ./src/chip8 ../roms/maze.ch8
perf report
```

### Getting Help

- Check the [troubleshooting guide](TROUBLESHOOTING.md)
- Review the [architecture documentation](ARCHITECTURE.md)
- File an issue on the project repository
- Join the community discussion forum