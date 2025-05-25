import argparse
import numpy as np
from PIL import Image
import os
import colorsys

def add_noise(image):
    noise = np.random.normal(0, 25, image.shape)
    noisy_image = np.clip(image + noise, 0, 255)
    return noisy_image.astype(np.uint8)

def generate_textures(num_textures, texture_size):
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
        noisy_image.save(f"{i + 1 : 04d}.bmp", format='BMP')

def main():
    parser = argparse.ArgumentParser(description="Generate textures with noise and specified size.")
    parser.add_argument("-n", "--num_textures", type=int, required=True, help="Number of textures to generate.")
    parser.add_argument("-s", "--size", type=int, nargs=2, required=True, help="Texture size as width and height.")

    args = parser.parse_args()

    num_textures = args.num_textures
    texture_size = tuple(args.size)

    generate_textures(num_textures, texture_size)

if __name__ == "__main__":
    main()
