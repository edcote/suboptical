#include "include/planar_canvas.h"

#include <dos.h>

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

}  // namespace subdemo
