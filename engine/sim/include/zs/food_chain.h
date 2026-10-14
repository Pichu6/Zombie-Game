// Spec 80: food in play. The block's town's food chain runs cold (the distribution centre refills the outlets once a day,
// the town's people buy their day as counts); the block's households get food only when one of them runs an errand to an
// outlet and brings it home (decision 0079). Food is the ledger's food_j, the resource the block's pantry holds and the
// eat process takes. Spec: docs/tier0/specs/80-food-in-play.md.
#pragma once
#include <cstdint>
#include <string>
#include <vector>

#include "zs/world.h"

namespace zs {

class Bodies;
class Actors;

// Given by zs::gen::make_world from spec 79; empty outlets: no chain.
struct FoodSetup {
  struct Outlet {
    std::string name, kind;
    double share = 0;       // of the town's food
    double stock_days = 0;  // shelves and back room
    std::int64_t minutes = 0;  // from the block by the town's streets (spec 75)
  };
  double town_people = 0, block_people = 0;
  double kj_per_person_day = 10000;  // spec 51
  std::vector<Outlet> outlets;
  double centre_days = 0;            // spec 79 rule 4
  double centre_hours = 0;           // its road hours to the town (spec 79 rule 5)
};

// data/life/errands.toml (estimates).
struct ErrandParams {
  double carry_kg = 20, mj_per_kg = 6, shop_minutes = 20;
};
ErrandParams load_errand_params(const std::string& path);

inline constexpr std::uint16_t FoodErrand = 0x2101;  // subject = a pop's body, a = the outlet index

class FoodChain : public System {
 public:
  FoodChain(FoodSetup setup, ErrandParams errand, Bodies* bodies, Actors* actors, OwnerId pantry);
  const char* name() const override { return "food_chain"; }
  void setup(World& w);  // owners and the stocks at their targets (rule 2); before close_setup
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  std::int64_t next_wake(const World& w, std::int64_t now) const override;
  void save(ArchiveWriter& a) const override;
  void load(ArchiveReader& a) override;

  const FoodSetup& food() const { return setup_; }
  std::int64_t centre_stock(const World& w) const;
  std::int64_t outlet_stock(const World& w, std::size_t outlet) const;
  std::int64_t outlet_target(std::size_t outlet) const;
  std::int64_t town_need_j() const;  // the town's people who are not the block's, a day
  std::int64_t eaten_by_town(const World& w) const;
  struct Delivery {
    std::size_t outlet = 0;
    std::int64_t amount = 0, arrives = 0;
  };
  struct Errand {
    EntityId body = 0;
    std::size_t outlet = 0;
    std::int64_t amount = 0, back = 0;
    EventId cause = 0;
  };
  const std::vector<Delivery>& deliveries() const { return deliveries_; }
  const std::vector<Errand>& errands() const { return errands_; }
  std::int64_t errand_minutes(std::size_t outlet) const;
  std::int64_t carry_j() const;

 private:
  std::int64_t next_day_tick(std::int64_t now) const;  // the next 05:00
  void daily(World& w);
  void refuse(World& w, const Command& c, EventId cause, int why);

  FoodSetup setup_;
  ErrandParams errand_;
  Bodies* bodies_;  // never saved
  Actors* actors_;  // never saved
  OwnerId pantry_;
  ResourceId food_ = 0;
  OwnerId centre_ = 0, transit_ = 0, homeward_ = 0;
  std::vector<OwnerId> outlets_;
  std::int64_t next_day_ = 0;
  std::int64_t eaten_ = 0;  // what the town's counts ate, in J (declared outflows)
  std::vector<Delivery> deliveries_;
  std::vector<Errand> errands_;
};

}  // namespace zs
