#!/bin/bash

###########################################
# Install git submodules
echo "Setting up git submodules..."

# Reset build directories and external libraries
rm -rf tmp/
rm -rf external/

# Initialize and update git submodules
git submodule update --init --recursive

