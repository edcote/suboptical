#include "include/resource_manager.h"

namespace resource {

ResourceManager::ResourceManager() {
  for (uint32_t i = 0; i < kMaxResources; ++i) {
    resources_[i] = nullptr;
  }
}

ResourceHandle ResourceManager::Add(void* resource) {
  if (resource == nullptr) {
    return kInvalidHandle;
  }

  for (uint32_t i = 0; i < kMaxResources; ++i) {
    if (resources_[i] == nullptr) {
      resources_[i] = resource;
      // Handle is index + 1 to avoid kInvalidHandle (0).
      return i + 1;
    }
  }

  return kInvalidHandle;
}

void* ResourceManager::Get(ResourceHandle handle) const {
  if (handle == kInvalidHandle || handle > kMaxResources) {
    return nullptr;
  }

  return resources_[handle - 1];
}

void ResourceManager::Remove(ResourceHandle handle) {
  if (handle == kInvalidHandle || handle > kMaxResources) {
    return;
  }

  resources_[handle - 1] = nullptr;
}

}  // namespace resource
