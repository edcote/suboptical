#include <dos.h>
#include <dpmi.h>
#include <go32.h>

#include "include/logger.h"
#include "include/system_context.h"

namespace subdemo {
namespace {
constexpr int kPitDivisor1000Hz = 1193;
constexpr int kPitDefaultDivisor = 0;
constexpr int kPic1CommandPort = 0x20;
constexpr int kPicEndOfInterrupt = 0x20;
constexpr int kTimerInterruptVector = 0x08;
constexpr int kPitControlPort = 0x43;
constexpr int kPitChannel0DataPort = 0x40;

// Reprograms the Programmable Interval Timer (PIT) Channel 0.
void ProgramPit(int divisor) {
  // PIT Control Word
  // Value: 0x36 (Binary: 00110110)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  0  | BCD/Binary        |   0   | 16-bit binary counter
  // 1-3 | Operating Mode    |  011  | Mode 3: Square wave generator
  // 4-5 | Access Mode       |  11   | Access lobyte/hibyte
  // 6-7 | Select Channel    |  00   | Channel 0
  outportb(kPitControlPort, 0x36);

  // Index 0x40: PIT Channel 0 Data Port (Low Byte)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Divisor LSB       |  VAR  | Lower 8 bits of the 16-bit divisor
  outportb(kPitChannel0DataPort, divisor & 0xFF);

  // Index 0x40: PIT Channel 0 Data Port (High Byte)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Divisor MSB       |  VAR  | Upper 8 bits of the 16-bit divisor
  outportb(kPitChannel0DataPort, (divisor >> 8) & 0xFF);
}

// Prevent memory from being paged out to disk as ISRs must stay in RAM.
bool LockRegion(void* address, size_t length) {
  unsigned long base_address;
  if (__dpmi_get_segment_base_address(_go32_my_ds(), &base_address) != 0) {
    LogError("Failed to get segment base address.");
    return false;
  }

  __dpmi_meminfo region;
  region.address = base_address + reinterpret_cast<uintptr_t>(address);
  region.size = length;

  if (__dpmi_lock_linear_region(&region) != 0) {
    LogError("Failed to lock memory region at 0x%lx, length %u.",
             region.address, region.size);
    return false;
  }

  return true;
}
}  // namespace

_go32_dpmi_seginfo SystemContext::original_timer_isr_;
_go32_dpmi_seginfo SystemContext::timer_isr_;
volatile uint64_t SystemContext::timer_ticks_ = 0;

extern "C" {
// The interrupt service routine (ISR) called by the hardware at 1000Hz.
void TimerISR() {
  SystemContext::timer_ticks_ = SystemContext::timer_ticks_ + 1;

  // PIC End-Of-Interrupt (EOI) Command
  // Value: 0x20 (Binary: 00100000)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  //  5  | EOI Command       |   1   | 1 = Non-specific End-of-Interrupt
  outportb(kPic1CommandPort, kPicEndOfInterrupt);

  asm volatile(
      ".globl _TimerISREnd \n"
      "_TimerISREnd:");
}
}

extern "C" char TimerISREnd;

bool SystemContext::StartTimers() {
  if (timers_installed_) return true;

  const auto isr_len = reinterpret_cast<uintptr_t>(&TimerISREnd) -
                       reinterpret_cast<uintptr_t>(TimerISR);

  if (isr_len == 0 || isr_len > 4096) {
    LogError("Invalid calculated ISR size. Memory locking may fail.");
    return false;
  }

  if (!LockRegion(reinterpret_cast<void*>(TimerISR), isr_len) ||
      !LockRegion(reinterpret_cast<void*>(&original_timer_isr_),
                  sizeof(original_timer_isr_)) ||
      !LockRegion(reinterpret_cast<void*>(const_cast<uint64_t*>(&timer_ticks_)),
                  sizeof(timer_ticks_))) {
    return false;
  }

  ProgramPit(kPitDivisor1000Hz);

  _go32_dpmi_get_protected_mode_interrupt_vector(kTimerInterruptVector,
                                                 &original_timer_isr_);

  timer_isr_.pm_offset = reinterpret_cast<uintptr_t>(TimerISR);
  timer_isr_.pm_selector = _go32_my_cs();

  if (_go32_dpmi_allocate_iret_wrapper(&timer_isr_) != 0) {
    LogError("Failed to allocate DPMI IRET wrapper.");
    ProgramPit(kPitDefaultDivisor);
    return false;
  }

  _go32_dpmi_set_protected_mode_interrupt_vector(kTimerInterruptVector,
                                                 &timer_isr_);

  timers_installed_ = true;
  return true;
}

void SystemContext::StopTimers() {
  if (!timers_installed_) return;

  _go32_dpmi_set_protected_mode_interrupt_vector(kTimerInterruptVector,
                                                 &original_timer_isr_);
  _go32_dpmi_free_iret_wrapper(&timer_isr_);
  ProgramPit(kPitDefaultDivisor);
  timers_installed_ = false;
}

uint64_t SystemContext::GetTimeNanoseconds() {
  uint64_t ticks;
  uint16_t count;

  // Disable interrupts (cli) to ensure that the 1000Hz timer_ticks_
  // and the PIT hardware counter are read as a single atomic snapshot.
  // This prevents the ISR from firing and incrementing ticks between reads.
  asm volatile("cli");
  ticks = timer_ticks_;
  // PIT Latch Command
  // Value: 0x00 (Binary: 00000000)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 4-5 | Latch Count       |  00   | Counter Latch Command
  // 6-7 | Select Channel    |  00   | Channel 0
  outportb(kPitControlPort, 0x00);
  const uint8_t low = inportb(kPitChannel0DataPort);
  const uint8_t high = inportb(kPitChannel0DataPort);
  count = (high << 8) | low;

  // PIC Read Request Register (IRR) Command
  // Value: 0x0A (Binary: 00001010)
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-1 | Select Register   |  10   | 10 = IRR (Request), 11 = ISR (Service)
  //  3  | Read Register     |   1   | 1 = Perform Read
  outportb(kPic1CommandPort, 0x0A);
  const uint8_t irr = inportb(kPic1CommandPort);
  if ((irr & 0x01) && count > (kPitDivisor1000Hz / 2)) {
    ticks++;
  }
  asm volatile("sti");  // re-enable interrupts

  const uint64_t elapsed_pit_ticks = kPitDivisor1000Hz - count;
  // `timer_ticks` is 1ms units. `elapsed_pit_ticks` is ~838ns units.
  return (ticks * 1000000ULL) + (elapsed_pit_ticks * 838ULL);
}

}  // namespace subdemo
