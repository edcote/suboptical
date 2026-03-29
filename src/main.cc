#include <dos.h>
#include <stdio.h>

#include <cstdint>

#include "include/build_info.h"
#include "include/bundle_reader.h"
#include "include/color_cycle_effect.h"
#include "include/effect.h"
#include "include/input.h"
#include "include/logger.h"
#include "include/system_context.h"
#include "include/video.h"

int main(void) {
  using namespace subdemo;
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

    // Effect lifecycle management.
    auto current_effect = std::make_unique<ColorCycleEffect>();
    if (!current_effect->Setup(context.get())) {
      LogError("Failed to setup effect.");
      return 1;
    }

    start_time = SystemContext::GetTimeNanoseconds();
    const uint64_t kFrameDurationNs = 1000000000ULL / 30;
    uint64_t next_frame_time = start_time;
    uint32_t demo_tick = 0;

    while (!IsEscapePressed()) {
      bool need_render = false;
      const uint64_t current_time = SystemContext::GetTimeNanoseconds();

      // Fixed-step logic update (30 Hz). If lagging, update multiple times
      // and skip rendering (frame skipping) to maintain real-time speed.
      while (current_time >= next_frame_time) {
        // demo::UpdateState();  // Future logic update
        // music::Update();      // Future music player logic

        current_effect->Update(demo_tick++);

        next_frame_time += kFrameDurationNs;
        need_render = true;

        // Prevent the 'spiral of death' by capping the catch-up loop if the
        // machine is too slow to ever hit the target frame rate.
        if (current_time > next_frame_time + (kFrameDurationNs * 10)) {
          next_frame_time = current_time;
          break;
        }
      }

      if (need_render) {
        current_effect->Render(context.get());
        context->video()->SwapBuffers();
        frames_drawn++;
      }
    }

    current_effect->Cleanup(context.get());
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
  while (IsEscapePressed()) {
  }
  while (!IsEscapePressed()) {
  }

  return 0;
}
