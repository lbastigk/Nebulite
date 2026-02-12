#!/bin/bash

###########################################
# Install Python packages
echo "Installing Python dependencies..."

# Check for python command (python3 or python)
if command -v python3 >/dev/null; then
    PYTHON_CMD="python3"
elif command -v python >/dev/null; then
    PYTHON_CMD="python"
else
    echo "Python is not installed. Please install Python 3 and try again." >&2
    exit 1
fi

# Check for pip command (pip3 or pip)
if command -v pip3 >/dev/null; then
    PIP_CMD="pip3"
elif command -v pip >/dev/null; then
    PIP_CMD="pip"
else
    echo "pip is not installed. Please install pip for Python 3 and try again." >&2
    exit 1
fi

# Make a new venv directory
VENV_DIR="$PWD/.venv"
rm -rf "$VENV_DIR"
$PYTHON_CMD -m venv "$VENV_DIR"

# Activate virtual environment and install packages
source "$VENV_DIR/bin/activate"
$PIP_CMD install --upgrade pip
$PIP_CMD install -r requirements.txt