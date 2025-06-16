from PIL import Image

img = Image.new("RGB", (10, 10), color=(73, 109, 137))
img.save("www/images/sample_image.jpg")