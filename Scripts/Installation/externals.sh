#!/bin/bash

###########################################
# Install git submodules
echo "Setting up git submodules..."

# Reset build directories and external libraries
rm -rf tmp/
rm -rf external/

# Initialize and update git submodules
git submodule update --init --recursive

# Check if all directories were created successfully
# For all dirs in external/ if they are empty, print an error and exit
for dir in external/*; do
    if [ -d "$dir" ] && [ -z "$(ls -A "$dir")" ]; then
        echo "Error: Submodule directory '$dir' is empty. Please retry submodule initialization." >&2
        exit 1
    fi
done
