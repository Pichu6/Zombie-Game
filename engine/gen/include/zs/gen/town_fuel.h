// Spec 97: the town's fuel: its filling stations by the country's income group (data/gen/fuel_retail.toml, research
// fuel-stations-and-stocks), their tanks and stock, and the depot that sends them tankers (the nearest refinery or port
// over the routes, spec 74). docs/tier0/specs/97-the-towns-fuel.md.
#pragma once
#include <array>
#include <cstdint>
#include <string>
#include <vector>

#include "zs/gen/town_layout.h"
#include "zs/gen/world_industry.h"
#include "zs/gen/world_people.h"
#include "zs/gen/world_routes.h"

namespace zs::gen {

struct FuelRetailGroup {
  double people_per_station = 0, litres_per_person_day = 0, tank_l = 0, stock_share = 0, generator_share = 0;
};
struct FuelRetailParams {
  std::array<FuelRetailGroup, kIncomeGroups> groups{};
  double fill_minutes = 15, hand_pump_minutes = 45;
};
FuelRetailParams load_fuel_retail(const std::string& gen_dir);

// Rule 2: the depot: the nearest town by route hours with an oil refinery (spec 93) or a port, in the town's country if
// it has one, else anywhere; -1 when no route reaches one.
struct FuelDepot {
  int town = -1;
  double hours = 0;
  bool refinery = false;
};
FuelDepot fuel_depot(const WorldRoutes& routes, const WorldPeople& people, const WorldIndustry& industry, int town);

struct FuelStation {
  std::string name, address;
  int plot = -1;              // the layout's plot; -1: no plot left
  double share = 0;           // of the town's fuel
  double tank_l = 0, stock_l = 0;
  bool generator = false;     // it can pump without the mains
};
struct TownFuel {
  int income = 0;
  std::vector<FuelStation> stations;
  double litres_per_day = 0;  // the town's vehicles
  FuelDepot depot;
};
// Rule 1: `taken` are plots already used (the food outlets'); stations take commercial plots from the town's edge inward.
TownFuel plan_town_fuel(const TownLayout& layout, double people, int income, const FuelRetailParams& params, std::uint64_t seed,
                        const FuelDepot& depot = {}, const std::vector<int>& taken = {});

}  // namespace zs::gen
