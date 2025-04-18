#!/bin/bash

# Automatic build test for Nebulite
mkdir -p _deployment-test

cd _deployment-test

gh clone lbastigk/Nebulite
cd ./Nebulite

chmod +x ./install.sh
./install.sh