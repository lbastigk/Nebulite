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
  echo "Not in Nebulite project root. Run the script from the Nebulite directory." >&2
  exit 1
fi

set -euo pipefail

###########################################
# Install system packages
echo "Installing system dependencies..."

# Define package lists for each distro
APT_PACKAGES="build-essential git make \
              pkg-config cmake ninja-build gnome-desktop-testing libasound2-dev libpulse-dev \
              libaudio-dev libfribidi-dev libjack-dev libsndio-dev libx11-dev libxext-dev \
              libxrandr-dev libxcursor-dev libxfixes-dev libxi-dev libxss-dev libxtst-dev \
              libxkbcommon-dev libdrm-dev libgbm-dev libgl1-mesa-dev libgles2-mesa-dev \
              libegl1-mesa-dev libdbus-1-dev libibus-1.0-dev libudev-dev libthai-dev"
DNF_PACKAGES="gcc git-core make cmake \
              alsa-lib-devel fribidi-devel pulseaudio-libs-devel pipewire-devel \
              libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
              libXi-devel libXScrnSaver-devel libXtst-devel dbus-devel ibus-devel \
              systemd-devel mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
              mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
              libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
              pipewire-jack-audio-connection-kit-devel libthai-devel zlib-ng-compat-static"
YUM_PACKAGES="make cmake ninja-build automake autoconf libtool m4 perl python3 python3-pip python3-numpy @development-tools mingw64-gcc mingw64-gcc-c++ alsa-lib-devel pulseaudio-libs-devel cloc libXcursor-devel libX11-devel libXext-devel libXrandr-devel libXi-devel libXinerama-devel libXxf86vm-devel libXss-devel libXrender-devel libxkbcommon-devel libxkbcommon-x11-devel"
BRW_PACKAGES="cmake ninja automake autoconf libtool python3 numpy mingw-w64 cloc libxcursor libx11 libxrandr libxi libxinerama libxxf86vm libxss libxrender libxkbcommon"


if command -v apt-get >/dev/null; then
    echo "Detected APT package manager (Ubuntu/Debian)"
    sudo apt-get update
    sudo apt-get install -y $APT_PACKAGES
elif command -v dnf >/dev/null; then
    echo "Detected DNF package manager (Fedora)"
    sudo dnf install -y --skip-unavailable $DNF_PACKAGES
elif command -v yum >/dev/null; then
    echo "Detected YUM package manager (CentOS/RHEL)"
    sudo yum install -y "$YUM_PACKAGES"
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