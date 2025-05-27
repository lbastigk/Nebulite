#!/bin/bash
rootDir=$(pwd)

# Function to download only if file doesn't exist
download_if_not_exists() {
  local filename="$1"
  local url="$2"

  if [ ! -f "$filename" ]; then
    wget -O "$filename" "$url"
  else
    echo "Skipping '$filename' (already exists)"
  fi
}

#----------------------------------------
# Main Directories
mkdir -p ./Creatures
mkdir -p ./Cursor
mkdir -p ./Editor
mkdir -p ./Fonts
mkdir -p ./Invokes
mkdir -p ./Levels
mkdir -p ./Renderobjects
mkdir -p ./Sprites

#----------------------------------------
# Fonts
cd $rootDir/Fonts/
git clone https://github.com/googlefonts/Arimo
mv ./Arimo/fonts/ttf/*.ttf ./
rm -rf ./Arimo/

#----------------------------------------
# Others

# Cursor, inspired by drakensang
cd $rootDir/Cursor/
python3 $rootDir/CreationScripts/CreateCursor.py

# Editor images
mkdir $rootDir/Editor/Selection/
cd $rootDir/Editor/Selection/

python3 $rootDir/CreationScripts/CreateSelectionFrame.py

#----------------------------------------
# Sprites
cd "$rootDir/Sprites"

mkdir $rootDir/Sprites/Pokemon
mkdir $rootDir/Sprites/Pokemon_Transparent
mkdir $rootDir/Sprites/TEST001P
mkdir $rootDir/Sprites/TEST100P

# Lilac chaser test image
download_if_not_exists "TEST_BMP_SMALL.bmp" https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif
convert https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif[0] TEST_BMP_SMALL.bmp
rm $rootDir/Sprites/Lilac-Chaser.gif


# Pokemon test images
cd $rootDir/Sprites/Pokemon
download_if_not_exists "Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png"     https://www.spriters-resource.com/resources/sheets/4/3698.png
download_if_not_exists "Game Boy Advance - Pokemon FireRed LeafGreen - Buildings.png"          https://www.spriters-resource.com/resources/sheets/4/3849.png
download_if_not_exists "Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 1.png"          https://www.spriters-resource.com/resources/sheets/4/3862.png
download_if_not_exists "Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png"          https://www.spriters-resource.com/resources/sheets/4/3863.png
download_if_not_exists "Game Boy Advance - Pokemon FireRed LeafGreen - Tileset.png"            https://www.spriters-resource.com/resources/sheets/4/3870.png
download_if_not_exists "Game Boy Advance - Pokemon FireRed LeafGreen - Animated Tiles.png"     https://www.spriters-resource.com/resources/sheets/161/164543.png

# Make transparent
cd $rootDir/Sprites/Pokemon
cp "./Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png" "$rootDir/Sprites/Pokemon_Transparent/Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png"
cp "./Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png"      "$rootDir/Sprites/Pokemon_Transparent/Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png"
cd $rootDir/Sprites/Pokemon_Transparent
python3 $rootDir/CreationScripts/CreateImageTransparent.py -i "./Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png" -ymin 0 -ymax 2966 -kc 255 127 39 255 -o "./Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png"


# Test001P
cd $rootDir/Sprites/TEST001P
python3 $rootDir/CreationScripts/CreateNoisyImages.py -n 128 -s 1 1

# Test100P
cd $rootDir/Sprites/TEST100P

python3 $rootDir/CreationScripts/CreateNoisyImages.py -n 128 -s 100 100

echo "DONE!"
