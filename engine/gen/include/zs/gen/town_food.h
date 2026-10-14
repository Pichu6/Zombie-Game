// Spec 79: the town's food. Its outlets by the country's income group (supermarkets, convenience stores, small shops in
// house fronts, markets) placed on the town's plots, each with its share of the town's food and its days of stock; the
// distribution centre that serves the town over the routes; households' pantry and fridge days. Records only: spec 80
// runs them. A pure function of the town's record, the planet's routes and the seed.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/pops.h"
#include "zs/gen/town_layout.h"
#include "zs/gen/world_routes.h"

namespace zs::gen {

enum class OutletKind : std::uint8_t { Supermarket, Convenience, Small, Market };
inline constexpr int kOutletKinds = 4;
const char* outlet_kind_name(OutletKind k);

struct OutletKindParams {
  double floor_m2 = 0, shelf_days = 0, back_days = 0;
};
struct FoodRetailGroup {
  std::array<double, kOutletKinds> per_10k{}, share{};
  double pantry_days = 0, fridge_days = 0, trips_per_week = 0;
};
struct FoodRetailParams {
  std::array<OutletKindParams, kOutletKinds> kinds{};
  std::array<FoodRetailGroup, kIncomeGroups> groups{};  // high first
  double dc_people_served = 0, dc_max_km = 0, dc_min_town_people = 0, dc_dry_days = 0, dc_fresh_days = 0;
};
FoodRetailParams load_food_retail(const std::string& gen_dir);

struct Outlet {
  OutletKind kind = OutletKind::Small;
  std::string name, address;
  int plot = -1;            // the layout's plot; -1: no plot left, the directory address
  double floor_m2 = 0;
  double share = 0;         // of the town's food
  double stock_days = 0;    // shelves and back room
};

// Rule 5: the distribution centre serving a town: the nearest town by road time with enough people (or the town itself).
struct ServingCentre {
  int town = -1;            // spec 48 index
  double hours = 0, km = 0; // by the routes (0 when the town is its own)
};
ServingCentre serving_centre(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, const WorldRoutes& routes,
                             int town, const FoodRetailParams& params);
// Spec 80: the same without the routes (the one world does not build them): the nearest large town in a straight line,
// its road km the straight km times spec 74's measured circuity, its hours at the town's road speed.
inline constexpr double kRoadCircuity = 1.27;  // spec 74 As built: 1.25-1.29 on seeds 1 and 4 (WSR-43)
ServingCentre nearest_centre(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, int town,
                             const FoodRetailParams& params);

struct TownFood {
  int income = 0;
  std::vector<Outlet> outlets;
  ServingCentre centre;
  double people_days_per_day = 0;  // the town's people: what it eats in a day, in people-days of food
  double pantry_days = 0, fridge_days = 0, trips_per_week = 0;
};
TownFood plan_town_food(const Town& town, const TownLayout& layout, double people, int income, const FoodRetailParams& params,
                        std::uint64_t seed, const ServingCentre& centre = {});
std::uint64_t town_food_digest(const TownFood& food);

}  // namespace zs::gen
