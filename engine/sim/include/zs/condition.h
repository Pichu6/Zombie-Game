// Condition: wear and damage, separate from quality. Spec: docs/tier0/specs/07-condition.md
#pragma once
#include <algorithm>
#include <cstdint>

namespace zs {

inline constexpr std::int64_t kConditionFull = 1'000'000;  // parts per million

struct Condition {
  std::int64_t value = kConditionFull;
  std::int64_t max = kConditionFull;

  bool failed(std::int64_t threshold) const { return value < threshold; }

  // Wear from use: base x load x (1.5 - Q/100). Higher Q wears less. Truncates toward zero.
  // Returns the condition lost.
  std::int64_t wear(double base_ppm, double load, int quality) {
    const double amount = base_ppm * load * (1.5 - quality / 100.0);
    const std::int64_t loss = std::clamp<std::int64_t>(static_cast<std::int64_t>(amount), 0, value);
    value -= loss;
    return loss;
  }

  // Direct damage (hits, salvage loss).
  void damage(std::int64_t ppm) { value -= std::clamp<std::int64_t>(ppm, 0, value); }

  // Repair: restores up to max, then lowers max by `max_loss` (a repaired thing is never as new).
  void repair(std::int64_t ppm, std::int64_t max_loss) {
    value = std::min(max, value + std::max<std::int64_t>(ppm, 0));
    max = std::max<std::int64_t>(0, max - std::max<std::int64_t>(max_loss, 0));
    value = std::min(value, max);
  }
};

}  // namespace zs
