// The block's place on a generated planet. Spec: docs/tier0/specs/42-place.md
#include "zs/gen/place.h"

#include <cmath>
#include <optional>

#include "zs/fields.h"

namespace zs::gen {

namespace {

constexpr double kTargetMeanC = 12.0;
constexpr double kPi = 3.14159265358979323846;
constexpr std::uint32_t kPlaceTieField = 1000;  // keyed draw field for ties (fields.h numbers stay below 100)

bool temperate(Biome b) { return b == Biome::TemperateForest || b == Biome::Grassland; }

}  // namespace

CellId choose_place(const WorldGrid& grid, const WorldFields& f, std::uint64_t seed) {
  std::optional<CellId> best, best_land;
  auto better = [&](CellId c, const std::optional<CellId>& than) {
    if (!than) return true;
    const double dc = std::abs(f.temp_mean_c[c] - kTargetMeanC), dt = std::abs(f.temp_mean_c[*than] - kTargetMeanC);
    if (dc != dt) return dc < dt;
    return fields::keyed_unit(seed, c, kPlaceTieField, 0) < fields::keyed_unit(seed, *than, kPlaceTieField, 0);
  };
  for (CellId c = 0; c < grid.size(); ++c) {
    if (!f.land(c)) continue;
    if (better(c, best_land)) best_land = c;
    if (temperate(f.biome[c]) && better(c, best)) best = c;
  }
  if (best) return *best;
  if (best_land) return *best_land;
  return 0;  // a world with no land (land share 0): the block stands on cell 0
}

zs::Place place_of(const WorldGrid& grid, const WorldFields& f, CellId cell, std::uint64_t seed) {
  zs::Place p;
  p.cell = cell;
  p.frequency = grid.frequency();
  p.world_seed = seed;
  const Vec3& c = grid.centre(cell);
  p.lat_deg = 90.0 - angle_between(c, Vec3{0, 0, 1}) * 180.0 / kPi;
  const double r = std::sqrt(c.x * c.x + c.y * c.y);
  const double lon = r > 0 ? angle_between(Vec3{c.x / r, c.y / r, 0}, Vec3{1, 0, 0}) * 180.0 / kPi : 0.0;
  p.lon_deg = c.y < 0 ? -lon : lon;
  p.elevation_m = f.elevation_m[cell];
  p.biome = biome_name(f.biome[cell]);
  p.climate = {f.temp_mean_c[cell], f.temp_range_c[cell], f.rain_mm[cell], c.z < 0};
  return p;
}

int choose_town(const WorldFields& f, const WorldPeople& people, std::uint64_t seed) {
  int best = -1;
  for (std::size_t i = 0; i < people.towns.size(); ++i) {
    const Settlement& t = people.towns[i];
    if (t.people < 10'000 || t.people > 200'000 || !temperate(f.biome[t.cell])) continue;
    if (best < 0) { best = static_cast<int>(i); continue; }
    const Settlement& b = people.towns[static_cast<std::size_t>(best)];
    const double dt = std::abs(f.temp_mean_c[t.cell] - kTargetMeanC), db = std::abs(f.temp_mean_c[b.cell] - kTargetMeanC);
    if (dt < db || (dt == db && fields::keyed_unit(seed, i, kPlaceTieField, 1) < fields::keyed_unit(seed, static_cast<std::uint64_t>(best), kPlaceTieField, 1)))
      best = static_cast<int>(i);
  }
  return best;
}

zs::Place place_of_town(const WorldGrid& grid, const WorldFields& f, const WorldPeople& people, int town, std::uint64_t seed) {
  const Settlement& t = people.towns.at(static_cast<std::size_t>(town));
  zs::Place p = place_of(grid, f, t.cell, seed);
  p.town_people = t.people;
  p.sector = people.sector_of[t.cell];
  p.country = t.country;
  const Country& country = people.countries[static_cast<std::size_t>(t.country)];
  p.income = kIncomeGroupIds[static_cast<std::size_t>(country.income)];
  p.profile = country.profile;  // spec 49
  p.country_name = country.name;
  return p;
}

zs::tier0::Tier0 make_world(std::uint64_t seed) {
  const WorldGrid grid(kOneWorldFrequency, 6371.0);
  WorldParams wp;
  wp.seed = seed;
  const WorldFields f = generate_fields(grid, wp);
  const std::string data = zs::tier0::data_dir();
  const WorldLand land = generate_land(grid, f);
  DepositParams dp;
  dp.seed = seed;
  const WorldDeposits deposits = generate_deposits(grid, f, dp);
  const WorldPeople people = populate_world(grid, f, land, deposits, seed, load_group_tables(data), load_real_profiles(data));
  const int town = choose_town(f, people, seed);
  const zs::Place place = town >= 0 ? place_of_town(grid, f, people, town, seed) : place_of(grid, f, choose_place(grid, f, seed), seed);
  return zs::tier0::make_run(seed, place);
}

}  // namespace zs::gen
