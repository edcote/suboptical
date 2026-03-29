#pragma once
#include <cstdint>

#include "include/effect.h"

namespace subdemo {

// Simple test effect that cycles the screen's background color.
class ColorCycleEffect : public Effect {
 public:
  ColorCycleEffect();
  ~ColorCycleEffect() override = default;

  bool Setup(SystemContext* context) override;
  void Update(uint32_t tick) override;
  void Render(SystemContext* context) override;
  void Cleanup(SystemContext* context) override;

 private:
  uint8_t current_color_index_;
};

}  // namespace subdemo
