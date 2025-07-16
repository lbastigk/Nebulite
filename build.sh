#!/bin/bash
set -euo pipefail

echo "    _   ____________  __  ____    ________________"
echo "   / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/"
echo "  /  |/ / __/ / __  / / / / /    / /  / / / __/   "
echo " / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___   "
echo "/_/ |_/_____/_____/\____/_____/___/ /_/ /_____/   "
echo "                                                  "                                                 
echo ""

function clean_src() {
    echo "Cleaning only Nebulite object files and binaries"

    rm -rf "$1"

    OBJ_DIR="build/debug/CMakeFiles/Nebulite.dir/src/"
    if [ -d "$OBJ_DIR" ]; then
        find "$OBJ_DIR" -name '*.o' -delete
        find "$OBJ_DIR" -name '*.o.d' -delete
    else
        echo "No object directory found at $OBJ_DIR — skipping clean"
    fi
}

function build_debug() {
    echo "#############################################################"
    echo ""
    echo "Step 1: Building debug binary"
    clean_src "build/debug/Application/bin/Nebulite"

    cmake -DCMAKE_BUILD_TYPE=Debug -B build/debug -S .
    cmake --build build/debug
    make -j$(nproc)
    cp build/debug/Application/bin/Nebulite Application/bin/Nebulite_Debug
}

function build_release() {
    echo "#############################################################"
    echo ""
    echo "Step 2: Building release binary"
    clean_src "build/release/Application/bin/Nebulite"

    cmake -DCMAKE_BUILD_TYPE=Release \
          -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -march=native" \
          -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=TRUE \
          -B build/release -S .

    cmake --build build/release --target Nebulite -j$(nproc)
    cp build/release/Application/bin/Nebulite Application/bin/Nebulite
    strip "./Application/bin/Nebulite"
}


function build_debug_windows() {
      echo "#############################################################"
      echo ""
      echo "Step 1: Building debug binary"
      clean_src "build/windows-debug/Application/bin/Nebulite.exe"

      cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
            -DCMAKE_BUILD_TYPE=Debug \
            -B build/windows-debug -S .
      cmake --build build/windows-debug --target Nebulite -j$(nproc)
      cp build/windows-debug/Application/bin/Nebulite.exe Application/bin/Nebulite_Debug.exe
}

function build_release_windows() {
      echo "#############################################################"
      echo ""
      echo "Step 2: Building release binary"
      clean_src "build/windows-release/Application/bin/Nebulite.exe"

      cmake -DCMAKE_TOOLCHAIN_FILE=mingw-toolchain.cmake \
            -DCMAKE_BUILD_TYPE=Release \
            -B build/windows-release -S .
      cmake --build build/windows-release --target Nebulite -j$(nproc)
      cp build/windows-release/Application/bin/Nebulite.exe Application/bin/Nebulite.exe
}

function generate_standards() {
    cd Application
    ./bin/Nebulite standard-render-object
    cd ..
}

#############################################################
# [BUILD]
# Try-catch wrapper
{
      rm -rf "./Application/bin/Nebulite"
      rm -rf "./Application/bin/Nebulite_Debug"
      rm -rf "./Application/bin/Nebulite.exe"
      rm -rf "./Application/bin/Nebulite_Debug.exe"

      build_debug
      build_release
      build_debug_windows
      build_release_windows

      echo "Build done!"
      echo ""
      echo "Generating standards from Binary:"
      generate_standards
      echo ""
      echo "Newest Nebulite build + files generated."
} || {
      echo ""
      echo "[ERROR] Build failed!"
      echo "Consider running a full clean with make clean"
      exit 1
}
