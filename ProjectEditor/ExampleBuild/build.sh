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


# Starting the app after build is not used anymore, as this, for some reason, breaks the explorer widget from time to time...

# Start the app
# mv ./app ../../../Application/bin/

# cd ../../../Application/

# ./_StartQtApp.sh
