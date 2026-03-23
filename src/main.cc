#include <dos.h>
#include <stdio.h>

#include <cstdint>

#include "include/build_info.h"
#include "include/bundle_reader.h"
#include "include/input.h"
#include "include/logger.h"
#include "include/system_context.h"
#include "include/video.h"

int main(void) {
  LogInfo("subdemo3[build %d]", BUILD_NUMBER);

  uint32_t frames_drawn = 0;
  uint64_t start_time = 0;
  uint64_t end_time = 0;

  {
    const auto context = SystemContext::Create();
    if (!context) {
      LogError("Failed to create system context.");
      return 1;
    }

    if (!context->StartTimers()) {
      LogError("Failed to start timers.");
      return 1;
    }

    start_time = SystemContext::GetTimeNanoseconds();
    uint64_t last_time = start_time;
    // Cap at ~30 FPS. If elapsed time is less than 33.3ms, we skip drawing.
    const uint64_t kWaitNs = 33333333ULL;

    while (true) {
      if (input::IsEscapePressed()) {
        break;
      }

      const uint64_t current_time = SystemContext::GetTimeNanoseconds();
      if (current_time - last_time < kWaitNs) {
        continue;
      }
      last_time = current_time;

      const uint8_t color =
          static_cast<uint8_t>(video::Video::GetFrameCount() % 256);
      context->video()->ClearBackBuffer(color);

      // SwapBuffers() calls WaitVSync(), which blocks until the next retrace
      // (~70Hz). If the 30 FPS cap and the 70Hz retrace are out of sync, the
      // effective frame rate will be limited by the slower of the two or a
      // multiple.
      context->video()->SwapBuffers();
      frames_drawn++;
    }
    end_time = SystemContext::GetTimeNanoseconds();
  }

  const uint64_t elapsed_ns = end_time - start_time;
  const uint64_t elapsed_ms = elapsed_ns / 1000000ULL;

  // Calculate FPS using integer math: (frames * 1000) / elapsed_ms.
  uint32_t fps = 0;
  if (elapsed_ms > 0) {
    fps = (frames_drawn * 1000U) / static_cast<uint32_t>(elapsed_ms);
  }

  LogInfo("Frames drawn: %u", frames_drawn);
  LogInfo("Seconds elapsed: %llu", elapsed_ms / 1000ULL);
  LogInfo("Average FPS: %u", fps);

  LogInfo("Press ESC to exit...");
  while (input::IsEscapePressed()) {
  }
  while (!input::IsEscapePressed()) {
  }

  return 0;
}
