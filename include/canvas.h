#pragma once
#include <cstdint>

namespace subdemo {

class Font;

// Canvas provides an abstract interface for drawing to a memory buffer.
class Canvas {
 public:
  virtual ~Canvas() = default;

  // Fills the entire buffer with the specified color index.
  virtual void Clear(uint8_t color_index) = 0;

  // Draws a single pixel with the specified color index.
  virtual void PutPixel(int x, int y, uint8_t color_index) = 0;

  // Sets the internal buffer pointer.
  virtual void SetBuffer(uint8_t* buffer) = 0;

  // Returns the current target memory buffer.
  virtual uint8_t* buffer() const = 0;

  // Draws a single character using the provided font.
  virtual void DrawChar(int x, int y, char c, uint8_t color,
                        const Font& font) = 0;

  // Draws a string of text using the provided font.
  virtual void DrawText(int x, int y, const char* text, uint8_t color,
                        const Font& font) = 0;

  virtual int width() const = 0;
  virtual int height() const = 0;
};

}  // namespace subdemo
