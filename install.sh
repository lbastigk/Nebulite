#!/bin/bash
echo "    _   ____________  __  ____    ________________     _____   ________________    __    __ ";
echo "   / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/    /  _/ | / / ___/_  __/   |  / /   / / ";
echo "  /  |/ / __/ / __  / / / / /    / /  / / / __/       / //  |/ /\__ \ / / / /| | / /   / /  ";
echo " / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___     _/ // /|  /___/ // / / ___ |/ /___/ /___";
echo "/_/ |_/_____/_____/\____/_____/___/ /_/ /_____/____/___/_/ |_//____//_/ /_/  |_/_____/_____/";
echo "                                             /_____/                                        ";
echo ""

####################################
# starttime of script
start=`date +%s`

####################################
# Function for echoing errors
echoerr() { echo "$@" 1>&2; }

####################################
# Checking prerequisites
if [[ "$PWD" =~ [[:space:]] ]]; then
  echo "Error: SDL2 and some build tools like libtool may fail in directories with whitespace."
  echo "Please move your source or build directory to a path without spaces."
  exit 1
fi

####################################
# install necessary components
sudo apt-get update
sudo apt-get upgrade
sudo apt-get install cmake automake build-essential autoconf libtool m4 perl mingw-w64 gcc-mingw-w64 g++-mingw-w64 python3 python3-pip python3-numpy

####################################
# Set Start directory
START_DIR=$(pwd)

####################################
# Basic directories
mkdir -p ./Application/bin

####################################
# Synonyms for SDL_ttf

# Ensure aclocal-1.16 and automake-1.16 are available as symlinks if only unversioned ones exist
if ! command -v aclocal-1.16 >/dev/null 2>&1; then
    aclocal_path=$(command -v aclocal)
    if [ -n "$aclocal_path" ]; then
        ln -sf "$aclocal_path" "$HOME/.local/bin/aclocal-1.16"
        export PATH="$HOME/.local/bin:$PATH"
    fi
fi
if ! command -v automake-1.16 >/dev/null 2>&1; then
    automake_path=$(command -v automake)
    if [ -n "$automake_path" ]; then
        ln -sf "$automake_path" "$HOME/.local/bin/automake-1.16"
        export PATH="$HOME/.local/bin:$PATH"
    fi
fi

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

#=== RESET ===
git submodule foreach --recursive git reset --hard
git submodule foreach --recursive git clean -fdx

#=== SDL2 NATIVE BUILDS ===
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL2 static-native"
cd "$externalsDir/SDL2"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
#autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/static" --enable-static --disable-shared CFLAGS=-fPIC
make -j"$(nproc)" || { echoerr "[ERROR] SDL2 static-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL2 static-native failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL2 shared-native"
cd "$externalsDir/SDL2"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
#autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/shared" --disable-static --enable-shared CFLAGS=-fPIC
make -j"$(nproc)" || { echoerr "[ERROR] SDL2 shared-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL2 shared-native failed (install)"; exit 1; }

#=== SDL_ttf NATIVE BUILDS ===
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_ttf static-native"
cd "$externalsDir/SDL_ttf"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/static" --enable-static --disable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_ttf static-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_ttf static-native failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_ttf shared-native"
cd "$externalsDir/SDL_ttf"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/shared" --disable-static --enable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_ttf shared-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_ttf shared-native failed (install)"; exit 1; }

#=== SDL_image NATIVE BUILDS ===
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_image static-native"
cd "$externalsDir/SDL_image"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/static" --enable-static --disable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_image static-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_image static-native failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_image shared-native"
cd "$externalsDir/SDL_image"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/shared" --disable-static --enable-shared CFLAGS=-fPIC --with-sdl-prefix="$externalsDir/SDL2_build/shared"
make -j"$(nproc)" || { echoerr "[ERROR] SDL_image shared-native failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL_image shared-native failed (install)"; exit 1; }

#=== RESET ===
git submodule foreach --recursive git reset --hard
git submodule foreach --recursive git clean -fdx

#=== WINDOWS (CROSS) BUILDS ===
echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL2 cross-compile"
cd "$externalsDir/SDL2"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
#autoreconf -f -i
./configure --prefix="$externalsDir/SDL2_build/shared_windows" --disable-static --enable-shared --host=x86_64-w64-mingw32
make -j"$(nproc)" || { echoerr "[ERROR] SDL2 cross-compile failed"; exit 1; }
make install      || { echoerr "[ERROR] SDL2 cross-compile failed (install)"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_ttf cross-compile"
unset CFLAGS CPPFLAGS LDFLAGS PKG_CONFIG_PATH PKG_CONFIG_LIBDIR
sdl2_win_prefix="$externalsDir/SDL2_build/shared_windows"
export PKG_CONFIG_LIBDIR="${sdl2_win_prefix}/lib/pkgconfig"
export PKG_CONFIG_PATH="${sdl2_win_prefix}/lib/pkgconfig"
export SDL2_CONFIG=/bin/false

cd "$externalsDir/SDL_ttf"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
autoreconf -f -i

CPPFLAGS="-I${sdl2_win_prefix}/include -I${sdl2_win_prefix}/include/SDL2" \
LDFLAGS="-L${sdl2_win_prefix}/lib -lSDL2" \
./configure --prefix="$externalsDir/SDL2_build/shared_windows" \
    --disable-static --enable-shared \
    --host=x86_64-w64-mingw32 \
    --with-sdl-prefix="${sdl2_win_prefix}"

sed -i -E 's/(,)?-Wl,--enable-new-dtags(,)?//g; s/(,)?--enable-new-dtags(,)?//g' libtool Makefile
# Remove example programs to avoid WinMain error
sed -i '/^bin_PROGRAMS/s/showfont\.exe//g' Makefile
sed -i '/^bin_PROGRAMS/s/glfont\.exe//g' Makefile
sed -i '/^noinst_PROGRAMS/s/showfont\.exe//g' Makefile
sed -i '/^noinst_PROGRAMS/s/glfont\.exe//g' Makefile

make -j"$(nproc)" || { echoerr "SDL_ttf encountered an error, assuming non-critical and continuing..."; }

mkdir -p "$externalsDir/SDL2_build/shared_windows/bin" "$externalsDir/SDL2_build/shared_windows/lib"
dllfile=$(ls .libs/libSDL2_ttf-*.dll 2>/dev/null | head -n1)
if [ -n "$dllfile" ]; then
    cp "$dllfile" "$externalsDir/SDL2_build/shared_windows/bin/"
else
    echoerr "[ERROR] SDL_ttf cross-compile failed: no dll file found"
    exit 1
fi
cp .libs/*.dll.a   "$externalsDir/SDL2_build/shared_windows/lib/"              || { echoerr "[ERROR] SDL_ttf cross-compile failed: no dll.a file found"; exit 1; }
cp SDL_ttf.h       "$externalsDir/SDL2_build/shared_windows/include/SDL2/"     || { echoerr "[ERROR] SDL_ttf cross-compile failed: no header file found"; exit 1; }

echo ""
echo "---------------------------------------------------"
echo "[INFO] Building SDL_image cross-compile"
cd "$externalsDir/SDL_image"
unset CFLAGS CPPFLAGS LDFLAGS PKG_CONFIG_PATH PKG_CONFIG_LIBDIR
sdl2_win_prefix="$externalsDir/SDL2_build/shared_windows"
export PKG_CONFIG_LIBDIR="${sdl2_win_prefix}/lib/pkgconfig"
export PKG_CONFIG_PATH="${sdl2_win_prefix}/lib/pkgconfig"
export SDL2_CONFIG=/bin/false

cd "$externalsDir/SDL_image"
[ -f Makefile ] && make clean || true
[ -f configure ] || ./autogen.sh
autoreconf -f -i

CPPFLAGS="-I${sdl2_win_prefix}/include -I${sdl2_win_prefix}/include/SDL2" \
LDFLAGS="-L${sdl2_win_prefix}/lib -lSDL2" \
./configure --prefix="$externalsDir/SDL2_build/shared_windows" \
    --disable-static --enable-shared \
    --host=x86_64-w64-mingw32 \
    --with-sdl-prefix="${sdl2_win_prefix}"

sed -i -E 's/(,)?-Wl,--enable-new-dtags(,)?//g; s/(,)?--enable-new-dtags(,)?//g' libtool Makefile
# Remove example programs to avoid WinMain error
sed -i '/^bin_PROGRAMS/s/showfont\.exe//g' Makefile
sed -i '/^bin_PROGRAMS/s/glfont\.exe//g' Makefile
sed -i '/^noinst_PROGRAMS/s/showfont\.exe//g' Makefile
sed -i '/^noinst_PROGRAMS/s/glfont\.exe//g' Makefile

make -j"$(nproc)" || { echoerr "SDL_image encountered an error, assuming non-critical and continuing..."; }

mkdir -p "$externalsDir/SDL2_build/shared_windows/bin" "$externalsDir/SDL2_build/shared_windows/lib"
dllfile=$(ls .libs/libSDL2_image-*.dll 2>/dev/null | head -n1)
if [ -n "$dllfile" ]; then
    cp "$dllfile" "$externalsDir/SDL2_build/shared_windows/bin/"
else
    echoerr "[ERROR] SDL_image cross-compile failed: no dll file found"
    exit 1
fi
cp .libs/*.dll.a        "$externalsDir/SDL2_build/shared_windows/lib/"              || { echoerr "[ERROR] SDL_image cross-compile failed: no dll.a file found"; exit 1; }
cp include/SDL_image.h  "$externalsDir/SDL2_build/shared_windows/include/SDL2/"     || { echoerr "[ERROR] SDL_image cross-compile failed: no header file found"; exit 1; }


####################################
# Another Reset of submodules after build
cd "$START_DIR"
git submodule foreach --recursive git reset --hard
git submodule foreach --recursive git clean -fdx

####################################
# create binaries
cd "$START_DIR"
./build.sh    || { echoerr "build.sh failed";      exit 1; }

####################################
# Copy necessary dlls
cd "$START_DIR"


if [ -f /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll ]; then
    cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll ./Application/bin/
else
    echoerr "libwinpthread-1.dll not found in /usr/x86_64-w64-mingw32/lib/"
    exit 1;
fi


####################################
# make all scripts executable
cd "$START_DIR"
find ./Application -type f -iname "*.sh" -exec chmod +x {} \;

####################################
# Run tests:

cd "$START_DIR"
cd ./Application
./Tests.sh

####################################
# Show runtime
end=`date +%s`
runtime=$((end-start))

echo ""
echo "-----------------------------------------------------"
echo "Installing + Running tests took $runtime Seconds."
echo ""

