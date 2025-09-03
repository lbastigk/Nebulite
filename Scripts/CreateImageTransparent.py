import argparse
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

def key_out_color(pixels, ymin, ymax, key_color):
    # Get the height and width of the image
    height, width, _ = pixels.shape

    # Validate ymin and ymax
    if ymin < 0 or ymax >= height or ymin > ymax:
        raise ValueError("Invalid ymin or ymax values!")

    # Perform color keying by setting matching pixels in the range [ymin, ymax] to transparent
    key_color = tuple(key_color)  # Ensure key_color is a tuple
    for y in range(ymin, ymax+1):
        for x in range(width):
            if tuple(pixels[y, x, :]) == key_color:
                pixels[y, x, 3] = 0  # Set alpha to 0 for transparency

    # Create a new image from the modified pixel array
    output_image = Image.fromarray(pixels)
    return output_image

def main():
    parser = argparse.ArgumentParser(description="Key out a specified color from an image.")
    parser.add_argument("-i", "--image", type=str, required=True, help="Input image file path.")
    parser.add_argument("-ymin", type=int, required=True, help="Minimum Y value for the region to key out.")
    parser.add_argument("-ymax", type=int, required=True, help="Maximum Y value for the region to key out.")
    parser.add_argument("-kc", "--key_color", nargs=4, type=int, required=True,
                        help="Key color in RGBA format to make transparent.")
    parser.add_argument("-o", "--output", type=str, required=True, help="Output image file path.")

    args = parser.parse_args()

    image = turnToPixels(args.image)
    result_image = key_out_color(image, args.ymin, args.ymax, args.key_color)

    if result_image:
        result_image.save(args.output)

if __name__ == "__main__":
    main()
