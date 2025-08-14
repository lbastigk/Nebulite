#!/bin/bash
# Build and install Nebulite Script VS Code extension
set -e

EXT_NAME="nebulite-script-language-0.0.1.vsix"

# Build the VSIX package
npx vsce package

# Find the generated VSIX file (handle version bumps)
VSIX_FILE=$(ls *.vsix | head -n 1)

# Install the extension
code --install-extension "$VSIX_FILE"

echo "Nebulite Script extension installed: $VSIX_FILE"

code --uninstall-extension lbastigk.nebulite-script-language || true
code --install-extension "$VSIX_FILE"