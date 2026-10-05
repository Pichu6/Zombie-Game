// Spec 02: keyed randomness.
#include <doctest.h>

#include <array>

#include "zs/detmath.h"
#include "zs/draw.h"
#include "zs/philox.h"

using namespace zs;

TEST_CASE("Philox-4x32-10 known-answer vectors from Random123 (A1 across compilers)") {
  CHECK(philox4x32_10({0, 0, 0, 0}, {0, 0}) == PhiloxCounter{0x6627e8d5, 0xe169c58d, 0xbc57ac4c, 0x9b00dbd8});
  CHECK(philox4x32_10({0xffffffff, 0xffffffff, 0xffffffff, 0xffffffff}, {0xffffffff, 0xffffffff}) ==
        PhiloxCounter{0x408f276d, 0x41c83b0e, 0xa20bc7c6, 0x6d5451fd});
  CHECK(philox4x32_10({0x243f6a88, 0x85a308d3, 0x13198a2e, 0x03707344}, {0xa4093822, 0x299f31d0}) ==
        PhiloxCounter{0xd16cfe09, 0x94fdcceb, 0x5001e420, 0x24126ea1});
}

TEST_CASE("streams are independent: drawing one purpose never shifts another (A1)") {
  Rng a(42), b(42);
  for (int i = 0; i < 100; ++i) a.next_u64(7, Purpose::BiteHit);  // only `a` uses BiteHit
  for (int i = 0; i < 10; ++i) CHECK(a.next_u64(7, Purpose::WoundInfection) == b.next_u64(7, Purpose::WoundInfection));
  CHECK(Rng(42).next_u64(1, Purpose::Test) != Rng(43).next_u64(1, Purpose::Test));
  CHECK(Rng(42).next_u64(1, Purpose::Test) != Rng(42).next_u64(2, Purpose::Test));
}

TEST_CASE("sampler moments over 1e6 draws (A2)") {
  Rng r(7);
  const int n = 1000000;
  double s_u = 0, s_e = 0, s_n = 0, s_n2 = 0;
  for (int i = 0; i < n; ++i) {
    s_u += r.uniform01(1, Purpose::Test);
    s_e += r.exponential(2, Purpose::Test, 4.0);
    const double z = r.normal(3, Purpose::Test, 10.0, 2.0);
    s_n += z;
    s_n2 += z * z;
  }
  const double mean_n = s_n / n;
  CHECK(s_u / n == doctest::Approx(0.5).epsilon(0.002));
  CHECK(s_e / n == doctest::Approx(0.25).epsilon(0.005));
  CHECK(mean_n == doctest::Approx(10.0).epsilon(0.001));
  CHECK(detmath::sqrt(s_n2 / n - mean_n * mean_n) == doctest::Approx(2.0).epsilon(0.005));

  std::array<int, 6> counts{};
  for (int i = 0; i < 600000; ++i) ++counts[r.uniform_int(4, Purpose::Test, 6)];
  for (int c : counts) CHECK(c == doctest::Approx(100000).epsilon(0.01));

  const std::array<double, 3> w = {1.0, 2.0, 7.0};
  std::array<int, 3> picks{};
  for (int i = 0; i < 100000; ++i) ++picks[r.weighted_choice(5, Purpose::Test, w)];
  CHECK(picks[2] == doctest::Approx(70000).epsilon(0.02));
}

TEST_CASE("relabelling an entity changes its draws, never their distribution (A2, metamorphic)") {
  Rng r(9);
  double s1 = 0, s2 = 0;
  for (int i = 0; i < 200000; ++i) {
    s1 += r.uniform01(100, Purpose::Test);
    s2 += r.uniform01(999999, Purpose::Test);
  }
  CHECK(s1 / 200000 == doctest::Approx(s2 / 200000).epsilon(0.01));
}
