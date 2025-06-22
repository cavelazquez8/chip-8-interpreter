#!/bin/bash

# Static Analysis Script for CHIP-8 Interpreter
set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
BUILD_DIR="${PROJECT_ROOT}/build"

echo "=== CHIP-8 Static Analysis ==="
echo "Project root: ${PROJECT_ROOT}"
echo "Build directory: ${BUILD_DIR}"

# Ensure build directory exists
if [ ! -d "${BUILD_DIR}" ]; then
    echo "Build directory not found. Please run CMake first."
    exit 1
fi

# Check for required tools
check_tool() {
    if ! command -v "$1" &> /dev/null; then
        echo "Error: $1 is not installed or not in PATH"
        return 1
    fi
    echo "✓ Found $1: $(command -v "$1")"
}

echo
echo "=== Checking for required tools ==="
check_tool clang-tidy
check_tool cppcheck
check_tool clang-format

# Run clang-tidy
echo
echo "=== Running clang-tidy ==="
cd "${BUILD_DIR}"

# Generate compile_commands.json if it doesn't exist
if [ ! -f "compile_commands.json" ]; then
    echo "Generating compile commands..."
    cmake .. -DCMAKE_EXPORT_COMPILE_COMMANDS=ON
fi

# Run clang-tidy on source files
echo "Running clang-tidy on source files..."
find "${PROJECT_ROOT}/src" -name "*.cpp" -o -name "*.h" | while read -r file; do
    echo "Analyzing: $(basename "$file")"
    clang-tidy "$file" -p "${BUILD_DIR}" --config-file="${PROJECT_ROOT}/.clang-tidy" || true
done

# Run clang-tidy on test files
echo "Running clang-tidy on test files..."
find "${PROJECT_ROOT}/tests" -name "*.cpp" -o -name "*.h" | while read -r file; do
    echo "Analyzing: $(basename "$file")"
    clang-tidy "$file" -p "${BUILD_DIR}" --config-file="${PROJECT_ROOT}/.clang-tidy" || true
done

# Run cppcheck
echo
echo "=== Running cppcheck ==="
cppcheck \
    --enable=all \
    --std=c++20 \
    --language=c++ \
    --platform=unix64 \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --suppress=useStlAlgorithm \
    --inline-suppr \
    --quiet \
    --xml \
    --xml-version=2 \
    --output-file="${BUILD_DIR}/cppcheck_report.xml" \
    "${PROJECT_ROOT}/src" "${PROJECT_ROOT}/tests" 2>&1 || true

echo "cppcheck XML report saved to: ${BUILD_DIR}/cppcheck_report.xml"

# Generate human-readable cppcheck report
cppcheck \
    --enable=all \
    --std=c++20 \
    --language=c++ \
    --platform=unix64 \
    --suppress=missingIncludeSystem \
    --suppress=unmatchedSuppression \
    --suppress=useStlAlgorithm \
    --inline-suppr \
    --template="[{severity}][{id}] {message} ({file}:{line})" \
    "${PROJECT_ROOT}/src" "${PROJECT_ROOT}/tests" 2>&1 | tee "${BUILD_DIR}/cppcheck_report.txt" || true

echo "cppcheck text report saved to: ${BUILD_DIR}/cppcheck_report.txt"

# Check code formatting
echo
echo "=== Checking code formatting ==="
format_issues=0

check_format() {
    local file="$1"
    if ! clang-format --dry-run --Werror "$file" &>/dev/null; then
        echo "Format issue in: $file"
        ((format_issues++))
    fi
}

find "${PROJECT_ROOT}/src" -name "*.cpp" -o -name "*.h" | while read -r file; do
    check_format "$file"
done

find "${PROJECT_ROOT}/tests" -name "*.cpp" -o -name "*.h" | while read -r file; do
    check_format "$file"
done

if [ $format_issues -eq 0 ]; then
    echo "✓ All files are properly formatted"
else
    echo "⚠ Found $format_issues files with formatting issues"
    echo "Run 'scripts/format_code.sh' to fix formatting"
fi

# Summary
echo
echo "=== Analysis Summary ==="
echo "✓ clang-tidy analysis completed"
echo "✓ cppcheck analysis completed"
echo "✓ Format check completed"
echo
echo "Reports generated in ${BUILD_DIR}:"
echo "  - cppcheck_report.xml (machine-readable)"
echo "  - cppcheck_report.txt (human-readable)"
echo
echo "To fix formatting issues, run: scripts/format_code.sh"
echo "Static analysis complete!"