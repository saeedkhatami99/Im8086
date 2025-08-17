#!/bin/bash

# Test script for 8086 Emulator GUI with various OpenGL configurations
echo "=== Testing 8086 Emulator GUI ==="

BUILD_DIR="/home/fujitsu/DEV/Im8086/build"
if [ ! -f "$BUILD_DIR/8086emu" ]; then
    echo "Error: Executable not found at $BUILD_DIR/8086emu"
    echo "Please build the project first: cd build && make"
    exit 1
fi

cd "$BUILD_DIR"

echo ""
echo "1. Testing with hardware acceleration (normal):"
echo "   ./8086emu --gui"
timeout 10s ./8086emu --gui 2>&1 | head -20
echo ""

echo "2. Testing with software rendering (fallback):"
echo "   LIBGL_ALWAYS_SOFTWARE=1 ./8086emu --gui"
timeout 10s env LIBGL_ALWAYS_SOFTWARE=1 ./8086emu --gui 2>&1 | head -20
echo ""

echo "3. Testing with Mesa debug info:"
echo "   MESA_DEBUG=1 ./8086emu --gui"
timeout 10s env MESA_DEBUG=1 ./8086emu --gui 2>&1 | head -20
echo ""

echo "4. Testing with legacy OpenGL:"
echo "   MESA_GL_VERSION_OVERRIDE=2.1 ./8086emu --gui"
timeout 10s env MESA_GL_VERSION_OVERRIDE=2.1 ./8086emu --gui 2>&1 | head -20
echo ""

echo "=== Test Complete ==="
echo ""
echo "If none of the above worked, try:"
echo "1. Enable XFCE compositor: Settings Manager → Window Manager Tweaks → Compositor"
echo "2. Install latest Mesa drivers: sudo apt update && sudo apt upgrade"
echo "3. Restart your X session (logout/login)"
echo "4. Try running from a different terminal (not inside VS Code)"
