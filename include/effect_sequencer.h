#pragma once

#include <cstddef>
#include <cstdint>

#include "etl/vector.h"
#include "include/effect.h"

namespace subdemo {

class SystemContext;

enum class BlendMode { kNormal, kAdditive, kXOR };

// A single segment of a demoscene effect in the timeline.
struct TimelineEntry {
  uint32_t start_frame;
  uint32_t end_frame;
  int16_t priority;
  Effect* effect;
  BlendMode blend;
  Fixed<16> t_step = Fixed<16>(0);
};

// A one-shot signal sent to all effects at a specific tick.
struct TriggerEvent {
  uint32_t frame;
  uint8_t trigger_id;
};

// EffectSequencer manages the timeline and handles effect execution,
// blending, and triggers based on a global tick counter.
class EffectSequencer {
 public:
  static constexpr std::size_t kMaxEffects = 32;
  static constexpr std::size_t kMaxTimelineEntries = 64;
  static constexpr std::size_t kMaxTriggers = 128;

  EffectSequencer() = default;
  ~EffectSequencer() = default;

  // Registers an effect instance with a unique name for timeline lookup.
  void RegisterEffect(const char* name, Effect* effect);

  // Prototype for loading a timeline from a text file. Returns false on error.
  bool LoadTimeline(const char* filename);

  // Manually adds an entry to the timeline for static orchestration.
  void AddTimelineEntry(const TimelineEntry& entry);

  // Updates the timeline state and triggers registered effects.
  void Update(uint32_t frame);

  // Renders all currently active timeline entries in priority order.
  void Render(SystemContext* context);

  // Forwards a trigger signal to all registered effects.
  void Trigger(uint32_t trigger_id);

 private:
  struct NamedEffect {
    const char* name;
    Effect* effect;
  };

  etl::vector<NamedEffect, kMaxEffects> effects_;
  etl::vector<TimelineEntry, kMaxTimelineEntries> timeline_;
  etl::vector<TriggerEvent, kMaxTriggers> triggers_;
  etl::vector<TimelineEntry*, kMaxTimelineEntries> active_entries_;
  uint32_t current_frame_ = 0;
};

}  // namespace subdemo
