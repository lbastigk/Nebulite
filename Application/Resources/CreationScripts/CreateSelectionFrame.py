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
