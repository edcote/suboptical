"""Raster font binary generator for subdemo3.

Generates a sequential binary font resource (8bpp) and a grid-based preview PNG.
The binary format starts with an 8-byte header:
Magic (4 bytes): 'SBDF'
Version (1 byte): 0x01
Width (1 byte): glyph_width
Height (1 byte): glyph_height
Count (1 byte): num_characters
"""

import argparse
import os
from PIL import Image, ImageDraw, ImageFont


def main():
    """Parses arguments and generates an 8bpp raw font binary and PNG preview."""
    parser = argparse.ArgumentParser(description="Generate a raw font binary.")
    parser.add_argument("--output_dir", required=True, help="Output directory")
    parser.add_argument("--font_name", required=True, help="Font resource name")
    parser.add_argument("--input_font", required=True, help="Path to TTF font file")
    parser.add_argument("--glyph_width", type=int, default=8, help="Glyph width")
    parser.add_argument("--glyph_height", type=int, default=12, help="Glyph height")
    args = parser.parse_args()

    # Define the 96 contiguous ASCII characters (32 to 127)
    chars = [chr(i) for i in range(32, 128)]
    grid_cols = 16
    grid_rows = (len(chars) + grid_cols - 1) // grid_cols

    # We render each character individually into a sequential buffer
    # for the binary, while maintaining a grid for the preview PNG.
    packed_data = bytearray()

    # Header (8 bytes)
    packed_data.extend(b"SBDF")
    packed_data.append(1)  # Version
    packed_data.append(args.glyph_width)
    packed_data.append(args.glyph_height)
    packed_data.append(len(chars))

    try:
        font = ImageFont.truetype(args.input_font, args.glyph_height - 1)
    except OSError:
        print(f"Warning: Could not load font {args.input_font}, using default.")
        font = ImageFont.load_default()

    # Preview image setup
    img_width = grid_cols * args.glyph_width
    img_height = grid_rows * args.glyph_height
    preview_image = Image.new("L", (img_width, img_height), color=0)
    preview_draw = ImageDraw.Draw(preview_image)

    for i, char in enumerate(chars):
        row = i // grid_cols
        col = i % grid_cols

        # Calculate position in preview grid
        grid_x = col * args.glyph_width
        grid_y = row * args.glyph_height

        # Draw to preview grid
        # Offset by 1 pixel horizontally for better centering of most fonts
        preview_draw.text((grid_x + 1, grid_y), char, font=font, fill=255)

        # For the binary, we want glyph-sequential data.
        # We'll create a temporary image for just this glyph.
        char_img = Image.new("L", (args.glyph_width, args.glyph_height), color=0)
        char_draw = ImageDraw.Draw(char_img)
        char_draw.text((1, 0), char, font=font, fill=255)

        # Store character pixels as 8bpp (one byte per pixel)
        char_pixels = list(char_img.getdata())
        for p in char_pixels:
            packed_data.append(p)

    # Save Binary
    bin_name = f"{args.font_name}.bin"
    with open(os.path.join(args.output_dir, bin_name), "wb") as f:
        f.write(packed_data)

    # Save PNG Preview
    png_name = f"{args.font_name}.png"
    preview_image.save(os.path.join(args.output_dir, png_name))

    print(f"Raster font '{args.font_name}' ({args.glyph_width}x{args.glyph_height})")
    print(f"Saved to: {os.path.join(args.output_dir, bin_name)}")


if __name__ == "__main__":
    main()
