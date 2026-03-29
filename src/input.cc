#include "include/input.h"

#include <dos.h>

namespace subdemo {

bool IsEscapePressed() { return inportb(0x60) == 0x01; }

}  // namespace subdemo
