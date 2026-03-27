#pragma once
#include <cstdint>

class SystemContext;

namespace demo {

// Effect is the base class for all demoscene visual effects.
// It separates initialization, logic updates, and rendering.
class Effect {
 public:
  virtual ~Effect() = default;

  // Setup is called once during effect initialization.
  // Return false if resources fail to load.
  virtual bool Setup(SystemContext* context) = 0;

  // Update is called at a fixed 30 Hz rate for demo logic.
  // tick: The current global 30 Hz frame counter.
  virtual void Update(uint32_t tick) = 0;

  // Render is called to draw the visual state into the back buffer.
  virtual void Render(SystemContext* context) = 0;

  // Cleanup is called once when the effect ends.
  virtual void Cleanup(SystemContext* context) = 0;
};

}  // namespace demo
