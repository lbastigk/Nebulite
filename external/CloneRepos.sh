#!/bin/bash

# Store the current working directory
externalsDir=$(pwd)

# Clone only if not already present
clone_if_missing() {
    local repo_url=$1
    local folder_name=$2
    local branch_name=$3  # Add an optional parameter for branch

    if [ -d "$folder_name" ]; then
        echo "✅ $folder_name already exists, skipping..."
    else
        echo "➡️ Cloning $folder_name..."
        git clone "$repo_url" "$folder_name"

        if [ -n "$branch_name" ]; then
            echo "➡️ Checking out branch $branch_name for $folder_name"
            cd "$folder_name"
            git checkout "$branch_name"
            cd ..
        fi
    fi
}

####################
# Clone the correct SDL2, SDL_ttf, SDL_image repositories and check out the SDL2 branch
clone_if_missing https://github.com/libsdl-org/SDL.git          SDL2        SDL2
clone_if_missing https://github.com/libsdl-org/SDL_ttf.git      SDL_ttf     SDL2
clone_if_missing https://github.com/libsdl-org/SDL_image.git    SDL_image   SDL2
clone_if_missing https://github.com/Tencent/rapidjson.git       rapidjson
clone_if_missing https://github.com/ArashPartow/exprtk.git      exprtk

####################
# Build SDL2, SDL_ttf, SDL_image libraries


build_sdl_library() {
    local folder=$1
    echo "Building $folder..."

    local build_dir="$(pwd)/${folder}/build"


    mkdir -p "$build_dir"

    cd "$folder"

    # Clean previous build
    make clean || true
    rm -rf "$build_dir"

    # Run autogen if configure doesn't exist
    [ ! -f configure ] && ./autogen.sh

    ./configure \
        --prefix="$build_dir" \
        --enable-static \
        --disable-shared \
        CFLAGS="-fPIC" || exit 1

    make -j$(nproc)
    make install

    echo "$folder built and installed to $build_dir"
    cd ..
}

# Download externals for SDL_ttf
cd ./SDL_ttf/external/
# Only download freetype and harfbuzz if not already present
if [ ! -d "./freetype" ] || [ ! -d "./harfbuzz" ]; then
    echo "➡️ Missing freetype or harfbuzz — downloading..."
    rm -rf ./freetype/
    rm -rf ./harfbuzz/
    ./download.sh
else
    echo "✅ freetype and harfbuzz already exist, skipping download."
fi
cd "$externalsDir"

# Build libraries
build_sdl_library SDL2
build_sdl_library SDL_ttf
build_sdl_library SDL_image

# Create build directory
rm -rf ./SDL2_build
mkdir SDL2_build
for lib in SDL2 SDL_ttf SDL_image; do
    rsync -a "$lib/build/" SDL2_build/
    rm -rf "$lib/build/"
done


####################
# Build Qt5 (minimal setup for Core + Gui + Widgets)

# Clone if missing
clone_if_missing https://code.qt.io/qt/qt5.git qt5

# Enter qt5 and init only what we need
cd qt5
./init-repository --module-subset=qtbase,qttools,qtsvg

# Build inside qt5/build
mkdir -p build
cd build

# Configure and build only qtbase
../qtbase/configure \
    -prefix "$PWD/install" \
    -opensource \
    -confirm-license \
    -release \
    -nomake tests \
    -nomake examples \
    -static \
    -skip qtwebengine \
    -skip qtwebkit || exit 1

make -j$(nproc)
make install

cd "$externalsDir"
rm -rf qt5_build
mkdir -p qt5_build

# Sync only installed files (binaries, headers, libs, etc.)
rsync -a --progress ./qt5/build/install/ qt5_build/
rm -rf ./qt5/build/


####################
# Final message
echo ""
echo ""
echo "-----------------------------"
echo "Done!"
echo "You can now delete:"
echo "./qt5"
echo "./SDL_image"
echo "./SDL_ttf"
echo "./SDL2"
