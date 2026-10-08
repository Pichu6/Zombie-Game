// Orders and belief as a world system: signing and repealing edicts, their daily application through the economy
// and pop systems with the edict as cause, broadcasts that become beliefs when communications work, and retelling
// between pops. Spec: docs/tier0/specs/39-orders-and-belief.md
#pragma once
#include <cstdint>
#include <vector>

#include "zs/beliefs.h"
#include "zs/economy_system.h"
#include "zs/edicts.h"
#include "zs/pop_system.h"
#include "zs/world.h"

namespace zs {

enum OrderCommand : std::uint16_t {
  EdictSign = 0x4001,    // subject = signer owner, a = edict def
  EdictRepeal = 0x4002,  // subject = edict index
  Tell = 0x4003,         // subject = from pop, a = to pop, b = the thing (entity id) it believes something about
  SetComms = 0x4004,     // subject = 0, a = 1 up, 0 down: whether broadcasts reach people (0014 item 8)
};

enum class OrderRefusal : std::int64_t {
  None = 0,
  UnknownDef = 1,
  NoSuchEdict = 2,
  NotInForce = 3,
  NothingKnown = 4,   // the teller believes nothing about it
  Nothing = 5,
  NoSystem = 6,       // the edict needs a system that is not attached
};

// Entity ids for things people believe something about: an edict is (1 << 40) | its index.
inline constexpr EntityId kEdictEntityBase = EntityId{1} << 40;
inline EntityId edict_entity(std::size_t edict) { return kEdictEntityBase | edict; }

class OrdersSystem : public System {
 public:
  OrdersSystem(EconomySystem* econ, PopSystem* pops) : econ_(econ), pops_(pops) {}
  const char* name() const override { return "orders"; }

  Edicts& edicts() { return edicts_; }
  const Edicts& edicts() const { return edicts_; }
  Beliefs& beliefs() { return beliefs_; }
  const Beliefs& beliefs() const { return beliefs_; }
  // Who hears a broadcast (setup; the region's people until the registry provides them).
  void set_audience(std::vector<PopId> pops) { audience_ = std::move(pops); }
  bool comms_up() const { return comms_up_; }
  double distortion_per_hop() const { return distortion_; }
  void set_distortion_per_hop(double d) { distortion_ = d; }  // setup

  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;  // applies the standing edicts once a game day
  std::int64_t next_wake(const World&, std::int64_t now) const override {
    return ((now + kTicksPerGameDay - 1) / kTicksPerGameDay) * kTicksPerGameDay;
  }
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

 private:
  void refuse(World& w, const Command& c, EventId cause, OrderRefusal why);
  void apply_edict(World& w, std::size_t edict);
  void broadcast(World& w, std::size_t edict, std::int64_t claim, EventId cause);

  EconomySystem* econ_;
  PopSystem* pops_;
  Edicts edicts_;
  Beliefs beliefs_;
  std::vector<PopId> audience_;
  bool comms_up_ = true;
  double distortion_ = 0.15;
};

}  // namespace zs
