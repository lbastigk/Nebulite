#!/bin/bash

# Nebulite Release Archive Creator
# Creates platform-specific archives for GitHub releases
set -e

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

# Compile binaries
make all

# Generate documentation and merge
make docs
git add Docs/
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

SDL2 - Simple DirectMedia Layer (zlib License)
-----------------------------------------------
Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>
  
This software is provided 'as-is', without any express or implied
warranty.  In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:
  
1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required. 
2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.

FreeType - Font Rendering Library (FreeType License)
----------------------------------------------------
Copyright (C) 1996-2024 David Turner, Robert Wilhelm, and Werner Lemberg.

The FreeType Project LICENSE (FreeType License)
This license applies to all files found in such packages, and
which do not fall under their own explicit license.

Full license: https://www.freetype.org/license.html

HarfBuzz - Text Shaping Library ("OLD MIT" License)
----------------------------------------------
Copyright © 2010-2022  Google, Inc.
Copyright © 2015-2020  Ebrahim Byagowi
Copyright © 2019,2020  Facebook, Inc.
Copyright © 2012,2015  Mozilla Foundation
Copyright © 2011  Codethink Limited
Copyright © 2008,2010  Nokia Corporation and/or its subsidiary(-ies)
Copyright © 2009  Keith Stribley
Copyright © 2011  Martin Hosken and SIL International
Copyright © 2007  Chris Wilson
Copyright © 2005,2006,2020,2021,2022,2023  Behdad Esfahbod
Copyright © 2004,2007,2008,2009,2010,2013,2021,2022,2023  Red Hat, Inc.
Copyright © 1998-2005  David Turner and Werner Lemberg
Copyright © 2016  Igalia S.L.
Copyright © 2022  Matthias Clasen
Copyright © 2018,2021  Khaled Hosny
Copyright © 2018,2019,2020  Adobe, Inc
Copyright © 2013-2015  Alexei Podtelezhnikov

For full copyright notices consult the individual files in the package.


Permission is hereby granted, without written agreement and without
license or royalty fees, to use, copy, modify, and distribute this
software and its documentation for any purpose, provided that the
above copyright notice and the following two paragraphs appear in
all copies of this software.

IN NO EVENT SHALL THE COPYRIGHT HOLDER BE LIABLE TO ANY PARTY FOR
DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR CONSEQUENTIAL DAMAGES
ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS DOCUMENTATION, EVEN
IF THE COPYRIGHT HOLDER HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.

THE COPYRIGHT HOLDER SPECIFICALLY DISCLAIMS ANY WARRANTIES, INCLUDING,
BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE.  THE SOFTWARE PROVIDED HEREUNDER IS
ON AN "AS IS" BASIS, AND THE COPYRIGHT HOLDER HAS NO OBLIGATION TO
PROVIDE MAINTENANCE, SUPPORT, UPDATES, ENHANCEMENTS, OR MODIFICATIONS.

MinGW-w64 Runtime (Various Permissive Licenses)
-----------------------------------------------
MinGW-w64 runtime libraries are distributed under various permissive licenses
including public domain and MIT-style licenses.

Full details: https://www.mingw-w64.org/

RapidJSON (MIT License)
-----------------------
Copyright (C) 2015 THL A29 Limited, a Tencent company, and Milo Yip.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

TinyExpr (zlib License)
-----------------------
Copyright (C) 2015, 2016 Lewis Van Winkle

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

Abseil (Apache License 2.0)
----------------------------
Copyright 2017 The Abseil Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
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