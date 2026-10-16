// Spec 97: the town's fuel in play. Its filling stations hold gasoline in their tanks; the town's vehicles take their day
// from them while the pumps run (the mains, or a station's own generator); the depot's tankers refill them while the
// depot has staff. A pop of the block can take its carried cans to a station and fill them, by hand pump when the pumps
// have no power. Fuel is the ledger's gasoline_ul, what the block's generator burns. docs/tier0/specs/97-the-towns-fuel.md.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/world.h"

namespace zs {

class Bodies;
class Actors;
class Assemblies;

// Given by zs::gen::make_world from spec 97's plan; no stations: no chain.
struct FuelSetup {
  struct Station {
    std::string name;
    double share = 0;           // of the town's fuel
    double stock_l = 0;         // its tanks at the start, and the level it reorders to
    bool generator = false;     // it can pump without the mains
    std::int64_t minutes = 0;   // from the block by the town's streets (spec 75)
  };
  std::vector<Station> stations;
  double town_litres_day = 0;     // the town's vehicles
  double depot_hours = 0;         // the tankers' hours from the depot
  std::int64_t depot_until = -1;  // the tick the depot's staff leave (spec 96's staff day); -1: never
  std::int64_t mains_until = -1;  // the tick the town's mains fail (spec 96); -1: never
  double fill_minutes = 15, hand_pump_minutes = 45;
};

inline constexpr std::uint16_t FuelErrand = 0x2102;  // subject = a pop's body, a = the station index

class FuelChain : public System {
 public:
  FuelChain(FuelSetup setup, Bodies* bodies, Actors* actors, Assemblies* items);
  const char* name() const override { return "fuel_chain"; }
  void setup(World& w);  // owners and the stations' stocks
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  std::int64_t next_wake(const World& w, std::int64_t now) const override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& a) override;

  const FuelSetup& fuel() const { return setup_; }
  std::int64_t station_stock(const World& w, std::size_t s) const;  // uL
  bool pumps_run(const World& w, std::size_t s) const;               // rule 3
  std::int64_t sold_to_town() const { return sold_; }                // uL
  std::int64_t errand_minutes(const World& w, std::size_t s) const;

  struct Delivery {
    std::size_t station = 0;
    std::int64_t amount = 0, arrives = 0;
  };
  struct Errand {
    EntityId body = 0;
    std::size_t station = 0;
    std::int64_t amount = 0, back = 0;
    EventId cause = 0;
  };
  const std::vector<Delivery>& deliveries() const { return deliveries_; }
  const std::vector<Errand>& errands() const { return errands_; }

 private:
  std::int64_t next_at(std::int64_t now, int hour) const;
  void daily(World& w);
  std::int64_t cans_room(const World& w, EntityId body) const;  // uL its carried containers can still take
  void refuse(World& w, const Command& c, EventId cause, int why);

  FuelSetup setup_;
  Bodies* bodies_;     // never saved
  Actors* actors_;     // never saved
  Assemblies* items_;  // never saved
  ResourceId fuel_ = 0;
  OwnerId homeward_ = 0;
  std::vector<OwnerId> stations_;
  std::int64_t next_day_ = 0;
  std::int64_t sold_ = 0;
  std::vector<Delivery> deliveries_;
  std::vector<Errand> errands_;
};

}  // namespace zs
