// Spec 80: food in play. The block's town's food chain runs cold (the distribution centre refills the outlets once a day,
// the town's people buy their day as counts); the block's households get food when one of them runs an errand to an
// outlet and brings it home (decision 0079). Spec 81 (decision 0080): the block's households shop by themselves when
// their food falls low, and start at a random point of their shopping cycle. Food is the ledger's food_j, the resource
// the block's pantries and fridges hold and the eat process takes. Specs: docs/tier0/specs/80-food-in-play.md, 81-*.
#pragma once
#include <cstdint>
#include <functional>
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
  // Spec 81: households' stocks and trips by the country's income group (spec 79 rule 6).
  double pantry_days = 3, fridge_days = 2, trips_per_week = 4;
};

// data/life/errands.toml (estimates).
struct ErrandParams {
  double carry_kg = 20, mj_per_kg = 6, shop_minutes = 20;
};
ErrandParams load_errand_params(const std::string& path);

inline constexpr std::uint16_t FoodErrand = 0x2101;  // subject = a pop's body, a = the outlet index

class FoodChain : public System {
 public:
  // `held`: whether a player holds a body (it cannot be taken off the map; spec 81 rule 4). Set by the setup, never saved.
  using Held = std::function<bool(EntityId)>;
  FoodChain(FoodSetup setup, ErrandParams errand, Bodies* bodies, Actors* actors, OwnerId pantry, std::uint64_t world_seed);
  const char* name() const override { return "food_chain"; }
  // Spec 81 rule 1: a household of the block, before setup(); members in the order they are asked to go.
  void add_household(std::string name, OwnerId pantry, OwnerId fridge, std::vector<EntityId> members);
  void set_held(Held held) { held_ = std::move(held); }
  void setup(World& w);  // owners, the stocks at their targets (spec 80 rule 2), the households' starts (spec 81 rule 3)
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
  std::int64_t eaten_before_start(const World& w) const;

  struct Household {
    std::string name;
    OwnerId pantry = 0, fridge = 0;
    std::vector<EntityId> members;
    std::int64_t need_j = 0;  // a day
    std::int64_t full_j = 0;  // rule 2
    std::size_t outlet = 0;   // the nearest by the streets
  };
  const std::vector<Household>& households() const { return households_; }
  std::int64_t household_stock(const World& w, std::size_t h) const;
  std::int64_t household_low(std::size_t h) const;  // rule 2: full less one interval's need

  struct Delivery {
    std::size_t outlet = 0;
    std::int64_t amount = 0, arrives = 0;
  };
  struct Errand {
    EntityId body = 0;
    std::size_t outlet = 0;
    std::int64_t amount = 0, back = 0;
    EventId cause = 0;
    int household = -1;  // -1: the block's first pantry (a body of no household)
  };
  const std::vector<Delivery>& deliveries() const { return deliveries_; }
  const std::vector<Errand>& errands() const { return errands_; }
  std::int64_t errand_minutes(std::size_t outlet) const;
  std::int64_t carry_j() const;

 private:
  std::int64_t next_at(std::int64_t now, int hour) const;  // the next tick at that hour of the day
  void daily(World& w);
  void shop(World& w);   // spec 81 rule 4
  bool can_go(const World& w, EntityId body) const;
  int household_of(EntityId body) const;
  bool start_errand(World& w, EntityId body, std::size_t outlet, std::int64_t want, int household, EventId cause);
  void refuse(World& w, const Command& c, EventId cause, int why);

  FoodSetup setup_;
  ErrandParams errand_;
  Bodies* bodies_;  // never saved
  Actors* actors_;  // never saved
  Held held_;       // never saved
  OwnerId pantry_;
  std::uint64_t seed_;
  ResourceId food_ = 0;
  OwnerId centre_ = 0, transit_ = 0, homeward_ = 0, before_ = 0;
  std::vector<OwnerId> outlets_;
  std::vector<Household> households_;
  std::int64_t next_day_ = 0, next_shop_ = 0;
  std::int64_t eaten_ = 0;  // what the town's counts ate, in J (declared outflows)
  std::vector<Delivery> deliveries_;
  std::vector<Errand> errands_;
};

}  // namespace zs
