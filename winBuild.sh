#!/bin/bash
set -e
echo "Building for Windows (MinGW 32-bit)..."

# Clean previous builds
rm -rf build/windows-release build/windows-debug

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