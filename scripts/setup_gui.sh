#!/bin/bash

set -e
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
THIRD_PARTY_DIR="$PROJECT_ROOT/third_party"
IMGUI_DIR="$THIRD_PARTY_DIR/imgui"

echo "Setting up 8086 Emulator GUI dependencies..."

if [ -d "$PROJECT_ROOT/.git" ] && [ -f "$PROJECT_ROOT/.gitmodules" ]; then
    echo "Git repository detected with submodules configuration"
    if [ ! -d "$IMGUI_DIR" ] || [ ! "$(ls -A "$IMGUI_DIR")" ]; then
        echo "Initializing ImGui submodule..."
        cd "$PROJECT_ROOT"
        git submodule update --init --recursive third_party/imgui
        echo "ImGui submodule initialized successfully"
    else
        echo "ImGui submodule already exists and is populated"
        cd "$IMGUI_DIR"
        if git describe --tags 2>/dev/null | grep -q "v1.90.1"; then
            echo "ImGui v1.90.1 confirmed"
        else
            echo "Updating ImGui submodule to correct version..."
            cd "$PROJECT_ROOT"
            git submodule update --init --recursive third_party/imgui
        fi
    fi
else
    echo "No git repository or submodules detected, falling back to direct download"
    mkdir -p "$THIRD_PARTY_DIR"
    if [ ! -d "$IMGUI_DIR" ]; then
        echo "Downloading ImGui v1.90.1..."
        cd "$THIRD_PARTY_DIR"  
        git clone --depth 1 --branch v1.90.1 https://github.com/ocornut/imgui.git
        echo "ImGui downloaded successfully"
    else
        echo "ImGui already exists in $IMGUI_DIR"
    fi
fi
echo "Checking system dependencies..."
command_exists() {
    command -v "$1" >/dev/null 2>&1
}
if ! command_exists cmake; then
    echo "ERROR: cmake is required but not installed"
    echo "Please install cmake (version 3.16 or higher)"
    exit 1
fi
if ! command_exists pkg-config; then
    echo "ERROR: pkg-config is required but not installed"
    exit 1
fi
case "$(uname -s)" in
    Linux*)
        echo "Detected Linux"  
        if ! pkg-config --exists sdl2; then
            echo "ERROR: SDL2 development libraries not found"
            echo "Please install SDL2:"
            echo "  Ubuntu/Debian: sudo apt-get install libsdl2-dev"
            echo "  Fedora/RHEL: sudo dnf install SDL2-devel"
            echo "  Arch: sudo pacman -S sdl2"
            exit 1
        fi
        if ! pkg-config --exists gl; then
            echo "WARNING: OpenGL development libraries might not be available"
            echo "You might need to install mesa development packages:"
            echo "  Ubuntu/Debian: sudo apt-get install libgl1-mesa-dev"
            echo "  Fedora/RHEL: sudo dnf install mesa-libGL-devel"
        fi
        if pkg-config --exists ncurses; then
            echo "Found ncurses - TUI support will be available"
        else
            echo "ncurses not found - TUI support will be disabled"
            echo "To enable TUI: sudo apt-get install libncurses5-dev (Ubuntu/Debian)"
        fi
        ;;     
    Darwin*)
        echo "Detected macOS"
        if command_exists brew; then
            echo "Homebrew detected"
            if ! brew list sdl2 &>/dev/null; then
                echo "SDL2 not found via Homebrew"
                echo "Install with: brew install sdl2"
                exit 1
            fi
        else
            echo "WARNING: Homebrew not found. You may need to install dependencies manually."
        fi
        ;;   
    MINGW*|MSYS*|CYGWIN*)
        echo "Detected Windows (MSYS2/MinGW)"
        echo "Please ensure you have installed:"
        echo "  pacman -S mingw-w64-x86_64-SDL2"
        echo "  pacman -S mingw-w64-x86_64-cmake"
        ;;
        
    *)
        echo "Unknown platform: $(uname -s)"
        echo "Please ensure SDL2 and OpenGL development libraries are installed"
        ;;
esac
echo "Setup complete!"
echo ""
echo "To build the project:"
echo "  mkdir build && cd build"
echo "  cmake .."
echo "  make -j\$(nproc)"
echo ""
echo "To run:"
echo "  ./Im8086 --gui"
