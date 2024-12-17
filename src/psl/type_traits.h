#pragma once

#include <psl/stdint.h>

namespace psl {

struct Any {};

struct TrueType {
  static constexpr bool value = true;
};
struct FalseType {
  static constexpr bool value = false;
};

template <bool value, typename T, typename U>
struct _Conditional;
template <typename T, typename U>
struct _Conditional<false, T, U> {
  using Type = U;
};
template <typename T, typename U>
struct _Conditional<true, T, U> {
  using Type = T;
};
template <bool value, typename T, typename U>
using Conditional = typename _Conditional<value, T, U>::Type;

template <typename T>
struct _TypeIdentity {
  using Type = T;
};
template <typename T>
using TypeIdentity = typename _TypeIdentity<T>::Type;

template <typename... Ts>
using Voidify = void;

template <typename T, typename U>
struct _SameAs : FalseType {};
template <typename T>
struct _SameAs<T, T> : TrueType {};
template <typename T, typename U>
constexpr bool same_as = _SameAs<T, U>::value;
template <typename T, typename U>
concept SameAs = _SameAs<T, U>::value;

template <typename T>
constexpr bool is_void = same_as<T, void>;
template <typename T>
concept IsVoid = same_as<T, void>;

template <typename T, typename U>
constexpr bool different_from = !same_as<T, U>;
template <typename T, typename U>
concept DifferentFrom = !same_as<T, U>;

template <typename T>
struct _IsArray : FalseType {};
template <typename T>
struct _IsArray<T[]> : TrueType {};
template <typename T, int N>
struct _IsArray<T[N]> : TrueType {};
template <typename T>
constexpr bool is_array = _IsArray<T>::value;

template <typename T>
struct _IsFunction : FalseType {};
template <typename R, typename... Args>
struct _IsFunction<R(Args...)> : TrueType {};
template <typename R, typename... Args>
struct _IsFunction<R (*)(Args...)> : TrueType {};
template <typename T, typename R, typename... Args>
struct _IsFunction<R (T::*)(Args...)> : TrueType {};
template <typename T, typename R, typename... Args>
struct _IsFunction<R (T::*)(Args...) const> : TrueType {};
template <typename T>
constexpr bool is_function = _IsFunction<T>::value;

template <typename T>
struct _IsPointer : FalseType {};
template <typename T>
struct _IsPointer<T *> : TrueType {};
template <typename T>
constexpr bool is_pointer = _IsPointer<T>::value;

template <typename T>
struct _IsIntegral : FalseType {};
template <>
struct _IsIntegral<int8_t> : TrueType {};
template <>
struct _IsIntegral<int16_t> : TrueType {};
template <>
struct _IsIntegral<int32_t> : TrueType {};
template <>
struct _IsIntegral<int64_t> : TrueType {};
template <>
struct _IsIntegral<uint8_t> : TrueType {};
template <>
struct _IsIntegral<uint16_t> : TrueType {};
template <>
struct _IsIntegral<uint32_t> : TrueType {};
template <>
struct _IsIntegral<uint64_t> : TrueType {};
template <typename T>
constexpr bool is_integral = _IsIntegral<T>::value;
template <typename T>
concept Integral = _IsIntegral<T>::value;

template <typename T>
struct _IsFloatingPoint : FalseType {};
template <>
struct _IsFloatingPoint<float> : TrueType {};
template <>
struct _IsFloatingPoint<double> : TrueType {};
template <typename T>
constexpr bool is_floating_point = _IsFloatingPoint<T>::value;
template <typename T>
concept FloatingPoint = _IsFloatingPoint<T>::value;

template <typename T>
concept FundamentalNumerical = Integral<T> || FloatingPoint<T>;
template <typename T>
constexpr bool is_fundamental =
    FundamentalNumerical<T> || is_array<T> || is_pointer<T> || is_function<T>;
template <typename T>
concept FundamentalType = FundamentalNumerical<T> || is_array<T> || is_pointer<T> || is_function<T>;

template <typename T>
concept Arithmetic = requires(T x) {
  x *x;
  x / x;
  x + x;
  x - x;
};
template <typename T>
concept LinearArithmetic = requires(T x) {
  x + x;
  x - x;
};

template <typename T>
struct _IsLvReference : FalseType {};
template <typename T>
struct _IsLvReference<T &> : TrueType {};
template <typename T>
constexpr bool is_lv_reference = _IsLvReference<T>::value;

template <typename T>
struct _IsRvReference : FalseType {};
template <typename T>
struct _IsRvReference<T &&> : TrueType {};
template <typename T>
concept is_rv_reference = _IsRvReference<T>::value;

template <typename T>
struct _IsReference : FalseType {};
template <typename T>
struct _IsReference<T &> : TrueType {};
template <typename T>
struct _IsReference<T &&> : TrueType {};
template <typename T>
constexpr bool is_reference = _IsReference<T>::value;

template <typename T>
struct _IsConst : FalseType {};
template <typename T>
struct _IsConst<const T> : TrueType {};
template <typename T>
constexpr bool is_const = _IsConst<T>::value;

template <typename T>
struct _IsConstRef : FalseType {};
template <typename T>
struct _IsConstRef<const T &> : TrueType {};
template <typename T>
constexpr bool is_const_ref = _IsConstRef<T>::value;

template <typename T>
struct _RemoveConst {
  using Type = T;
};
template <typename T>
struct _RemoveConst<const T> {
  using Type = T;
};
template <typename T>
using RemoveConst = typename _RemoveConst<T>::Type;

template <typename T>
struct _RemoveReference {
  using Type = T;
};
template <typename T>
struct _RemoveReference<T &> {
  using Type = T;
};
template <typename T>
struct _RemoveReference<T &&> {
  using Type = T;
};
template <typename T>
using RemoveReference = typename _RemoveReference<T>::Type;

template <typename T>
struct _RemoveExtent {
  using Type = T;
};
template <typename T>
struct _RemoveExtent<T[]> {
  using Type = T;
};
template <typename T, size_t N>
struct _RemoveExtent<T[N]> {
  using Type = T;
};
template <typename T>
using RemoveExtent = typename _RemoveExtent<T>::Type;

template <typename T, bool is_array_ = is_array<T>, bool is_function_ = is_function<T>>
struct _DecaySelector;
template <typename T>
struct _DecaySelector<T, false, false> {
  using Type = RemoveConst<T>;
};
template <typename T>
struct _DecaySelector<T, true, false> {
  using Type = RemoveExtent<T> *;
};
template <typename T>
struct _DecaySelector<T, false, true> {
  using Type = T *;
};
template <typename T>
struct _Decay {
private:
  using NoRef = RemoveReference<T>;

public:
  using Type = typename _DecaySelector<NoRef>::Type;
};
template <typename T>
using Decay = typename _Decay<T>::Type;

template <typename T>
T declval() {
  return declval<T>();
}

template <typename F, typename... Args>
using ReturnType = decltype(psl::declval<F>()(psl::declval<Args>()...));

template <typename From, typename To>
constexpr bool convertible = requires(From x) { To(x); };

template <typename T>
constexpr bool copyable = requires(T x) { T(x); };
template <typename T>
constexpr bool movable = requires(T x) { T(static_cast<T&&>(x)); };
template <typename T>
constexpr bool copy_assignable = requires(T x) { x = x; };
template <typename T>
constexpr bool move_assignable = requires(T x) { x = static_cast<T&&>(x); };

template <typename Derived, typename Base>
constexpr bool derived_from = convertible<const Derived *, const Base *>;
template <typename Derived, typename Base>
concept DerivedFrom = convertible<const Derived *, const Base *>;

template <typename T>
struct _CorrespondingInt;
template <>
struct _CorrespondingInt<float> {
  using Type = int32_t;
};
template <>
struct _CorrespondingInt<double> {
  using Type = int64_t;
};
template <typename T>
using CorrespondingInt = typename _CorrespondingInt<T>::Type;

template <typename T, typename = void>
struct _IsDereferenceable {
  static constexpr bool value = false;
};
template <typename T>
struct _IsDereferenceable<T, Voidify<decltype(*psl::declval<T>())>> {
  static constexpr bool value = true;
};
template <typename T>
constexpr bool dereferenceable = _IsDereferenceable<T>::value;

template <bool value, typename... Ts>
constexpr bool deferred_bool = value;

template <int I>
struct PriorityTag : PriorityTag<I - 1> {};
template <>
struct PriorityTag<0> {};

template <typename I, I...>
struct IntegerSequence {};

template <typename T, int N, int... Is>
struct _MakeIntegerSequence : _MakeIntegerSequence<T, N - 1, N - 1, Is...> {};
template <typename T, int... Is>
struct _MakeIntegerSequence<T, 0, Is...> : IntegerSequence<T, Is...> {};

template <typename T, int N>
auto make_integer_sequence() {
  IntegerSequence seq = _MakeIntegerSequence<T, N>{};
  return seq;
}

template <typename T, int I>
struct IndexedType {
  using Type = T;
  static constexpr int index = I;
};
template <typename... Ts>
struct IndexedTypeSequence {};

template <size_t I, typename Seq, typename... Ts>
struct MakeIndexedTypeSequenceHelper;
template <size_t I, typename... Ps, typename T, typename... Ts>
struct MakeIndexedTypeSequenceHelper<I, IndexedTypeSequence<Ps...>, T, Ts...>
    : MakeIndexedTypeSequenceHelper<I + 1, IndexedTypeSequence<Ps..., IndexedType<T, I>>, Ts...> {};
template <size_t I, typename... Ps>
struct MakeIndexedTypeSequenceHelper<I, IndexedTypeSequence<Ps...>> : IndexedTypeSequence<Ps...> {};
template <typename... Ts>
auto make_indexed_type_sequence() {
  return IndexedTypeSequence{MakeIndexedTypeSequenceHelper<0, IndexedTypeSequence<>, Ts...>{}};
};

template <typename T, typename... Ts>
struct _FirstType {
  using Type = T;
};
template <typename... Ts>
using FirstType = typename _FirstType<Ts...>::Type;

template <int I, typename T, typename... Ts>
struct _NthType : _NthType<I - 1, Ts...> {};
template <typename T, typename... Ts>
struct _NthType<0, T, Ts...> {
  using Type = T;
};
template <int I, typename... Ts>
using NthType = typename _NthType<I, Ts...>::Type;

template <typename... Ts>
struct TypePack {};

template <typename T, typename TP>
constexpr bool one_of = false;
template <typename T, typename... Ts>
constexpr bool one_of<T, TypePack<Ts...>> = (SameAs<T, Ts> || ...);

template <template <typename...> typename R, typename T>
struct _CopyTemplateArguments;
template <template <typename...> typename R, typename... Ts>
struct _CopyTemplateArguments<R, TypePack<Ts...>> {
  using Type = R<Ts...>;
};
template <template <typename...> typename R, typename T>
using CopyTemplateArguments = typename _CopyTemplateArguments<R, T>::Type;

template <typename T, typename U, char C>
struct _OpResult;
template <typename T, typename U, char C>
using OpResult = typename _OpResult<T, U, C>::Type;
template <typename T, typename U>
struct _OpResult<T, U, '+'> {
  using Type = decltype(declval<T>() + declval<U>());
};
template <typename T, typename U>
struct _OpResult<T, U, '-'> {
  using Type = decltype(declval<T>() - declval<U>());
};
template <typename T, typename U>
struct _OpResult<T, U, '*'> {
  using Type = decltype(declval<T>() * declval<U>());
};
template <typename T, typename U>
struct _OpResult<T, U, '/'> {
  using Type = decltype(declval<T>() / declval<U>());
};
template <typename T, typename U>
struct _OpResult<T, U, '%'> {
  using Type = decltype(declval<T>() & declval<U>());
};

template <typename T>
struct _MpClassType;
template <typename T>
using MpClassType = typename _MpClassType<T>::Type;
template <typename T, typename R, typename... Args>
struct _MpClassType<R (T::*)(Args...)> {
  using Type = T;
};
template <typename T>
struct _MpReturnType;
template <typename T>
using MpReturnType = typename _MpReturnType<T>::Type;
template <typename T, typename R, typename... Args>
struct _MpReturnType<R (T::*)(Args...)> {
  using Type = R;
};

template <typename... Ts>
constexpr bool same_type = false;
template <>
inline constexpr bool same_type<> = true;
template <typename T>
constexpr bool same_type<T> = true;
template <typename T, typename... Ts>
constexpr bool same_type<T, T, Ts...> = same_type<T, Ts...>;

template <typename T>
concept DefaultConstructible = requires { T(); };
template <typename T>
concept Destructible = requires(T x) { x.~T(); };
template <typename T>
concept MoveConstructible = requires(T x) { T(static_cast<T &&>(x)); };
template <typename T>
concept CopyConstructible = MoveConstructible<T> && requires(T x) { T(x); };
template <typename T>
concept MoveAssignable = requires(T x) { x = static_cast<T &&>(x); };
template <typename T>
concept CopyAssignable = MoveAssignable<T> && requires(T x) { x = x; };
template <typename T>
concept Movable = MoveConstructible<T> && MoveAssignable<T>;
template <typename T>
concept Copyable = CopyConstructible<T> && CopyAssignable<T>;
template <typename T, typename U>
concept EqualityComparable = requires(T a, U b) {
  { a == b } -> SameAs<bool>;
  { a != b } -> SameAs<bool>;
};
template <typename T, typename U>
concept Comparable = requires(T a, U b) {
  { a == b } -> SameAs<bool>;
  { a != b } -> SameAs<bool>;
  { a < b } -> SameAs<bool>;
  { a > b } -> SameAs<bool>;
  { a <= b } -> SameAs<bool>;
  { a >= b } -> SameAs<bool>;
};
template <typename T>
concept Semiregular = Copyable<T> && DefaultConstructible<T>;
template <typename T>
concept Regular = Semiregular<T> && EqualityComparable<T, T>;

template <typename T>
struct IteratorTraits;
template <typename T>
using IteratorValueType = typename IteratorTraits<T>::ValueType;
template <typename T>
using IteratorReferenceType = typename IteratorTraits<T>::ReferenceType;
template <typename T>
using IteratorDifferenceType = typename IteratorTraits<T>::DifferenceType;

template <typename T>
struct IteratorTraits<T *> {
  using ValueType = T;
  using ReferenceType = T &;
  using DifferenceType = ptrdiff_t;
};

template <typename T>
requires requires {
  typename T::ValueType;
  typename T::ReferenceType;
  typename T::DifferenceType;
}
struct IteratorTraits<T> {
  using ValueType = typename T::ValueType;
  using ReferenceType = typename T::ReferenceType;
  using DifferenceType = typename T::DifferenceType;
};

template <typename T>
concept ForwardIterator =
    Copyable<T> && dereferenceable<T> && EqualityComparable<T, T> && requires(T it) {
      typename IteratorValueType<T>;
      { ++it } -> SameAs<T &>;
      it++;
    };
template <typename T>
concept BackwardIterator =
    Copyable<T> && dereferenceable<T> && EqualityComparable<T, T> && requires(T it) {
      typename IteratorValueType<T>;
      { --it } -> SameAs<T &>;
      it--;
    };
template <typename T>
concept BidirectionalIterator =
    Copyable<T> && dereferenceable<T> && EqualityComparable<T, T> && requires(T it) {
      typename IteratorValueType<T>;
      { ++it } -> SameAs<T &>;
      { --it } -> SameAs<T &>;
      it++;
      it--;
    };
template <typename T>
concept OutputIterator =
    ForwardIterator<T> && requires(T it) { *it = declval<IteratorValueType<T>>(); };
template <typename T>
concept RandomAccessIterator =
    BidirectionalIterator<T> && requires(T it, IteratorDifferenceType<T> n) {
      { it[n] } -> SameAs<IteratorReferenceType<T>>;
    };

}  // namespace psl
