#include "zs/pop_body.h"

#include <algorithm>

#include "zs/detmath.h"

namespace zs {

namespace {
// Roughly normal, mean 0, sd 1: four keyed uniforms (Irwin-Hall); `which` picks the draw, so each value has its own key.
double normal(std::uint64_t seed, PopId id, std::uint64_t which) {
  double s = 0;
  for (std::uint64_t i = 0; i < 4; ++i) s += keyed_uniform01(seed, Purpose::PopBody, id, which * 4 + i);
  return (s - 2.0) / detmath::sqrt(1.0 / 3.0);
}
}  // namespace

BodySpec body_spec_for(std::uint64_t seed, PopId id, double age, PopSex pop_sex) {
  BodySpec s;
  s.sex = body_sex(pop_sex);
  s.age = age;
  s.walker = false;
  const bool male = pop_sex == PopSex::Male;
  if (age >= 18) {
    s.height_m = (male ? 1.75 : 1.62) + (male ? 0.075 : 0.07) * normal(seed, id, 0);
    const double bmi = std::clamp(27.5 * detmath::exp(0.18 * normal(seed, id, 1)), 16.0, 50.0);
    s.mass_kg = bmi * s.height_m * s.height_m;
    s.fat_fraction = std::clamp((1.2 * bmi + 0.23 * age - 10.8 * (male ? 1 : 0) - 5.4) / 100, 0.05, 0.55);
    s.vo2max = std::clamp((male ? 48 : 40) - (male ? 0.3 : 0.27) * (age - 20) + 4 * normal(seed, id, 2), 15.0, 65.0);
  } else {
    s.height_m = std::clamp(0.80 + 0.058 * age + 0.05 * normal(seed, id, 0), 0.9, 1.85);
    const double bmi = std::clamp(15.5 + 0.25 * age + 2 * normal(seed, id, 1), 13.0, 35.0);
    s.mass_kg = bmi * s.height_m * s.height_m;
    s.fat_fraction = std::clamp((1.51 * bmi - 0.70 * age - 3.6 * (male ? 1 : 0) + 1.4) / 100, 0.08, 0.45);
    s.vo2max = male ? 47 : 42;
  }
  return s;
}

}  // namespace zs
