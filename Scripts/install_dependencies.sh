#!/bin/bash

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

# Install system packages
echo "Installing system dependencies"

# Define package lists for each distro
# TODO: Verify package names and see if they're actually needed
# TODO: Check if freetype is needed, project should use the bundled version from the submodule!
APT_PACKAGES="cmake ninja-build automake build-essential autoconf libtool m4 perl mingw-w64 gcc-mingw-w64 g++-mingw-w64 python3 python3-pip python3-numpy libasound2-dev libpulse-dev cloc"
DNF_PACKAGES="cmake ninja-build automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel cloc"
YUM_PACKAGES="cmake ninja-build automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel cloc"

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
    brew install cmake ninja automake autoconf libtool python3 numpy mingw-w64 cloc
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

# Reset build directories and external libraries
rm -rf tmp/
rm -rf external/

# Initialize and update git submodules, fix permissions if needed
git submodule update --init --recursive
chmod -R u+rw external/