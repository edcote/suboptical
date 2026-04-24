#pragma once
#include "include/canvas.h"

namespace subdemo {

// Specialization of Canvas for VGA Mode X unchained planar memory.
// It uses hardware bitmasks (Sequencer Map Mask) for drawing.
class PlanarCanvas : public Canvas {
 public:
  // Creates a new PlanarCanvas instance for a specific region of memory.
  PlanarCanvas(uint8_t* buffer, int width, int height);
  ~PlanarCanvas() override = default;

  void Clear(uint8_t color_index) override;
  void PutPixel(int x, int y, uint8_t color_index) override;
  void SetBuffer(uint8_t* buffer) override { buffer_ = buffer; }
  uint8_t* buffer() const override { return buffer_; }

  void DrawChar(int x, int y, char c, uint8_t color, const Font& font) override;
  void DrawText(int x, int y, const char* text, uint8_t color,
                const Font& font) override;

  int width() const override { return width_; }
  int height() const override { return height_; }

 private:
  uint8_t* buffer_;
  int width_;
  int height_;
};

}  // namespace subdemo
