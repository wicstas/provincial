#pragma once

#include <psl/stdint.h>

#ifndef NOMINMAX
#define NOMINMAX
#endif

#if defined(__GNUC__) || defined(__clang__)
#define PINE_LIKELY(x) __builtin_expect(x, true)
#define PINE_UNLIKELY(x) __builtin_expect(x, false)
#define PINE_RESTRICT __restrict
#define PINE_ALWAYS_INLINE __attribute__((always_inline))
#define PINE_UNREACHABLE                                             \
  {                                                                  \
    FATAL("Should never be reached[", __FILE__, ":", __LINE__, "]"); \
    __builtin_unreachable();                                         \
  }
#else
#define PINE_LIKELY(x) x
#define PINE_UNLIKELY(x) x
#define PINE_RESTRICT
#define PINE_ALWAYS_INLINE
#define PINE_UNREACHABLE Fatal("Should never be_ reached");
#endif

namespace psl {
class string;
class string_view;
};  // namespace psl

using str = psl::string;
using str_view = psl::string_view;