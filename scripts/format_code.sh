#!/bin/bash

# Code Formatting Script for CHIP-8 Interpreter
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "=== CHIP-8 Code Formatter ==="
echo "Project root: ${PROJECT_ROOT}"

# Check for clang-format
if ! command -v clang-format &> /dev/null; then
    echo "Error: clang-format is not installed or not in PATH"
    exit 1
fi

echo "✓ Found clang-format: $(command -v clang-format)"
echo "Version: $(clang-format --version)"

# Format source files
echo
echo "=== Formatting source files ==="
find "${PROJECT_ROOT}/src" -name "*.cpp" -o -name "*.h" | while read -r file; do
    echo "Formatting: $(basename "$file")"
    clang-format -i "$file"
done

# Format test files
echo
echo "=== Formatting test files ==="
find "${PROJECT_ROOT}/tests" -name "*.cpp" -o -name "*.h" | while read -r file; do
    echo "Formatting: $(basename "$file")"
    clang-format -i "$file"
done

echo
echo "✓ Code formatting complete!"
echo "All C++ files have been formatted according to project style."