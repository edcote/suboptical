#include "include/fixed_math.h"

#include <cstdint>

namespace math {

template <int N>
Fixed<N> FixedMath<N>::Sin(uint32_t angle) {
  angle &= 1023;  // Fast modulo for power-of-two size.
  int32_t val = kSinTableQ30.data[angle];

  // Convert from Q30 to QN.
  if constexpr (N <= 30) {
    return Fixed<N>::FromRaw(val >> (30 - N));
  } else {
    return Fixed<N>::FromRaw(val << (N - 30));
  }
}

template <int N>
Fixed<N> FixedMath<N>::Cos(uint32_t angle) {
  // Cos(x) = Sin(x + 90 degrees).
  return Sin(angle + 256);
}

template <int N>
Fixed<N> FixedMath<N>::Inv(Type v) {
  int32_t raw_val = v.Raw();
  if (raw_val <= 0) return Fixed<N>::FromRaw(0);

  // Convert raw value to "integer" equivalent for LUT index.
  int32_t idx = raw_val >> N;
  if (idx > 0 && idx < 1024) {
    int32_t val = kInvTableQ16.data[idx];
    // LUT result is Q16. Convert to QN.
    if constexpr (N <= 16) {
      return Fixed<N>::FromRaw(val >> (16 - N));
    } else {
      return Fixed<N>::FromRaw(val << (N - 16));
    }
  }

  // Fallback to high-precision hardware division.
  // result = (1.0 in 2*N scale) / raw_val.
  return Fixed<N>::FromRaw(
      static_cast<int32_t>((static_cast<int64_t>(1) << (2 * N)) / raw_val));
}

template <int N>
Fixed<N> FixedMath<N>::Sqrt(Type v) {
  int32_t raw_val = v.Raw();
  if (raw_val <= 0) return Fixed<N>::FromRaw(0);

  // Integer square root on (val << N).
  int64_t x = static_cast<int64_t>(raw_val) << N;
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
  return Fixed<N>::FromRaw(static_cast<int32_t>(res));
}

// Explicit template instantiations for common demoscene formats.
template class FixedMath<16>;
template class FixedMath<8>;
template class FixedMath<24>;
template class FixedMath<30>;

}  // namespace math
