#include "include/video.h"

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>

#include "include/planar_canvas.h"

namespace video {

namespace {
constexpr int kVgaStatusRegisterPort = 0x3DA;
constexpr int kVgaVerticalRetraceBit = 0x08;
constexpr int kVideoInterrupt = 0x10;
constexpr int kSetVideoMode = 0x00;
constexpr int kGetVideoMode = 0x0F;

constexpr int kPaletteIndexWritePort = 0x3C8;
constexpr int kPaletteDataPort = 0x3C9;
}  // namespace

void SetVideoMode(int mode) {
  __dpmi_regs regs = {};
  regs.h.ah = kSetVideoMode;
  regs.h.al = mode;
  __dpmi_int(kVideoInterrupt, &regs);
}

int GetVideoMode() {
  __dpmi_regs regs = {};
  regs.h.ah = kGetVideoMode;
  __dpmi_int(kVideoInterrupt, &regs);
  return regs.h.al;
}

void WaitVSync() {
  while (inportb(kVgaStatusRegisterPort) & kVgaVerticalRetraceBit) {
  }
  while (!(inportb(kVgaStatusRegisterPort) & kVgaVerticalRetraceBit)) {
  }
}

void SetPalette(const uint8_t* palette) {
  outportb(kPaletteIndexWritePort, 0);
  for (int i = 0; i < 768; ++i) {
    outportb(kPaletteDataPort, palette[i]);
  }
}

namespace {
constexpr int kSequencerIndexPort = 0x3C4;
constexpr int kCrtcIndexPort = 0x3D4;
constexpr int kCrtcDataPort = 0x3D5;

constexpr uintptr_t kVgaMemoryAddress = 0xA0000;
}  // namespace

Video::Video()
    : vga_memory_(nullptr),
      front_buffer_(nullptr),
      back_buffer_(nullptr),
      active_page_(0) {}

Video::~Video() {}

bool Video::InitModeX() {
  SetVideoMode(kMode13h);

  // Unchain-4: Each of the 4 memory planes is now independently addressable.
  // Note: outportw(port, 0xVVii) writes index 0xii then value 0xVV.

  // Index 0x04, Sequencer Memory Mode: 0x06 (bit 3=0 for Unchain-4).
  outportw(kSequencerIndexPort, 0x0604);
  // Index 0x00, Reset: 0x01 (synchronous reset for clock stability).
  outportw(kSequencerIndexPort, 0x0100);
  // Index 0x17, CRTC Mode Control: 0xE3 (bit 6=1 for byte mode).
  outportw(kCrtcIndexPort, 0xE317);
  // Index 0x14, CRTC Underline Location: 0x00 (bit 6=0 for byte mode).
  outportw(kCrtcIndexPort, 0x0014);
  // Index 0x02, Sequencer Map Mask: 0x0F (enable all 4 planes for writing).
  outportw(kSequencerIndexPort, 0x0F02);

  vga_memory_ =
      reinterpret_cast<uint8_t*>(__djgpp_conventional_base + kVgaMemoryAddress);

  // Set Mode-X 256x200 centered timings @ 60 Hz.
  // 31.5 kHz / 525 lines = 60 Hz.
  // Horizontal (using 25MHz clock):
  // Index 0x00, H-Total: 0x5F (100 chars * 8 = 800 pixels).
  outportw(kCrtcIndexPort, 0x5F00);
  // Index 0x01, H-Display-End: 0x1F (31 chars * 8 = 256 pixels).
  outportw(kCrtcIndexPort, 0x1F01);
  // Index 0x02, H-Blank-Start: 0x20 (32 chars).
  outportw(kCrtcIndexPort, 0x2002);
  // Index 0x03, H-Blank-End: 0x2E (bit 7=1 for compatibility).
  outportw(kCrtcIndexPort, 0x2E03);
  // Index 0x04, H-Retrace-Start: 0x25 (37 chars).
  outportw(kCrtcIndexPort, 0x2504);
  // Index 0x05, H-Retrace-End: 0x38 (56 chars).
  outportw(kCrtcIndexPort, 0x3805);
  // Index 0x13, Offset / Logical Width: 0x20 (256px / 8dots / 4planes = 32
  // bytes).
  outportw(kCrtcIndexPort, 0x2013);

  // Vertical (targeting 525 total lines for 60 Hz):
  // VT = 524 (0x20C). Register 6 = 0x0C (bits 0-7), Overflow bit 0 = 0, bit 5
  // = 1. Index 0x06, V-Total: 0x0C (12).
  outportw(kCrtcIndexPort, 0x0C06);
  // Index 0x07, Overflow: 0x3E (VT8=0, VT9=1, V-Display-End8=1,
  // V-Display-End9=0).
  outportw(kCrtcIndexPort, 0x3E07);
  // Index 0x10, V-Retrace-Start: 0xEA (Bits 0-7 = 234).
  outportw(kCrtcIndexPort, 0xEA10);
  // Index 0x11, V-Retrace-End / Protection Off: 0x8C (Bits 0-3 = 12, Bit 7 =
  // 1).
  outportw(kCrtcIndexPort, 0x8C11);
  // Index 0x12, V-Display-End: 0xC7 (Bits 0-7 = 199).
  outportw(kCrtcIndexPort, 0xC712);
  // Index 0x15, V-Blank-Start: 0xE7 (Bits 0-7 = 231).
  outportw(kCrtcIndexPort, 0xE715);
  // Index 0x16, V-Blank-End: 0x06 (Bits 0-7 = 6).
  outportw(kCrtcIndexPort, 0x0616);

  active_page_ = 0;
  front_buffer_ = vga_memory_;
  back_buffer_ = vga_memory_ + kPageSize;

  canvas_ = std::make_unique<PlanarCanvas>(back_buffer_, 256, 200);

  return true;
}

void Video::SwapBuffers() {
  WaitVSync();

  active_page_ = (active_page_ == 0) ? 1 : 0;
  int inactive_page = (active_page_ == 0) ? 1 : 0;

  front_buffer_ = vga_memory_ + (active_page_ * kPageSize);
  back_buffer_ = vga_memory_ + (inactive_page * kPageSize);

  if (canvas_) {
    canvas_->SetBuffer(back_buffer_);
  }

  // Update VGA hardware to display the new front buffer.
  // The Start Address register expects an offset in words.
  const uint16_t offset = active_page_ * kPageSize;
  // Index 0x0C, Start Address High: High byte of word offset.
  outportw(kCrtcIndexPort, (uint16_t)((offset & 0xFF00) | 0x0C));
  // Index 0x0D, Start Address Low: Low byte of word offset.
  outportw(kCrtcIndexPort, (uint16_t)(((offset & 0x00FF) << 8) | 0x0D));
}

}  // namespace video
