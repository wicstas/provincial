#pragma once

#include <psl/type_traits.h>
#include <psl/utility.h>
#include <psl/memory.h>

namespace psl {

namespace detail {

template <typename T, typename... Ts>
struct Union {
  using First = T;
  using Rest = Union<Ts...>;

  Union() {
  }
  ~Union() {
  }
  Union(const Union&) = delete;
  Union(Union&&) = delete;
  Union& operator=(Union&) = delete;
  Union& operator=(Union&&) = delete;

  template <typename X>
  void construct_from(X x) {
    if constexpr (SameAs<X, First>)
      psl::construct_at(&first, psl::move(x));
    else
      rest.construct_from(psl::move(x));
  }

  template <typename X>
  __attribute__((always_inline)) X& as() {
    if constexpr (SameAs<X, First>)
      return first;
    else
      return rest.template as<X>();
  }
  template <typename X>
  __attribute__((always_inline)) const X& as() const {
    if constexpr (SameAs<X, First>)
      return first;
    else
      return rest.template as<X>();
  }

  union {
    First first;
    Rest rest;
  };
};

template <typename T>
struct Union<T> {
  using First = T;

  Union() {
  }
  ~Union() {
  }
  Union(const Union&) = delete;
  Union(Union&&) = delete;
  Union& operator=(Union&) = delete;
  Union& operator=(Union&&) = delete;

  template <typename X>
  void construct_from(X x) {
    static_assert(SameAs<X, First>, "type X is not one of the Union's");
    psl::construct_at(&first, psl::forward<X>(x));
  }

  template <typename X>
  __attribute__((always_inline)) X& as() {
    static_assert(SameAs<X, First>, "type X is not one of the Union's");

    return first;
  }
  template <typename X>
  __attribute__((always_inline)) const X& as() const {
    static_assert(SameAs<X, First>, "type X is not one of the Union's");

    return first;
  }

  union {
    First first;
  };
};

}  // namespace detail

template <typename T, typename First, typename... Rest>
constexpr int index() {
  static_assert(SameAs<T, First> || sizeof...(Rest) != 0, "type T is not in the parameter pack");

  if constexpr (SameAs<T, First>)
    return 0;
  else
    return 1 + index<T, Rest...>();
}
template <typename T, typename First, typename... Rest>
constexpr int index_or(int fallback) {
  if constexpr (SameAs<T, First>)
    return 0;
  else {
    if constexpr (sizeof...(Rest) != 0)
      return 1 + index<T, Rest...>();
    else
      return fallback;
  }
}

template <typename VariantType, typename F, typename TargetType>
constexpr decltype(auto) dispatch_helper(VariantType&& object, F&& f) {
  return f(psl::forward<VariantType>(object).template as<TargetType>());
}

template <typename... Ts, typename VariantType, typename F>
decltype(auto) dispatch(VariantType&& object [[maybe_unused]], int index [[maybe_unused]],
                        F&& f [[maybe_unused]]) {
  using Fwt = decltype(+dispatch_helper<VariantType, F, FirstType<Ts...>>);
  constexpr static Fwt table[]{dispatch_helper<VariantType, F, Ts>...};

  return table[index](psl::forward<VariantType>(object), psl::forward<F>(f));
}

template <typename... Ts>
struct variant {
  static_assert(sizeof...(Ts) < (1 << 8), "variant only support 1 ~ 255 possible types");
  using Aggregate = detail::Union<Ts...>;

  template <typename T>
  static constexpr bool one_of() {
    return (psl::same_as<T, Ts> || ...);
  }

  variant() = default;
  ~variant() {
    reset();
  }

  variant(const variant& rhs) {
    rhs.try_dispatch([&](const auto& x) {
      value.construct_from(x);
      tag_ = rhs.tag_;
    });
  }
  variant(variant&& rhs) {
    rhs.try_dispatch([&](auto& x) {
      value.construct_from(psl::move(x));
      tag_ = rhs.tag_;
    });
  }
  variant& operator=(variant rhs) {
    reset();
    rhs.try_dispatch([&](auto& rx) {
      value.construct_from(psl::move(rx));
      tag_ = rhs.tag_;
    });
    return *this;
  }
  template <typename T>
  requires(one_of<T>())
  variant(T x) {
    value.construct_from(psl::move(x));
    tag_ = psl::index<Decay<T>, Ts...>();
  }
  template <typename T>
  requires(one_of<T>())
  variant& operator=(T x) {
    reset();
    value.construct_from(psl::move(x));
    tag_ = psl::index<Decay<T>, Ts...>();
    return *this;
  }

  template <typename F>
  decltype(auto) dispatch(F&& f) {
    return psl::dispatch<Ts...>(value, tag_, psl::forward<F>(f));
  }

  template <typename F>
  decltype(auto) dispatch(F&& f) const {
    return psl::dispatch<Ts...>(value, tag_, psl::forward<F>(f));
  }

  template <typename F>
  void try_dispatch(F&& f) {
    if (is_valid())
      psl::dispatch<Ts...>(value, tag_, psl::forward<F>(f));
  }

  template <typename F>
  void try_dispatch(F&& f) const {
    if (is_valid())
      psl::dispatch<Ts...>(value, tag_, psl::forward<F>(f));
  }

  bool is_valid() const {
    return tag_ != invalid_tag;
  }

  int tag() const {
    return tag_;
  }

  template <typename T>
  static int index() {
    return psl::index<T, Ts...>();
  }

  template <typename T>
  bool is() const {
    return tag_ == psl::index_or<T, Ts...>(-1);
  }

  template <typename T>
  T& as() {
    return value.template as<T>();
  }

  template <typename T>
  const T& as() const {
    return value.template as<T>();
  }

  void reset() {
    try_dispatch([](auto& x) {
      using T = Decay<decltype(x)>;
      x.~T();
    });
    tag_ = invalid_tag;
  }

  void* ptr() {
    return &value;
  };

private:
  Aggregate value;
  uint8_t tag_ = invalid_tag;
  static constexpr uint8_t invalid_tag = static_cast<uint8_t>(-1);
};

}  // namespace psl