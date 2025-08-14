#!/bin/bash

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
