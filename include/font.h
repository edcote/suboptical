#pragma once
#include <cstdint>

namespace subdemo {

// Represents a raster font resource.
// The font data must be in the Suboptical Bitmap Digital Font (SBDF) format:
// Offset | Description
// -------|----------------------
// 0-3    | Magic: 'SBDF'
// 4      | Version: 0x01
// 5      | Glyph Width
// 6      | Glyph Height
// 7      | Number of Characters (Hardcoded to 96)
// 8+     | Glyph Data (8bpp sequential bytes)
class Font {
 public:
  static constexpr int kNumChars = 96;

  // Initializes a font from a raw memory buffer.
  // Performs validation and issues LogFatal if magic or char count mismatch.
  // N.B.: Does not take ownership of the memory.
  explicit Font(const uint8_t* data);

  uint16_t glyph_width() const { return glyph_width_; }
  uint16_t glyph_height() const { return glyph_height_; }

  // Returns a pointer to the start of the 4bpp glyph data for the character.
  // Returns nullptr if the character is out of the font's 32-127 range.
  const uint8_t* GetGlyphData(char c) const;

 private:
  const uint8_t* data_;
  uint8_t glyph_width_ = 0;
  uint8_t glyph_height_ = 0;
};

}  // namespace subdemo
