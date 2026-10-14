// zs --food [--seed N] [--days D] [--errand-day E]: the one world's food chain day by day (spec 80, decision 0079).
// Each noon: the distribution centre's and the outlets' stock in days of what they sell, the block's pantry in days for
// its people, what the town has eaten. On the errand day a pop of the block goes to the largest outlet and brings food home.
#include "food_mode.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "zs/body.h"
#include "zs/clock.h"
#include "zs/events.h"
#include "zs/food_chain.h"
#include "zs/gen/place.h"
#include "zs/tier0.h"
#include "zs/world.h"

namespace zs::atlas {

namespace {

const char* arg(int argc, char** argv, const char* key, const char* fallback) {
  for (int i = 1; i + 1 < argc; ++i)
    if (!std::strcmp(argv[i], key)) return argv[i + 1];
  return fallback;
}

}  // namespace

int food_main(int argc, char** argv) {
  const std::uint64_t seed = std::strtoull(arg(argc, argv, "--seed", "1"), nullptr, 10);
  const int days = std::atoi(arg(argc, argv, "--days", "5"));
  const int errand_day = std::atoi(arg(argc, argv, "--errand-day", "2"));
  tier0::Tier0 t = gen::make_world(seed);
  if (!t.food) {
    std::printf("this world has no town food chain\n");
    return 1;
  }
  World& w = *t.world;
  w.check_every_tick = false;
  const FoodChain& c = *t.food;
  const FoodSetup& s = c.food();
  const double day_j = s.town_people * s.kj_per_person_day * 1000.0;
  std::printf("The food of the block's town (spec 80): %.0f people (%.0f of them on the block), %zu outlets; the distribution "
              "centre %.1f h away holding %.1f days\n",
              s.town_people, s.block_people, s.outlets.size(), s.centre_hours, s.centre_days);
  std::size_t big = 0;
  for (std::size_t o = 0; o < s.outlets.size(); ++o)
    if (s.outlets[o].share > s.outlets[big].share) big = o;
  const auto pantry_owner = t.actors->first_store("pantry");
  const auto& names = w.ledger.resource_names();
  const auto food = static_cast<ResourceId>(std::find(names.begin(), names.end(), std::string("food_j")) - names.begin());
  for (int d = 0; d <= days; ++d) {
    w.jump(d * kTicksPerGameDay + 4 * kTicksPerGameHour);  // noon (tick 0 is 08:00)
    if (d == errand_day && !t.group.empty()) {
      w.submit({w.clock.tick, FoodErrand, t.group.front(), static_cast<std::int64_t>(big), 0});
      w.run_ticks(1);
      for (const Event& e : w.log.all())
        if (e.type == EventType::FoodErrandBegan || e.type == EventType::FoodRefused)
          std::printf("  day %d: %s to %s\n", d, e.type == EventType::FoodErrandBegan ? "an errand set out" : "the errand was refused",
                      s.outlets[big].name.c_str());
    }
    const double pantry = pantry_owner ? static_cast<double>(w.ledger.balance(*pantry_owner, food)) : 0;
    std::printf("day %d noon: centre %.1f days; %s %.1f days; block pantry %.1f days for its %.0f people; the town ate %.0f people-days\n",
                d, static_cast<double>(c.centre_stock(w)) / day_j, s.outlets[big].name.c_str(),
                static_cast<double>(c.outlet_stock(w, big)) / std::max(1.0, day_j * s.outlets[big].share),
                pantry / std::max(1.0, s.block_people * s.kj_per_person_day * 1000.0), s.block_people,
                static_cast<double>(c.eaten_by_town(w)) / (s.kj_per_person_day * 1000.0));
  }
  for (const Event& e : w.log.all())
    if (e.type == EventType::FoodErrandEnded)
      std::printf("the errand came back with %.0f MJ (%.1f person-days)\n", static_cast<double>(e.b) / 1e6,
                  static_cast<double>(e.b) / (s.kj_per_person_day * 1000.0));
  return 0;
}

}  // namespace zs::atlas
