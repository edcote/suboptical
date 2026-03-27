#include "include/color_cycle_effect.h"

#include "include/canvas.h"
#include "include/system_context.h"

namespace demo {

ColorCycleEffect::ColorCycleEffect() : current_color_index_(0) {}

bool ColorCycleEffect::Setup(SystemContext* /*context*/) {
  current_color_index_ = 0;
  return true;
}

void ColorCycleEffect::Update(uint32_t tick) {
  current_color_index_ = static_cast<uint8_t>(tick % 256);
}

void ColorCycleEffect::Render(SystemContext* context) {
  context->canvas()->Clear(current_color_index_);
}

void ColorCycleEffect::Cleanup(SystemContext* /*context*/) {}

}  // namespace demo
