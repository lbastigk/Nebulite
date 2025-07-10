#!/bin/bash
set -e
echo "Building for Windows (MinGW 32-bit)..."

# Clean previous build
rm -rf build/windows

# Configure with CMake
cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
      -DCMAKE_BUILD_TYPE=Release \
      -B build/windows -S .

# Build target
cmake --build build/windows --target Nebulite -j$(nproc)

cp build/windows/Application/bin/Nebulite.exe Application/bin/
