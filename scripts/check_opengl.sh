#!/bin/bash

echo "=== OpenGL Diagnostic Tool ==="
echo "Checking OpenGL support on your system..."
echo ""
echo "GPU Information:"
lspci | grep -i vga
lspci | grep -i 3d
echo ""
echo "OpenGL Support:"
if command -v glxinfo >/dev/null 2>&1; then
    glxinfo | grep "OpenGL version"
    glxinfo | grep "OpenGL renderer"
    glxinfo | grep "OpenGL vendor"
    glxinfo | grep "direct rendering"
else
    echo "glxinfo not found. Installing mesa-utils..."
    sudo apt-get update && sudo apt-get install -y mesa-utils
    if command -v glxinfo >/dev/null 2>&1; then
        glxinfo | grep "OpenGL version"
        glxinfo | grep "OpenGL renderer" 
        glxinfo | grep "OpenGL vendor"
        glxinfo | grep "direct rendering"
    fi
fi
echo ""
echo "Checking for common issues:"
if glxinfo 2>/dev/null | grep -q "llvmpipe\|softpipe\|software"; then
    echo "⚠️  WARNING: Using software rendering (Mesa/LLVM)"
    echo "   This means you don't have proper GPU drivers installed."
    echo "   Performance will be very poor."
    echo ""
    echo "   Solutions:"
    echo "   - For NVIDIA: sudo apt install nvidia-driver-xxx"
    echo "   - For AMD: sudo apt install mesa-vulkan-drivers"
    echo "   - For Intel: sudo apt install intel-media-va-driver"
elif glxinfo 2>/dev/null | grep -q "direct rendering: Yes"; then
    echo "✅ Hardware acceleration is working"
else
    echo "❓ Could not determine rendering status"
fi
echo ""
echo "XFCE Compositor status:"
if command -v xfconf-query >/dev/null 2>&1; then
    compositor_enabled=$(xfconf-query -c xfwm4 -p /general/use_compositing 2>/dev/null)
    if [ "$compositor_enabled" = "true" ]; then
        echo "✅ XFCE Compositor is enabled"
    else
        echo "⚠️  XFCE Compositor is disabled"
        echo "   You may want to enable it: Settings Manager → Window Manager Tweaks → Compositor"
    fi
else
    echo "❓ Could not check XFCE compositor status"
fi
echo ""
echo "Checking for package conflicts:"
if dpkg -l | grep -q "fglrx\|nvidia-.*-binary"; then
    echo "⚠️  Old proprietary drivers detected"
    echo "   You may need to remove old drivers and install new ones"
else
    echo "✅ No conflicting driver packages found"
fi
echo ""
echo "=== Recommendations ==="
echo "If the GUI still doesn't work after checking above:"
echo ""
echo "1. Update your graphics drivers:"
echo "   sudo ubuntu-drivers autoinstall"
echo ""
echo "2. Install additional Mesa libraries:"
echo "   sudo apt install mesa-utils libgl1-mesa-dri libgl1-mesa-glx"
echo ""
echo "3. Try running with software rendering (slow but works):"
echo "   LIBGL_ALWAYS_SOFTWARE=1 ./Im8086 --gui"
echo ""
echo "4. Check if running as different user works:"
echo "   sudo -u \$USER ./Im8086 --gui"
echo ""
echo "5. Restart X11 session (logout/login) after driver changes"
