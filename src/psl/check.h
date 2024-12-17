#pragma once

namespace psl {

class string_view;

struct Exception {
  virtual ~Exception() = default;

  virtual string_view what() const = 0;
};

void throw_check_failure(const char* expr, const char* file, int line, const char* func);

#ifndef NDEBUG
#define psl_check(expr)                                         \
  {                                                             \
    if (!(expr))                                                \
      throw_check_failure(#expr, __FILE__, __LINE__, __func__); \
  }
#else
#define psl_check(expr)
#endif

#define psl_check_always(expr)                                  \
  {                                                             \
    if (!(expr))                                                \
      psl::throw_check_failure(#expr, __FILE__, __LINE__, __func__); \
  }

}  // namespace psl
