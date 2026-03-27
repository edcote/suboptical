#pragma once
#include "include/canvas.h"

namespace video {

// Specialization of Canvas for VGA Mode X unchained planar memory.
// It uses hardware bitmasks (Sequencer Map Mask) for drawing.
class PlanarCanvas : public Canvas {
 public:
  PlanarCanvas(uint8_t* buffer, int width, int height);
  ~PlanarCanvas() override = default;

  void Clear(uint8_t color_index) override;
  void PutPixel(int x, int y, uint8_t color_index) override;
  void SetBuffer(uint8_t* buffer) override { buffer_ = buffer; }
  uint8_t* buffer() const override { return buffer_; }

 private:
  uint8_t* buffer_;
  int width_;
  int height_;
};

}  // namespace video
