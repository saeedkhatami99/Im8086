#!/bin/bash

# Migration script from Makefile to CMake build system
# This script helps clean up old build artifacts and set up the new CMake build

set -e

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$PROJECT_ROOT/build"

echo "=== 8086 Emulator Build System Migration ==="
echo "Migrating from Makefile to CMake..."

# Clean up old build artifacts
echo "Cleaning up old build artifacts..."
if [ -d "$BUILD_DIR" ]; then
    echo "Removing old build directory..."
    rm -rf "$BUILD_DIR"
fi

# Remove any old object files that might be in src/
find "$PROJECT_ROOT/src" -name "*.o" -delete 2>/dev/null || true
find "$PROJECT_ROOT" -name "*.d" -delete 2>/dev/null || true

echo "Old build artifacts cleaned."

# Create new build directory
echo "Setting up CMake build..."
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR"

# Configure with CMake
echo "Configuring with CMake..."
cmake .. || {
    echo "ERROR: CMake configuration failed!"
    echo "Make sure you have run ./setup_gui.sh if you want GUI support."
    exit 1
}

# Build the project
echo "Building project..."
make -j$(nproc) || {
    echo "ERROR: Build failed!"
    exit 1
}

echo ""
echo "=== Migration Complete! ==="
echo ""
echo "New executable: $BUILD_DIR/8086emu"
echo ""
echo "Available make targets:"
echo "  make            - Build the executable"
echo "  make run        - Run in REPL mode"
echo "  make run-ide    - Run in IDE mode (if TUI enabled)"
echo "  make run-gui    - Run in GUI mode (if GUI enabled)"
echo "  make dist       - Create distribution package"
echo "  make help       - Show all available targets"
echo ""
echo "To run the emulator:"
echo "  cd build"
echo "  ./8086emu --help"
echo ""

# Test the executable
if [ -f "$BUILD_DIR/8086emu" ]; then
    echo "Testing executable..."
    cd "$BUILD_DIR"
    ./8086emu --help 2>/dev/null || echo "Help command test completed."
    echo ""
    echo "✓ Build successful! Executable is ready."
else
    echo "✗ Build failed - executable not found."
    exit 1
fi

echo ""
echo "Migration from Makefile to CMake completed successfully!"
echo "You can now remove the old Makefile if desired."
