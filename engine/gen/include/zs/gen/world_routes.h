// Spec 74: routes between towns. Roads, navigable rivers and sea lanes on the world grid, laid as the tree of least
// travel time between town cells plus loops where the network's detour is large (WSR-34); the time between any two
// towns. A pure function of (grid, fields, land, people, params); data, never ticked.
#pragma once
#include <array>
#include <cstdint>
#include <vector>

#include "zs/gen/world_land.h"
#include "zs/gen/world_people.h"

namespace zs::gen {

enum class RouteMode : std::uint8_t { Road, River, Sea };
inline constexpr int kRouteModes = 3;
const char* route_mode_name(RouteMode m);

// Every number is an estimate taken by Claude (spec 74 State).
struct RouteParams {
  std::array<double, kIncomeGroups> flat_kmh = {90, 70, 55, 40};  // road speed on flat open land, by income group (high first)
  // Terrain factor on the road speed, by biome (Ocean, Ice, Tundra, Boreal, Temperate, Grassland, Desert, Savanna,
  // Tropical forest, Mountain).
  std::array<double, biome_count> terrain = {1.0, 0.15, 0.5, 0.6, 0.9, 1.0, 0.7, 0.9, 0.45, 0.35};
  double climb_hours_per_km = 0.5;  // per 1,000 m of height between two cells, either way
  double river_kmh = 12;            // barge on a large river
  double sea_kmh = 30;              // a cargo ship, about 16 knots
  double port_hours = 12;           // loading or unloading at a port
  // Spec 78 rule 4: crossing into another country on land costs the poorer group's hours (high first): estimates
  // anchored on DR-8 (none inside Schengen, 9-24 h on average in southern Africa).
  std::array<double, kIncomeGroups> border_hours = {0.5, 3, 8, 24};
  double loop_detour = 1.8;         // rule 5: calibrated so road circuity lands at WSR-43 (spec 74 As built)
};

struct RouteNode {
  CellId cell = 0;
  std::vector<int> towns;  // spec 48 indices, ascending
};

enum class RouteWhy : std::uint8_t { Tree, Loop };

struct Route {
  int a = 0, b = 0;                   // node indices, a < b
  std::vector<CellId> path;           // from a's cell to b's, through neighbouring cells
  std::array<double, kRouteModes> km{};
  double hours = 0;
  bool crosses_border = false;
  int ports = 0, borders = 0;         // spec 76: steps through a port, steps into another country
  double border_hours = 0;            // spec 78 rule 4: the hours its borders cost
  RouteWhy why = RouteWhy::Tree;
};

struct WorldRoutes {
  std::vector<RouteNode> nodes;            // by cell, ascending
  std::vector<int> node_of;                // per cell; -1 without a town
  std::vector<int> region_of;              // spec 76: per cell, the node nearest in time (rule 3's search)
  std::vector<Route> routes;               // tree routes first, then loops, each in the order added
  std::vector<std::vector<int>> of_node;   // route indices per node
  RouteParams params;
};

// Rule 1: the hours, km and mode of one step between neighbouring cells (symmetric).
struct Step {
  double hours = 0, km = 0;
  RouteMode mode = RouteMode::Road;
  bool port = false, border = false;
  double border_hours = 0;  // spec 78 rule 4
};
Step route_step(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldPeople& people,
                const RouteParams& params, CellId u, CellId v);
// The road speed in a cell (km/h): its country's flat speed (the lowest group's when unruled) times its terrain factor.
double road_kmh(const WorldFields& fields, const WorldPeople& people, const RouteParams& params, CellId c);

WorldRoutes build_routes(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldPeople& people,
                         const RouteParams& params = {});

// Rule 6: the shortest time over the routes between two nodes; -1 if not connected.
double node_hours(const WorldRoutes& routes, int from, int to);
// The routes taken by the fastest way between two nodes, in order; empty if none or the same node.
std::vector<int> node_route_path(const WorldRoutes& routes, int from, int to);
// Rule 8: network km over great-circle km, the mean over keyed pairs of nodes `min_km` to 4 x `min_km` apart joined by
// land alone.
double road_circuity(const WorldGrid& grid, const WorldRoutes& routes, double min_km, int pairs, std::uint64_t seed);
// Rule 6: between two towns (spec 48 indices), with rule 2's in-cell term when they share a cell.
double town_hours(const WorldGrid& grid, const WorldFields& fields, const WorldPeople& people, const WorldRoutes& routes,
                  int from, int to);

std::uint64_t routes_digest(const WorldRoutes& routes);

}  // namespace zs::gen
