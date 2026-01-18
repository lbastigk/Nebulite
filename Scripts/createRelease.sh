#!/bin/bash

##########################################################################################
# Nebulite Release Archive Creator
# Creates platform-specific archives for GitHub releases
##########################################################################################

#############################################
# Abort on errors
set -e

#############################################
# Get licenses from LICENSE files
LIC_ABSEIL=$(cat ./external/abseil/LICENSE)
LIC_IMGUI=$(cat ./external/imgui/LICENSE.txt)
LIC_RAPIDJSON=$(cat ./external/rapidjson/license.txt)
LIC_SDL=$(cat ./external/SDL3/LICENSE.txt)
LIC_SDL_IMAGE=$(cat ./external/SDL3_image/LICENSE.txt)
LIC_SDL_TTF=$(cat ./external/SDL3_ttf/LICENSE.txt)

# Generate array of licenses
LICENSES=(
  "$LIC_ABSEIL"
  "$LIC_IMGUI"
  "$LIC_RAPIDJSON"
  "$LIC_SDL"
  "$LIC_SDL_IMAGE"
  "$LIC_SDL_TTF"
)

# Check if any are empty
for lic in "${LICENSES[@]}"; do
    if [ -z "$lic" ]; then
        echo -e "\033[0;31mError: One or more license files are empty. Please check the LICENSE files in the external libraries.\033[0m"
        exit 1
    fi
done

#############################################
# Prerequisites:

# Check if there are uncommitted changes
if [ -n "$(git status --porcelain)" ]; then
    echo -e "\033[0;31mError: You have uncommitted changes. Please commit or stash them before creating a release.\033[0m"
    exit 1
fi

# Check if the branch is main
CURRENT_BRANCH=$(git rev-parse --abbrev-ref HEAD)
if [ "$CURRENT_BRANCH" != "main" ]; then
    echo -e "\033[0;31mError: You must be on the 'main' branch to create a release. Current branch: $CURRENT_BRANCH\033[0m"
    exit 1
fi

# Check for version argument
if [ "$#" -ne 1 ]; then
    echo "Usage: $0 <version>"
    echo "Example: $0 1.0.0"
    # Ask for release version
    echo -n "Enter release version: "
    read VERSION
else
    VERSION="$1"
fi

# Confirm version
echo -n "Creating release for version ${VERSION}. Confirm? (y/n): "
read CONFIRM
if [ "$CONFIRM" != "y" ]; then
    echo "Release creation aborted."
    exit 1
fi

# Run tests first for now, as the wine tests may fail due to threading issues
# Only run native tests
make clean-build-and-test-native

# Compile binaries
make all

# Check for unstaged tracked changes before building docs
git diff --quiet || {
    echo -e "\033[0;33mWarning: There are unstaged changes after before documentation.\033[0m"
    git status
    exit 1
}

# Generate documentation + standards, commit and push
make docs
make standards
git add doc/
git add Languages/
git add Resources/

# Check for unstaged tracked changes
git diff --quiet || {
    echo -e "\033[0;33mWarning: There are unstaged changes after building documentation.\033[0m"
    git status
    exit 1
}

# Check for untracked files
if [ -n "$(git ls-files --others --exclude-standard)" ]; then
    echo -e "\033[0;33mWarning: There are untracked files after building documentation.\033[0m"
    git status
    exit 1
fi

git commit -m "Update documentation for release ${VERSION}" || echo "No changes in documentation to commit."
git push origin main


# Configuration
BUILD_DIR="release_build"
WINDOWS_ARCHIVE="Nebulite-${VERSION}-windows.zip"
LINUX_ARCHIVE="Nebulite-${VERSION}-linux.tar.gz"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

echo -e "${BLUE}Creating Nebulite ${VERSION} release archives...${NC}"

# Function to create LICENSE.txt content
create_license_txt(){
    cat << 'EOF'
The MIT License (MIT)
=====================

Copyright © 2025 lbastigk

Permission is hereby granted, free of charge, to any person
obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without
restriction, including without limitation the rights to use,
copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following
conditions:

The above copyright notice and this permission notice shall be
included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.
EOF
}

# Function to create THIRD_PARTY_LICENSES.txt content
create_third_party_licenses_txt(){
    cat << 'EOF'
Nebulite Third-Party Library Licenses
=====================================

This software includes the following third-party libraries:

Dear ImGui (MIT License)
----------------------
${LIC_IMGUI}

SDL3 - Simple DirectMedia Layer (zlib License)
-----------------------
${LIC_SDL}

RapidJSON (MIT License)
-----------------------
${LIC_RAPIDJSON}

TinyExpr (zlib License)
-----------------------
${LIC_TINYEXPR}

Abseil (Apache License 2.0)
----------------------------
${LIC_ABSEIL}

EOF
}

# Function to create README.txt for releases
create_readme_txt(){
    cat << EOF
Nebulite ${VERSION} - Game Engine
=================================

Thank you for downloading Nebulite ${VERSION}!

What's Included:
- Nebulite: Release executable optimized for performance
- Nebulite_Debug: Debug executable with additional logging and validation
- Required runtime libraries (Windows version only)

Quick Start:
1. Extract this archive to your desired location
2. Extract additional resources from the 'Resources' folder in the main repository if needed
3. Run Nebulite.exe (Windows) or ./Nebulite (Linux)
4. Use command line arguments or the interactive console to run tasks

Examples:
- Run a task file: ./Nebulite task TaskFiles/Demos/example.nebs
- Interactive mode: ./Nebulite
- Help: ./Nebulite help

For full documentation, visit: https://github.com/lbastigk/Nebulite

License: MIT (see LICENSE.txt)
EOF
}

# Check if binaries exist
check_binaries(){
    local missing=0
    
    if [ ! -f "bin/Nebulite" ]; then
        echo -e "${RED}Error: bin/Nebulite not found. Please build the project first.${NC}"
        missing=1
    fi
    
    if [ ! -f "bin/Nebulite_Debug" ]; then
        echo -e "${RED}Error: bin/Nebulite_Debug not found. Please build the project first.${NC}"
        missing=1
    fi
    
    if [ ! -f "bin/Nebulite.exe" ]; then
        echo -e "${YELLOW}Warning: bin/Nebulite.exe not found. Windows archive will be skipped.${NC}"
    fi
    
    if [ $missing -eq 1 ]; then
        echo -e "${RED}Please run './build.sh' to build the binaries first.${NC}"
        exit 1
    fi
}

# Create build directory
prepare_build_dir(){
    echo -e "${BLUE}Preparing build directory...${NC}"
    rm -rf "$BUILD_DIR"
    mkdir -p "$BUILD_DIR"
}

# Create Windows archive
create_windows_archive(){
    if [ ! -f "bin/Nebulite.exe" ] || [ ! -f "bin/Nebulite_Debug.exe" ]; then
        echo -e "${YELLOW}Skipping Windows archive - Windows binaries not found${NC}"
        return
    fi
    
    echo -e "${BLUE}Creating Windows archive...${NC}"
    
    local win_dir="$BUILD_DIR/windows"
    mkdir -p "$win_dir"
    
    # Copy Windows binaries
    cp bin/Nebulite.exe "$win_dir/"
    cp bin/Nebulite_Debug.exe "$win_dir/"
    
    # Copy Windows DLLs
    cp bin/*.dll "$win_dir/" 2>/dev/null || echo -e "${YELLOW}Warning: No DLL files found${NC}"
    
    # Create license files
    create_license_txt > "$win_dir/LICENSE.txt"
    create_third_party_licenses_txt > "$win_dir/THIRD_PARTY_LICENSES.txt"
    create_readme_txt > "$win_dir/README.txt"
    
    # Create archive
    cd "$BUILD_DIR"
    zip -r "../$WINDOWS_ARCHIVE" windows/
    cd ..
    
    echo -e "${GREEN}✓ Created $WINDOWS_ARCHIVE${NC}"
}

# Create Linux archive
create_linux_archive(){
    echo -e "${BLUE}Creating Linux archive...${NC}"
    
    local linux_dir="$BUILD_DIR/linux"
    mkdir -p "$linux_dir"
    
    # Copy Linux binaries
    cp bin/Nebulite "$linux_dir/"
    cp bin/Nebulite_Debug "$linux_dir/"
    
    # Make binaries executable
    chmod +x "$linux_dir/Nebulite"
    chmod +x "$linux_dir/Nebulite_Debug"
    
    # Create license files
    create_license_txt > "$linux_dir/LICENSE.txt"
    create_third_party_licenses_txt > "$linux_dir/THIRD_PARTY_LICENSES.txt"
    create_readme_txt > "$linux_dir/README.txt"
    
    # Create archive
    cd "$BUILD_DIR"
    tar -czf "../$LINUX_ARCHIVE" linux/
    cd ..
    
    echo -e "${GREEN}✓ Created $LINUX_ARCHIVE${NC}"
}

# Show archive info
show_info(){
    echo -e "\n${GREEN}Release archives created successfully!${NC}"
    echo -e "${BLUE}Archives:${NC}"
    
    if [ -f "$WINDOWS_ARCHIVE" ]; then
        echo -e "  📦 $WINDOWS_ARCHIVE ($(du -h "$WINDOWS_ARCHIVE" | cut -f1))"
    fi
    
    if [ -f "$LINUX_ARCHIVE" ]; then
        echo -e "  📦 $LINUX_ARCHIVE ($(du -h "$LINUX_ARCHIVE" | cut -f1))"
    fi
    
    echo -e "\n${BLUE}Upload these archives to your GitHub release page.${NC}"
    echo -e "${BLUE}Release URL: https://github.com/lbastigk/Nebulite/releases/new?tag=${VERSION}${NC}"
}

# Main execution
main(){
    echo -e "${BLUE}Nebulite Release Creator${NC}"
    echo -e "${BLUE}Version: ${VERSION}${NC}\n"
    
    check_binaries
    prepare_build_dir
    create_windows_archive
    create_linux_archive
    show_info
    
    # Cleanup
    rm -rf "$BUILD_DIR"
}

# Run main function
main "$@"