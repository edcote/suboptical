#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace math {

// Fixed is a template for signed 32-bit fixed-point arithmetic.
// N is the number of fractional bits.
template <int N>
class Fixed {
 public:
  constexpr Fixed() : raw_(0) {}
  constexpr explicit Fixed(std::integral auto val)
      : raw_(static_cast<int32_t>(val) << N) {}
  consteval explicit Fixed(std::floating_point auto val)
      : raw_(static_cast<int32_t>(val * static_cast<double>(1LL << N))) {}

  static constexpr Fixed FromRaw(int32_t raw_val) {
    Fixed f;
    f.raw_ = raw_val;
    return f;
  }

  constexpr int32_t Raw() const { return raw_; }
  constexpr int32_t ToInt() const { return raw_ >> N; }
  consteval float ToFloat() const {
    return static_cast<float>(raw_) / (1LL << N);
  }

  constexpr Fixed operator+(const Fixed& other) const {
    return FromRaw(raw_ + other.raw_);
  }
  constexpr Fixed operator-(const Fixed& other) const {
    return FromRaw(raw_ - other.raw_);
  }

  constexpr Fixed operator*(const Fixed& other) const {
    return FromRaw(
        static_cast<int32_t>((static_cast<int64_t>(raw_) * other.raw_) >> N));
  }

  constexpr Fixed operator/(const Fixed& other) const {
    if (other.raw_ == 0) return FromRaw(0);
    return FromRaw(
        static_cast<int32_t>((static_cast<int64_t>(raw_) << N) / other.raw_));
  }

  constexpr Fixed& operator+=(const Fixed& other) {
    raw_ += other.raw_;
    return *this;
  }
  constexpr Fixed& operator-=(const Fixed& other) {
    raw_ -= other.raw_;
    return *this;
  }

  constexpr bool operator>(const Fixed& other) const {
    return raw_ > other.raw_;
  }
  constexpr bool operator<(const Fixed& other) const {
    return raw_ < other.raw_;
  }
  constexpr bool operator>=(const Fixed& other) const {
    return raw_ >= other.raw_;
  }
  constexpr bool operator<=(const Fixed& other) const {
    return raw_ <= other.raw_;
  }
  constexpr bool operator==(const Fixed& other) const {
    return raw_ == other.raw_;
  }

  static constexpr Fixed One() { return FromRaw(1 << N); }

 private:
  int32_t raw_;
};

using Q16 = Fixed<16>;
using Q8 = Fixed<8>;
using Q24 = Fixed<24>;
using Q30 = Fixed<30>;

}  // namespace math
