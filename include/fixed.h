#pragma once
#include <concepts>
#include <cstdint>
#include <type_traits>

namespace subdemo {

// Fixed is a template for signed 32-bit fixed-point arithmetic.
// The template parameter N specifies the number of fractional bits.
template <int N>
class Fixed {
 public:
  constexpr Fixed() : bits_(0) {}
  constexpr explicit Fixed(std::integral auto val)
      : bits_(static_cast<int32_t>(val) << N) {}
  consteval explicit Fixed(std::floating_point auto val)
      : bits_(static_cast<int32_t>(val * static_cast<double>(1LL << N))) {}

  // Creates a Fixed object from its bit-pattern.
  static constexpr Fixed FromBits(int32_t bits_val) {
    Fixed f;
    f.bits_ = bits_val;
    return f;
  }

  // Returns the underlying bit-pattern of the fixed-point number.
  constexpr int32_t bits() const { return bits_; }

  // Converts the fixed-point value to an integer by truncation.
  constexpr int32_t ToInt() const { return bits_ >> N; }

  // Converts the fixed-point value to a floating-point value at compile-time.
  // This is evaluated by the compiler and cannot be called at runtime,
  // making it safe for targets without an FPU.
  consteval float ToFloat() const {
    return static_cast<float>(bits_) / (1LL << N);
  }

  constexpr Fixed operator+(const Fixed& other) const {
    return FromBits(bits_ + other.bits_);
  }
  constexpr Fixed operator-(const Fixed& other) const {
    return FromBits(bits_ - other.bits_);
  }

  constexpr Fixed operator*(const Fixed& other) const {
    return FromBits(
        static_cast<int32_t>((static_cast<int64_t>(bits_) * other.bits_) >> N));
  }

  constexpr Fixed operator*(std::integral auto scalar) const {
    return FromBits(bits_ * static_cast<int32_t>(scalar));
  }

  constexpr Fixed operator/(const Fixed& other) const {
    if (other.bits_ == 0) return FromBits(0);
    return FromBits(
        static_cast<int32_t>((static_cast<int64_t>(bits_) << N) / other.bits_));
  }

  constexpr Fixed& operator+=(const Fixed& other) {
    bits_ += other.bits_;
    return *this;
  }
  constexpr Fixed& operator-=(const Fixed& other) {
    bits_ -= other.bits_;
    return *this;
  }

  constexpr bool operator>(const Fixed& other) const {
    return bits_ > other.bits_;
  }
  constexpr bool operator<(const Fixed& other) const {
    return bits_ < other.bits_;
  }
  constexpr bool operator>=(const Fixed& other) const {
    return bits_ >= other.bits_;
  }
  constexpr bool operator<=(const Fixed& other) const {
    return bits_ <= other.bits_;
  }
  constexpr bool operator==(const Fixed& other) const {
    return bits_ == other.bits_;
  }

  // Returns a Fixed object representing the value 1.0 in this format.
  static constexpr Fixed One() { return FromBits(1 << N); }

 private:
  int32_t bits_;
};

}  // namespace subdemo
