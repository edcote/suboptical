#pragma once

#include <cstdint>
#include <memory>

#include "include/canvas.h"

namespace video {

constexpr int kMode13h = 0x13;
constexpr int kMode03h = 0x03;
constexpr int kPageSize = 256 * 200 / 4;

// Gets current video mode via BIOS.
int GetVideoMode();

// Sets video mode via BIOS.
void SetVideoMode(int mode);

// Waits for vertical retrace.
void WaitVSync();

// Sets 768-byte RGB palette (0-63).
void SetPalette(const uint8_t* palette);

class Video {
 public:
  Video();
  ~Video();

  // Initializes VGA Mode X (256x200, 256 colors, 60 Hz).
  bool InitModeX();

  // Returns the canvas instance for current software drawing.
  Canvas* canvas() const { return canvas_.get(); }

  // Swaps the front and back buffers (page flipping).
  void SwapBuffers();

  uint8_t* back_buffer() const { return back_buffer_; }
  uint8_t* front_buffer() const { return front_buffer_; }

 private:
  std::unique_ptr<Canvas> canvas_;
  uint8_t* vga_memory_;
  uint8_t* front_buffer_;
  uint8_t* back_buffer_;
  int active_page_;
};

}  // namespace video
