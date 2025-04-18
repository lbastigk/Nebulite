#!/bin/bash

START_DIR=$(pwd)

cd ./Application/Resources
./CreateResourcesFolder.sh


cd "$START_DIR"

cd ./external
./CloneRepos.sh

cd "$START_DIR"
ls


cmake ./
make

echo "Installer is done!"
