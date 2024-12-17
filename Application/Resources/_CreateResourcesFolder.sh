#!/bin/bash
rootDir=$(pwd)


#----------------------------------------
# Main Directories
mkdir ./Creatures
mkdir ./Cursor
mkdir ./Editor
mkdir ./Fonts
mkdir ./Levels
mkdir ./Moverulesets
mkdir ./Renderobjects
mkdir ./Sprites

#----------------------------------------
# Fonts
cd $rootDir/Fonts/

wget https://www.fontsquirrel.com/fonts/download/arimo
unzip -n $rootDir/Fonts/arimo
rm $rootDir/Fonts/arimo
rm "$rootDir/Fonts/Apache License.txt"


#----------------------------------------
# Others

# Cursor, inspired by drakensang
cd $rootDir/Cursor/
python3 << EOF
from PIL import Image, ImageDraw, ImageFilter

def create_cursor(output_path="Drakensang.png"):
    # Image size
    width, height = 24, 24

    # Base transparent background for the final image
    final_image = Image.new("RGBA", (width, height), (255, 0, 0, 0))

    # Step 1: Create a separate layer for the outline
    outline_layer = Image.new("RGBA", (width, height), (255, 0, 0, 0))
    outline_draw = ImageDraw.Draw(outline_layer)

    # Define arrow coordinates
    arrow_coords = [
        (0, 0), (21, 7), (15, 10), (23, 17), (17, 23), (10, 15), (7, 21)
    ]
    highlight_coords = [
        (1, 1), (15, 5), (5, 5), (5, 15)
    ]

    # Draw the sharp outline on the outline layer
    outline_draw.polygon(arrow_coords, outline=(0, 0, 0, 255), fill=None, width=1)

    # Step 2: Create another layer for the blurred content
    blur_layer = Image.new("RGBA", (width, height), (255, 0, 0, 0))
    blur_draw = ImageDraw.Draw(blur_layer)

    # Draw the base arrow shape in gold-like color
    blur_draw.polygon(arrow_coords, fill=(215, 200, 0, 255))

    # Add a highlight (optional)
    blur_draw.polygon(highlight_coords, fill=(255, 255, 128, 255))

    # Apply Gaussian Blur to the blur layer
    blurred_content = blur_layer.filter(ImageFilter.GaussianBlur(radius=0.7))

    # Step 3: Combine the outline and blurred content
    final_image = Image.alpha_composite(final_image, blurred_content)  # Add blurred content
    final_image = Image.alpha_composite(final_image, outline_layer)    # Add sharp outline

    # Save the final image
    final_image.save(output_path)
    print(f"Cursor saved as '{output_path}'")

# Create and save the cursor
create_cursor()
EOF

# Editor images
mkdir $rootDir/Editor/Selection/
cd $rootDir/Editor/Selection/

python3 << EOF
from PIL import Image, ImageDraw

# Create a new 32x32 image with an alpha (transparency) channel
image = Image.new("RGBA", (32, 32), (0, 0, 0, 0))  # (0, 0, 0, 0) is fully transparent

# Create a drawing context
draw = ImageDraw.Draw(image)

# Define frame width and color (white)
frame_width = 3
frame_color = (255, 255, 255, 255)  # White color with full opacity

# Draw the white frame
draw.rectangle([frame_width, frame_width, 32 - frame_width, 32 - frame_width], outline=frame_color, width=frame_width)

# Save the image as a PNG file
image.save("Selection_00000.png")

EOF

#----------------------------------------
# Sprites
cd "$rootDir/Sprites"

mkdir $rootDir/Sprites/Pokemon
mkdir $rootDir/Sprites/Pokemon_Transparent
mkdir $rootDir/Sprites/TEST001P
mkdir $rootDir/Sprites/TEST100P

# Lilac chaser test image
wget https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif
convert https://upload.wikimedia.org/wikipedia/commons/6/6e/Lilac-Chaser.gif[0] TEST_BMP_SMALL.bmp
rm $rootDir/Sprites/Lilac-Chaser.gif


# Pokemon test images
cd $rootDir/Sprites/Pokemon
wget -O "Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png"     https://www.spriters-resource.com/resources/sheets/4/3698.png
wget -O "Game Boy Advance - Pokemon FireRed LeafGreen - Buildings.png"          https://www.spriters-resource.com/resources/sheets/4/3849.png
wget -O "Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 1.png"          https://www.spriters-resource.com/resources/sheets/4/3862.png
wget -O "Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png"          https://www.spriters-resource.com/resources/sheets/4/3863.png
wget -O "Game Boy Advance - Pokemon FireRed LeafGreen - Tileset.png"            https://www.spriters-resource.com/resources/sheets/4/3870.png
wget -O "Game Boy Advance - Pokemon FireRed LeafGreen - Animated Tiles.png"     https://www.spriters-resource.com/resources/sheets/161/164543.png

# Make transparent
cd $rootDir/Sprites/Pokemon
cp "./Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png" "$rootDir/Sprites/Pokemon_Transparent/Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png"
cp "./Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png"      "$rootDir/Sprites/Pokemon_Transparent/Game Boy Advance - Pokemon FireRed LeafGreen - Tileset 2.png"
cd $rootDir/Sprites/Pokemon_Transparent
python3 << EOF
import numpy as np
from PIL import Image
from collections import Counter
import math

def turnToPixels(image_path):
    # Open the image
    img = Image.open(image_path)
    img = img.convert("RGBA")  # Ensure the image is in RGBA format
    pixels = np.array(img)
    return pixels

def key_out_color(pixels, ymin, ymax, key_color=None):


    # Get the height and width of the image
    height, width, _ = pixels.shape

    # Validate ymin and ymax
    if ymin < 0 or ymax >= height or ymin > ymax:
        raise ValueError("Invalid ymin or ymax values!")

    # Extract the region within the Y-range [ymin, ymax]
    region_pixels = pixels[ymin:ymax+1, :, :]

    # If the key_color is NaN, find the most common colors in the specified region
    if key_color is None or (isinstance(key_color, float) and math.isnan(key_color)):
        flattened_pixels = region_pixels.reshape(-1, 4)  # Flatten the array for easier counting
        pixel_colors = [tuple(pixel) for pixel in flattened_pixels]

        # Count occurrences of each color
        color_count = Counter(pixel_colors)

        # Get the top 5 most common colors
        most_common_colors = color_count.most_common(5)

        print(f"Most common colors in the region [{ymin}, {ymax}] are:")
        for color, count in most_common_colors:
            print(f"Color {color} appears {count} times")

        return  # Exit after printing colors if key_color is NaN

    # Perform color keying by setting matching pixels in the range [ymin, ymax] to transparent
    key_color = tuple(key_color)  # Ensure key_color is a tuple
    for y in range(ymin, ymax+1):
        for x in range(width):
            if tuple(pixels[y, x, :]) == key_color:
                pixels[y, x, 3] = 0  # Set alpha to 0 for transparency

    # Create a new image from the modified pixel array
    output_image = Image.fromarray(pixels)
    return output_image


if __name__ == "__main__":
    image_file = './Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png'
    ymin = 0  # Replace with the desired ymin value
    ymax = 2966  # Replace with the desired ymax value
    key_color = (255, 127, 39, 255)  # Replace with the color to key out (R, G, B, A), or set to None for NaN
    key_color2 = (34, 177, 76, 255)  # Replace with the color to key out (R, G, B, A), or set to None for NaN
    # key_color = None

    image = turnToPixels(image_file)
    result_image = key_out_color(image, ymin, ymax, key_color)
    result_image = key_out_color(image, ymin, ymax, key_color2)

    if result_image:
        result_image.save('Game Boy Advance - Pokemon FireRed LeafGreen - Overworld NPCs.png')
EOF

# Test001P
cd $rootDir/Sprites/TEST001P

python3 << EOF
from PIL import Image
import os
import numpy as np
import colorsys

# Number of textures to generate
num_textures = 128

# Texture size
texture_size = (1, 1)

# Function to add random noise to an image
def add_noise(image):
    noise = np.random.normal(0, 25, image.shape)
    noisy_image = np.clip(image + noise, 0, 255)
    return noisy_image.astype(np.uint8)

for i in range(num_textures):
    # Generate a hue value based on the index
    hue = i / num_textures  # Normalize hue value between 0 and 1
    saturation = 1  # Full saturation
    value = 1  # Full brightness

    # Convert HSV to RGB using colorsys
    red, green, blue = colorsys.hsv_to_rgb(hue, saturation, value)

    # Scale the RGB values to 0-255
    red, green, blue = int(red * 255), int(green * 255), int(blue * 255)

    # Create a new image with the specified size and color
    image = Image.new("RGB", texture_size, (red, green, blue))

    # Add random noise to the image
    image_array = np.array(image)
    noisy_image_array = add_noise(image_array)
    noisy_image = Image.fromarray(noisy_image_array)

    # Save the image as BMP to the output directory
    noisy_image.save(f"{i + 1}.bmp", format='BMP')
EOF
cd ../

# Test100P
cd $rootDir/Sprites/TEST100P

python3 << EOF
from PIL import Image
import os
import numpy as np
import colorsys

# Number of textures to generate
num_textures = 128

# Texture size
texture_size = (100, 100)

# Function to add random noise to an image
def add_noise(image):
    noise = np.random.normal(0, 25, image.shape)
    noisy_image = np.clip(image + noise, 0, 255)
    return noisy_image.astype(np.uint8)

for i in range(num_textures):
    # Generate a hue value based on the index
    hue = i / num_textures  # Normalize hue value between 0 and 1
    saturation = 1  # Full saturation
    value = 1  # Full brightness

    # Convert HSV to RGB using colorsys
    red, green, blue = colorsys.hsv_to_rgb(hue, saturation, value)

    # Scale the RGB values to 0-255
    red, green, blue = int(red * 255), int(green * 255), int(blue * 255)

    # Create a new image with the specified size and color
    image = Image.new("RGB", texture_size, (red, green, blue))

    # Add random noise to the image
    image_array = np.array(image)
    noisy_image_array = add_noise(image_array)
    noisy_image = Image.fromarray(noisy_image_array)

    # Save the image as BMP to the output directory
    noisy_image.save(f"{i + 1}.bmp", format='BMP')
EOF
cd ../

echo "DONE!"
