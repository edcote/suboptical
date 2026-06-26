#include "include/hello_world_effect.h"

#include "include/canvas.h"
#include "include/logger.h"
#include "include/resource_manager.h"
#include "include/system_context.h"

namespace subdemo {

bool HelloWorldEffect::Setup(SystemContext* context) {
  const uint8_t* font_data =
      context->resource_manager()->GetResource("djvumono.bin");
  if (!font_data) {
    LogFatal("Failed to load font resource: djvumono.bin");
  }

  // The Font constructor will perform internal validation and LogFatal if the
  // SBDF magic is missing or the character count is incorrect.
  font_ = std::make_unique<Font>(font_data);
  return true;
}

void HelloWorldEffect::Update(uint32_t /*frame*/, uint32_t /*effect_frame*/,
                              Fixed<16> /*t*/) {}

void HelloWorldEffect::Render(SystemContext* context) {
  Canvas* canvas = context->canvas();
  if (canvas && font_) {
    // Clear screen to black and draw yellow text at the center.
    // We assume a 256x200 canvas (Mode-X width varies).
    canvas->Clear(0);
    canvas->DrawText(80, 90, "HELLO WORLD!", 14, *font_);
  }
}

void HelloWorldEffect::Cleanup(SystemContext* /*context*/) {}

}  // namespace subdemo
