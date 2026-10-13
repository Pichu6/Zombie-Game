// Spec 76: food flows on the routes. Each town cell's region makes food (spec 52's capacity) and eats it (spec 48's
// people); what is short comes from the cheapest supplier with a surplus over the routes (spec 74), in rounds. Yearly
// data, never ticked: a pure function of (grid, land, people, routes, params).
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/world_routes.h"

namespace zs::gen {

// Relative freight costs (road tonne-km = 1). Estimates taken by Claude (WSR-45 gives only the order). Spec 78 rule 5: a
// border costs its hours (spec 74 by income) at the open road's moving speed (DR-6: 40-60 km/h).
struct FreightParams {
  double road = 1.0, river = 0.3, sea = 0.1;  // per km
  double port_road_km = 150;                   // a port's handling, in road-km
  double border_road_km_per_hour = 50;         // spec 78 rule 5: a border hour, in road-km
  // Rule 1: what is grown, against what is needed: the land's capacity (spec 52) is what it could feed, not what is farmed.
  // WSR-44: world production supports 2,700-3,000 kcal a person a day against the 2,500 spec 52 counts a person needs.
  double grown_vs_need = 1.15;
};

struct NodeFood {
  double made = 0, demand = 0;    // people-years of food a year
  double eaten = 0;               // made and eaten in the region (rule 2)
  double sent = 0, received = 0;  // by the flows
  double surplus = 0, unmet = 0;  // what is left over and what is still short
};

struct FoodFlow {
  int from = 0, to = 0;  // node indices
  double amount = 0;     // people-years
  double cost = 0;       // freight cost of the cheapest path, in road-km
  bool international = false;
};

struct WorldFood {
  std::vector<NodeFood> nodes;
  std::vector<FoodFlow> flows;  // in the order made, one per pair
  double production = 0, demand = 0, traded = 0, international = 0, unmet = 0;
  int rounds = 0;
  double international_share() const { return production > 0 ? international / production : 0; }
};

// Spec 77: food by kind.
enum class FoodKind : std::uint8_t { Grain, Crops, Animal, Fish };
inline constexpr int kFoodKinds = 4;
const char* food_kind_name(FoodKind k);

// Estimates taken by Claude, chosen so the people-weighted world diet matches WSR-46 (cereals 42 %, livestock 16 %), with
// cereals falling and animal foods rising with income.
struct DietParams {
  // Share of calories by kind (grain, other crops, animal foods, fish) per income group (high first).
  std::array<std::array<double, kFoodKinds>, kIncomeGroups> share = {{
      {0.30, 0.42, 0.26, 0.02}, {0.40, 0.40, 0.18, 0.02}, {0.48, 0.40, 0.10, 0.02}, {0.55, 0.37, 0.07, 0.01}}};
  double crops_at_5c = 0.25, crops_at_25c = 0.65;  // rule 1: other crops' share of crop calories by warmth
  double feed_conversion = 0.15;                   // rule 1: crop feed calories that become animal food
};

struct WorldDiet {
  std::array<WorldFood, kFoodKinds> kinds;
  double production = 0, demand = 0, traded = 0, international = 0, unmet = 0;
  double international_share() const { return production > 0 ? international / production : 0; }
};

double freight_cost(const Route& route, const FreightParams& params);
double crops_share(const DietParams& diet, double temp_c);
WorldDiet food_by_kind(const WorldFields& fields, const WorldLand& land, const WorldPeople& people, const WorldRoutes& routes,
                       const FreightParams& params = {}, const DietParams& diet = {}, const LandParams& land_params = {});
std::uint64_t diet_digest(const WorldDiet& diet);
WorldFood food_flows(const WorldLand& land, const WorldPeople& people, const WorldRoutes& routes, const FreightParams& params = {});
std::uint64_t food_digest(const WorldFood& food);

}  // namespace zs::gen
