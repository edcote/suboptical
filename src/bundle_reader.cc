#include "include/bundle_reader.h"

#include <cstring>

#include "include/logger.h"

namespace subdemo {

namespace {
constexpr int kHeaderSize = 8;
constexpr int kIndexEntrySize = 32;

struct RawIndexEntry {
  char name[13];
  uint32_t offset;
  uint32_t packed_size;
  uint32_t unpacked_size;
  uint16_t flags;
  uint8_t reserved[7];
};
}  // namespace

BundleReader::BundleReader() : file_(nullptr), file_count_(0) {}

BundleReader::~BundleReader() { Close(); }

bool BundleReader::Open(const char* path) {
  Close();

  file_ = fopen(path, "rb");
  if (!file_) {
    LogError("Failed to open bundle: %s", path);
    return false;
  }

  char magic[4];
  if (fread(magic, 1, 4, file_) != 4 || memcmp(magic, "SB01", 4) != 0) {
    LogError("Invalid bundle magic in: %s", path);
    Close();
    return false;
  }

  if (fread(&file_count_, 1, 4, file_) != 4) {
    LogError("Failed to read file count from bundle: %s", path);
    Close();
    return false;
  }

  return true;
}

void BundleReader::Close() {
  if (file_) {
    fclose(file_);
    file_ = nullptr;
  }
  file_count_ = 0;
}

bool BundleReader::GetFileInfo(const char* name, BundleFileInfo* info) const {
  if (!file_ || !info) {
    return false;
  }

  // Linear search through the index.
  for (uint32_t i = 0; i < file_count_; ++i) {
    fseek(file_, kHeaderSize + (i * kIndexEntrySize), SEEK_SET);

    RawIndexEntry entry;
    if (fread(&entry, 1, sizeof(entry), file_) != sizeof(entry)) {
      break;
    }

    if (strcasecmp(entry.name, name) == 0) {
      memcpy(info->name, entry.name, 13);
      info->offset = entry.offset;
      info->packed_size = entry.packed_size;
      info->unpacked_size = entry.unpacked_size;
      info->is_compressed = (entry.flags & 1) != 0;
      return true;
    }
  }

  return false;
}

bool BundleReader::LoadFile(const BundleFileInfo& info, void* buffer) {
  if (!file_ || !buffer) {
    return false;
  }

  fseek(file_, info.offset, SEEK_SET);

  if (!info.is_compressed) {
    return fread(buffer, 1, info.unpacked_size, file_) == info.unpacked_size;
  }

  // Simple RLE decompression.
  uint8_t* dest = static_cast<uint8_t*>(buffer);
  uint32_t packed_read = 0;
  uint32_t unpacked_written = 0;

  while (packed_read < info.packed_size &&
         unpacked_written < info.unpacked_size) {
    uint8_t token;
    if (fread(&token, 1, 1, file_) != 1) {
      return false;
    }
    packed_read++;

    if (token < 128) {
      // Literal run: Copy token + 1 bytes.
      uint32_t count = token + 1;
      if (fread(dest + unpacked_written, 1, count, file_) != count) {
        return false;
      }
      packed_read += count;
      unpacked_written += count;
    } else {
      // Repeat run: Repeat next byte (token - 128) + 2 times.
      uint32_t count = (token - 128) + 2;
      uint8_t val;
      if (fread(&val, 1, 1, file_) != 1) {
        return false;
      }
      packed_read++;
      for (uint32_t i = 0; i < count; ++i) {
        dest[unpacked_written++] = val;
      }
    }
  }

  return unpacked_written == info.unpacked_size;
}

}  // namespace subdemo
