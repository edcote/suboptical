#pragma once

#include <cstdint>

#include "include/effect.h"

namespace subdemo {

// A simple effect that clears the system canvas to a solid color.
class ClearEffect : public Effect {
 public:
  explicit ClearEffect(uint8_t color_index = 0);
  ~ClearEffect() override = default;

  bool Setup(SystemContext* context) override;
  void Update(uint32_t frame, uint32_t effect_frame, Fixed<16> t) override;
  void Render(SystemContext* context) override;
  void Cleanup(SystemContext* context) override;

  // Set the color index to clear the canvas with.
  void SetColor(uint8_t color_index) { color_index_ = color_index; }

 private:
  uint8_t color_index_;
};

}  // namespace subdemo
