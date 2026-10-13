// Players and possession (decisions 0066 and 0068, spec 66). A player is the outside will: it joins a world with a pop (its
// home) or as a free camera, and moves between pops. Per player: a state (embodied, detached, observer), a detachment level
// (free, knowledge, off), a home pop, the pop it is in, and its pins in order (the home first). Possessing a pop that is
// not your home takes it away from its life: an absence begins (spec 59), and its household, employer and the police react
// as to any other. Releasing it sends it home: the absence ends after the time it takes to get there. Calling in as the
// pop before anyone has reported it turns the absence into an ordinary one. Pins only show; they change nothing.
// Spec 67: the body. A pop whose body is on the map is controlled where it is (its AI stops, and starts again when the
// player leaves); a pop with no body anywhere gets one from the body pool of spec 29 while a player is in it.
#pragma once
#include <cstdint>
#include <functional>
#include <string>
#include <vector>

#include "zs/pop.h"
#include "zs/world.h"

namespace zs {

class Expectations;

// 0066 rule 2: in a pop and deciding; out of the body with a home; no pop at all.
enum class PlayerMode : std::uint8_t { Embodied = 0, Detached = 1, Observer = 2 };
// 0066 rule 3: what the player may see and do outside its body. Ordered from most to least: raising is toward Free.
enum class DetachLevel : std::uint8_t { Free = 0, Knowledge = 1, Off = 2, kCount = 3 };
const char* player_mode_name(PlayerMode m);
const char* detach_level_name(DetachLevel l);

// data/life/possession.toml. Defaults taken by Claude.
struct PossessionParams {
  int return_home_min = 120;  // a released pop is home this long after the release (until travel has distances)
  int body_slots = 4;         // spec 67: the body pool the one world reserves for pops with no body on the map (a setup setting)
};
PossessionParams load_possession_params(const std::string& path);

// Commands (a high range of their own). `subject` is the player, except for a join.
enum PlayerCommand : std::uint16_t {
  PlayerJoin = 0x1201,      // subject = 0, a = the pop to enter with (-1, kNoPop: free cam), b = DetachLevel
  PlayerPossess = 0x1202,   // a = the pop
  PlayerDetach = 0x1203,    // leave the body
  PlayerReturn = 0x1204,    // back into the home
  PlayerMakeHome = 0x1205,  // the pop the player is in becomes its home
  PlayerPin = 0x1206,       // a = the pop
  PlayerUnpin = 0x1207,     // a = the pop
  PlayerCallIn = 0x1208,    // as the pop it is in, phone the people who expect it
  PlayerSetLevel = 0x1209,  // a = DetachLevel
};

// Why a player command was refused (PlayerRefused's `b`).
enum class PlayerRefusal : std::int64_t {
  NoSuchPlayer = 1,
  NotAlive = 2,      // no such pop, or dead
  Reserved = 3,      // another player's home or the pop another player is in (0068 rule 5)
  Level = 4,         // the detachment level does not allow it (free cam and possessing others need free; off cannot leave the body)
  NotEmbodied = 5,   // the player is in no pop
  AlreadyThere = 6,  // already in that pop, already home, already pinned
  Away = 7,          // the pop is away for another reason (a real absence): it is not where anyone could take it
  HomeLocked = 8,    // the home cannot be unpinned while it lives
  NoHome = 9,
  NotPinned = 10,
  TooLate = 11,      // the pop was already reported missing: only a return closes the case
  NothingToCall = 12,// nobody is waiting for the pop
  NoBody = 13,       // spec 67: the pop has no body and the pool has no free slot
  MapTaken = 14,     // spec 67: another player holds a body on the map (the map has one player's hands)
};

// Spec 67: how the players reach bodies. Set by the setup, never saved: the one world answers from the census, the pop AI
// and the pop system; tests answer with a fake.
struct Hands {
  enum class Where : std::uint8_t {
    Nowhere = 0,      // the pop has no body in the world: one comes from the pool
    OnMap = 1,        // its body is on the map: the player controls it there
    Unavailable = 2,  // it has a body that cannot be taken now (away, a walker)
  };
  struct Found {
    Where where = Where::Nowhere;
    EntityId body = 0;
  };
  virtual ~Hands() = default;
  virtual Found body_of(const World& w, PopId pop) const = 0;
  virtual void take(World& w, EntityId body) = 0;       // its AI stops driving it
  virtual void give_back(World& w, EntityId body) = 0;  // its AI drives it again
  virtual EntityId embody(World& w, PopId pop, EventId cause) = 0;  // a pool body for the pop; 0 if refused
  virtual bool disembody(World& w, PopId pop, EventId cause) = 0;   // the pool body goes back; false if refused
  virtual std::int64_t dwell_ticks() const { return kTicksPerGameHour; }  // the least time in a pool body (spec 29 rule 4)
  // The pop the scenario hands control to when this one died (spec 26: the run passes control to the next member); kNoPop if none.
  virtual PopId successor(const World&, PopId /*dead*/) const { return kNoPop; }
};

class Players : public System {
 public:
  using Alive = std::function<bool(PopId, std::int64_t tick)>;
  // `exp` runs the absences a possession begins; `alive` says whether a pop exists and lives at a tick. Both set by the setup.
  Players(PossessionParams params, Expectations* exp, Alive alive) : params_(params), exp_(exp), alive_(std::move(alive)) {}
  const char* name() const override { return "players"; }
  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World& w) override;
  std::int64_t next_wake(const World&, std::int64_t now) const override;
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;
  void set_hands(Hands* hands) { hands_ = hands; }  // setup; without hands a possession has no body (spec 66 alone)

  struct Player {
    std::uint64_t id = 0;  // from 1, in the order they joined
    PlayerMode mode = PlayerMode::Observer;
    DetachLevel level = DetachLevel::Free;
    bool marked = false;   // the level was raised during play (0066 rule 4): the save says so
    PopId home = kNoPop;   // none: free cam, or not chosen yet
    bool home_dead = false;
    PopId in = kNoPop;     // the pop it is in; none while detached or observing
    EntityId body = 0;     // spec 67: the body it controls (on the map or from the pool); 0 without hands
    std::vector<PopId> pins;  // the home first while it has one
  };
  // A released pop on its way home: its absence ends at `at`.
  struct Return {
    PopId pop = 0;
    std::int64_t at = 0;
    EventId cause = 0;
  };
  const std::vector<Player>& players() const { return players_; }
  const Player* find(std::uint64_t player) const;
  // Spec 67: a pool body held for a pop: while a player is in it, then until the dwell is over.
  struct Pooled {
    PopId pop = 0;
    EntityId body = 0;
    std::int64_t since = 0;
    std::int64_t release_at = kNever;  // kNever while a player is in it
    EventId cause = 0;
  };
  const std::vector<Return>& returns() const { return returns_; }
  const std::vector<Pooled>& pooled() const { return pooled_; }
  // The player holding a pop (as home or as the pop it is in), 0 if none.
  std::uint64_t holder_of(PopId pop) const;

 private:
  Player* get(std::uint64_t player);
  void refuse(World& w, const Command& c, EventId cause, PlayerRefusal why);
  PlayerRefusal possess(World& w, Player& p, PopId pop, EventId cause);  // 0 when the player is in the pop
  void release(World& w, Player& p, EventId cause);  // leaves the pop it is in
  bool can_take(const Player& p, PopId pop) const;   // free of other players
  void body_check(World& w);
  void pool_step(World& w);
  bool on_map_held_by_other(const Player& p) const;

  PossessionParams params_;
  Expectations* exp_;  // never saved
  Alive alive_;        // never saved
  Hands* hands_ = nullptr;  // never saved
  std::vector<Player> players_;
  std::vector<Return> returns_;
  std::vector<Pooled> pooled_;
};

}  // namespace zs
