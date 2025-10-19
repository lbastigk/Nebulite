#!/bin/bash
# Nebulite Dependency Installation Script
# Modern replacement for install.sh

set -euo pipefail

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[0;33m'
BLUE='\033[0;34m'
PURPLE='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[0;37m'
NC='\033[0m' # No Color

# Helper functions
print_header() {
    echo -e "${CYAN}############################################################${NC}"
    echo -e "${CYAN}# $1${NC}"
    echo -e "${CYAN}############################################################${NC}"
    echo ""
}

print_step() {
    echo -e "${YELLOW}➤ $1${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}⚠ $1${NC}"
}

# Check if running as root
if [ "$EUID" -eq 0 ]; then
    print_error "This script should NOT be run as root or with sudo. Please run as a regular user."
    exit 1
fi

# Check for whitespace in path
if [[ "$PWD" =~ [[:space:]] ]]; then
    print_error "SDL2 and some build tools may fail in directories with whitespace."
    print_error "Please move your source directory to a path without spaces."
    exit 1
fi

print_header "Nebulite Dependency Installation"

# Start timer
start=$(date +%s)

# Clean old installations
print_step "Cleaning old build artifacts"
rm -rf ./.build
rm -rf ./bin

# Create necessary directories
print_step "Creating build directories"
mkdir -p ./.build/SDL2
mkdir -p ./bin

# Install system packages
print_step "Installing system dependencies"

# Define package lists for each distro
APT_PACKAGES="cmake ninja-build automake build-essential autoconf libtool m4 perl mingw-w64 gcc-mingw-w64 g++-mingw-w64 python3 python3-pip python3-numpy libasound2-dev libpulse-dev cloc"
DNF_PACKAGES="cmake ninja-build automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel cloc"
YUM_PACKAGES="cmake ninja-build automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel cloc"

if command -v apt-get >/dev/null; then
    print_step "Detected APT package manager (Ubuntu/Debian)"
    sudo apt-get update
    sudo apt-get install -y $APT_PACKAGES
elif command -v dnf >/dev/null; then
    print_step "Detected DNF package manager (Fedora)"
    sudo dnf install -y $DNF_PACKAGES
elif command -v yum >/dev/null; then
    print_step "Detected YUM package manager (CentOS/RHEL)"
    sudo yum install -y $YUM_PACKAGES
elif command -v brew >/dev/null; then
    print_step "Detected Homebrew (macOS)"
    brew install cmake ninja automake autoconf libtool python3 numpy mingw-w64 cloc
else
    print_warning "Unknown package manager. Please install dependencies manually:"
    echo "  - cmake, ninja-build"
    echo "  - build tools (gcc, g++, automake, autoconf, libtool)"
    echo "  - mingw-w64 (for Windows cross-compilation)"
    echo "  - python3, python3-pip, python3-numpy"
    echo "  - audio libraries (alsa-lib-devel, pulseaudio-libs-devel on Linux)"
    echo "  - cloc (for line counting)"
    read -p "Press Enter when dependencies are installed, or Ctrl+C to exit..."
fi

# Initialize git submodules
print_step "Initializing git submodules"
git submodule update --init --recursive

# Build SDL2 dependencies
print_step "Building SDL2 dependencies"
externalsDir="external"

if [ ! -d "$externalsDir/SDL_Crossplatform_Local" ]; then
    print_error "SDL_Crossplatform_Local submodule not found!"
    print_error "Please ensure git submodules are properly initialized."
    exit 1
fi

# Build SDL2 using the existing cross-platform build script
cd "$externalsDir/SDL_Crossplatform_Local"
if [ ! -f "Scripts/build.sh" ]; then
    print_error "SDL2 build script not found at Scripts/build.sh"
    exit 1
fi

print_step "Running SDL2 build script"
Scripts/build.sh || {
    print_error "SDL2 build failed"
    exit 1
}

cd ../..

# Copy SDL2 build artifacts
print_step "Copying SDL2 build artifacts"
cp -r "$externalsDir/SDL_Crossplatform_Local/build/SDL2/"* .build/SDL2/

# Create bin directory for DLLs
mkdir -p ./.build/SDL2/bin

# Copy Windows DLLs
print_step "Copying Windows DLLs"
if [ -d "external/SDL_Crossplatform_Local/bin" ]; then
    cp external/SDL_Crossplatform_Local/bin/*.dll ./.build/SDL2/bin/
    print_success "Windows DLLs copied"
else
    print_warning "Windows DLLs not found - Windows builds may not work"
fi

# Copy any additional DLLs from the build directory
for PATTERN in external/SDL_Crossplatform_Local/build/SDL2/shared_windows/bin/*.dll; do
    if [ -f "$PATTERN" ]; then
        cp "$PATTERN" ./.build/SDL2/bin/
    fi
done

# Install Python dependencies
print_step "Installing Python dependencies"
if [ -f "requirements.txt" ]; then
    pip3 install --user -r requirements.txt
else
    print_warning "requirements.txt not found - skipping Python dependencies"
fi

# Verify installation
print_step "Verifying installation"

# Check for cmake
if ! command -v cmake >/dev/null; then
    print_error "cmake not found in PATH"
    exit 1
fi

# Check for ninja
if ! command -v ninja >/dev/null; then
    print_error "ninja not found in PATH"
    exit 1
fi

# Check for SDL2 build artifacts
if [ ! -d ".build/SDL2/static_linux" ] && [ ! -d ".build/SDL2/shared_windows" ]; then
    print_error "SDL2 build artifacts not found"
    exit 1
fi

# Calculate time taken
end=$(date +%s)
duration=$((end - start))
minutes=$((duration / 60))
seconds=$((duration % 60))

print_success "Dependencies installed successfully!"
print_success "Installation completed in ${minutes}m ${seconds}s"

echo ""
print_header "Next Steps"
echo -e "${WHITE}You can now build Nebulite using:${NC}"
echo -e "  ${GREEN}make linux${NC}          - Build for Linux"
echo -e "  ${GREEN}make windows${NC}        - Build for Windows"
echo -e "  ${GREEN}make windows-dlls${NC}   - Build Windows with DLLs"
echo -e "  ${GREEN}make help${NC}           - Show all available commands"
echo ""