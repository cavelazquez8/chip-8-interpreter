#!/bin/bash

# Uninstallation script for CHIP-8 Interpreter
set -e

PREFIX="${1:-/usr/local}"

echo "=== CHIP-8 Interpreter Uninstallation ==="
echo "Install prefix: ${PREFIX}"

# Check if we have write permissions
if [ ! -w "$(dirname "${PREFIX}")" ] && [ "${PREFIX}" != "${HOME}/.local" ]; then
    echo "Warning: You may need sudo permissions to uninstall from ${PREFIX}"
fi

echo "Removing CHIP-8 Interpreter files..."

# Remove binary
if [ -f "${PREFIX}/bin/chip8" ]; then
    rm -f "${PREFIX}/bin/chip8"
    echo "✓ Removed executable: ${PREFIX}/bin/chip8"
fi

# Remove libraries
rm -f "${PREFIX}/lib/libchip8_core.a"
rm -f "${PREFIX}/lib/libchip8_imgui.a"
echo "✓ Removed libraries from ${PREFIX}/lib/"

# Remove headers
if [ -d "${PREFIX}/include/chip8" ]; then
    rm -rf "${PREFIX}/include/chip8"
    echo "✓ Removed headers: ${PREFIX}/include/chip8/"
fi

# Remove pkg-config file
if [ -f "${PREFIX}/lib/pkgconfig/chip8.pc" ]; then
    rm -f "${PREFIX}/lib/pkgconfig/chip8.pc"
    echo "✓ Removed pkg-config file: ${PREFIX}/lib/pkgconfig/chip8.pc"
fi

# Remove documentation and data
if [ -d "${PREFIX}/share/chip8" ]; then
    rm -rf "${PREFIX}/share/chip8"
    echo "✓ Removed documentation: ${PREFIX}/share/chip8/"
fi

# Remove man page
if [ -f "${PREFIX}/share/man/man1/chip8.1" ]; then
    rm -f "${PREFIX}/share/man/man1/chip8.1"
    echo "✓ Removed man page: ${PREFIX}/share/man/man1/chip8.1"
fi

# Update library cache if uninstalling system-wide
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
echo "✓ CHIP-8 Interpreter uninstalled successfully!"