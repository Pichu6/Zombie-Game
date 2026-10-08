// Body lab: scripted scenarios that check the body against its calibration bands.
// Used by the tests (stage 3 "done when") and by `zs --body-lab`.
// Specs: docs/tier0/specs/11-body-composition.md, 12-body-wounds.md, 13-capabilities.md
#pragma once
#include <memory>
#include <string>
#include <vector>

#include "zs/body.h"

namespace zs::body_lab {

// A world with only the body system, a pantry and a tap.
struct Lab {
  std::unique_ptr<World> world;
  Bodies* bodies = nullptr;
  OwnerId pantry = 0, tap = 0;
};
Lab make_lab(std::uint64_t seed, const std::vector<BodySpec>& specs);
// Runs n ticks, then checks the ledger (throws on a violation).
void advance(Lab& lab, std::int64_t n);

// A day plan: asleep 23:00-07:00, otherwise the given activity.
void daily_routine(Lab& lab, EntityId body, const Activity& awake);
// Tops up water (and food, if `feed`) once an hour, like a careful player would.
void keep_supplied(Lab& lab, EntityId body, bool feed, bool water);

struct Result {
  std::string name;
  std::string band;   // what the calibration source says
  double value = 0;
  bool pass = false;
  std::string unit;
};

Result total_fast_14d(std::uint64_t seed = 1);         // kg lost, 4-8
Result semi_starvation_14d(std::uint64_t seed = 1);    // kg lost at ~1600 kcal/day, 1.5-4.5
Result no_water_critical_day(std::uint64_t seed = 1);  // first day at >= 35 % water deficit, 2-4
Result thigh_bleed_untreated(std::uint64_t seed = 1);  // hours to death, 1-8
Result thigh_bleed_dressed(std::uint64_t seed = 1);    // blood lost fraction after 24 h, survives (< 0.45)
Result infection_rate(Cleaned how, std::uint64_t seed = 1);  // share of 100 lacerations infected in 7 days
Result awake_40h(std::uint64_t seed = 1);              // consciousness after 40 h awake, <= 0.6
Result heavy_haul(std::uint64_t seed = 1);             // energy ratio 110 kg / 60 kg on the same haul, > 1
Result the_wall(std::uint64_t seed = 1);               // fasted / fed minutes to exhaustion, < 1
Result load_on_light_vs_strong(std::uint64_t seed = 1);  // light / strong speed with 30 kg, < 1
Result fracture_vs_hand(std::uint64_t seed = 1);       // moving with a shin fracture, < 0.6 (hand wound: 1)
Result lean_outruns_heavy(std::uint64_t seed = 1);     // lean / heavy run speed, > 1

std::vector<Result> run_all(std::uint64_t seed = 1);

}  // namespace zs::body_lab
