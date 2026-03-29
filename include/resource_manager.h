#pragma once

#include <cstddef>
#include <cstdint>

#include "include/bundle_reader.h"

namespace subdemo {

// Handle-based resource manager with an Arena Allocator for memory efficiency.
using ResourceHandle = uint32_t;
constexpr ResourceHandle kInvalidHandle = 0;

class ResourceManager {
 public:
  // 2MB Arena for all demo assets.
  static constexpr size_t kArenaSize = 2 * 1024 * 1024;
  static constexpr uint32_t kMaxResources = 256;

  ResourceManager();
  ~ResourceManager() = default;

  // Allocates memory from the arena. Returns nullptr if out of memory.
  // Note: All allocations are 4-byte aligned for 386/486 performance.
  void* Allocate(size_t size);

  // Markers for resetting the arena (Scene-based management).
  size_t GetMarker() const { return offset_; }
  void ResetToMarker(size_t marker);

  // High-level asset loading using a BundleReader.
  // Allocates arena memory, loads/decompresses the file, and returns a handle.
  ResourceHandle LoadFromBundle(const BundleReader& reader,
                                const char* filename);

  // Adds a resource pointer and returns a unique handle for tracking.
  // Returns kInvalidHandle if the manager is full.
  ResourceHandle Add(void* resource);

  // Retrieves a resource pointer by its handle.
  // Returns nullptr if the handle is invalid or the resource is not found.
  void* Get(ResourceHandle handle) const;

  // Removes a resource entry by its handle.
  // NOTE: This does NOT "free" the arena memory, it only stops tracking the
  // pointer. Use ResetToMarker() to reclaim arena memory.
  void Remove(ResourceHandle handle);

 private:
  uint8_t arena_[kArenaSize];
  size_t offset_;
  void* resources_[kMaxResources];
};

}  // namespace subdemo
