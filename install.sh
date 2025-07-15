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
    echo "➡️ freetype or harfbuzz missing or too small — downloading..."
    rm -rf ./freetype/
    rm -rf ./harfbuzz/
    ./download.sh
else
    echo "✅ freetype and harfbuzz already exist and are large enough, skipping download."
fi

cd "$externalsDir"

####################################
# SDL Build function

# Creates builds:
# ./external/SDL2_build/static/
# ./external/SDL2_build/shared/
# ./external/SDL2_build/shared_windows/
build_sdl_library() {
    local dir=$1
    local cross_env_vars="$2"
    local extra_config_opts="$3"
    local base_dir="$externalsDir/SDL2_build"
    local src_dir="$externalsDir/$dir"

    run_build() {
        local desc="$1"
        local config_opts="$2"
        local env_vars="$3"
        local prefix_dir="$4"

        echo ""
        echo "------------------------------------------------------------"
        echo "🔧 Building $desc for $dir ..."
        make clean || true

        # Run autogen.sh only if configure is missing
        if [ ! -f configure ]; then
            ./autogen.sh || { echoerr "autogen.sh failed for $dir"; return 1; }
        fi

        # Run configure with environment variables
        eval $env_vars ./configure --prefix="$prefix_dir" $config_opts || {
            echo "❌ configure failed for $desc build of $dir"
            return 1
        }

        make -j"$(nproc)" || { echoerr "make failed for $desc build of $dir"; return 1; }
        make install      || { echoerr "make install failed for $desc build of $dir"; return 1; }
    }

    cd "$src_dir" || { echoerr "Failed to cd into $dir"; return 1; }

    # Native static build
    run_build "static libs (native)" \
              "--enable-static --disable-shared CFLAGS=-fPIC" \
              "" \
              "${base_dir}/static" || { echoerr "Native static build failed"; return 1; }

    # Native shared build
    run_build "shared libs (native)" \
              "--disable-static --enable-shared CFLAGS=-fPIC" \
              "" \
              "${base_dir}/shared" || { echoerr "Native shared build failed"; return 1; }

    # Windows cross-compiled shared DLLs
    run_build "shared Windows DLLs (cross-compile)" \
              "--disable-static --enable-shared --host=x86_64-w64-mingw32 $extra_config_opts" \
              "$cross_env_vars" \
              "${base_dir}/shared_windows" || { echoerr "Windows DLL build failed"; return 1; }

    # Clean git state
    git reset --hard
    git clean -fdx
    git submodule foreach --recursive git reset --hard
    git submodule foreach --recursive git clean -fdx

    echo "$dir built: static -> ${base_dir}/static, shared -> ${base_dir}/shared, Windows DLLs -> ${base_dir}/shared_windows"

    cd - >/dev/null || return 1
}

####################################
# Build all SDL libraries
rm -rf   "$externalsDir/SDL2_build"
mkdir -p "$externalsDir/SDL2_build"

# Build SDL2 first — no env or extra flags needed
build_sdl_library SDL2

# SDL2 Windows install prefix
sdl2_win_prefix="$externalsDir/SDL2_build/shared_windows"

# Cross-compile environment
sdl_cross_env="SDL2_CONFIG= \
CPPFLAGS='-I${sdl2_win_prefix}/include -I${sdl2_win_prefix}/include/SDL2' \
LDFLAGS='-L${sdl2_win_prefix}/lib' \
PKG_CONFIG_PATH='${sdl2_win_prefix}/lib/pkgconfig'"

# SDL_ttf and SDL_image builds with SDL prefix
build_sdl_library SDL_ttf   "$sdl_cross_env" "--with-sdl-prefix=${sdl2_win_prefix}"
build_sdl_library SDL_image "$sdl_cross_env" "--with-sdl-prefix=${sdl2_win_prefix}


####################################
# create binaries

cd "$START_DIR"
./build.sh    || { echoerr "build.sh failed";      exit 1; }
./winBuild.sh || { echoerr "./winBuild.sh failed"; exit 1; }


####################################
# make all scripts executable
find ./Application -type f -iname "*.sh" -exec chmod +x {} \;

#!/bin/bash

# Helper function for error printing
echoerr() { echo "$@" 1>&2; }

START_DIR=$(pwd)
declare -A test_results

####################################
# Run tests:

# 1.) Linux
cd "$START_DIR"
cd ./Application
if ./Tests.sh ; then
  test_results[linux]="PASS"
else
  echoerr "Linux Release test failed"
  test_results[linux]="FAIL"
fi

# 2.) Windows

echo "Running windows build test"
echo ""

# TEST DEBUG
echo "TESTING DEBUG BINARY"
cd "$START_DIR/Application"
if timeout 10s wine ./bin/Nebulite_Debug.exe 'set-fps 60 ; spawn ./Resources/Renderobjects/Planets/sun.json ; wait 100 ; exit' ; then
  test_results[win_debug]="PASS"
else
  echoerr "Windows debug test failed"
  test_results[win_debug]="FAIL"
fi

# TEST RELEASE
echo ""
echo "TESTING RELEASE BINARY"
if timeout 10s wine ./bin/Nebulite.exe 'set-fps 60 ; spawn ./Resources/Renderobjects/Planets/sun.json ; wait 100 ; exit' ; then
  test_results[win_release]="PASS"
else
  echoerr "Windows release test failed"
  test_results[win_release]="FAIL"
fi

cd "$START_DIR"

####################################
# Summary
echo ""
echo "========== Test Summary =========="
for test in linux win_debug win_release; do
  status=${test_results[$test]:-NOT RUN}
  case $test in
    linux) label="Linux Release test" ;;
    win_debug) label="Windows Debug test" ;;
    win_release) label="Windows Release test" ;;
  esac
  echo "$label : $status"
done
echo "================================="
echo "Installer is done!"

