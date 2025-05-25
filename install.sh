#!/bin/bash

# install necessary components
sudo apt-get update
sudo apt-get install cmake
sudo apt-get install automake
sudo apt-get install build-essential
sudo apt-get install libsdl1.2-dev

START_DIR=$(pwd)

####################################
# Resources directory
cd ./Application/Resources
./CreateResourcesFolder.sh
cd "$START_DIR"

####################################
# external directory
git submodule update --init --recursive

set -e

externalsDir=$(pwd)/external

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

build_sdl_library() {
    local folder=$1
    echo "Building $folder..."

    local build_dir="$externalsDir/${folder}/build"
    mkdir -p "$build_dir"

    cd "$folder"

    make clean || true
    rm -rf "$build_dir"
    [ ! -f configure ] && ./autogen.sh

    ./configure \
        --prefix="$build_dir" \
        --enable-static \
        --disable-shared \
        CFLAGS="-fPIC"

    make -j$(nproc)
    make install

    echo "$folder built and installed to $build_dir"
    cd ..
}

# Build all SDL libraries
build_sdl_library SDL2
build_sdl_library SDL_ttf
build_sdl_library SDL_image

# Bundle build outputs
rm -rf "$externalsDir/SDL2_build"
mkdir "$externalsDir/SDL2_build"
for lib in SDL2 SDL_ttf SDL_image; do
    rsync -a "$lib/build/" "$externalsDir/SDL2_build"
    rm -rf "$lib/build/"
done

cd "$START_DIR"

####################################
# create binaries
cmake ./
make

echo "Installer is done!"
