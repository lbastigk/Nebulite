#!/bin/bash

# Function for echoing errors
echoerr() { echo "$@" 1>&2; }

# Checking prerequisites
if [[ "$PWD" =~ [[:space:]] ]]; then
  echo "Error: SDL2 and some build tools like libtool may fail in directories with whitespace."
  echo "Please move your source or build directory to a path without spaces."
  exit 1
fi

# install necessary components
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install cmake automake build-essential autoconf libtool m4 perl mingw-w64 gcc-mingw-w64 g++-mingw-w64 python3 python3-pip python3-numpy

# Set Start directory
START_DIR=$(pwd)

####################################
# Synonyms for SDL_ttf

# Define a function to use aclocal as aclocal-1.16 due to compatibility
# SDL_ttf build expects version-specific autotools commands; these aliases provide fallback wrappers.

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
cd ./Application/Resources      || exit 1
./CreateResourcesDirectory.sh   || exit 1
cd "$START_DIR"


####################################
# Submodules
git submodule update --init --recursive
set -e

# Ensure Git is allowed to access these dirs
for dir in ./external/*; do
    if [ -d "$dir/.git" ]; then
        git config --global --add safe.directory "$(realpath "$dir")"
    fi
done
externalsDir=$(pwd)/external

####################################
# build absl
cd "$externalsDir/abseil"

# place build into
mkdir -p "$externalsDir/abseil_build"
cd "$externalsDir/abseil_build"

# Run CMake to configure the build
cmake ../abseil -DCMAKE_POSITION_INDEPENDENT_CODE=ON

# Build Abseil
cmake --build . -- -j$(nproc)

####################################
# build sdl
cd "$externalsDir/SDL_ttf/external/"
FREETYPE_SIZE=$(du -k ./freetype 2>/dev/null | awk '{print $1}')
HARFBUZZ_SIZE=$(du -k ./harfbuzz 2>/dev/null | awk '{print $1}')

if [ "$FREETYPE_SIZE" -lt 10 ] || [ "$HARFBUZZ_SIZE" -lt 10 ]; then
    echo "[INFO] freetype or harfbuzz missing or too small — downloading..."
    rm -rf ./freetype/
    rm -rf ./harfbuzz/
    ./download.sh
else
    echo "[SUCCESS] freetype and harfbuzz already exist and are large enough, skipping download."
fi

cd "$externalsDir"

####################################
# SDL Build function

# Creates builds:
# ./external/SDL2_build/static/
# ./external/SDL2_build/shared/
# ./external/SDL2_build/shared_windows/
rm -rf "$externalsDir/SDL2_build"
mkdir -p "$externalsDir/SDL2_build"

#=== SDL2 NATIVE BUILDS ===
# Reset submodules for build
git submodule foreach --recursive git reset --hard
git submodule foreach --recursive git clean -fdx
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL2 static-native"
cd "$externalsDir/SDL2"
make clean || true
[ -f configure ] || ./autogen.sh
./configure --prefix="$externalsDir/SDL2_build/static" --enable-static --disable-shared CFLAGS=-fPIC
make -j"$(nproc)" || { echoerr "[ERROR] SDL2 static-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL2 static-native failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL2 shared-native"
cd "$externalsDir/SDL2"
make clean || true
./configure --prefix="$externalsDir/SDL2_build/shared" --disable-static --enable-shared CFLAGS=-fPIC
make -j"$(nproc)" || { echoerr "[ERROR] SDL2 shared-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL2 shared-native failed (install)"; exit 1; }

#=== SDL_ttf NATIVE BUILDS ===
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_ttf static-native"
cd "$externalsDir/SDL_ttf"
make clean || true
[ -f configure ] || ./autogen.sh
./configure --prefix="$externalsDir/SDL2_build/static" --enable-static --disable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_ttf static-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_ttf static-native failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_ttf shared-native"
cd "$externalsDir/SDL_ttf"
make clean || true
./configure --prefix="$externalsDir/SDL2_build/shared" --disable-static --enable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_ttf shared-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_ttf shared-native failed (install)"; exit 1; }

#=== SDL_image NATIVE BUILDS ===
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_image static-native"
cd "$externalsDir/SDL_image"
make clean || true
[ -f configure ] || ./autogen.sh
./configure --prefix="$externalsDir/SDL2_build/static" --enable-static --disable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_image static-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_image static-native failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_image shared-native"
cd "$externalsDir/SDL_image"
make clean || true
./configure --prefix="$externalsDir/SDL2_build/shared" --disable-static --enable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_image shared-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_image shared-native failed (install)"; exit 1; }

#=== WINDOWS (CROSS) BUILDS ===
# Reset submodules for build
git submodule foreach --recursive git reset --hard
git submodule foreach --recursive git clean -fdx
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL2 cross-compile"
cd "$externalsDir/SDL2"
make clean || true
./configure --prefix="$externalsDir/SDL2_build/shared_windows" --disable-static --enable-shared --host=x86_64-w64-mingw32
make -j"$(nproc)" || { echoerr "[ERROR] SDL2 cross-compile failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL2 cross-compile failed (install)"; exit 1; }

##################################################
# TODO: [ERROR] SDL_ttf cross-compile failed:
# ...
#/usr/bin/x86_64-w64-mingw32-ld: /home/leo/C_Projects/Nebulite/external/SDL_ttf/SDL_ttf.c:4283:(.text+0xc922): undefined reference to `SDL_SetError'
#/usr/bin/x86_64-w64-mingw32-ld: .libs/libSDL2_ttf_la-SDL_ttf.o: in function `TTF_CloseFont':
#/home/leo/C_Projects/Nebulite/external/SDL_ttf/SDL_ttf.c:2765:(.text+0x158e): undefined reference to `SDL_free'
#/usr/bin/x86_64-w64-mingw32-ld: .libs/libSDL2_ttf_la-SDL_ttf.o: in function `TTF_SetFontStyle':
#/home/leo/C_Projects/Nebulite/external/SDL_ttf/SDL_ttf.c:4085:(.text+0xc36e): undefined reference to `SDL_SetError'
#/usr/bin/x86_64-w64-mingw32-ld: .libs/libSDL2_ttf_la-SDL_ttf.o: in function `TTF_SetFontOutline':
#/home/leo/C_Projects/Nebulite/external/SDL_ttf/SDL_ttf.c:4132:(.text+0xc3fd): undefined reference to `SDL_SetError'
#/usr/bin/x86_64-w64-mingw32-ld: .libs/libSDL2_ttf_la-SDL_ttf.o: in function `TTF_SetFontHinting':
#/home/leo/C_Projects/Nebulite/external/SDL_ttf/SDL_ttf.c:4148:(.text+0xc4bd): undefined reference to `SDL_SetError'
#/usr/bin/x86_64-w64-mingw32-ld: .libs/libSDL2_ttf_la-SDL_ttf.o: in function `TTF_SetFontWrappedAlign':
#/home/leo/C_Projects/Nebulite/external/SDL_ttf/SDL_ttf.c:4208:(.text+0xc5d8): undefined reference to `SDL_SetError'
#collect2: error: ld returned 1 exit status
#make: *** [Makefile:1387: libSDL2_ttf.la] Error 1
#[ERROR] SDL_ttf cross-compile failed
##################################################

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_ttf cross-compile"
sdl2_win_prefix="$externalsDir/SDL2_build/shared_windows"
cd "$externalsDir/SDL_ttf"
make clean || true
[ -f configure ] || ./autogen.sh
SDL2_CONFIG=/bin/false \
CPPFLAGS="-I${sdl2_win_prefix}/include -I${sdl2_win_prefix}/include/SDL2" \
LDFLAGS="-L${sdl2_win_prefix}/lib -lSDL2" \
PKG_CONFIG_PATH="${sdl2_win_prefix}/lib/pkgconfig" \
./configure --prefix="$externalsDir/SDL2_build/shared_windows" \
  --disable-static --enable-shared \
  --host=x86_64-w64-mingw32 \
  --with-sdl-prefix="${sdl2_win_prefix}"
sed -i -E 's/(,)?-Wl,--enable-new-dtags(,)?//g; s/(,)?--enable-new-dtags(,)?//g' libtool Makefile
make -j"$(nproc)" || { echoerr "SDL_ttf encountered an error, assuming non-critical and continuing..."; }


echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_image cross-compile"
cd "$externalsDir/SDL_image"
make clean || true
[ -f configure ] || ./autogen.sh
SDL2_CONFIG=/bin/false \
CPPFLAGS="-I${sdl2_win_prefix}/include -I${sdl2_win_prefix}/include/SDL2" \
LDFLAGS="-L${sdl2_win_prefix}/lib -lSDL2" \
PKG_CONFIG_PATH="${sdl2_win_prefix}/lib/pkgconfig" \
./configure --prefix="$externalsDir/SDL2_build/shared_windows" \
  --disable-static --enable-shared \
  --host=x86_64-w64-mingw32 \
  --with-sdl-prefix="${sdl2_win_prefix}"
sed -i -E 's/(,)?-Wl,--enable-new-dtags(,)?//g; s/(,)?--enable-new-dtags(,)?//g' libtool Makefile
make -j"$(nproc)" || { echoerr "SDL_image encountered an error, assuming non-critical and continuing..."; }



# Copy files into build directory
# SDL_ttf will try to generate examples and fail, which is why the error check happens here:
mkdir -p "$externalsDir/SDL2_build/shared_windows/bin" "$externalsDir/SDL2_build/shared_windows/lib"
cd "$externalsDir/SDL_ttf"
cp .libs/SDL2_ttf.dll         "$externalsDir/SDL2_build/shared_windows/bin/"            || { echoerr "[ERROR] SDL_ttf cross-compile failed: no dll file found"; exit 1; }
cp .libs/libSDL2_ttf.dll.a    "$externalsDir/SDL2_build/shared_windows/lib/"            || { echoerr "[ERROR] SDL_ttf cross-compile failed: no dll.a file found"; exit 1; }
cp SDL_ttf.h                  "$externalsDir/SDL2_build/shared_windows/include/SDL2/"   || { echoerr "[ERROR] SDL_ttf cross-compile failed: no header file found"; exit 1; }
cd "$externalsDir/SDL_image"
cp .libs/SDL2_image.dll       "$externalsDir/SDL2_build/shared_windows/bin/"            || { echoerr "[ERROR] SDL_image cross-compile failed: no dll file found"; exit 1; }
cp .libs/libSDL2_image.dll.a  "$externalsDir/SDL2_build/shared_windows/lib/"            || { echoerr "[ERROR] SDL_image cross-compile failed: no dll.a file found"; exit 1; }
cp include/SDL_image.h        "$externalsDir/SDL2_build/shared_windows/include/SDL2/"   || { echoerr "[ERROR] SDL_image cross-compile failed: no header file found"; exit 1; }

cd "$START_DIR"

####################################
# Reset submodules after build
git submodule foreach --recursive git reset --hard
git submodule foreach --recursive git clean -fdx

####################################
# create binaries

cd "$START_DIR"
./build.sh    || { echoerr "build.sh failed";      exit 1; }

####################################
# make all scripts executable
find ./Application -type f -iname "*.sh" -exec chmod +x {} \;

####################################
# Run tests:

cd "$START_DIR"
cd ./Application
./Tests.sh

