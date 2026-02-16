#include <cstdint>
#include <stdio.h>

#include <dos.h>

#include "include/build_info.h"
#include "include/input.h"
#include "include/logger.h"
#include "include/system_context.h"
#include "include/video.h"

int main(void) {
  LogInfo("supademo[build %d]", BUILD_NUMBER);
  const auto context = SystemContext::Create();
  if (!context) {
    LogError("Failed to create system context.");
    return 1;
  }

  if (!context->StartTimers()) {
    LogError("Failed to start timers.");
    return 1;
  }

  const uint64_t start_time = SystemContext::GetTimeNanoseconds();
  uint64_t last_time = start_time;
  const uint64_t kWaitNs = 33333333ULL; // ~30 FPS
  uint32_t frames_drawn = 0;

  while (true) {
    if (input::IsEscapePressed()) {
      break;
    }

    // Caps frame rate at 30 FPS. If the frame finishes early, we wait until
    // 33.3ms have passed; if it's running late, we proceed immediately.
    const uint64_t current_time = SystemContext::GetTimeNanoseconds();
    if (current_time - last_time < kWaitNs) {
      continue;
    }
    last_time = current_time;

    // TODO: edc - Placeholder for graphics engine.
    const uint8_t color = static_cast<uint8_t>(video::Video::GetFrameCount() % 256);
    context->video()->ClearBackBuffer(color);

    // Wait for vertical sync then flip buffers.
    context->video()->SwapBuffers();
    frames_drawn++;
  }

  const uint64_t end_time = SystemContext::GetTimeNanoseconds();
  const uint64_t elapsed_ns = end_time - start_time;
  const uint64_t elapsed_seconds = elapsed_ns / 1000000000ULL;

  LogInfo("Frames drawn: %lu", static_cast<unsigned long>(frames_drawn));
  LogInfo("Seconds elapsed: %lu", static_cast<unsigned long>(elapsed_seconds));
  return 0;
}
