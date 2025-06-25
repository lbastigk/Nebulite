#!/bin/bash


# Change to the script's directory
cd "$(dirname "$0")"
rootDir=$(pwd)

#----------------------------------------
# Main Directories
mkdir -p ./Cursor
mkdir -p ./Fonts
mkdir -p ./Invokes
mkdir -p ./Levels
mkdir -p ./Renderobjects
mkdir -p ./Sprites

#----------------------------------------
# Fonts
cd $rootDir/Fonts/
if [ ! -f "Arimo-Bold.ttf" ] || [ ! -f "Arimo-BoldItalic.ttf" ] || [ ! -f "Arimo-Italic.ttf" ] || [ ! -f "Arimo-Regular.ttf" ]
then
  git clone https://github.com/googlefonts/Arimo
  mv ./Arimo/fonts/ttf/*.ttf ./
  rm -rf ./Arimo/
else
  echo "Skipping Arimo (already exists)"
fi

#----------------------------------------
# Others

# Cursor, inspired by drakensang
cd $rootDir/Cursor/
if [ ! -f "Drakensang.png" ]
then
  python3 $rootDir/CreationScripts/CreateCursor.py
else
  echo "Skipping Cursor Drakensang.png (already exists)"
fi


#----------------------------------------
# Sprites
cd "$rootDir/Sprites"

mkdir -p $rootDir/Sprites/miniworldsprites
mkdir -p $rootDir/Sprites/TEST001P
mkdir -p $rootDir/Sprites/TEST100P

# Lilac chaser test image
if [ ! -f "TEST_BMP_SMALL.bmp" ]
then
  wget https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif
  convert https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif[0] TEST_BMP_SMALL.bmp
  rm $rootDir/Sprites/Lilac-Chaser.gif
else
  echo "Skipping TEST_BMP_SMALL.bmp (already exists)"
fi

# Sprites
cd "$rootDir/Sprites/"
if [ ! -d "MiniWorldSprites" ]
then
  wget https://opengameart.org/sites/default/files/miniworldsprites_1.zip
  unzip miniworldsprites_1.zip
  rm -rf miniworldsprites_1.zip
else
  echo "Skipping MiniWorldSprites (already exists)"
fi

# Test001P
cd $rootDir/Sprites/TEST001P
python3 $rootDir/CreationScripts/CreateNoisyImages.py -n 128 -s 1 1

# Test100P
cd $rootDir/Sprites/TEST100P

python3 $rootDir/CreationScripts/CreateNoisyImages.py -n 128 -s 100 100

echo "Resources directory creation done"
