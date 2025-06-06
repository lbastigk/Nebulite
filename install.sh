#!/bin/bash

# Checking prerequisites
if [[ "$PWD" =~ [[:space:]] ]]; then
  echo "Error: SDL2 and some build tools like libtool may fail in directories with whitespace."
  echo "Please move your source or build directory to a path without spaces."
  exit 1
fi

# install necessary components
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install automake
sudo apt-get install build-essential
sudo apt-get install libsdl1.2-dev
sudo apt-get install autoconf libtool m4 perl

START_DIR=$(pwd)

####################################
# Synonyms for SDL_ttf

# Define a function to use aclocal as aclocal-1.16
aclocal-1.16() {
    aclocal "$@"
}
export -f aclocal-1.16

# Same for automake
automake-1.16() {
    automake "$@"
}
export -f automake-1.16

####################################
# Resources directory
cd ./Application/Resources
./CreateResourcesDirectory.sh
cd "$START_DIR"

####################################
# Submodules
git submodule update --init --recursive
set -e

git config --global --add safe.directory ./external/rapidjson
git config --global --add safe.directory ./external/SDL_image
git config --global --add safe.directory ./external/SDL_ttf
git config --global --add safe.directory ./external/SDL2
git config --global --add safe.directory ./external/tinyexpr
git config --global --add safe.directory ./external/abseil


externalsDir=$(pwd)/external

#-----------
# build absl
cd "$externalsDir/abseil"

# place build into
mkdir -p "$externalsDir/abseil_build"
cd "$externalsDir/abseil_build"

# Run CMake to configure the build
cmake ../abseil

# Build Abseil
cmake --build . -- -j$(nproc)

#-----------
# build sdl
cd "$externalsDir/SDL_ttf/external/"
FREETYPE_SIZE=$(du -k ./freetype 2>/dev/null | awk '{print $1}')
HARFBUZZ_SIZE=$(du -k ./harfbuzz 2>/dev/null | awk '{print $1}')

if [ "$FREETYPE_SIZE" -lt 10 ] || [ "$HARFBUZZ_SIZE" -lt 10 ]; then
    echo "➡️ freetype or harfbuzz missing or too small — downloading..."
    rm -rf ./freetype/
    rm -rf ./harfbuzz/
    ./download.sh
else
    echo "✅ freetype and harfbuzz already exist and are large enough, skipping download."
fi

cd "$externalsDir"

# SDL modules are in $externalsDir/$folder
# build all of them into $externalsDir/SDL2_build/
build_sdl_library() {
    local folder=$1
    echo "Building $folder..."

    local build_dir="$externalsDir/SDL2_build"

    # Enter the source folder
    cd "$folder" || { echo "Failed to cd into $folder"; return 1; }

    # Clean previous builds
    make clean || true

    # Run autogen.sh if configure does not exist
    if [ ! -f configure ]; then
        ./autogen.sh || { echo "autogen.sh failed"; return 1; }
    fi

    # Configure with prefix to build_dir
    ./configure \
        --prefix="$build_dir" \
        --enable-static \
        --disable-shared \
        CFLAGS="-fPIC" || { echo "configure failed"; return 1; }

    # Build and install
    make -j$(nproc) || { echo "make failed"; return 1; }
    make install || { echo "make install failed"; return 1; }

    # Reset any changes and clean untracked files inside the folder
    git reset --hard
    git clean -fdx
    git submodule foreach --recursive git reset --hard
    git submodule foreach --recursive git clean -fdx

    echo "$folder built and installed to $build_dir"

    # Return to previous directory
    cd - >/dev/null || return 1
}


# Build all SDL libraries
rm -rf "$externalsDir/SDL2_build"
mkdir "$externalsDir/SDL2_build"
build_sdl_library SDL2
build_sdl_library SDL_ttf
build_sdl_library SDL_image
sudo chmod -R 777 "$externalsDir/SDL2_build"
cd "$START_DIR"

####################################
# create binaries
./build.sh

####################################
# make all scripts executable
sudo chmod -R 777 ./Application
cd ./Application
for script in ./*.sh
do
    chmod +x "$script"
done


echo "Installer is done!"
