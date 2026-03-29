#pragma once
#include <cmath>

#include "include/fixed.h"

namespace subdemo {

// Provides optimized mathematical operations for Fixed<N> fixed-point types,
// including lookup-table based trigonometry, fast inverse, square root, and
// interpolation.
template <int N>
class FixedMath {
 public:
  using Type = Fixed<N>;

  // angle: 0..1023 maps to 0..2PI. Returns Sine in the target QN format.
  static Type Sin(uint32_t angle);

  // angle: 0..1023 maps to 0..2PI. Returns Cosine in the target QN format.
  static Type Cos(uint32_t angle);

  // Returns 1/value in the target QN format.
  // Fast LUT for value (interpreted as integer) in [1..1024], else hardware
  // idiv.
  static Type Inv(Type value);

  // Returns the square root of value using the binary-search integer root
  // algorithm.
  static Type Sqrt(Type value);

  // Clamps value between min and max.
  static inline Type Clamp(Type value, Type min, Type max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  // Linear interpolation between start and end.
  // result = start + (end - start) * amount.
  static inline Type Lerp(Type start, Type end, Type amount) {
    return start + (end - start) * amount;
  }

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
        double radians = (2.0 * 3.14159265358979323846 * i) / 1024.0;
        data[i] = static_cast<int32_t>(std::sin(radians) * 1073741824.0);
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

}  // namespace subdemo
