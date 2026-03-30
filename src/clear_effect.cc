#include "include/clear_effect.h"

#include "include/canvas.h"
#include "include/system_context.h"

namespace subdemo {

ClearEffect::ClearEffect(uint8_t color_index) : color_index_(color_index) {}

bool ClearEffect::Setup(SystemContext* /*context*/) { return true; }

void ClearEffect::Update(uint32_t /*frame*/, uint32_t /*effect_frame*/,
                         Fixed<16> /*t*/) {}

void ClearEffect::Render(SystemContext* context) {
  if (context && context->canvas()) {
    context->canvas()->Clear(color_index_);
  }
}

void ClearEffect::Cleanup(SystemContext* /*context*/) {}

}  // namespace subdemo
