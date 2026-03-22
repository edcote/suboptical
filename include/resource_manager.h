#pragma once

#include <cstdint>

namespace resource {

// Handle-based resource manager for lightweight resource tracking.
using ResourceHandle = uint32_t;
constexpr ResourceHandle kInvalidHandle = 0;

class ResourceManager {
 public:
  static constexpr uint32_t kMaxResources = 256;

  ResourceManager();
  ~ResourceManager() = default;

  // Adds a resource pointer and returns a unique handle.
  // Returns kInvalidHandle if the manager is full.
  ResourceHandle Add(void* resource);

  // Retrieves a resource pointer by its handle.
  // Returns nullptr if the handle is invalid or the resource is not found.
  void* Get(ResourceHandle handle) const;

  // Removes a resource entry by its handle.
  // NOTE: This does NOT delete the resource; the caller is responsible for
  // memory management of the raw pointer.
  void Remove(ResourceHandle handle);

 private:
  void* resources_[kMaxResources];
};

}  // namespace resource
