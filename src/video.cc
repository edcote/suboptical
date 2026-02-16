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
  while (inportb(kVgaStatusRegisterPort) & kVgaVerticalRetraceBit) {}
  while (!(inportb(kVgaStatusRegisterPort) & kVgaVerticalRetraceBit)) {}
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

Video::Video() : vga_memory_(nullptr), front_buffer_(nullptr), back_buffer_(nullptr), active_page_(0) {}

Video::~Video() {}

bool Video::InitModeX() {
  SetVideoMode(kMode13h);

  // Initialize VGA Mode X (320x240) by modifying sequencer and CRTC registers.
  outportw(kSequencerIndexPort, 0x0604);
  outportw(kSequencerIndexPort, 0x0100);
  outportw(kCrtcIndexPort, 0xE317);
  outportw(kCrtcIndexPort, 0x0014);
  outportw(kSequencerIndexPort, 0x0F02);

  vga_memory_ = reinterpret_cast<uint8_t*>(__djgpp_conventional_base + kVgaMemoryAddress);

  outportb(kCrtcIndexPort, 0x11);
  const uint8_t crtc11 = inportb(kCrtcDataPort);
  outportw(kCrtcIndexPort, (uint16_t)((crtc11 & 0x7F) << 8) | 0x11);

  outportw(kCrtcIndexPort, 0x0D06);
  outportw(kCrtcIndexPort, 0x3E07);
  outportw(kCrtcIndexPort, 0x4109);
  outportw(kCrtcIndexPort, 0xEA10);
  outportw(kCrtcIndexPort, 0xAC11);
  outportw(kCrtcIndexPort, 0xDF12);
  outportw(kCrtcIndexPort, 0xE715);
  outportw(kCrtcIndexPort, 0x0616);

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
  const uint16_t offset = active_page_ * kPageSize;
  outportw(kCrtcIndexPort, (uint16_t)((offset & 0xFF00) | 0x0C));
  outportw(kCrtcIndexPort, (uint16_t)(((offset & 0x00FF) << 8) | 0x0D));
}

void Video::ClearBackBuffer(uint8_t color) {
  outportw(kSequencerIndexPort, 0x0F02);

  const uint32_t color32 = color | (color << 8) | (color << 16) | (color << 24);
  int dwords_to_clear = kPageSize / 4;

  asm volatile(
      "cld\n\t"
      "rep stosl\n\t"
      : "+D"(back_buffer_), "+c"(dwords_to_clear)
      : "a"(color32)
      : "memory"
  );
}

}  // namespace video
