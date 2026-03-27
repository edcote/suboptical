#pragma once
#include <cstdint>

namespace math {

// Fixed is a template for signed 32-bit fixed-point arithmetic.
// N is the number of fractional bits.
template <int N>
class Fixed {
 public:
  Fixed() : raw_(0) {}
  explicit Fixed(int32_t val) : raw_(val << N) {}
  explicit Fixed(float val) : raw_(static_cast<int32_t>(val * (1LL << N))) {}

  static Fixed FromRaw(int32_t raw_val) {
    Fixed f;
    f.raw_ = raw_val;
    return f;
  }

  int32_t Raw() const { return raw_; }
  int32_t ToInt() const { return raw_ >> N; }
  float ToFloat() const { return static_cast<float>(raw_) / (1LL << N); }

  Fixed operator+(const Fixed& other) const {
    return FromRaw(raw_ + other.raw_);
  }
  Fixed operator-(const Fixed& other) const {
    return FromRaw(raw_ - other.raw_);
  }

  Fixed operator*(const Fixed& other) const {
    return FromRaw(
        static_cast<int32_t>((static_cast<int64_t>(raw_) * other.raw_) >> N));
  }

  Fixed operator/(const Fixed& other) const {
    if (other.raw_ == 0) return FromRaw(0);
    return FromRaw(
        static_cast<int32_t>((static_cast<int64_t>(raw_) << N) / other.raw_));
  }

  Fixed& operator+=(const Fixed& other) {
    raw_ += other.raw_;
    return *this;
  }
  Fixed& operator-=(const Fixed& other) {
    raw_ -= other.raw_;
    return *this;
  }

  bool operator>(const Fixed& other) const { return raw_ > other.raw_; }
  bool operator<(const Fixed& other) const { return raw_ < other.raw_; }
  bool operator>=(const Fixed& other) const { return raw_ >= other.raw_; }
  bool operator<=(const Fixed& other) const { return raw_ <= other.raw_; }
  bool operator==(const Fixed& other) const { return raw_ == other.raw_; }

 private:
  int32_t raw_;
};

// Common fixed-point types for demoscene work.
using fxp_16_16 = Fixed<16>;
using fxp_24_8 = Fixed<8>;
using fxp_8_24 = Fixed<24>;
using fxp_2_30 = Fixed<30>;

}  // namespace math
