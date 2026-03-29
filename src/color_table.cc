#include "include/color_table.h"

#include <dos.h>

#include <algorithm>
#include <cstring>

#include "include/fixed_math.h"
#include "include/video.h"

namespace subdemo {

void ColorTable::Interpolate(const ColorTable& target, Q16 amount) {
  // Clamp amount to [0, 1] range.
  if (amount <= Q16(0)) return;
  if (amount >= Q16::One()) {
    memcpy(colors_, target.data(), 256 * sizeof(RgbColor));
    return;
  }

  for (int i = 0; i < 256; ++i) {
    const auto& c1 = colors_[i];
    const auto& c2 = target.data()[i];

    colors_[i].r = static_cast<uint8_t>(
        FixedMath<16>::Lerp(Q16(c1.r), Q16(c2.r), amount).ToInt());
    colors_[i].g = static_cast<uint8_t>(
        FixedMath<16>::Lerp(Q16(c1.g), Q16(c2.g), amount).ToInt());
    colors_[i].b = static_cast<uint8_t>(
        FixedMath<16>::Lerp(Q16(c1.b), Q16(c2.b), amount).ToInt());
  }
}

void ColorTable::Cycle(uint8_t start, uint8_t end, int offset) {
  if (start >= end) return;

  const int size = end - start + 1;
  // Normalize offset to range [0, size-1].
  offset %= size;
  if (offset < 0) offset += size;
  if (offset == 0) return;

  // We need a small temporary buffer for cycling.
  // 256 entries is only 768 bytes, safe for stack in DJGPP.
  RgbColor temp[256];
  memcpy(temp, &colors_[start], size * sizeof(RgbColor));

  for (int i = 0; i < size; ++i) {
    int target_idx = (i + offset) % size;
    colors_[start + target_idx] = temp[i];
  }
}

void ColorTable::Clear(const RgbColor& color) {
  for (int i = 0; i < 256; ++i) {
    colors_[i] = color;
  }
}

void ColorTable::Apply() const {
  // Forward to the centralized SetPalette function to avoid duplication.
  SetPalette(reinterpret_cast<const uint8_t*>(colors_));
}

}  // namespace subdemo
