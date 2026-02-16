#include <dos.h>
#include <dpmi.h>
#include <go32.h>

#include "include/logger.h"
#include "include/system_context.h"

namespace {
constexpr int kPitDivisor1000Hz = 1193;
constexpr int kPitDefaultDivisor = 0;
constexpr int kPic1CommandPort = 0x20;
constexpr int kPicEndOfInterrupt = 0x20;
constexpr int kTimerInterruptVector = 0x08;
constexpr int kPitControlPort = 0x43;
constexpr int kPitChannel0DataPort = 0x40;

void ProgramPit(int divisor) {
  outportb(kPitControlPort, 0x36);  // Mode 3
  outportb(kPitChannel0DataPort, divisor & 0xFF);
  outportb(kPitChannel0DataPort, (divisor >> 8) & 0xFF);
}

bool LockRegion(void* address, size_t length) {
  unsigned long base_address;
  if (__dpmi_get_segment_base_address(_go32_my_ds(), &base_address) != 0) {
    LogError("Failed to get segment base address");
    return false;
  }

  __dpmi_meminfo region;
  region.address = base_address + reinterpret_cast<uintptr_t>(address);
  region.size = length;

  if (__dpmi_lock_linear_region(&region) != 0) {
    LogError("Failed to lock memory region at 0x%lx, length %u", region.address,
             region.size);
    return false;
  }

  return true;
}
}  // namespace

_go32_dpmi_seginfo SystemContext::original_timer_isr_;
_go32_dpmi_seginfo SystemContext::timer_isr_;
volatile uint64_t SystemContext::timer_ticks_ = 0;
volatile int SystemContext::frame_accumulator_ = 0;

extern "C" {
void TimerISR() {
  SystemContext::timer_ticks_ = SystemContext::timer_ticks_ + 1;

  SystemContext::frame_accumulator_ = SystemContext::frame_accumulator_ + 30;
  if (SystemContext::frame_accumulator_ >= 1000) {
    SystemContext::frame_accumulator_ =
        SystemContext::frame_accumulator_ - 1000;
    // TODO: edc - Shouldn't this be a function?
    video::Video::frame_count_ = video::Video::frame_count_ + 1;
  }

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
                  sizeof(timer_ticks_)) ||
      !LockRegion(reinterpret_cast<void*>(
                      const_cast<uint32_t*>(&video::Video::frame_count_)),
                  sizeof(video::Video::frame_count_)) ||
      !LockRegion(
          reinterpret_cast<void*>(const_cast<int*>(&frame_accumulator_)),
          sizeof(frame_accumulator_))) {
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
  outportb(kPitControlPort, 0x00);  // latch PIT count
  const uint8_t low = inportb(kPitChannel0DataPort);
  const uint8_t high = inportb(kPitChannel0DataPort);
  count = (high << 8) | low;

  // Check if an interrupt is pending. If the counter just rolled over but
  // the ISR hasn't run yet, we manually increment our local tick copy.
  outportb(kPic1CommandPort, 0x0A);
  const uint8_t irr = inportb(kPic1CommandPort);
  if ((irr & 0x01) && count > (kPitDivisor1000Hz / 2)) {
    ticks++;
  }
  asm volatile("sti");  // re-enable interrupts

  const uint64_t elapsed_pit_ticks = kPitDivisor1000Hz - count;
  // `timer_ticks` is 1ms units. `elapsed_pit_ticks is ~838ns units.
  return (ticks * 1000000ULL) + (elapsed_pit_ticks * 838ULL);
}
