#pragma once
#include <cstdint>

namespace video {

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
};

}  // namespace video
