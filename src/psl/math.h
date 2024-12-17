#pragma once

#include <psl/type_traits.h>
#include <psl/memory.h>
#include <psl/stdint.h>

#include <cmath>

namespace psl {

inline constexpr float E = 2.7182818284590452354f;
inline constexpr float Log2E = 1.4426950408889634074f;
inline constexpr float Log10E = 0.43429448190325182765f;
inline constexpr float Ln2 = 0.69314718055994530942f;
inline constexpr float Ln10 = 2.30258509299404568402f;
inline constexpr float Pi = 3.14159265358979323846f;
inline constexpr float Sqrt2 = 1.41421356237309504880f;

template <typename T>
inline constexpr T min(T a, T b) {
  return a < b ? a : b;
}
template <typename T>
inline constexpr T max(T a, T b) {
  return a > b ? a : b;
}
template <typename T>
inline constexpr int index_of_min(T a, T b) {
  return a < b ? 0 : 1;
}
template <typename T>
inline constexpr int index_of_max(T a, T b) {
  return a > b ? 0 : 1;
}
template <typename T>
inline constexpr int index_of_min(T a, T b, T c) {
  if (a < b)
    return a < c ? 0 : 2;
  else
    return b < c ? 1 : 2;
}
template <typename T>
inline constexpr int index_of_max(T a, T b, T c) {
  if (a > b)
    return a > c ? 0 : 2;
  else
    return b > c ? 1 : 2;
}

template <typename T, typename... Ts>
requires(sizeof...(Ts) > 0)
inline constexpr T min(T a, T b, Ts... c) {
  return psl::min(a, psl::min(b, c...));
}
template <typename T, typename... Ts>
requires(sizeof...(Ts) > 0)
inline constexpr T max(T a, T b, Ts... c) {
  return psl::max(a, psl::max(b, c...));
}

template <typename T>
inline constexpr T abs(T x) {
  return x > 0 ? x : -x;
}
inline constexpr auto abs(float x) {
  return std::abs(x);
}
inline constexpr auto abs(double x) {
  return std::abs(x);
}

template <typename T>
inline constexpr T copysign(T mag, T sgn) {
  return sgn > 0 ? mag : -mag;
}
template <typename T>
inline constexpr int signbit(T sgn) {
  return sgn < 0 ? 1 : 0;
}

inline int ieeeexp(float x) {
  return (0xff & (psl::bitcast<uint32_t>(x) >> 23)) - 127;
}

inline float exp2i(int e) {
  return psl::bitcast<float>((e + 127) << 23);
}

inline int hsb(uint32_t x) {
  return psl::ieeeexp((float)x);
}

template <typename T>
int ctz(T x) {
  return psl::hsb(x & -x);
}

template <typename T>
constexpr T roundup2(T x) {
  if (x == 0)
    return 0;
  x -= 1;

  for (size_t i = 1; i != sizeof(x) * 8; i <<= 1)
    x |= x >> i;

  return x + 1;
}

template <typename T>
inline T rounddown2(T x) {
  return (T)1 << psl::hsb(x);
}

template <typename T>
inline constexpr T mod(T a, T b) {
  T r = a - (a / b) * b;
  return r < 0 ? r + b : r;
}

template <typename T>
inline constexpr T clamp(T val, T a, T b) {
  return psl::min(psl::max(val, a), b);
}

inline constexpr bool inside(auto val, auto min_inclusive, auto max_exclusive) {
  return val >= min_inclusive && val < max_exclusive;
}

template <typename T, typename U>
inline constexpr U lerp(T t, U a, U b) {
  return a * ((T)1 - t) + b * t;
}

template <typename T>
inline constexpr T sqr(T v) {
  return v * v;
}

template <typename T>
inline constexpr int sign(T v) {
  return v > 0 ? 1 : -1;
}

// TODO
template <FloatingPoint T>
inline constexpr T floor(T v) {
  return std::floor(v);
  auto i = (CorrespondingInt<T>)v;
  if (v < 0 && i != v)
    i -= 1;
  return i;
}

// TODO
template <FloatingPoint T>
inline constexpr T ceil(T v) {
  return std::ceil(v);
  auto i = (CorrespondingInt<T>)v;
  if (v > 0 && i != v)
    i += 1;
  return i;
}

template <FloatingPoint T>
inline constexpr T fract(T v) {
  return v - floor(v);
}

template <FloatingPoint T>
inline constexpr T absfract(T v) {
  v = psl::abs(v);
  return v - (CorrespondingInt<T>)v;
}

template <FloatingPoint T>
inline T sqrt(T y) {
  return std::sqrt(y);
  T x = psl::exp2i(psl::ieeeexp(y) / 2);

  x = x / 2 + y / (2 * x);
  x = x / 2 + y / (2 * x);
  x = x / 2 + y / (2 * x);
  x = x / 2 + y / (2 * x);

  return x;
}
template <FloatingPoint T>
inline T safe_sqrt(T y) {
  return psl::sqrt(psl::max(y, T(0)));
  T x = psl::exp2i(psl::ieeeexp(y) / 2);

  x = x / 2 + y / (2 * x);
  x = x / 2 + y / (2 * x);
  x = x / 2 + y / (2 * x);
  x = x / 2 + y / (2 * x);

  return x;
}

template <typename T>
inline constexpr T powi(T x, int e) {
  T y = 1;

  while (e) {
    if (e & 1)
      y *= x;
    x *= x;
    e >>= 1;
  };

  return y;
}

template <typename T>
inline constexpr T pow(T x, T e) {
  return std::pow(x, e);
  int ei = psl::floor(e);
  e -= ei;
  T y = ei > 0 ? psl::powi(x, ei) : 1 / psl::powi(x, -ei);

  const int n = 8;

  for (int i = 0; i < n; ++i) {
    if (e > 1.0f) {
      y *= x;
      e -= 1.0f;
    }
    e *= 2;
    x = psl::sqrt(x);
  }

  return y;
}

template <FloatingPoint T>
inline T log2(T y) {
  return std::log2(y);
  int log = psl::ieeeexp(y);

  T logl = 0;
  T logr = 1;
  T expl = psl::exp2i(logl);
  T expr = psl::exp2i(logr);
  y /= psl::exp2i(log);

  const int n = 8;

  for (int i = 0; i < n; ++i) {
    T logm = (logl + logr) / 2;
    T expm = psl::sqrt(expl * expr);
    if (expm < y) {
      logl = logm;
      expl = expm;
    } else {
      logr = logm;
      expr = expm;
    }
  }

  return log + (logl + logr) / 2;
}

template <FloatingPoint T>
inline T log(T y) {
  return std::log(y);
  return Ln2 * psl::log2(y);
}

template <FloatingPoint T>
inline T log10(T y) {
  return std::log10(y);
  return Ln2 / Ln10 * psl::log2(y);
}

template <typename T>
inline T log2i(T y) {
  return psl::hsb(y);
}

template <typename T>
bool is_power_of_2(T y) {
  return (y & (y - 1)) == 0;
}

template <FloatingPoint T>
inline T exp(T x) {
  return std::exp(x);
  if (x < 0)
    return T{1} / exp(-x);
  T y = 0;

  const int n = 8;

  for (int i = n; i > 0; --i)
    y = 1 + x * y / i;

  return y;
}

template <FloatingPoint T>
inline bool isnan(T x) {
  return !(x == x);
}

template <FloatingPoint T>
inline bool isinf(T x) {
  return __builtin_isinf(x);
}

template <FloatingPoint T>
inline T cos_0_pi_2(T x) {
  T y = 1;

  const int n = 8;
  for (int i = n; i > 0; i -= 2)
    y = 1 - y * x * x / (i * (i - 1));

  return y;
}

template <FloatingPoint T>
inline T cos(T x) {
  return std::cos(x);
  if (x < 0)
    x = -x;
  // TODO
  CorrespondingInt<T> y = x / (Pi * 2);
  x -= y * (Pi * 2);
  if (x > Pi)
    x = Pi * 2 - x;

  if (x < Pi / 2)
    return cos_0_pi_2(x);
  else
    return -cos_0_pi_2(Pi - x);
}

template <FloatingPoint T>
inline T sin(T x) {
  return std::sin(x);
  return psl::cos(x - Pi / 2);
}

template <FloatingPoint T>
inline T tan(T x) {
  return std::tan(x);
  // TODO
  return psl::sin(x) / psl::cos(x);
}

template <FloatingPoint T>
inline T acos(T y) {
  return std::acos(y);
  // TODO
  if (y < -1 || y > 1)
    return 0;

  T x = (1 - y) * Pi / 2;

  const int n = 8;

  for (int i = 0; i < n; ++i) {
    T y_ = psl::cos(x) - y;
    T dydx = -psl::sin(x);
    x -= y_ / dydx;
  }

  return x;
}

template <FloatingPoint T>
inline T asin(T y) {
  return std::asin(y);
  // TODO
  if (y < -1 || y > 1)
    return 0;

  T x = y * Pi / 2;

  const int n = 8;

  for (int i = 0; i < n; ++i) {
    T y_ = psl::sin(x) - y;
    T dydx = psl::cos(x);
    x -= y_ / dydx;
  }

  return x;
}

template <FloatingPoint T>
inline T atan(T y) {
  return std::atan(y);
  // TODO
  T x = (y > 0 ? 1 - 1 / (1 + y) : -1 + 1 / (1 - y)) * Pi / 2;

  const int n = 8;

  for (int i = 0; i < n; ++i) {
    T y_ = psl::tan(x) - y;
    T dydx = 1.0f / psl::sqr(psl::cos(x));
    x -= y_ / dydx;
  }

  return x;
}

template <FloatingPoint T>
inline T atan2(T y, T x) {
  return std::atan2(y, x);
  if (x > 0)
    return psl::atan(y / x);
  else if (y > 0)
    return Pi - psl::atan(-y / x);
  else
    return -Pi - psl::atan(-y / x);
}

}  // namespace psl
