#!/bin/bash

# Enable debugging
set -x

# Debugging: Check Python version
python3 --version

# Debugging: List files in the current directory
ls -la

# Debugging: Check if vsce is installed
vsce --version || echo "vsce is not installed"

# Debugging: Check if the extension package is created
if [ -f *.vsix ]; then
  echo "Extension package created successfully."
else
  echo "Extension package not found."
fi

# Debugging: Check installed extensions
code --list-extensions | grep nebulite || echo "Nebulite extension not found in installed extensions"

# Run python scripts to update config
python3 ./scripts/extract_keywords.py

# Uninstall old versions of the extension
code --uninstall-extension lbastigk.nebulite-script

# Build the VS Code extension
vsce package

# Install the extension
code --install-extension *.vsix

# Clean up
rm *.vsix

echo "Extension built and installed successfully."
