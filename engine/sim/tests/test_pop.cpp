// Spec 28: pop identity, random-access life, cohort permutation.
#include <doctest.h>

#include <algorithm>
#include <numeric>
#include <set>
#include <vector>

#include "zs/pop.h"

using namespace zs;

TEST_CASE("keyed draws are pure: order of calls never matters (A1)") {
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) == keyed_u64(7, Purpose::PopLife, 42, 1));
  const auto first = keyed_u64(7, Purpose::PopLife, 42, 1);
  for (int i = 0; i < 100; ++i) keyed_u64(7, Purpose::PopLife, 1000 + i, 2);  // other draws in between
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) == first);
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) != keyed_u64(8, Purpose::PopLife, 42, 1));
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) != keyed_u64(7, Purpose::PopRank, 42, 1));
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) != keyed_u64(7, Purpose::PopLife, 43, 1));
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) != keyed_u64(7, Purpose::PopLife, 42, 2));
}

TEST_CASE("pop ids: census ids and born ids are distinct, stable and invertible (A1)") {
  std::set<PopId> seen;
  for (std::uint32_t s = 0; s < 1000; ++s) {
    CHECK(seen.insert(census_pop_id(s)).second);
    CHECK_FALSE(is_born(census_pop_id(s)));
    CHECK(generation_of(census_pop_id(s)) == 0);
    CHECK(mother_of(census_pop_id(s)) == kNoPop);
  }
  for (std::uint32_t s = 0; s < 200; ++s) {
    const PopId founder = census_pop_id(s * 5);
    for (unsigned k = 0; k < 15; ++k) {
      const PopId child = birth_pop_id(founder, k);
      REQUIRE(child != kNoPop);
      CHECK(child == birth_pop_id(founder, k));
      CHECK(seen.insert(child).second);
      CHECK(is_born(child));
      CHECK(generation_of(child) == 1);
      CHECK(mother_of(child) == founder);
      CHECK(child_index_of(child) == k);
      CHECK(founder_slot_of(child) == s * 5);
      const PopId grandchild = birth_pop_id(child, (k * 3) % 15);
      REQUIRE(grandchild != kNoPop);
      CHECK(generation_of(grandchild) == 2);
      CHECK(mother_of(grandchild) == child);
      CHECK(child_index_of(grandchild) == (k * 3) % 15);
      CHECK(founder_slot_of(grandchild) == s * 5);
    }
  }
}

TEST_CASE("born ids walk seven generations and refuse the eighth and the sixteenth child (A1)") {
  PopId p = census_pop_id(77);
  std::vector<PopId> chain{p};
  for (unsigned g = 1; g <= 7; ++g) {
    p = birth_pop_id(p, g % 15);
    REQUIRE(p != kNoPop);
    CHECK(generation_of(p) == g);
    chain.push_back(p);
  }
  CHECK(birth_pop_id(p, 0) == kNoPop);                       // generation 8
  CHECK(birth_pop_id(census_pop_id(77), 15) == kNoPop);      // child number 15
  for (unsigned g = 7; g >= 1; --g) {
    CHECK(mother_of(chain[g]) == chain[g - 1]);
    CHECK(child_index_of(chain[g]) == g % 15);
  }
}

TEST_CASE("derive is pure: the same pop is the same in any order of evaluation (A1)") {
  const int n = 10000;
  std::vector<PopBase> in_order(n);
  for (int i = 0; i < n; ++i) in_order[i] = derive_base(5, census_pop_id(i), -30.0 + i * 0.001, 30.0 - i * 0.001);
  std::vector<int> order(n);
  std::iota(order.begin(), order.end(), 0);
  // A fixed shuffle (no std distributions: they differ across standard libraries).
  for (int i = n - 1; i > 0; --i) std::swap(order[i], order[keyed_u64(11, Purpose::Test, i) % (i + 1)]);
  for (int i : order) {
    const PopBase p = derive_base(5, census_pop_id(i), -30.0 + i * 0.001, 30.0 - i * 0.001);
    CHECK(p.sex == in_order[i].sex);
    CHECK(p.birth_year == in_order[i].birth_year);
    CHECK(p.death_year == in_order[i].death_year);
  }
}

TEST_CASE("lifetimes sit near real life expectancy and sex shares near birth ratios (A3)") {
  const int n = 100000;
  double sum_f = 0, sum_m = 0;
  int nf = 0, nm = 0;
  for (int i = 0; i < n; ++i) {
    const PopBase p = derive_base(3, birth_pop_id(census_pop_id(static_cast<std::uint32_t>(i)), 0), 0.0);
    CHECK(p.death_year > 0.0);
    if (p.sex == PopSex::Female) { sum_f += p.death_year; ++nf; } else { sum_m += p.death_year; ++nm; }
  }
  CHECK(static_cast<double>(nm) / n == doctest::Approx(0.512).epsilon(0.02));
  CHECK(sum_f / nf > 78.0);
  CHECK(sum_f / nf < 84.0);
  CHECK(sum_m / nm > 72.0);
  CHECK(sum_m / nm < 78.0);
}

TEST_CASE("a pop known to be alive at age a never dies before a, and lives longer on average (A1, A3)") {
  const int n = 20000;
  double sum_from_birth = 0, sum_alive60 = 0;
  for (int i = 0; i < n; ++i) {
    const PopId id = census_pop_id(i);
    const PopBase young = derive_base(4, id, 0.0);
    const PopBase old = derive_base(4, id, -60.0, 60.0);
    CHECK(old.death_year >= 0.0);  // born at -60, alive at 60 means dying at or after year 0
    sum_from_birth += young.death_year;
    sum_alive60 += old.death_year + 60.0;
  }
  CHECK(sum_alive60 > sum_from_birth);
}

TEST_CASE("cohort permutation is a bijection onto 0..n-1 and inverse undoes forward (A1)") {
  for (std::uint64_t n : {1ull, 2ull, 3ull, 7ull, 10ull, 64ull, 100ull, 1000ull, 4097ull, 100000ull}) {
    const CohortPermutation perm(21, 1000 + n, n);
    std::vector<char> seen(n, 0);
    for (std::uint64_t i = 0; i < n; ++i) {
      const std::uint64_t r = perm.forward(i);
      REQUIRE(r < n);
      CHECK_FALSE(seen[r]);
      seen[r] = 1;
      CHECK(perm.inverse(r) == i);
    }
  }
}

TEST_CASE("an aggregate event hitting k members is exactly k members, by rank and by inverse (A1)") {
  for (int e = 0; e < 200; ++e) {
    const std::uint64_t n = 10 + keyed_u64(1, Purpose::Test, e, 0) % 20000;
    const std::uint64_t k = keyed_u64(1, Purpose::Test, e, 1) % (n + 1);
    const CohortPermutation perm(33, e, n);
    std::uint64_t counted = 0;
    for (std::uint64_t i = 0; i < n; ++i) counted += perm.hit(i, k) ? 1 : 0;
    CHECK(counted == k);
    std::set<std::uint64_t> by_inverse;
    for (std::uint64_t r = 0; r < k; ++r) by_inverse.insert(perm.inverse(r));
    CHECK(by_inverse.size() == k);
    for (std::uint64_t m : by_inverse) CHECK(perm.hit(m, k));
  }
}

TEST_CASE("permutations differ between cohorts and seeds, and put a member's rank uniformly (A3)") {
  const std::uint64_t n = 10;
  std::vector<int> counts(n, 0);
  const int cohorts = 20000;
  for (int c = 0; c < cohorts; ++c) ++counts[CohortPermutation(77, c, n).forward(0)];
  for (int v : counts) CHECK(v == doctest::Approx(cohorts / static_cast<double>(n)).epsilon(0.1));
  bool differs = false;
  for (std::uint64_t i = 0; i < n; ++i)
    differs = differs || CohortPermutation(77, 1, n).forward(i) != CohortPermutation(78, 1, n).forward(i);
  CHECK(differs);
}

TEST_CASE("pinned known answers, the same on GCC and Clang (A1 across compilers)") {
  CHECK(keyed_u64(7, Purpose::PopLife, 42, 1) == 11686822105769717297ull);
  CHECK(birth_pop_id(census_pop_id(12), 3) == 0x9000000400000000ull + 12);
  const CohortPermutation p(21, 5, 1000);
  CHECK(p.forward(0) == 896);
  CHECK(p.forward(1) == 891);
  CHECK(p.forward(999) == 783);
  const PopBase b = derive_base(5, census_pop_id(17), -30.0, 30.0);
  CHECK(b.sex == PopSex::Female);
  CHECK(b.death_year == 65.073244276057125);
}
