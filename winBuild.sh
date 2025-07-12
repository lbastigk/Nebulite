#!/bin/bash
set -e
echo "Building for Windows (MinGW 32-bit)..."

# Clean previous builds
rm -rf build/
rm -rf Application/bin/Nebulite.exe
rm -rf Application/bin/Nebulite_Debug.exe


# Build Release
cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -B build/windows-release -S .
cmake --build build/windows-release --target Nebulite -j$(nproc)
cp build/windows-release/Application/bin/Nebulite.exe Application/bin/Nebulite.exe

# Build Debug
cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Debug \
      -B build/windows-debug -S .
cmake --build build/windows-debug --target Nebulite -j$(nproc)
cp build/windows-debug/Application/bin/Nebulite.exe Application/bin/Nebulite_Debug.exe

# TEST DEBUG and RELEASE
echo ""
echo "TESTING DEBUG BINARY"
cd ./Application
wine ./bin/Nebulite_Debug.exe 'set-fps 60 ; spawn ./Resources/Renderobjects/Planets/sun.json ; wait 100 ; exit'
cd ..

echo ""
echo "TESTING RELEASE BINARY"
cd ./Application
wine ./bin/Nebulite.exe       'set-fps 60 ; spawn ./Resources/Renderobjects/Planets/sun.json ; wait 100 ; exit'
cd ..
