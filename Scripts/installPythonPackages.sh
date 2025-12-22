#!/bin/bash

###########################################
# Install Python packages
echo "Installing Python dependencies..."

# Make a new venv directory
VENV_DIR="$PWD/.venv"
rm -rf "$VENV_DIR"
python3 -m venv "$VENV_DIR"

# Activate virtual environment and install packages
source "$VENV_DIR/bin/activate"
pip install --upgrade pip
pip install -r requirements.txt