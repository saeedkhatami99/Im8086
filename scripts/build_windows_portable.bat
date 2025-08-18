@echo off

echo ==========================================
echo Building Im8086 Portable Windows Binary
echo ==========================================
if exist build-windows-portable rmdir /s /q build-windows-portable
mkdir build-windows-portable
cd build-windows-portable
echo Configuring for Windows portable build...
cmake .. ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DBUILD_PORTABLE=ON ^
    -DBUILD_TESTS=ON ^
    -DCMAKE_TOOLCHAIN_FILE=C:/vcpkg/scripts/buildsystems/vcpkg.cmake

echo Building...
cmake --build . --config Release
echo Testing...
Release\test_emulator.exe
Release\test_gui.exe
echo Creating portable package...
mkdir Im8086-Windows-Portable
copy Release\Im8086.exe Im8086-Windows-Portable\
copy Release\test_emulator.exe Im8086-Windows-Portable\
copy Release\test_gui.exe Im8086-Windows-Portable\
xcopy ..\samples Im8086-Windows-Portable\samples\ /E /I
copy ..\README.md Im8086-Windows-Portable\
echo Bundling required DLLs...
if exist "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" (
    copy "C:\vcpkg\installed\x64-windows\bin\SDL2.dll" Im8086-Windows-Portable\
    echo SDL2.dll bundled
)
if exist "C:\vcpkg\installed\x64-windows\bin\*.dll" (
    copy "C:\vcpkg\installed\x64-windows\bin\*.dll" Im8086-Windows-Portable\
    echo Additional DLLs bundled
)

echo @echo off > Im8086-Windows-Portable\launch.bat
echo echo Starting Im8086 Emulator... >> Im8086-Windows-Portable\launch.bat
echo %%~dp0\Im8086.exe %%* >> Im8086-Windows-Portable\launch.bat
tar -czf Im8086-Windows-Portable.tar.gz Im8086-Windows-Portable\
echo ==========================================
echo Windows portable build complete!
echo Package: build-windows-portable\Im8086-Windows-Portable.tar.gz
echo ==========================================
pause
