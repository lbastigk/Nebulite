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
#rm -rf ./external #  Uncomment if you want to force re-download of externals

####################################
# starttime of script
start=$(date +%s)

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
        if ! sudo apt-get install $APT_PACKAGES; then
            echoerr "Error installing packages with apt-get. Please check your package manager settings."
            sleep 10
        fi
        ;;
    dnf)
        if ! sudo dnf install $DNF_PACKAGES; then
            echoerr "Error installing packages with dnf. Please check your package manager settings."
            sleep 10
        fi
        ;;
    yum)
        if ! sudo yum install $YUM_PACKAGES; then
            echoerr "Error installing packages with yum. Please check your package manager settings."
            sleep 10
        fi
        ;;
    unknown)
        echo "Please ensure the following packages are installed:"
        echo "$APT_PACKAGES"
        sleep 10
        ;;
esac

####################################
# Setting up directories
PROGRESS="Setting up directories"

ROOT_DIR=$(pwd)

# Basic directories
mkdir -p ./.build
mkdir -p ./.build/SDL2
mkdir -p ./bin
mkdir -p ./external

# Resources directory
cd ./Resources      || exit 1
../Scripts/CreateResourcesDirectory.sh   || exit 1
cd "$ROOT_DIR"

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
# Submodules: SDL2
PROGRESS="Setting up SDL2"

cd "$externalsDir/SDL_Crossplatform_Local" || exit 1
Scripts/install.sh          || exit 1
Scripts/build.sh            || exit 1 # Creates simple SDL2 applications and places all necessary dll files into bin/
#Scripts/test_binaries.sh    || exit 1 # Tests the from build.sh created binaries

cd "$ROOT_DIR"
cp -r "$externalsDir/SDL_Crossplatform_Local/build/SDL2/"* .build/SDL2/

####################################
# Place dlls into .build directory
mkdir -p ./.build/SDL2/bin

# Copy dlls from install.sh-created SDL2_build into the application bin
cp external/SDL_Crossplatform_Local/bin/*.dll ./.build/SDL2/bin/

# Copy libwinpthread-1.dll if it exists
PATTERN=$(find /usr/*x86* -type f -name libwinpthread-1.dll | head -n 1)
if [ -f "$PATTERN" ]; then
    cp "$PATTERN" ./.build/SDL2/bin/
else
    echoerr "libwinpthread-1.dll not found in /usr/x86_64-w64-mingw32/lib/"
    #exit 1;
fi

####################################
# Building the project
PROGRESS="Building the project"
cd "$ROOT_DIR"
./build.sh    || { echoerr "build.sh failed";      exit 1; }

####################################
# Install the vscode extensions
PROGRESS="Installing the VS Code extensions"
cd "$ROOT_DIR"
# .nebs language extension
./Languages/nebs/nebulite-script-vscode/build-and-install.sh || { echoerr "build-and-install.sh of nebs language extension failed"; exit 1; }
# .nebl language extension
# Work in progress...

####################################
# make all scripts executable
PROGRESS="Making scripts executable"
cd "$ROOT_DIR"
find ./Scripts/ -type f -iname "*.sh" -exec chmod +x {} \;

####################################
# Run tests
PROGRESS="Running tests"
cd "$ROOT_DIR"
python ./Scripts/validate_json.py
python ./Scripts/Tests.py

####################################
# Finishing up
PROGRESS="Finishing up"

# Show runtime
end=$(date +%s)
runtime=$((end-start))
echo ""
echo "-----------------------------------------------------"
echo "Installing + Running tests took $runtime Seconds."
echo ""

