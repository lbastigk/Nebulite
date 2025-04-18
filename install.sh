#!/bin/bash

START_DIR=$(pwd)

# Resources directory
cd ./Application/Resources
./CreateResourcesFolder.sh
cd "$START_DIR"

# external directory
chmod +x ./external/CloneRepos.sh
cd ./external
./CloneRepos.sh
cd "$START_DIR"

# create binaries
cmake ./
make

echo "Installer is done!"
