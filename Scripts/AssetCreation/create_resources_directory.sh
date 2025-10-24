#!/bin/bash

# Making sure we are inside directory "Resources"
# We do this by checking if the directory exists
if [ -d "Resources" ]; then
  cd Resources
fi

#----------------------------------------
# Get the Resources root directory
ResourcesDir=$(pwd)

#----------------------------------------
# Main Directories
mkdir -p ./Cursor
mkdir -p ./Editor
mkdir -p ./Fonts
mkdir -p ./Invokes
mkdir -p ./Levels
mkdir -p ./Renderobjects
mkdir -p ./Sprites

#----------------------------------------
# Fonts
cd $ResourcesDir/Fonts/
if ! ls Arimo*.ttf JetBrains*.ttf 1> /dev/null 2>&1; then
  git clone https://github.com/googlefonts/Arimo
  mv ./Arimo/fonts/ttf/*.ttf ./
  rm -rf ./Arimo/

  git clone https://github.com/JetBrains/JetBrainsMono.git
  mv ./JetBrainsMono/fonts/ttf/*.ttf ./
  rm -rf ./JetBrainsMono/
else
  echo "Skipping Arimo/JetBrainsMono (already exists)"
fi

#----------------------------------------
# Others

# Cursor, inspired by drakensang
cd $ResourcesDir/Cursor/
if [ ! -f "Drakensang.png" ]
then
  python3 $ResourcesDir/../Scripts/AssetCreation/Cursor.py
else
  echo "Skipping Cursor Drakensang.png (already exists)"
fi

# Selection
cd $ResourcesDir/Editor/
if [ ! -f "Selection.png" ]
then
  python3 $ResourcesDir/../Scripts/AssetCreation/Selection.py
else
  echo "Skipping Editor Selection.png (already exists)"
fi

#----------------------------------------
# Sprites
cd "$ResourcesDir/Sprites"

#mkdir -p $ResourcesDir/Sprites/miniworldsprites
mkdir -p $ResourcesDir/Sprites/TEST001P
mkdir -p $ResourcesDir/Sprites/TEST100P

# Lilac chaser test image
if [ ! -f "TEST_BMP_SMALL.bmp" ]
then
  wget https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif
  convert https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif[0] TEST_BMP_SMALL.bmp
  rm $ResourcesDir/Sprites/Lilac-Chaser.gif
else
  echo "Skipping TEST_BMP_SMALL.bmp (already exists)"
fi

# MiniWorldSprites
cd "$ResourcesDir/Sprites/"
if [ ! -d "MiniWorldSprites" ]
then
  wget https://opengameart.org/sites/default/files/miniworldsprites_1.zip
  unzip miniworldsprites_1.zip
  rm -rf miniworldsprites_1.zip
else
  echo "Skipping MiniWorldSprites (already exists)"
fi

# Test001P
echo "Creating noisy images for TEST001P"
cd $ResourcesDir/Sprites/TEST001P
python3 $ResourcesDir/../Scripts/AssetCreation/NoisyColouredImages.py -n 128 -s 1 1

# Test100P
echo "Creating noisy images for TEST100P"
cd $ResourcesDir/Sprites/TEST100P
python3 $ResourcesDir/../Scripts/AssetCreation/NoisyColouredImages.py -n 128 -s 100 100
echo "Resources directory creation done"
