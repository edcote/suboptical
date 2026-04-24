#pragma once
#include "include/fixed.h"

namespace subdemo {

class SystemContext;

// Effect is the base class for all demoscene visual effects.
class Effect {
 public:
  virtual ~Effect() = default;

  // Initializes resources for the effect.
  // Returns false if resources fail to load.
  virtual bool Setup(SystemContext* context) = 0;

  // Configures the effect with a parameter string. Called when the effect
  // starts in the timeline.
  virtual void Configure(const char* /*config*/) {}

  // Updates the effect state.
  virtual void Update(uint32_t frame, uint32_t effect_frame, Fixed<16> t) = 0;

  // Draws the visual state into the system back buffer.
  virtual void Render(SystemContext* context) = 0;

  // Releases resources and cleans up the effect state.
  virtual void Cleanup(SystemContext* context) = 0;

  // Signals a state change or periodic event to the effect.
  virtual void Trigger(uint32_t /*trigger_id*/) {}

  // Returns true if the effect is active and should be updated and rendered.
  bool is_active() const { return is_active_; }

  // Sets whether the effect is active.
  void set_is_active(bool is_active) { is_active_ = is_active; }

 protected:
  bool is_active_ = false;
};

}  // namespace subdemo
