// Spec 88: extraction zones. The land and sea put to work around the towns that buy what they make: cropland, pasture
// and working forest in von Thuenen's rings, worked mines and oil and gas fields on their deposits, fishing grounds
// from the ports. A pure function of the world above it. docs/tier0/specs/88-extraction-zones.md (research EZ).
#pragma once
#include <cstdint>
#include <vector>

#include "zs/gen/world_deposits.h"
#include "zs/gen/world_fields.h"
#include "zs/gen/world_grid.h"
#include "zs/gen/world_land.h"
#include "zs/gen/world_people.h"

namespace zs::gen {

// Rules 2-4 (reaches calibrated on seeds 1-5 against EZ-1, EZ-2 and EZ-5, see the spec's As built; work_rel is an
// estimate).
struct LandUseParams {
  double crop_km = 600, pasture_km = 5000, forest_km = 50;
  double work_rel = 0.5;          // a deposit is worked at this many times its kind's median or more
  double fish_km = 1500;          // fleets reach beyond the 370 km zone (distant-water fishing)
  double fish_target_t = 80e6;    // EZ-5: marine capture, tonnes a year
  double shelf_m = -200;          // offshore oil and gas are worked above this depth
};

enum class SiteKind : std::uint8_t { Mine, Field };  // a mining district (coal, ores) or an oil or gas field
const char* site_kind_name(SiteKind k);

struct WorkSite {
  SiteKind kind = SiteKind::Mine;
  CellId cell = 0;
  int deposit = 0;   // index into WorldDeposits::deposits
  int country = -1;  // -1 offshore
  int market = -1;   // the town it sells to
};

struct WorldLandUse {
  std::vector<int> market;            // per cell: the nearest town over land (rule 1), -1 none
  std::vector<double> market_km;      // per cell: its distance, km (a large number with no market)
  std::vector<double> crop, pasture, forest;  // per land cell: share of the cell in use
  std::vector<double> fish_t;         // per sea cell: tonnes landed a year
  std::vector<int> fish_port;         // per sea cell: the port town it is fished from, -1 none
  std::vector<WorkSite> sites;
  // Totals for the report (rule 6).
  double land_km2 = 0, crop_km2 = 0, pasture_km2 = 0, forest_km2 = 0, forest_worked_km2 = 0;
  double fish_landed_t = 0, fish_available_t = 0;
};

WorldLandUse generate_land_use(const WorldGrid& grid, const WorldFields& fields, const WorldLand& land, const WorldDeposits& deposits,
                          const WorldPeople& people, const LandUseParams& params = {});

}  // namespace zs::gen
