#pragma once

#include <cstdint>

#include "include/fixed.h"

namespace subdemo {

// Represents a single VGA color entry (R, G, B).
// N.B.: VGA color components are 6-bit (0-63).
struct RgbColor {
  uint8_t r;
  uint8_t g;
  uint8_t b;
};

// Represents a 256-color palette.
// This class does not own the palette memory; it acts as a wrapper for
// memory managed by the ResourceManager or other system buffers.
class ColorTable {
 public:
  // Points to an existing buffer of 256 RgbColor records.
  explicit ColorTable(RgbColor* colors) : colors_(colors) {}

  // Sets the color at the specified index.
  void SetColor(uint8_t index, const RgbColor& color) {
    colors_[index] = color;
  }

  // Gets a copy of the color at the specified index.
  RgbColor GetColor(uint8_t index) const { return colors_[index]; }

  // Transitions the palette toward 'target' using linear interpolation.
  // 'amount' is a fixed-point value from 0 (this) to 1 (target).
  void Interpolate(const ColorTable& target, Q16 amount);

  // Shifts a range of colors for animation (classic demoscene effect).
  // 'start' and 'end' are the inclusive indices to shift.
  // 'offset' is the number of positions to shift right (can be negative).
  void Cycle(uint8_t start, uint8_t end, int offset);

  // Clears the palette to a specific color (defaults to black).
  void Clear(const RgbColor& color = {0, 0, 0});

  // Uploads the palette to the VGA hardware DAC.
  // Should typically be called during vertical retrace (after WaitVSync).
  void Apply() const;

  // Returns a raw pointer to the underlying color data (768 bytes).
  RgbColor* data() { return colors_; }
  const RgbColor* data() const { return colors_; }

 private:
  RgbColor* colors_;
};

}  // namespace subdemo
