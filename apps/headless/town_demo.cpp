// A generated town run as pops inside a world: the content generators, the world and the pop levels together.
#include "town_demo.h"

#include <algorithm>
#include <cstdio>
#include <map>

#include "zs/gen/registry_bridge.h"
#include "zs/pop_system.h"

namespace {

void pyramid(const zs::gen::Town& t) {
  std::map<int, std::pair<int, int>> by_decade;  // decade -> (women, men)
  for (const auto& p : t.pops) (p.sex == 'f' ? by_decade[p.age / 10].first : by_decade[p.age / 10].second)++;
  std::puts("age pyramid (women | men):");
  for (auto it = by_decade.rbegin(); it != by_decade.rend(); ++it)
    std::printf("  %2d-%-2d  %5d | %-5d\n", it->first * 10, it->first * 10 + 9, it->second.first, it->second.second);
}

}  // namespace

int town_demo(std::uint64_t seed, int people, double years) {
  const std::string data = ZS_DATA_DIR;
  const zs::gen::Catalog cat = zs::gen::load_catalog(data + "/gen");
  zs::gen::Mix mix = zs::gen::load_mix(data + "/gen/populations/tier0_block.toml");
  mix.people = people;
  mix.on_map_homes.clear();
  const zs::gen::Town town = zs::gen::generate_town(cat, mix, seed);

  zs::World world(seed);
  auto pops = std::make_unique<zs::PopSystem>(world);
  const zs::gen::TownRegistryMap map = zs::gen::populate_registry(town, pops->setup(world), 0, 1);
  world.add_system(std::move(pops));
  world.close_setup();
  world.check_every_tick = true;
  const zs::PopRegistry& reg = world.system<zs::PopSystem>()->registry();

  std::printf("%s (seed %llu): %zu people, %zu households, %zu businesses and schools\n", town.name.c_str(),
              static_cast<unsigned long long>(seed), town.pops.size(), town.households.size(), town.businesses.size());
  std::printf("%zu have a job and %zu a school place; %zu named a workplace the directory does not list\n", map.employed, map.students,
              map.unmatched_workplaces);
  pyramid(town);

  std::vector<std::pair<std::uint64_t, std::string>> sizes;
  for (std::size_t b = 0; b < town.businesses.size(); ++b)
    sizes.push_back({reg.roster(zs::gen::workplace_roster(b))->size(0.0), town.businesses[b].name});
  std::sort(sizes.rbegin(), sizes.rend());
  std::puts("biggest employers (staff):");
  for (std::size_t i = 0; i < std::min<std::size_t>(5, sizes.size()); ++i)
    std::printf("  %5llu  %s\n", static_cast<unsigned long long>(sizes[i].first), sizes[i].second.c_str());

  // A Macro-style order: 15% of the town's jobs go. It is a command, so it is replayable and has a reason.
  std::uint64_t jobs = 0;
  for (std::size_t b = 0; b < town.businesses.size(); ++b) jobs += reg.roster(zs::gen::workplace_roster(b))->size(0.0);
  const auto lose = static_cast<std::int64_t>(jobs * 15 / 100);
  const std::int64_t aimed = static_cast<std::int64_t>(zs::EffectKind::LaidOff) |
                             ((static_cast<std::int64_t>(zs::RosterKind::Workplace) + 1) << 8);
  world.run_ticks(zs::kTicksPerGameDay);
  world.submit({0, zs::RegionApply, 0, aimed, lose});
  world.run_ticks(zs::kTicksPerGameDay);
  const double now = zs::year_of_tick(world.clock.tick);
  std::printf("\nday 1: order to cut %lld of %llu jobs (15%%) -> %llu laid off; %zu households lost an earner\n",
              static_cast<long long>(lose), static_cast<unsigned long long>(jobs),
              static_cast<unsigned long long>(reg.roster_count_region(0, zs::EffectKind::LaidOff, now)),
              [&] {
                std::size_t n = 0;
                for (const auto& h : town.households)
                  for (std::size_t m : h.members) {
                    zs::PopSnapshot s;
                    if (reg.snapshot(map.pop_ids[m], now, s) && (s.roster_bits & 1u)) { ++n; break; }
                  }
                return n;
              }());
  for (std::size_t i = 0; i < town.pops.size(); ++i) {
    zs::PopSnapshot s;
    if (!reg.snapshot(map.pop_ids[i], now, s) || !(s.roster_bits & 1u)) continue;
    std::uint64_t reason = 0;
    const auto* roster = reg.roster(zs::gen::workplace_roster(static_cast<std::size_t>(map.business_of_pop[i])));
    if (roster) reason = roster->reason_of(map.pop_ids[i], zs::EffectKind::LaidOff, now);
    std::printf("\none of them, with the order that caused it (event %llu, %s):\n%s", static_cast<unsigned long long>(reason),
                reason && world.log.get(reason) ? "an Intent" : "unknown", zs::gen::dossier(town, town.pops[i]).c_str());
    break;
  }

  const zs::DemographyCounts c = reg.project_demography(0.0, years);
  const double per_k = 1000.0 / (static_cast<double>(c.alive_at_start) * years);
  std::printf("\n%.0f game years with no outbreak: population %llu -> %llu; %llu births and %llu deaths (%.1f and %.1f per 1000 per year)\n",
              years, static_cast<unsigned long long>(c.alive_at_start), static_cast<unsigned long long>(c.alive_at_end),
              static_cast<unsigned long long>(c.births), static_cast<unsigned long long>(c.deaths), c.births * per_k, c.deaths * per_k);
  std::printf("alive by generation below the town's people:");
  for (int g = 0; g < 8; ++g) std::printf(" %llu", static_cast<unsigned long long>(c.by_generation_alive[g]));
  std::printf("\nworld state hash %llu\n", static_cast<unsigned long long>(world.state_hash()));
  return 0;
}
