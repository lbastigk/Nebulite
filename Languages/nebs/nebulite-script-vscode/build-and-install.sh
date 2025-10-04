#!/bin/bash

# Check for apt or dnf and install dependencies
if command -v apt &> /dev/null; then
    PACKAGE_MANAGER="apt"
elif command -v dnf &> /dev/null; then
    PACKAGE_MANAGER="dnf"
else
    echo "Unknown package manager. Please install npm and vsce manually."
fi

sudo $PACKAGE_MANAGER install npm
sudo npm install -g vsce

# Enable debugging
set -x

# Debugging: Check Python version
python3 --version

# Debugging: List files in the current directory
ls -la

# Debugging: Check if vsce is installed
vsce --version || echo "vsce is not installed"

# Debugging: Check if the extension package is created
if ls ./*.vsix 1> /dev/null 2>&1; then
  echo "Extension package created successfully."
else
  echo "Extension package not found."
fi

# Debugging: Check installed extensions
code --list-extensions | grep nebulite || echo "Nebulite extension not found in installed extensions"

# Run python scripts to update config
python3 ./Languages/nebs/nebulite-script-vscode/scripts/extract_keywords.py

# Uninstall old versions of the extension
#code --uninstall-extension lbastigk.nebulite-script

# Build the VS Code extension
ROOT_DIR=$(pwd)
cd ./Languages/nebs/nebulite-script-vscode || exit 1
vsce package

# Install the extension
code --install-extension ./*.vsix

# Clean up
rm ./*.vsix

cd "$ROOT_DIR" || exit 1

echo "Extension built and installed successfully."
