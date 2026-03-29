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
  // angle: 0..1023 maps to 0..2PI. Returns Sine in the target QN format.
  static Fixed<N> Sin(uint32_t angle);

  // angle: 0..1023 maps to 0..2PI. Returns Cosine in the target QN format.
  static Fixed<N> Cos(uint32_t angle);

  // Returns 1/value in the target QN format.
  // Fast LUT for value (interpreted as integer) in [1..1024], else hardware
  // idiv.
  static Fixed<N> Inv(Fixed<N> value);

  // Returns the square root of value using the binary-search integer root
  // algorithm.
  static Fixed<N> Sqrt(Fixed<N> value);

  // Clamps value between min and max.
  static inline Fixed<N> Clamp(Fixed<N> value, Fixed<N> min, Fixed<N> max) {
    if (value < min) return min;
    if (value > max) return max;
    return value;
  }

  // Linear interpolation between start and end.
  // result = start + (end - start) * amount.
  static inline Fixed<N> Lerp(Fixed<N> start, Fixed<N> end, Fixed<N> amount) {
    return start + (end - start) * amount;
  }

  // Returns the Cartesian X component from polar coordinates.
  static inline Fixed<N> PolarX(Fixed<N> radius, uint32_t angle) {
    return radius * Cos(angle);
  }

  // Returns the Cartesian Y component from polar coordinates.
  static inline Fixed<N> PolarY(Fixed<N> radius, uint32_t angle) {
    return radius * Sin(angle);
  }

 private:
  // Internal compile-time table generators, specialized for format N.
  struct SinTable {
    int32_t data[1024];
    consteval SinTable() : data{} {
      double scale = static_cast<double>(1LL << N);
      for (int i = 0; i < 1024; ++i) {
        double radians = (2.0 * 3.14159265358979323846 * i) / 1024.0;
        data[i] = static_cast<int32_t>(std::sin(radians) * scale);
      }
    }
  };

  struct InvTable {
    int32_t data[1024];
    consteval InvTable() : data{} {
      double scale = static_cast<double>(1LL << N);
      data[0] = 0;
      for (int i = 1; i < 1024; ++i) {
        data[i] = static_cast<int32_t>(scale / i);
      }
    }
  };

  static constexpr SinTable kSinTable = SinTable();
  static constexpr InvTable kInvTable = InvTable();
};

}  // namespace subdemo
