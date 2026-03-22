#include "include/video.h"

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>

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

volatile uint32_t Video::frame_count_ = 0;

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

  // Set Mode-X 256x200 horizontal timing tweaks.
  // Index 0x01, H-Display-End: 0x1F (31 chars * 8 dots = 256 pixels).
  outportw(kCrtcIndexPort, 0x1F01);
  // Index 0x02, H-Blank-Start: 0x20 (32 chars).
  outportw(kCrtcIndexPort, 0x2002);
  // Index 0x03, H-Blank-End: 0x2E (bit 7=1 for compatibility).
  outportw(kCrtcIndexPort, 0x2E03);
  // Index 0x04, H-Retrace-Start: 0x22 (34 chars).
  outportw(kCrtcIndexPort, 0x2204);
  // Index 0x05, H-Retrace-End: 0x38 (56 chars).
  outportw(kCrtcIndexPort, 0x3805);
  // Index 0x13, Offset / Logical Width: 0x20 (256px / 8dots / 4planes = 32).
  outportw(kCrtcIndexPort, 0x2013);

  active_page_ = 0;
  front_buffer_ = vga_memory_;
  back_buffer_ = vga_memory_ + kPageSize;

  return true;
}

void Video::SwapBuffers() {
  WaitVSync();

  active_page_ = (active_page_ == 0) ? 1 : 0;
  int inactive_page = (active_page_ == 0) ? 1 : 0;

  front_buffer_ = vga_memory_ + (active_page_ * kPageSize);
  back_buffer_ = vga_memory_ + (inactive_page * kPageSize);

  // Update VGA hardware to display the new front buffer.
  // The Start Address register expects an offset in words.
  const uint16_t offset = active_page_ * kPageSize;
  // Index 0x0C, Start Address High: High byte of word offset.
  outportw(kCrtcIndexPort, (uint16_t)((offset & 0xFF00) | 0x0C));
  // Index 0x0D, Start Address Low: Low byte of word offset.
  outportw(kCrtcIndexPort, (uint16_t)(((offset & 0x00FF) << 8) | 0x0D));
}

void Video::ClearBackBuffer(uint8_t color) {
  // Index 0x02, Sequencer Map Mask: 0x0F (enable all 4 planes for clearing).
  outportw(kSequencerIndexPort, 0x0F02);

  const uint32_t color32 = color | (color << 8) | (color << 16) | (color << 24);
  int dwords_to_clear = kPageSize / 4;
  uint8_t* dest = back_buffer_;

  asm volatile(
      "cld\n\t"
      "rep stosl\n\t"
      : "+D"(dest), "+c"(dwords_to_clear)
      : "a"(color32)
      : "memory");
}

}  // namespace video
