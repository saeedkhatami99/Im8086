#!/bin/bash

set -e
echo "======================================"
echo "Building Im8086 Static Linux Binary"
echo "======================================"
rm -rf build-static
mkdir build-static
cd build-static
echo "Configuring with static linking..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_STATIC=ON \
    -DBUILD_TESTS=ON \
    -DCMAKE_CXX_FLAGS="-static-libgcc -static-libstdc++" \
    -DCMAKE_EXE_LINKER_FLAGS="-static"
echo "Building..."
make -j$(nproc)
echo "Testing..."
./test_emulator
echo "Creating portable package..."
mkdir -p Im8086-Linux-Static
cp Im8086 Im8086-Linux-Static/
cp test_emulator Im8086-Linux-Static/
cp test_gui Im8086-Linux-Static/ 2>/dev/null || echo "GUI not built (missing dependencies)"
cp -r ../samples Im8086-Linux-Static/
cp ../README.md Im8086-Linux-Static/
echo "Checking dependencies of static binary:"
ldd Im8086 || echo "Statically linked (no dependencies)"
tar -czf Im8086-Linux-Static.tar.gz Im8086-Linux-Static/
echo "======================================"
echo "Static build complete!"
echo "Package: build-static/Im8086-Linux-Static.tar.gz"
echo "======================================"
