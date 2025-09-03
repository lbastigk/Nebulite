from PIL import Image, ImageDraw, ImageFilter

def create_selection(output_path="Selection.png"):
    # Creating a 16x16 selection image
    # 1pix wide white frame
    # rest transparent
    image_size = (16, 16)

    img = Image.new("RGBA", image_size, (0, 0, 0, 0))  # Fully transparent image
    draw = ImageDraw.Draw(img)
    # Draw the outer frame
    draw.rectangle([0, 0, 15, 15], outline=(255, 255, 255, 255), width=1)
    img.save(output_path)

create_selection()