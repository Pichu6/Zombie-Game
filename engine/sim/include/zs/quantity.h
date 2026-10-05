// Quantities with units. Spec: docs/tier0/specs/01-quantity.md
#pragma once
#include <cstdint>
#include <type_traits>

namespace zs {

// Unit tags. Conserved values use integer base units (g, mL, kJ, counts, ticks).
namespace unit {
struct Gram {};
struct Millilitre {};
struct Kilojoule {};
struct Count {};
struct Tick {};
struct Metre {};
struct Second {};
struct Watt {};
struct Decibel {};
struct Lux {};
struct Kelvin {};
struct PerSecond {};  // hazard rates
}  // namespace unit

template <class Unit, class Rep>
struct Quantity {
  static_assert(std::is_same_v<Rep, std::int64_t> || std::is_same_v<Rep, double>,
                "Quantity is either Integer (int64) or Continuous (double)");
  using unit_type = Unit;
  using rep_type = Rep;
  Rep value{};

  constexpr Quantity() = default;
  constexpr explicit Quantity(Rep v) : value(v) {}

  constexpr Quantity operator+(Quantity o) const { return Quantity(value + o.value); }
  constexpr Quantity operator-(Quantity o) const { return Quantity(value - o.value); }
  constexpr Quantity& operator+=(Quantity o) { value += o.value; return *this; }
  constexpr Quantity& operator-=(Quantity o) { value -= o.value; return *this; }
  constexpr Quantity operator*(Rep k) const requires std::is_same_v<Rep, double> { return Quantity(value * k); }
  constexpr auto operator<=>(const Quantity&) const = default;
};

// Integer (conserved) and Continuous quantities.
template <class Unit> using Amount = Quantity<Unit, std::int64_t>;
template <class Unit> using Real = Quantity<Unit, double>;

using Grams = Amount<unit::Gram>;
using Millilitres = Amount<unit::Millilitre>;
using Kilojoules = Amount<unit::Kilojoule>;
using Count = Amount<unit::Count>;
using Ticks = Amount<unit::Tick>;

// Integer division never rounds silently: the caller decides where the remainder goes.
template <class Unit>
struct DivResult {
  Amount<Unit> quotient;
  Amount<Unit> remainder;
};

template <class Unit>
constexpr DivResult<Unit> divide(Amount<Unit> a, std::int64_t parts) {
  return {Amount<Unit>(a.value / parts), Amount<Unit>(a.value % parts)};
}

}  // namespace zs
