#pragma once

#include <memory>

#include "include/effect.h"
#include "include/font.h"

namespace subdemo {

class HelloWorldEffect : public Effect {
 public:
  HelloWorldEffect() = default;
  ~HelloWorldEffect() override = default;

  bool Setup(SystemContext* context) override;
  void Update(uint32_t frame, uint32_t effect_frame, Fixed<16> t) override;
  void Render(SystemContext* context) override;
  void Cleanup(SystemContext* context) override;

 private:
  std::unique_ptr<Font> font_;
};

}  // namespace subdemo
