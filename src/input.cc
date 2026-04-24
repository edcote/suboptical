#include "include/input.h"

#include <dos.h>

namespace subdemo {

bool IsEscapePressed() {
  // Index 0x60: Keyboard Controller Data Port
  // Bit | Field Name        | Value | Description
  // ----|-------------------|-------|-------------------------------------------
  // 0-7 | Scan Code         |  0x01 | 0x01 is the "Make" code for the Escape
  // key
  return inportb(0x60) == 0x01;
}

}  // namespace subdemo
