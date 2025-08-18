#!/bin/bash

set -e
echo "========================================"
echo "Building Im8086 Ultra-Portable Binary"
echo "========================================"
echo "This script creates the most portable Linux binary possible."
echo "Target compatibility: Any Linux distribution with kernel 3.2.0+ (2012+)"
echo ""
rm -rf build-ultra-portable
mkdir build-ultra-portable
cd build-ultra-portable
echo "Configuring for ultra-portable static build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC=ON \
    -DBUILD_TESTS=ON \
    -DCMAKE_CXX_FLAGS="-D_GLIBCXX_USE_CXX11_ABI=0 -static-libgcc -static-libstdc++" \
    -DCMAKE_EXE_LINKER_FLAGS="-static" \
    -DCMAKE_FIND_LIBRARY_SUFFIXES=".a"

echo "Building..."
make -j$(nproc)
echo "Testing..."
./test_emulator
echo "Verifying portability..."
echo "File type:"
file Im8086
echo ""
echo "Library dependencies (should show 'not a dynamic executable'):"
ldd Im8086 || echo "✓ Statically linked - maximum portability!"
echo ""
echo "Creating ultra-portable package..."
mkdir -p Im8086-Ultra-Portable-Linux
cp Im8086 Im8086-Ultra-Portable-Linux/Im8086-static
cp test_emulator Im8086-Ultra-Portable-Linux/
cp -r ../samples Im8086-Ultra-Portable-Linux/
cp ../README.md Im8086-Ultra-Portable-Linux/
cat > Im8086-Ultra-Portable-Linux/README-PORTABLE.txt << 'EOF'
Im8086 Ultra-Portable Linux Binary
===================================

This is a statically linked version of Im8086 for maximum portability.

COMPATIBILITY:
- Works on ANY Linux distribution
- No library dependencies required
- Runs on systems as old as Linux kernel 3.2.0+ (2012+)
- Compatible with: Ubuntu 12.04+, CentOS 6+, RHEL 6+, Debian 7+, etc.

FEATURES:
- TUI mode only (for maximum portability)
- Full 8086 instruction set emulation
- Interactive debugger and IDE
- No installation required

USAGE:
  ./Im8086-static                     - Interactive command line mode
  ./Im8086-static --ide               - IDE mode with integrated editor
  ./Im8086-static --tui program.asm   - TUI debugger mode
  ./Im8086-static --help              - Show all options

NOTES:
- GUI mode is not available in this ultra-portable build
- For GUI features, use the regular portable build
- This binary is completely self-contained
EOF

tar -czf Im8086-Ultra-Portable-Linux.tar.gz Im8086-Ultra-Portable-Linux/
echo "========================================"
echo "Ultra-portable build complete!"
echo ""
echo "Package: build-ultra-portable/Im8086-Ultra-Portable-Linux.tar.gz"
echo "Binary: Im8086-static (works on any Linux system)"
echo "Compatibility: Linux kernel 3.2.0+ (2012+)"
echo "Dependencies: NONE"
echo "========================================"
