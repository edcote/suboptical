#include "include/fixed_math.h"

#include <cstdint>

namespace subdemo {

template <int N>
Fixed<N> FixedMath<N>::Sin(uint32_t angle) {
  angle &= 1023;  // Fast modulo for power-of-two size.
  return Fixed<N>::FromBits(kSinTable.data[angle]);
}

template <int N>
Fixed<N> FixedMath<N>::Cos(uint32_t angle) {
  // Cos(x) = Sin(x + 90 degrees).
  return Sin(angle + 256);
}

template <int N>
Fixed<N> FixedMath<N>::Inv(Fixed<N> value) {
  int32_t bits_val = value.bits();
  if (bits_val <= 0) return Fixed<N>::FromBits(0);

  // Convert raw bits value to "integer" equivalent for LUT index.
  int32_t idx = bits_val >> N;
  if (idx > 0 && idx < 1024) {
    return Fixed<N>::FromBits(kInvTable.data[idx]);
  }

  // Fallback to high-precision hardware division.
  // result = (1.0 in 2*N scale) / bits_val.
  return Fixed<N>::FromBits(
      static_cast<int32_t>((static_cast<int64_t>(1) << (2 * N)) / bits_val));
}

template <int N>
Fixed<N> FixedMath<N>::Sqrt(Fixed<N> value) {
  int32_t bits_val = value.bits();
  if (bits_val <= 0) return Fixed<N>::FromBits(0);

  // Integer square root on (bits_val << N).
  int64_t x = static_cast<int64_t>(bits_val) << N;
  int64_t res = 0;
  int64_t bit = 1LL << 62;

  while (bit > x) bit >>= 2;
  while (bit != 0) {
    if (x >= res + bit) {
      x -= res + bit;
      res = (res >> 1) + bit;
    } else {
      res >>= 1;
    }
    bit >>= 2;
  }
  return Fixed<N>::FromBits(static_cast<int32_t>(res));
}

// Explicit template instantiations for common demoscene formats.
template class FixedMath<16>;
template class FixedMath<8>;
template class FixedMath<24>;
template class FixedMath<30>;

}  // namespace subdemo
