#pragma once

#include <cstdint>
#include <cstdio>

namespace bundle {

// Information about a file within a SubBundle archive.
struct BundleFileInfo {
  char name[13];
  uint32_t packed_size;
  uint32_t unpacked_size;
  bool is_compressed;
  // Internal absolute offset to the start of data in the archive.
  uint32_t offset;
};

// Simple, memory-efficient reader for SubBundle (.sb) archives.
// This reader performs no internal heap allocations during file loading.
class BundleReader {
 public:
  BundleReader();
  ~BundleReader();

  // Opens a SubBundle archive file. Returns true on success.
  bool Open(const char* path);

  // Closes the archive file.
  void Close();

  // Finds a file in the archive by name.
  // Returns true and fills the provided 'info' pointer if found.
  // The 'info' pointer must not be nullptr.
  bool GetFileInfo(const char* name, BundleFileInfo* info) const;

  // Loads and decompresses (if necessary) a file into the user-provided buffer.
  // Returns true on success. The buffer must be large enough to hold
  // 'info.unpacked_size' bytes.
  bool LoadFile(const BundleFileInfo& info, void* buffer);

 private:
  FILE* file_ = nullptr;
  uint32_t file_count_ = 0;
};

}  // namespace bundle
