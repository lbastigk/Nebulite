#!/bin/bash 
echo "    _   ____________  __  ____    ________________     _____   ________________    __    __ ";
echo "   / | / / ____/ __ )/ / / / /   /  _/_  __/ ____/    /  _/ | / / ___/_  __/   |  / /   / / ";
echo "  /  |/ / __/ / __  / / / / /    / /  / / / __/       / //  |/ /\__ \ / / / /| | / /   / /  ";
echo " / /|  / /___/ /_/ / /_/ / /____/ /  / / / /___     _/ // /|  /___/ // / / ___ |/ /___/ /___";
echo "/_/ |_/_____/_____/\____/_____/___/ /_/ /_____/____/___/_/ |_//____//_/ /_/  |_/_____/_____/";
echo "                                             /_____/                                        ";
echo ""
if [ "$EUID" -eq 0 ]; then
  echo "This script should NOT be run as root or with sudo. Please run as a regular user."
  exit 1
fi

# Set error message on exit
PROGRESS="Starting"
trap 'echoerr "An error occurred at $PROGRESS. Exiting..."; exit 1;' ERR

####################################
# Removing old installations of local files
rm -rf ./.build
rm -rf ./bin
rm -rf ./external

####################################
# starttime of script
start=`date +%s`

####################################
# Function for echoing errors
echoerr() { echo "$@" 1>&2; }

####################################
# Checking prerequisites
PROGRESS="Checking prerequisites"
if [[ "$PWD" =~ [[:space:]] ]]; then
  echo "Error: SDL2 and some build tools like libtool may fail in directories with whitespace."
  echo "Please move your source or build directory to a path without spaces."
  exit 1
fi

####################################
# install necessary components
PROGRESS="Installing necessary components"

# Define package lists for each distro
APT_PACKAGES="cmake automake build-essential autoconf libtool m4 perl mingw-w64 gcc-mingw-w64 g++-mingw-w64 python3 python3-pip python3-numpy libasound2-dev libpulse-dev"
DNF_PACKAGES="cmake automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel"
YUM_PACKAGES="cmake automake @development-tools autoconf libtool m4 perl mingw64-gcc mingw64-gcc-c++ python3 python3-pip python3-numpy alsa-lib-devel pulseaudio-libs-devel"

if command -v apt-get >/dev/null; then
    PACKAGE_MANAGER="apt"
elif command -v dnf >/dev/null; then
    PACKAGE_MANAGER="dnf"
elif command -v yum >/dev/null; then
    PACKAGE_MANAGER="yum"
else
    echo "Unsupported package manager. Please install dependencies manually."
    sleep 5
    PACKAGE_MANAGER="unknown"
fi

case $PACKAGE_MANAGER in
    apt)
        sudo apt-get update
        sudo apt-get upgrade
        sudo apt-get install $APT_PACKAGES
        if [ $? -ne 0 ]; then
            echoerr "Error installing packages with apt-get. Please check your package manager settings."
            sleep 10
        fi
        ;;
    dnf)
        sudo dnf install $DNF_PACKAGES
        if [ $? -ne 0 ]; then
            echoerr "Error installing packages with dnf. Please check your package manager settings."
            sleep 10
        fi
        ;;
    yum)
        sudo yum install $YUM_PACKAGES
        if [ $? -ne 0 ]; then
            echoerr "Error installing packages with yum. Please check your package manager settings."
            sleep 10
        fi
        ;;
    unknown)
        echo "Please ensure the following packages are installed: SDL2, SDL2_image, SDL2_ttf, freetype, development tools, python3, numpy, alsa-lib, pulseaudio-libs, mingw64-gcc, mingw64-gcc-c++"
        sleep 10
        ;;
esac

####################################
# Setting up directories
PROGRESS="Setting up directories"

START_DIR=$(pwd)

# Basic directories
mkdir -p ./.build
mkdir -p ./bin
mkdir -p ./external

# Resources directory
cd ./Resources      || exit 1
../Scripts/CreateResourcesDirectory.sh   || exit 1
cd "$START_DIR"

####################################
# Synonyms for SDL_ttf
PROGRESS="Setting up SDL_ttf synonyms"

# Ensure aclocal-1.16 and automake-1.16 are available as symlinks if only unversioned ones exist
#if ! command -v aclocal-1.16 >/dev/null 2>&1; then
#    aclocal_path=$(command -v aclocal)
#    if [ -n "$aclocal_path" ]; then
#        ln -sf "$aclocal_path" "$HOME/.local/bin/aclocal-1.16"
#        export PATH="$HOME/.local/bin:$PATH"
#    fi
#fi
#if ! command -v automake-1.16 >/dev/null 2>&1; then
#    automake_path=$(command -v automake)
#    if [ -n "$automake_path" ]; then
#        ln -sf "$automake_path" "$HOME/.local/bin/automake-1.16"
#        export PATH="$HOME/.local/bin:$PATH"
#    fi
#fi


####################################
# Submodules: Init
PROGRESS="Initializing git submodules"

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
# Submodules: SDL: Externals
PROGRESS="Setting up SDL externals"

####################################
# Submodules: SDL


####################################
# Building the project
PROGRESS="Building the project"
cd "$START_DIR"
./build.sh    || { echoerr "build.sh failed";      exit 1; }
####################################
# Copy necessary dlls
PROGRESS="Copying necessary DLLs"
cd "$START_DIR"
if [ -f /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll ]; then
    cp /usr/x86_64-w64-mingw32/lib/libwinpthread-1.dll ./bin/
else
    echoerr "libwinpthread-1.dll not found in /usr/x86_64-w64-mingw32/lib/"
    exit 1;
fi

####################################
# make all scripts executable
PROGRESS="Making scripts executable"
cd "$START_DIR"
find ./Scripts/ -type f -iname "*.sh" -exec chmod +x {} \;

####################################
# Run tests
PROGRESS="Running tests"
cd "$START_DIR"
python ./Scripts/validate_json.py
python ./Scripts/Tests.py

####################################
# Finishing up
PROGRESS="Finishing up"

# Show runtime
end=`date +%s`
runtime=$((end-start))

echo ""
echo "-----------------------------------------------------"
echo "Installing + Running tests took $runtime Seconds."
echo ""

