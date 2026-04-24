#include "include/planar_canvas.h"

#include <dos.h>

#include "include/font.h"

namespace subdemo {

namespace {
constexpr int kSequencerIndexPort = 0x3C4;
constexpr int kMapMaskIndex = 0x02;
}  // namespace

PlanarCanvas::PlanarCanvas(uint8_t* buffer, int width, int height)
    : buffer_(buffer), width_(width), height_(height) {}

void PlanarCanvas::Clear(uint8_t color_index) {
  // Index 0x02: Sequencer Map Mask
  // Value: 0x0F (Binary: 00001111)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-3 | Plane Enable      |  0x0F | Enable all 4 memory planes for block
  // clear
  outportb(kSequencerIndexPort, kMapMaskIndex);
  outportb(kSequencerIndexPort + 1, 0x0F);

  // Pack the 8-bit color index into all slots of a 32-bit dword.
  const uint32_t color_dword = color_index | (color_index << 8) |
                               (color_index << 16) | (color_index << 24);

  // Each dword write (4 bytes) clears 16 pixels (4 addresses * 4 planes).
  int dwords_to_clear = (width_ * height_) / 16;

  uint8_t* dest = buffer_;
  asm volatile(
      "cld\n\t"
      "rep stosl\n\t"
      : "+D"(dest), "+c"(dwords_to_clear)
      : "a"(color_dword)
      : "memory");
}

void PlanarCanvas::PutPixel(int x, int y, uint8_t color_index) {
  // Select which plane(s) to write to (bits 0-3 correspond to planes 0-3).
  // Index 0x02: Sequencer Map Mask
  // Value: 1 << (x & 3)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-3 | Plane Enable      |  VAR  | Select exactly one plane (0-3) based on X
  outportb(kSequencerIndexPort, kMapMaskIndex);
  outportb(kSequencerIndexPort + 1, 1 << (x & 3));

  // In unchained modes, address is (y * width + x) / 4.
  buffer_[(y * width_ + x) >> 2] = color_index;
}

void PlanarCanvas::DrawChar(int x, int y, char c, uint8_t color,
                            const Font& font) {
  // TODO: edc - Implement pre-planarized shifted fonts.
  const uint8_t* glyph_data = font.GetGlyphData(c);
  if (!glyph_data) return;

  const int glyph_width = font.glyph_width();
  const int glyph_height = font.glyph_height();
  const int x_offset = x % 4;

  // Optimization: Loop through VGA planes once per character.
  for (int plane = 0; plane < 4; ++plane) {
    // Index 0x02: Sequencer Map Mask
    // Bit | Field Name        | Value | Description
    // ----|-------------------|-------|-------------------------------------------
    // 0-3 | Plane Enable      | 1 << P| Select current plane for bulk drawing
    outportb(kSequencerIndexPort, kMapMaskIndex);
    outportb(kSequencerIndexPort + 1, 1 << plane);

    // Calculate which glyph X coordinate belongs to this hardware plane.
    const int first_glyph_x = (plane - x_offset + 4) % 4;

    for (int glyph_y = 0; glyph_y < glyph_height; ++glyph_y) {
      const int pixel_y = y + glyph_y;
      if (pixel_y < 0 || pixel_y >= height_) continue;

      // Pointer to the first VGA byte in this scanline that belongs to the
      // current plane.
      uint8_t* target = &buffer_[(pixel_y * width_ + (x + first_glyph_x)) >> 2];

      // Pointer to the first glyph source byte for this row/plane.
      const uint8_t* source =
          &glyph_data[glyph_y * glyph_width + first_glyph_x];

      for (int glyph_x = first_glyph_x; glyph_x < glyph_width; glyph_x += 4) {
        // Since we use 1 byte per pixel (8bpp), just check intensity.
        if (*source > 0) {
          *target = color;
        }
        source += 4;
        target++;
      }
    }
  }
}

void PlanarCanvas::DrawText(int x, int y, const char* text, uint8_t color,
                            const Font& font) {
  int current_x = x;
  while (*text) {
    DrawChar(current_x, y, *text, color, font);
    current_x += font.glyph_width();
    text++;
  }
}

}  // namespace subdemo
