// The block's place in the world: one cell of a generated planet, its climate, and the weather there.
// Spec: docs/tier0/specs/42-place.md
#pragma once
#include <cstdint>
#include <string>

#include "zs/fields.h"
#include "zs/world.h"

namespace zs {

struct Place {
  std::uint32_t cell = 0;        // world grid cell id
  int frequency = 0;             // world grid frequency (0 = no planet: the default place)
  std::uint64_t world_seed = 0;  // the planet's seed; keys the weather
  double lat_deg = 40, lon_deg = 0, elevation_m = 100;
  std::string biome = "temperate forest";
  fields::Climate climate{12.0, 20.0, 800.0, false};
  // Spec 48: the town the block is in, its sector and country, and the country's income group (the base country of
  // the one world: data/setup/nations/base_<income>.toml). The default place has no planet: an upper-middle town.
  double town_people = 50'000;
  int sector = -1, country = -1;
  std::string income = "upper_middle";
  // Spec 49: the real profile the country takes (ISO3; empty: the base country of its group) and its parody name.
  std::string profile, country_name;
  // Spec 96: when the town's mains fail (ticks from the run's start, from its grid's outage timeline) and why; -1: spec 17
  // rule 5's draw in [day 2, day 5). Read at setup; the tick is saved by Power, these are not.
  std::int64_t grid_fail_tick = -1;
  std::string grid_fail_cause;
};

// Rule 3: the placeholder place used when no planet was generated.
Place default_place();

class PlaceSystem : public System {
 public:
  explicit PlaceSystem(Place place) : place_(std::move(place)) {}
  const char* name() const override { return "place"; }
  void update(World&) override {}
  std::int64_t next_wake(const World&, std::int64_t) const override { return kNever; }
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& r) override;

  const Place& place() const { return place_; }
  // Rule 4: the outdoor temperature now, and today's rain and mean (cached per civil day, spec 53).
  double outdoor_c(const World& w) const;
  double today_mean_c(const World& w) const;
  double today_rain_mm(const World& w) const;
  // Rule 5: an unheated building's air.
  double indoor_c(const World& w) const;

 private:
  void refresh(const World& w) const;
  Place place_;
  // Derived cache, not saved.
  static constexpr std::int64_t kNoDay = INT64_MIN;
  mutable std::int64_t cached_day_ = kNoDay;  // the civil day number (spec 53)
  mutable double mean_c_ = 0, rain_mm_ = 0;
};

}  // namespace zs
