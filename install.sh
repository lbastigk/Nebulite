#!/bin/bash

# install necessary components
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install automake
sudo apt-get install build-essential
sudo apt-get install libsdl1.2-dev

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
