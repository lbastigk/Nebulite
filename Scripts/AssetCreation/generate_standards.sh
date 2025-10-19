#!/bin/bash

# Run script from project root
if [ "$(basename "$PWD")" != "Nebulite" ]; then
    echo "Please run this script from the Nebulite project root directory."
    exit 1
fi

# Generate standard assets
./bin/Nebulite standardfile renderobject