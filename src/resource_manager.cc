#include "include/resource_manager.h"

#include <cstring>

#include "include/logger.h"

namespace resource {

ResourceManager::ResourceManager() : offset_(0) {
  memset(resources_, 0, sizeof(resources_));
}

void* ResourceManager::Allocate(size_t size) {
  // 4-byte alignment (align offset up to the nearest multiple of 4).
  const size_t aligned_offset = (offset_ + 3) & ~3;

  if (aligned_offset + size > kArenaSize) {
    LogError("Arena out of memory. Requested %u bytes, %u remaining.", size,
             kArenaSize - offset_);
    return nullptr;
  }

  void* ptr = &arena_[aligned_offset];
  offset_ = aligned_offset + size;
  return ptr;
}

void ResourceManager::ResetToMarker(size_t marker) {
  if (marker > offset_) {
    LogError("Invalid marker reset. Attempting to reset past current offset.");
    return;
  }
  offset_ = marker;

  // Clean up any ResourceHandles that were tracking pointers in the "freed"
  // area.
  for (uint32_t i = 0; i < kMaxResources; ++i) {
    if (resources_[i] >= &arena_[offset_]) {
      resources_[i] = nullptr;
    }
  }
}

ResourceHandle ResourceManager::LoadFromBundle(
    const bundle::BundleReader& reader, const char* filename) {
  bundle::BundleFileInfo info;
  if (!reader.GetFileInfo(filename, &info)) {
    LogError("File not found in bundle: %s", filename);
    return kInvalidHandle;
  }

  void* buffer = Allocate(info.unpacked_size);
  if (!buffer) {
    LogError("Failed to allocate %u bytes for %s in arena.", info.unpacked_size,
             filename);
    return kInvalidHandle;
  }

  // Cast to non-const for LoadFile (the API needs a mutable buffer).
  if (!const_cast<bundle::BundleReader&>(reader).LoadFile(info, buffer)) {
    LogError("Failed to load/decompress %s from bundle.", filename);
    // Note: We don't rollback 'offset_' here to keep implementation simple.
    // The small leak is acceptable for a fatal loading failure.
    return kInvalidHandle;
  }

  return Add(buffer);
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

  LogError("ResourceManager full. No more ResourceHandles available.");
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
