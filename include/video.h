#pragma once

#include <cstdint>

namespace video {

constexpr int kMode13h = 0x13;
constexpr int kMode03h = 0x03;
constexpr int kPageSize = 320 * 240 / 4;

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

  // Initializes VGA Mode X (320x240, 256 colors, unchained).
  bool InitModeX();

  // Clears the active backbuffer with the given color.
  void ClearBackBuffer(uint8_t color);

  // Returns the global frame counter, incremented at 30 Hz.
  static uint32_t GetFrameCount() { return frame_count_; }

  // Swaps the front and back buffers (page flipping).
  void SwapBuffers();

  uint8_t* back_buffer() const { return back_buffer_; }
  uint8_t* front_buffer() const { return front_buffer_; }

  static volatile uint32_t frame_count_;

 private:
  uint8_t* vga_memory_;
  uint8_t* front_buffer_;
  uint8_t* back_buffer_;
  int active_page_;
};

}  // namespace video
