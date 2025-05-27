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
