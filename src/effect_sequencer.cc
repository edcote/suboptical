#include "include/effect_sequencer.h"

#include <algorithm>
#include <utility>

#include "include/effect.h"
#include "include/fixed_math.h"
#include "include/logger.h"
#include "include/system_context.h"

namespace subdemo {

void EffectSequencer::RegisterEffect(const char* name, Effect* effect) {
  if (effect == nullptr || effects_.full()) {
    return;
  }
  effects_.push_back({name, effect});
}

bool EffectSequencer::LoadTimeline(const char* /*filename*/) {
  // Not yet implemented.
  return false;
}

void EffectSequencer::AddTimelineEntry(const TimelineEntry& entry) {
  if (timeline_.full()) {
    LogFatal("Maximum number of timeline entries reached");
  }

  TimelineEntry new_entry = entry;
  uint32_t duration = entry.end_frame - entry.start_frame;
  if (duration > 0) {
    new_entry.t_step = FixedMath<16>::Inv(Fixed<16>(duration));
  } else {
    new_entry.t_step = Fixed<16>(0);
  }
  timeline_.push_back(new_entry);
}

void EffectSequencer::Update(uint32_t frame) {
  current_frame_ = frame;

  // Check for triggers.
  for (const auto& trigger : triggers_) {
    if (trigger.frame == frame) {
      Trigger(trigger.trigger_id);
    }
  }

  // Update active effects in the timeline.
  active_entries_.clear();
  for (auto& entry : timeline_) {
    if (frame >= entry.start_frame && frame < entry.end_frame) {
      active_entries_.push_back(&entry);

      // Calculate normalized time (0.0..1.0) relative to timeline entry start.
      uint32_t effect_frame = frame - entry.start_frame;

      // Calculate normalized time using pre-calculated tick rate.
      Fixed<16> t = entry.t_step * effect_frame;

      entry.effect->Update(frame, effect_frame, t);
    }
  }
}

void EffectSequencer::Render(SystemContext* context) {
  // Sort active entries by priority.
  for (size_t i = 0; i < active_entries_.size(); ++i) {
    for (size_t j = i + 1; j < active_entries_.size(); ++j) {
      if (active_entries_[j]->priority < active_entries_[i]->priority) {
        std::swap(active_entries_[i], active_entries_[j]);
      }
    }
  }

  for (auto* entry : active_entries_) {
    entry->effect->Render(context);
  }
}

void EffectSequencer::Trigger(uint32_t trigger_id) {
  for (const auto& named_effect : effects_) {
    named_effect.effect->Trigger(trigger_id);
  }
}

}  // namespace subdemo
