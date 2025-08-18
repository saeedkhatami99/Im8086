#!/bin/bash

set -e
echo "========================================"
echo "Building Im8086 Portable Linux Binary"
echo "========================================"
rm -rf build-portable
mkdir build-portable
cd build-portable
echo "Configuring for portable build..."
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_PORTABLE=ON \
    -DBUILD_TESTS=ON \
    -DCMAKE_INSTALL_RPATH='$ORIGIN/lib'
echo "Building..."
make -j$(nproc)
echo "Testing..."
./test_emulator
echo "Creating portable package with bundled libraries..."
mkdir -p Im8086-Linux-Portable
mkdir -p Im8086-Linux-Portable/lib
cp Im8086 Im8086-Linux-Portable/
cp test_emulator Im8086-Linux-Portable/
cp test_gui Im8086-Linux-Portable/ 2>/dev/null || echo "GUI not built"
cp -r ../samples Im8086-Linux-Portable/
cp ../README.md Im8086-Linux-Portable/
if [ -f "Im8086" ] && ldd Im8086 | grep -q SDL2; then
    echo "Bundling SDL2 and other shared libraries..."
    LIBS=$(ldd Im8086 | grep -E "(SDL2|ncurses)" | awk '{print $3}' | grep -v "not found")
    for lib in $LIBS; do
        if [ -f "$lib" ]; then
            echo "Bundling: $lib"
            cp "$lib" Im8086-Linux-Portable/lib/
        fi
    done
    cat > Im8086-Linux-Portable/launch.sh << 'EOF'
#!/bin/bash
DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
export LD_LIBRARY_PATH="$DIR/lib:$LD_LIBRARY_PATH"
exec "$DIR/Im8086" "$@"
EOF
    chmod +x Im8086-Linux-Portable/launch.sh
    echo "Use ./launch.sh to run the application with bundled libraries"
fi
tar -czf Im8086-Linux-Portable.tar.gz Im8086-Linux-Portable/
echo "========================================"
echo "Portable build complete!"
echo "Package: build-portable/Im8086-Linux-Portable.tar.gz"
echo "========================================"
