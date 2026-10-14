// The block's place on a generated planet. Spec: docs/tier0/specs/42-place.md
#include "zs/gen/place.h"
#include "zs/gen/town_food.h"
#include "zs/gen/town_travel.h"

#include <cmath>
#include <optional>

#include "toml.hpp"
#include "zs/fields.h"
#include "zs/gen/stations.h"
#include "zs/gen/town_layout.h"
#include "zs/demographics.h"
#include "zs/players.h"
#include "zs/police_staff.h"

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

zs::tier0::Tier0 make_world(std::uint64_t seed, const WorldSettings& settings) {
  const WorldGrid grid(kOneWorldFrequency, 6371.0);
  WorldParams wp;
  wp.seed = seed;
  wp.land_share = settings.land_share;
  wp.global_temp_c = settings.global_temp_c;
  wp.relief = settings.relief;
  wp.moisture = settings.moisture;
  const WorldFields f = generate_fields(grid, wp);
  const std::string data = zs::tier0::data_dir();
  const WorldLand land = generate_land(grid, f);
  DepositParams dp;
  dp.seed = seed;
  const WorldDeposits deposits = generate_deposits(grid, f, dp);
  CountryParams cp;
  cp.fragmentation = settings.fragmentation;
  const WorldPeople people = populate_world(grid, f, land, deposits, seed, load_group_tables(data), load_real_profiles(data), cp);
  const int town = choose_town(f, people, seed);
  const zs::Place place = town >= 0 ? place_of_town(grid, f, people, town, seed) : place_of(grid, f, choose_place(grid, f, seed), seed);
  // Specs 70-73: the block's town: its stations loaded beside the block, its plots opened when someone asks.
  const auto town_rec = town_record(place, seed);
  // Spec 80: the town's food chain from spec 79 (decision 0079).
  zs::FoodSetup food;
  if (town >= 0) {
    const FoodRetailParams fr = load_food_retail(data + "/gen");
    int income = 0;
    for (int g = 0; g < kIncomeGroups; ++g)
      if (place.income == kIncomeGroupIds[static_cast<std::size_t>(g)]) income = g;
    const ServingCentre centre = nearest_centre(grid, f, people, town, fr);
    const TownFood tf = plan_town_food(town_rec->town, town_rec->layout, town_rec->spec.people, income, fr, seed, centre);
    const TownTravel travel = load_town_travel(data + "/gen");
    const int block = town_rec->layout.lot_plot.empty() ? -1 : town_rec->layout.lot_plot.back();
    food.town_people = town_rec->spec.people;
    food.kj_per_person_day = town_rec->dwelling.food_kj_per_person_day;
    food.centre_days = kCentreDryShare * fr.dc_dry_days + (1 - kCentreDryShare) * fr.dc_fresh_days;
    food.centre_hours = centre.hours;
    food.pantry_days = tf.pantry_days;  // spec 81: the country's households (spec 79 rule 6)
    food.fridge_days = tf.fridge_days;
    food.trips_per_week = tf.trips_per_week;
    for (const Outlet& o : tf.outlets)
      food.outlets.push_back({o.name, outlet_kind_name(o.kind), o.share, o.stock_days,
                              o.plot >= 0 && block >= 0 ? plot_minutes(town_rec->layout, block, o.plot, travel) : kNoPlotMinutes});
  }
  zs::tier0::Tier0 t = zs::tier0::make_run(seed, place, settings.start_day, station_fragments(*town_rec, seed), food);
  if (t.places) t.places->set_source(plot_source(town_rec, seed));
  // Spec 75: a released pop gets home by the town's streets, from the plot its body stood on to the plot it lives on.
  if (t.players && t.census) t.players->set_travel(travel_in_town(town_rec, t.places, t.census->people().size(), seed));
  return t;
}

zs::Players::Travel travel_in_town(std::shared_ptr<const TownRecord> town, const zs::Places* places, std::size_t census_size,
                                   std::uint64_t seed) {
  const TownTravel travel = load_town_travel(zs::tier0::data_dir() + "/gen");
  return [town, places, census_size, seed, travel](PopId pop, int level) -> std::int64_t {
    static const std::vector<zs::Places::Open> none;
    const int from = plot_of_level(town->layout, town->stations.size(), places ? places->opened() : none, level);
    const int to = home_plot(town->layout, pop, census_size, seed);
    if (from < 0 || to < 0) return -1;
    return plot_minutes(town->layout, from, to, travel);
  };
}

std::shared_ptr<const TownRecord> town_record(const zs::Place& place, std::uint64_t seed) {
  const std::string data = zs::tier0::data_dir();
  auto r = std::make_shared<TownRecord>();
  r->cat = load_catalog(data + "/gen");
  Mix mix = load_mix(data + "/gen/populations/tier0_block.toml");  // the town record the block's directory comes from
  r->town = generate_town(r->cat, mix, seed);
  const zs::AgencyParams ap = zs::load_agency_params(data + "/services/police.toml", place.profile, place.income);
  const zs::ForceParams fp = zs::load_force_params(data + "/institutions/police/force.toml", place.profile, place.income);
  const zs::AgencyPlan agency = zs::plan_agency(ap, place.town_people, fp.min_officers);  // the same plan the one world sizes its force by
  r->dwelling = load_dwelling_rules(data + "/gen");
  r->plots = load_plot_rules(data + "/gen");
  // The frame a fragment must fit: the block's map. A floor may be as deep as the frame less the lot's front and back.
  const toml::table block = toml::parse_file(data + "/tier0/map/block.toml");
  r->frame = {block["map"]["width"].value_or(32), block["map"]["height"].value_or(28)};
  r->stations = plan_stations(r->cat, r->town, agency, ap.shifts, load_station_rules(data + "/gen"), r->dwelling, seed,
                              r->frame.h - r->dwelling.lot_front - r->dwelling.lot_back);
  // Spec 71: the town's streets and plots; each station takes a plot of its own and that plot's address.
  TownSpec& spec = r->spec;
  spec.name = r->town.name;
  spec.culture = r->town.culture;
  spec.income = place.income;
  spec.people = place.town_people;
  const zs::NationDemographics nation = zs::load_nation(
      data + (place.profile.empty() ? "/setup/nations/base_" + place.income : "/setup/nations/real/" + place.profile) + ".toml");
  spec.mean_household_size = zs::mean_household_size(nation);
  spec.household_shares = nation.household_sizes;  // spec 72
  for (const Station& s : r->stations) spec.lots.push_back({"police_station", s.name, s.plan.lot_width, s.plan.lot_depth});
  // Spec 72: the Tier 0 block stands on a residential plot of its town, its census households living there.
  spec.lots.push_back({"tier0_block", "the Tier 0 block", r->frame.w, r->frame.h, LandUse::Residential,
                       static_cast<int>(r->town.households.size())});
  r->layout = lay_out_town(r->cat, load_urban_form(data + "/gen"), spec, seed);
  for (std::size_t i = 0; i < r->stations.size(); ++i) {
    const int pi = r->layout.lot_plot[i];
    if (pi < 0) continue;  // no plot wide enough: the station keeps its directory address
    const Plot& plot = r->layout.plots[static_cast<std::size_t>(pi)];
    Station& s = r->stations[i];
    s.street = r->layout.streets[static_cast<std::size_t>(plot.street)].name;
    s.number = plot.number;
    s.address = plot.address;
    s.plan.street = s.street;
    s.plan.number = s.number;
    s.plan.address = s.address;
  }
  return r;
}

std::vector<zs::tier0::Fragment> station_fragments(const TownRecord& town, std::uint64_t seed) {
  std::vector<zs::tier0::Fragment> out;
  int first = 0;
  for (const Station& s : town.stations) {
    zs::tier0::Fragment fr;
    fr.toml = station_toml(town.town, s, town.dwelling, seed);
    fr.z = 10 * (s.index + 1);  // station 1 from level 10, station 2 from level 20, ...
    fr.kind = "police_station";
    fr.name = s.name;
    fr.first_officer = first;
    fr.officers = s.sworn;
    first += s.sworn;
    out.push_back(std::move(fr));
  }
  return out;
}

std::vector<zs::tier0::Fragment> station_fragments(const zs::Place& place, std::uint64_t seed) {
  return station_fragments(*town_record(place, seed), seed);
}

zs::PlotSource plot_source(std::shared_ptr<const TownRecord> town, std::uint64_t seed) {
  zs::PlotSource src;
  src.count = [town] { return static_cast<int>(town->layout.plots.size()); };
  src.address = [town](int plot) { return town->layout.plots.at(static_cast<std::size_t>(plot)).address; };
  src.site_toml = [town, seed](int plot) {
    return plot_toml(town->spec, town->layout, plot, town->dwelling, town->plots, seed, town->frame);
  };
  return src;
}

}  // namespace zs::gen
