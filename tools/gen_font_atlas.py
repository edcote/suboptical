"""Bitmap font atlas generator for subdemo3.

Generates a grid of 96 contiguous ASCII characters (32-127).
Outputs both a raw 4bpp binary blob and a preview PNG.
"""

import argparse
import os
from PIL import Image, ImageDraw, ImageFont


def main():
  """Parses arguments and generates a 4bpp raw font atlas binary and PNG."""
  parser = argparse.ArgumentParser(description="Generate a raw font atlas.")
  parser.add_argument("--output_dir", required=True, help="Output directory")
  parser.add_argument("--stem", required=True, help="Output filename stem")
  parser.add_argument("--font", required=True, help="Path to TTF font file")
  parser.add_argument("--size", type=int, default=8, help="Character width")
  parser.add_argument("--height", type=int, default=12, help="Character height")
  args = parser.parse_args()

  # Define the 96 contiguous ASCII characters (32 to 127)
  chars = [chr(i) for i in range(32, 128)]
  grid_cols = 16
  grid_rows = (len(chars) + grid_cols - 1) // grid_cols

  img_width = grid_cols * args.size
  img_height = grid_rows * args.height

  image = Image.new("L", (img_width, img_height), color=0)
  draw = ImageDraw.Draw(image)

  try:
    font = ImageFont.truetype(args.font, args.height - 1)
  except OSError:
    print(f"Warning: Could not load font {args.font}, falling back to default.")
    font = ImageFont.load_default()

  for i, char in enumerate(chars):
    row = i // grid_cols
    col = i % grid_cols
    draw.text((col * args.size + 1, row * args.height), char, font=font, fill=255)

  pixels = list(image.getdata())
  packed_data = bytearray()
  preview_pixels = []

  for i in range(0, len(pixels), 2):
    p0 = (pixels[i] >> 4) & 0x0F
    p1 = (pixels[i+1] >> 4) & 0x0F if i+1 < len(pixels) else 0
    packed_data.append((p0 << 4) | p1)
    preview_pixels.extend([p0 << 4, p1 << 4])

  # Save Binary
  bin_name = f"{args.stem}.bin"
  with open(os.path.join(args.output_dir, bin_name), "wb") as f:
    f.write(packed_data)

  # Save PNG Preview
  preview_image = Image.new("L", (img_width, img_height))
  preview_image.putdata(preview_pixels[:len(pixels)])
  png_name = f"{args.stem}.png"
  preview_image.save(os.path.join(args.output_dir, png_name))

  print(f"Font atlas '{args.stem}' saved to {args.output_dir}")


if __name__ == "__main__":
  main()
