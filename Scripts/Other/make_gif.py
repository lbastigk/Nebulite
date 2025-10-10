from PIL import Image
import glob

new_size = (500, 500)

frames = sorted(glob.glob('./Resources/Snapshots/ToGif/*.png'))

images = []
for frame in frames:
    img = Image.open(frame)
    img = img.resize(new_size, Image.Resampling.LANCZOS)
    images.append(img)

images[0].save(
    'output.gif',
    save_all=True,
    append_images=images[1:],
    duration=20,
    loop=0,
    optimize=True
)

print("GIF saved as output.gif")
