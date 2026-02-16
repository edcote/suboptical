#pragma once

#include <dpmi.h>
#include <go32.h>

#include <cstdint>
#include <memory>

#include "include/video.h"

// SystemContext manages the 32-bit DOS lifecycle, including high-res timers,
// VGA graphics state, and resource cleanup.
class SystemContext {
 public:
  static std::unique_ptr<SystemContext> Create();

  SystemContext();
  ~SystemContext();

  SystemContext(const SystemContext&) = delete;
  SystemContext& operator=(const SystemContext&) = delete;

  // Initializes resources and switches to VGA Mode X.
  bool Init();

  // Starts the nanosecond timer and 30Hz frame counter.
  bool StartTimers();

  // Stops timers and restores original ISRs.
  void StopTimers();

  // Returns the time elapsed since StartTimers in nanoseconds.
  static uint64_t GetTimeNanoseconds();

  // Returns a pointer to the video manager instance.
  video::Video* video() const {
    // Returns a pointer instead of a reference so the caller can safely check
    // for nullptr if video_ is uninitialized or Init() failed.
    return video_.get();
  }

  // These must be accessible to the C ISR in src/timer.cc
  static _go32_dpmi_seginfo original_timer_isr_;
  static _go32_dpmi_seginfo timer_isr_;
  static volatile uint64_t timer_ticks_;
  static volatile int frame_accumulator_;

 private:
  std::unique_ptr<video::Video> video_;
  int original_video_mode_;
  bool timers_installed_ = false;
};
