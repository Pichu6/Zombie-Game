// Spec 01: quantities with units.
#include <doctest.h>
#include "property.h"

#include "zs/quantity.h"

using namespace zs;

template <class A, class B>
concept Addable = requires(A a, B b) { a + b; };
template <class To, class From>
concept Assignable = requires(To t, From f) { t = f; };

// Compile-time checks: mixed units and continuous-into-conserved do not compile. (A1)
static_assert(Addable<Grams, Grams>);
static_assert(!Addable<Grams, Millilitres>);
static_assert(!Addable<Grams, Kilojoules>);
static_assert(!Assignable<Grams, Real<unit::Gram>>);
static_assert(!Assignable<Grams, double>);

TEST_CASE("divide keeps the remainder: split and merge returns the original exactly (A1)") {
  property("property", [](std::int64_t value, std::int64_t parts) {
    RC_PRE(parts > 0);
    const Grams g(value);
    const auto [q, r] = divide(g, parts);
    RC_ASSERT(q.value * parts + r.value == g.value);
  });
}
