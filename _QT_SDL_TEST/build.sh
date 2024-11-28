#!/bin/bash

rm -rf ./build/

# Create the build directory if it doesn't exist
mkdir -p build

# Navigate into the build directory
cd build

# Run cmake to configure the project
cmake ..

# Build the project using make
make

./SDLEncodedQtApp
