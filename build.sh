#!/bin/bash
set -e
echo "    _   ____________  __  ____    ________________"
echo "   / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/"
echo "  /  |/ / __/ / __  / / / / /    / /  / / / __/   "
echo " / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___   "
echo "/_/ |_/_____/_____/\____/_____/___/ /_/ /_____/   "
echo "                                                  "                                                 
echo ""

echo "Step 1: Building debug binary"
make clean
rm -rf CMakeFiles/ CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Debug ./
make -j$(nproc)
mv ./Application/bin/Nebulite ./Application/bin/Nebulite_Debug

echo ""
echo "Step 2: Building release binary"
make clean
rm -rf CMakeFiles/ CMakeCache.txt
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_CXX_FLAGS_RELEASE="-O3 -DNDEBUG -march=native" \
      -DCMAKE_INTERPROCEDURAL_OPTIMIZATION=TRUE \
      ./
make -j$(nproc)
strip ./Application/bin/Nebulite

echo "Done!"
