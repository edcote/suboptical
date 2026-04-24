#include "include/video.h"

#include <dos.h>
#include <dpmi.h>
#include <go32.h>
#include <sys/nearptr.h>

#include "include/planar_canvas.h"

namespace subdemo {

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
  // Index 0x3DA: VGA Status Register
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  3  | Vertical Retrace  |  VAR  | 1 = In Vertical Retrace, 0 = In Display
  while (inportb(kVgaStatusRegisterPort) & kVgaVerticalRetraceBit) {
  }
  while (!(inportb(kVgaStatusRegisterPort) & kVgaVerticalRetraceBit)) {
  }
}

void SetPalette(const uint8_t* palette) {
  // Index 0x3C8: VGA Palette Index Write Port
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Palette Index     |   0   | Start writing from color index 0
  outportb(kPaletteIndexWritePort, 0);

  for (int i = 0; i < 768; ++i) {
    // Index 0x3C9: VGA Palette Data Port
    // Bit | Field Name        | Value | Description
    // ----|-------------------|-------|-------------------------------------------
    // 0-5 | Color Component   |  VAR  | 6-bit R, G, or B component (0-63)
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

  // Index 0x04: Sequencer Memory Mode
  // Value: 0x06 (Binary: 00000110)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  1  | Extended Memory   |   1   | Enable access to memory > 64KB
  //  2  | Odd/Even Disable  |   1   | Disable odd/even memory addressing
  //  3  | Chain 4 Disable   |   0   | Disable Chain-4 (enables Unchain-4 mode)
  outportw(kSequencerIndexPort, 0x06'04);

  // Index 0x00: Sequencer Reset
  // Value: 0x01 (Binary: 00000001)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  0  | Async Reset       |   1   | 1 = Run, 0 = Reset
  //  1  | Sync Reset        |   0   | 0 = Reset
  outportw(kSequencerIndexPort, 0x01'00);

  // Index 0x17: CRTC Mode Control
  // Value: 0xE3 (Binary: 11100011)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  0  | Compatibility     |   1   | Standard CGA/VGA compatibility
  //  1  | Select Row Scan   |   1   | Double scan frequency (for 200/400 lines)
  //  5  | Address Wrap      |   1   | MA13/MA15 address wrap control
  //  6  | Word/Byte Mode    |   1   | 1 = Byte mode, 0 = Word mode
  //  7  | Hardware Reset    |   1   | 1 = No Reset, 0 = Force Reset
  outportw(kCrtcIndexPort, 0xE3'17);

  // Index 0x14: CRTC Underline Location
  // Value: 0x00 (Binary: 00000000)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-4 | Underline Loc     |  0x00 | Scan line at which underline occurs
  //  5  | Count by 4        |   0   | Divide-by-4 mode for memory addresses
  //  6  | Doubleword Mode   |   0   | 0 = Byte/Word mode, 1 = Doubleword mode
  outportw(kCrtcIndexPort, 0x00'14);

  // Index 0x02: Sequencer Map Mask
  // Value: 0x0F (Binary: 00001111)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-3 | Plane Enable      |  0x0F | Enable all 4 memory planes for writing
  outportw(kSequencerIndexPort, 0x0F'02);

  vga_memory_ =
      reinterpret_cast<uint8_t*>(__djgpp_conventional_base + kVgaMemoryAddress);

  // Set Mode-X 256x200 centered timings @ 60 Hz.
  // 31.5 kHz / 525 lines = 60 Hz.
  // Horizontal Timings (using 25MHz clock):

  // Index 0x00: H-Total
  // Value: 0x5F (Binary: 01011111)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Total Chars       |  0x5F | 95 + 5 = 100 character clocks (800
  // pixels)
  outportw(kCrtcIndexPort, 0x5F'00);

  // Index 0x01: H-Display-End
  // Value: 0x1F (Binary: 00011111)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Displayed Chars   |  0x1F | 31 + 1 = 32 character clocks (256 pixels)
  outportw(kCrtcIndexPort, 0x1F'01);

  // Index 0x02: H-Blank-Start
  // Value: 0x20 (Binary: 00100000)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Blanking Start    |  0x20 | Start of horizontal blanking interval
  outportw(kCrtcIndexPort, 0x20'02);

  // Index 0x03: H-Blank-End
  // Value: 0x2E (Binary: 00101110)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-4 | Blanking End      |  0x0E | Bits 0-4 of end position
  // 5-6 | Display Skew      |   1   | Character clock skew
  //  7  | Compatibility     |   0   | Standard/Compatibility mode bit
  outportw(kCrtcIndexPort, 0x2E'03);

  // Index 0x04: H-Retrace-Start
  // Value: 0x25 (Binary: 00100101)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Retrace Start     |  0x25 | Character clock at which retrace begins
  outportw(kCrtcIndexPort, 0x25'04);

  // Index 0x05: H-Retrace-End
  // Value: 0x38 (Binary: 00111000)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-4 | Retrace End       |  0x18 | Bits 0-4 of end position
  // 5-6 | Retrace Delay     |   1   | Horizontal retrace delay
  //  7  | EOP Status        |   0   | End-Of-Pulsing status bit
  outportw(kCrtcIndexPort, 0x38'05);

  // Index 0x13: Offset / Logical Width
  // Value: 0x20 (Binary: 00100000)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Offset            |  0x20 | 256px / 8 / 4 planes = 32 words per line
  outportw(kCrtcIndexPort, 0x20'13);

  // Vertical Timings (targeting 525 total lines for 60 Hz):

  // Index 0x06: Vertical Total
  // Value: 0x0C (Binary: 00001100)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | V-Total Bits 0-7  |  0x0C | Lower bits of total vertical scan lines
  outportw(kCrtcIndexPort, 0x0C'06);

  // Index 0x07: Overflow
  // Value: 0x3E (Binary: 00111110)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  0  | Vertical Total 8  |   0   | Bit 8 of Vertical Total
  //  1  | V-Display End 8   |   1   | Bit 8 of Vertical Display End
  //  2  | V-Retrace Start 8 |   1   | Bit 8 of Vertical Retrace Start
  //  3  | V-Blank Start 8   |   1   | Bit 8 of Vertical Blank Start
  //  4  | Line Compare 8    |   1   | Bit 8 of Line Compare
  //  5  | Vertical Total 9  |   1   | Bit 9 of Vertical Total
  //  6  | V-Display End 9   |   0   | Bit 9 of Vertical Display End
  //  7  | V-Retrace Start 9 |   0   | Bit 9 of Vertical Retrace Start
  outportw(kCrtcIndexPort, 0x3E'07);

  // Index 0x10: Vertical Retrace Start
  // Value: 0xEA (Binary: 11101010)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | V-Retrace Start   |  0xEA | Lower bits of line at which retrace
  // starts
  outportw(kCrtcIndexPort, 0xEA'10);

  // Index 0x11: Vertical Retrace End
  // Value: 0x8C (Binary: 10001100)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-3 | V-Retrace End     |  0x0C | Lower 4 bits of line at which retrace
  // ends
  //  5  | IRQ Clear         |   0   | 0 = Clear CRT vertical interrupt
  //  7  | Protect On/Off    |   1   | 1 = Write Protect Off for Indices
  //  0x00-0x07
  outportw(kCrtcIndexPort, 0x8C'11);

  // Index 0x12: Vertical Display End
  // Value: 0xC7 (Binary: 11000111)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | V-Display End     |  0xC7 | Lower bits of last displayed scan line
  outportw(kCrtcIndexPort, 0xC7'12);

  // Index 0x15: Vertical Blank Start
  // Value: 0xE7 (Binary: 11100111)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | V-Blank Start     |  0xE7 | Scan line at which blanking begins
  outportw(kCrtcIndexPort, 0xE7'15);

  // Index 0x16: Vertical Blank End
  // Value: 0x06 (Binary: 00000110)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | V-Blank End       |  0x06 | Scan line at which blanking ends
  outportw(kCrtcIndexPort, 0x06'16);

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
  const uint16_t offset = active_page_ * kPageSize;

  // Index 0x0C: Start Address High
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Start Address H   |  VAR  | Upper bits of page offset in words
  outportw(kCrtcIndexPort, (uint16_t)((offset & 0xFF00) | 0x0C));

  // Index 0x0D: Start Address Low
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Start Address L   |  VAR  | Lower bits of page offset in words
  outportw(kCrtcIndexPort, (uint16_t)(((offset & 0x00FF) << 8) | 0x0D));
}

}  // namespace subdemo
