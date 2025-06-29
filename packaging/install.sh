#!/bin/bash

# Installation script for CHIP-8 Interpreter
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"
PREFIX="${1:-/usr/local}"

echo "=== CHIP-8 Interpreter Installation ==="
echo "Project root: ${PROJECT_ROOT}"
echo "Build directory: ${BUILD_DIR}"
echo "Install prefix: ${PREFIX}"

# Check if build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Error: Build directory not found. Please build the project first."
    echo "Run: mkdir build && cd build && cmake .. && cmake --build ."
    exit 1
fi

# Check if we have write permissions to the install directory
if [ ! -w "$(dirname "${PREFIX}")" ] && [ "${PREFIX}" != "${HOME}/.local" ]; then
    echo "Warning: You may need sudo permissions to install to ${PREFIX}"
    echo "Consider using a user-local installation: $0 \$HOME/.local"
fi

# Create install directories
echo "Creating install directories..."
mkdir -p "${PREFIX}/bin"
mkdir -p "${PREFIX}/lib"
mkdir -p "${PREFIX}/include/chip8"
mkdir -p "${PREFIX}/lib/pkgconfig"
mkdir -p "${PREFIX}/share/chip8"
mkdir -p "${PREFIX}/share/man/man1"

# Install binary
echo "Installing executable..."
if [ -f "${BUILD_DIR}/src/chip8" ]; then
    cp "${BUILD_DIR}/src/chip8" "${PREFIX}/bin/"
    chmod 755 "${PREFIX}/bin/chip8"
    echo "✓ Installed chip8 executable to ${PREFIX}/bin/"
else
    echo "Error: chip8 executable not found in build directory"
    exit 1
fi

# Install libraries
echo "Installing libraries..."
if [ -f "${BUILD_DIR}/src/libchip8_core.a" ]; then
    cp "${BUILD_DIR}/src/libchip8_core.a" "${PREFIX}/lib/"
    echo "✓ Installed libchip8_core.a to ${PREFIX}/lib/"
fi

if [ -f "${BUILD_DIR}/src/libchip8_imgui.a" ]; then
    cp "${BUILD_DIR}/src/libchip8_imgui.a" "${PREFIX}/lib/"
    echo "✓ Installed libchip8_imgui.a to ${PREFIX}/lib/"
fi

# Install headers
echo "Installing headers..."
cp "${PROJECT_ROOT}/src/chip8.h" "${PREFIX}/include/chip8/"
cp "${PROJECT_ROOT}/src/random.h" "${PREFIX}/include/chip8/"
echo "✓ Installed headers to ${PREFIX}/include/chip8/"

# Install ImGui headers if they exist
if [ -d "${PROJECT_ROOT}/src/imgui" ]; then
    mkdir -p "${PREFIX}/include/chip8/imgui"
    cp "${PROJECT_ROOT}/src/imgui"/*.h "${PREFIX}/include/chip8/imgui/"
    echo "✓ Installed ImGui headers to ${PREFIX}/include/chip8/imgui/"
fi

# Install pkg-config file
echo "Installing pkg-config file..."
if [ -f "${BUILD_DIR}/chip8.pc" ]; then
    cp "${BUILD_DIR}/chip8.pc" "${PREFIX}/lib/pkgconfig/"
    echo "✓ Installed pkg-config file to ${PREFIX}/lib/pkgconfig/"
fi

# Install ROM files
echo "Installing ROM files..."
if [ -d "${PROJECT_ROOT}/roms" ]; then
    mkdir -p "${PREFIX}/share/chip8/roms"
    cp "${PROJECT_ROOT}/roms"/*.ch8 "${PREFIX}/share/chip8/roms/" 2>/dev/null || true
    echo "✓ Installed ROM files to ${PREFIX}/share/chip8/roms/"
fi

# Install documentation
echo "Installing documentation..."
if [ -f "${PROJECT_ROOT}/README.md" ]; then
    cp "${PROJECT_ROOT}/README.md" "${PREFIX}/share/chip8/"
    echo "✓ Installed README to ${PREFIX}/share/chip8/"
fi

# Create a simple man page
echo "Creating man page..."
cat > "${PREFIX}/share/man/man1/chip8.1" << 'EOF'
.TH CHIP8 1 "2024" "1.0.0" "CHIP-8 Interpreter"
.SH NAME
chip8 \- CHIP-8 interpreter and emulator
.SH SYNOPSIS
.B chip8
.I ROM_FILE
.SH DESCRIPTION
A modern CHIP-8 interpreter/emulator written in C++20 with SDL2 graphics.
The CHIP-8 is a simple, interpreted programming language developed in the 1970s.
.SH OPTIONS
.TP
.I ROM_FILE
Path to the CHIP-8 ROM file to load and execute
.SH CONTROLS
The CHIP-8 keypad is mapped to your keyboard as follows:
.PP
.nf
CHIP-8     Keyboard
1 2 3 C    1 2 3 4
4 5 6 D    Q W E R
7 8 9 E    A S D F
A 0 B F    Z X C V
.fi
.SH EXAMPLES
.TP
Run a ROM file:
.B chip8 /usr/share/chip8/roms/maze.ch8
.SH FILES
.TP
.I /usr/share/chip8/roms/
Directory containing sample ROM files
.SH AUTHOR
Generated with Claude Code
.SH SEE ALSO
.BR sdl2 (1)
EOF
echo "✓ Created man page at ${PREFIX}/share/man/man1/chip8.1"

# Update library cache if installing system-wide
if [ "${PREFIX}" = "/usr/local" ] || [ "${PREFIX}" = "/usr" ]; then
    if command -v ldconfig >/dev/null 2>&1; then
        echo "Updating library cache..."
        sudo ldconfig 2>/dev/null || true
    fi
fi

# Update man page database
if command -v mandb >/dev/null 2>&1; then
    echo "Updating man page database..."
    sudo mandb -q 2>/dev/null || mandb -q 2>/dev/null || true
fi

echo
echo "=== Installation Summary ==="
echo "✓ Executable: ${PREFIX}/bin/chip8"
echo "✓ Libraries: ${PREFIX}/lib/libchip8_*.a"
echo "✓ Headers: ${PREFIX}/include/chip8/"
echo "✓ Documentation: ${PREFIX}/share/chip8/"
echo "✓ Man page: ${PREFIX}/share/man/man1/chip8.1"
echo
echo "Installation complete!"
echo
echo "To use the installed executable:"
echo "  chip8 /path/to/rom.ch8"
echo
echo "To use the library in your project:"
echo "  pkg-config --cflags --libs chip8"
echo
echo "To uninstall, run: packaging/uninstall.sh ${PREFIX}"