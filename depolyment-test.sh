#!/bin/bash

# Automatic build test for Nebulite
rm -rf      ./_deployment-test
mkdir -p    ./_deployment-test

# Subdir for test
cd _deployment-test

# Clone
gh repo clone lbastigk/Nebulite
cd ./Nebulite

# install & build
chmod +x ./install.sh
./install.sh
./build.sh

# start
cd ./Application
chmod +x ./Tests.sh
./Tests.sh