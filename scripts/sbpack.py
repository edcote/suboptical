"""SubBundle archive creation and RLE compression tool.

Follows the SB01 format for bundling assets for the suboptical demo engine.
"""

import argparse
import os
import struct


def rle_compress(data: bytes) -> bytes:
  """Compresses data using a simple PackBits-inspired byte-run RLE.

  Args:
    data: The raw input bytes to compress.

  Returns:
    The compressed byte sequence.

  Token Byte (T):
    0 <= T <= 127: Literal. Copy the next T + 1 bytes.
    128 <= T <= 255: Repeat. Repeat the next byte (T - 128) + 2 times.
  """
  output = bytearray()
  i = 0
  while i < len(data):
    # Check for repeat run
    run_len = 1
    while (i + run_len < len(data) and 
           data[i + run_len] == data[i] and 
           run_len < 129): # Max repeat is (255-128)+2 = 129
      run_len += 1

    if run_len >= 2:
      # Repeat run: T = (run_len - 2) + 128
      output.append(run_len - 2 + 128)
      output.append(data[i])
      i += run_len
    else:
      # Literal run
      literal_run = bytearray()
      while (i < len(data) and run_len < 128):
        # Peek ahead for a repeat run of at least 2 bytes
        if i + 1 < len(data) and data[i] == data[i + 1]:
          break
        literal_run.append(data[i])
        i += 1
        run_len = len(literal_run)
      
      # Literal run: T = run_len - 1
      output.append(run_len - 1)
      output.extend(literal_run)
      
  return bytes(output)


def pack_archive(output_path: str, rle_files: list[str], raw_files: list[str]):
  """Packs multiple files into a single SB01 archive.

  Args:
    output_path: The filesystem path where the .sb archive will be written.
    rle_files: A list of paths to files that should be RLE compressed.
    raw_files: A list of paths to files that should be stored uncompressed.
  """
  all_files = []
  
  # Prepare file info
  for path in rle_files:
    with open(path, "rb") as f:
      data = f.read()
    packed = rle_compress(data)
    all_files.append({
        "name": os.path.basename(path),
        "data": packed,
        "unpacked_size": len(data),
        "flags": 1 # RLE
    })
    
  for path in raw_files:
    with open(path, "rb") as f:
      data = f.read()
    all_files.append({
        "name": os.path.basename(path),
        "data": data,
        "unpacked_size": len(data),
        "flags": 0 # Raw
    })

  # Sort by name for deterministic builds
  all_files.sort(key=lambda x: x["name"])
  
  file_count = len(all_files)
  header_size = 8
  index_size = 32 * file_count
  current_offset = header_size + index_size
  
  with open(output_path, "wb") as f:
    # Header: Magic(4), FileCount(4)
    f.write(b"SB01")
    f.write(struct.pack("<I", file_count))
    
    # Placeholder for index entries
    for entry in all_files:
      name_bytes = entry["name"].encode("ascii")[:12] # Max 12 + null
      name_field = name_bytes + b"\0" * (13 - len(name_bytes))
      
      packed_size = len(entry["data"])
      
      # Entry: Name(13), Offset(4), Packed(4), Unpacked(4), Flags(2), Reserved(5)
      index_entry = struct.pack(
          "<13sIIIIH", 
          name_field, 
          current_offset, 
          packed_size, 
          entry["unpacked_size"], 
          entry["flags"],
          0 # Reserved (placeholder for the H/uint16_t in the format string)
      )
      # Pad to 32 bytes
      index_entry += b"\0" * (32 - len(index_entry))
      f.write(index_entry)
      
      current_offset += packed_size
      
    # Write data blocks
    for entry in all_files:
      f.write(entry["data"])


def main():
  """Parses command line arguments and executes the packing process."""
  parser = argparse.ArgumentParser(description="SubBundle Packer")
  parser.add_argument("--output", required=True, help="Output archive path")
  parser.add_argument("--rle", nargs="*", default=[], help="Files to compress with RLE")
  parser.add_argument("--raw", nargs="*", default=[], help="Files to include as raw")
  
  args = parser.parse_args()
  
  pack_archive(args.output, args.rle, args.raw)


if __name__ == "__main__":
  main()
