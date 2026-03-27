#pragma once
#include <cmath>

#include "include/fixed.h"

namespace math {

// FixedMath provides high-performance LUT-based operations for the Fixed
// template.
template <int N>
class FixedMath {
 public:
  using Type = Fixed<N>;

  // angle: 0..1023 maps to 0..2PI. Returns Sine in the target QN format.
  static Type Sin(uint32_t angle);

  // angle: 0..1023 maps to 0..2PI. Returns Cosine in the target QN format.
  static Type Cos(uint32_t angle);

  // Returns 1/v in the target QN format.
  // Fast LUT for v (interpreted as integer) in [1..1024], else hardware idiv.
  static Type Inv(Type v);

  // Returns the square root of v using the binary-search integer root
  // algorithm.
  static Type Sqrt(Type v);

  // Clamps v between min and max.
  static inline Type Clamp(Type v, Type min, Type max) {
    if (v < min) return min;
    if (v > max) return max;
    return v;
  }

  // Linear interpolation between a and b.
  // result = a + (b - a) * t.
  static inline Type Lerp(Type a, Type b, Type t) { return a + (b - a) * t; }

  // Returns the Cartesian X component from polar coordinates.
  static inline Type PolarX(Type radius, uint32_t angle) {
    return radius * Cos(angle);
  }

  // Returns the Cartesian Y component from polar coordinates.
  static inline Type PolarY(Type radius, uint32_t angle) {
    return radius * Sin(angle);
  }

 private:
  // Internal compile-time table generators.
  struct SinTable {
    int32_t data[1024];
    consteval SinTable() : data{} {
      for (int i = 0; i < 1024; ++i) {
        double rad = (2.0 * 3.14159265358979323846 * i) / 1024.0;
        data[i] = static_cast<int32_t>(std::sin(rad) * 1073741824.0);
      }
    }
  };

  struct InvTable {
    int32_t data[1024];
    consteval InvTable() : data{} {
      data[0] = 0;
      for (int i = 1; i < 1024; ++i) {
        data[i] = static_cast<int32_t>(65536.0 / i);
      }
    }
  };

  static constexpr SinTable kSinTableQ30 = SinTable();
  static constexpr InvTable kInvTableQ16 = InvTable();
};

}  // namespace math
