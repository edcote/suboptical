#include "include/font.h"

#include "include/logger.h"

namespace subdemo {

Font::Font(const uint8_t* data) : data_(data) {
  if (!data) {
    LogFatal("Font data pointer is null.");
  }

  // Validate SBDF Magic Number
  if (data[0] != 'S' || data[1] != 'B' || data[2] != 'D' || data[3] != 'F') {
    LogFatal("Invalid font magic number. Expected 'SBDF'.");
  }

  // Validate Character Count (must be exactly 96 for this implementation).
  if (data[7] != kNumChars) {
    LogFatal("Invalid font character count. Expected %d, got %d.", kNumChars,
             static_cast<int>(data[7]));
  }

  glyph_width_ = data[5];
  glyph_height_ = data[6];
}

const uint8_t* Font::GetGlyphData(char c) const {
  const int index = static_cast<uint8_t>(c) - 32;
  if (index < 0 || index >= kNumChars) {
    return nullptr;
  }

  // Sequential 8bpp resource: 1 byte per pixel.
  const uint32_t glyph_size_bytes = glyph_width_ * glyph_height_;

  return data_ + 8 + (index * glyph_size_bytes);
}

}  // namespace subdemo
