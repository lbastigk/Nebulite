#!/bin/bash

###########################################
# Nebulite Dependency Installer Script
# This script installs all necessary system and Python
# dependencies for the Nebulite project.

###########################################
# Prerequisites + Settings
echo "Checking prerequisites..."

# Make sure script runs as sudo
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root (using sudo)"
    exit 1
fi

# Make sure the directory is the project root "Nebulite"
if [ "$(basename "$PWD")" != "Nebulite" ] || { [ ! -f "MAKEFILE" ] && [ ! -f "CMakeLists.txt" ] && [ ! -d ".git" ]; }; then
  echo "Not in `Nebulite` project root. Run the script from the `Nebulite` directory." >&2
  exit 1
fi

set -euo pipefail

###########################################
# Install system packages
echo "Installing system dependencies..."

# Define package lists for each distro
APT_PACKAGES="cmake ninja-build automake build-essential autoconf libtool m4 perl mingw-w64 gcc-mingw-w64 g++-mingw-w64 python3 python3-pip python3-numpy libasound2-dev libpulse-dev cloc"
DNF_PACKAGES="cmake ninja-build automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel cloc"
YUM_PACKAGES="cmake ninja-build automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel cloc"
BRW_PACKAGES="cmake ninja automake autoconf libtool python3 numpy mingw-w64 cloc"

if command -v apt-get >/dev/null; then
    echo "Detected APT package manager (Ubuntu/Debian)"
    sudo apt-get update
    sudo apt-get install -y $APT_PACKAGES
elif command -v dnf >/dev/null; then
    echo "Detected DNF package manager (Fedora)"
    sudo dnf install -y $DNF_PACKAGES
elif command -v yum >/dev/null; then
    echo "Detected YUM package manager (CentOS/RHEL)"
    sudo yum install -y $YUM_PACKAGES
elif command -v brew >/dev/null; then
    echo "Detected Homebrew (macOS)"
    brew install $BRW_PACKAGES
else
    echo "Unknown package manager. Please install dependencies manually:"
    echo "  - cmake, ninja-build"
    echo "  - build tools (gcc, g++, automake, autoconf, libtool, clang, etc.)"
    echo "  - mingw-w64 (for Windows cross-compilation)"
    echo "  - python3, python3-pip, python3-numpy"
    echo "  - audio libraries (alsa-lib-devel, pulseaudio-libs-devel on Linux)"
    echo "  - cloc (for line counting)"
    read -r -p "Press Enter when dependencies are installed, or Ctrl+C to exit..."
fi

###########################################
# Install git submodules
echo "Setting up git submodules..."

# Reset build directories and external libraries
rm -rf tmp/
rm -rf external/

# Initialize and update git submodules
sudo -u "$SUDO_USER" git submodule update --init --recursive

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