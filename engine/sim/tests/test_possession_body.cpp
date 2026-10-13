// Spec 67: the body of possession. A pop whose body is on the map is controlled where it is (its AI stops and starts again,
// and it is not absent); a pop with no body gets one from the pool for as long as a player is in it and the dwell after;
// when the home dies the scenario's next pop becomes the home.
#include <doctest.h>

#include <map>
#include <memory>
#include <set>
#include <vector>

#include "zs/body.h"
#include "zs/expectations.h"
#include "zs/players.h"
#include "zs/police_staff.h"
#include "zs/pop_ai.h"
#include "zs/pop_system.h"
#include "zs/run.h"
#include "zs/tier0.h"

using namespace zs;

namespace {

constexpr PopId kAda = 1, kBen = 2, kCop = 3, kWalker = 4, kDee = 5;

// Ada and Ben have bodies on the map (100, 200); the walker's body cannot be taken; the cop and Dee have none.
struct FakeHands : Hands {
  std::map<PopId, EntityId> on_map{{kAda, 100}, {kBen, 200}, {kWalker, 400}};
  std::set<EntityId> taken;  // bodies whose AI is stopped
  std::map<PopId, EntityId> pool;
  std::size_t slots = 1;
  int embodies = 0, disembodies = 0;
  bool refuse_disembody = false;
  PopId next = kNoPop;
  EntityId next_body = 900;
  Found body_of(const World&, PopId pop) const override {
    if (pop == kWalker) return {Where::Unavailable, 400};
    const auto it = on_map.find(pop);
    return it == on_map.end() ? Found{Where::Nowhere, 0} : Found{Where::OnMap, it->second};
  }
  void take(World&, EntityId body) override { taken.insert(body); }
  void give_back(World&, EntityId body) override { taken.erase(body); }
  EntityId embody(World&, PopId pop, EventId) override {
    if (pool.size() >= slots) return 0;
    ++embodies;
    return pool[pop] = next_body++;
  }
  bool disembody(World&, PopId pop, EventId) override {
    if (refuse_disembody) return false;
    ++disembodies;
    return pool.erase(pop) == 1;
  }
  PopId successor(const World&, PopId) const override { return next; }
  // Spec 68: pool bodies on the map.
  bool on_map_pool = false;
  std::vector<EntityId> arrived, left;
  bool pool_on_map() const override { return on_map_pool; }
  void arrive(World&, EntityId body, EventId) override { arrived.push_back(body); }
  void leave(World&, EntityId body, EventId) override { left.push_back(body); }
};

struct Rig {
  std::unique_ptr<World> world;
  Expectations* exp = nullptr;
  Players* players = nullptr;
  FakeHands hands;
  std::set<PopId> dead;
  explicit Rig() {
    world = std::make_unique<World>(1);
    world->check_every_tick = false;
    auto e = std::make_unique<Expectations>(load_expectation_params(tier0::data_dir() + "/life/expectations.toml"),
                                            [](PopId, std::int64_t) { return std::optional<Commitment>{}; });
    exp = e.get();
    auto p = std::make_unique<Players>(load_possession_params(tier0::data_dir() + "/life/possession.toml"), exp,
                                       [this](PopId id, std::int64_t) { return id >= 1 && id <= 9 && !dead.count(id); });
    players = p.get();
    players->set_hands(&hands);
    world->add_system(std::move(e));
    world->add_system(std::move(p));
    world->close_setup();
  }
  void cmd(std::uint16_t type, std::uint64_t player, std::int64_t a = 0, std::int64_t b = 0) {
    world->submit({0, type, player, a, b});
    world->run_ticks(1);
  }
  void join(PopId pop) { cmd(PlayerJoin, 0, static_cast<std::int64_t>(pop), static_cast<std::int64_t>(DetachLevel::Free)); }
  const Players::Player& me() const { return players->players().front(); }
  PlayerRefusal last_refusal() const {
    PlayerRefusal out{0};
    for (const Event& e : world->log.all())
      if (e.type == EventType::PlayerRefused) out = static_cast<PlayerRefusal>(e.b);
    return out;
  }
};

}  // namespace

TEST_CASE("the pool's size loads from data (A1)") {
  CHECK(load_possession_params(tier0::data_dir() + "/life/possession.toml").body_slots == 4);
}

TEST_CASE("a pop with a body on the map is controlled where it is: its AI stops, it is not absent, and gets its AI back (A1)") {
  Rig r;
  r.join(kAda);
  CHECK(r.me().body == 100);
  CHECK(r.hands.taken == std::set<EntityId>{100});
  r.cmd(PlayerPossess, 1, kBen);
  CHECK(r.me().body == 200);
  CHECK(r.hands.taken == std::set<EntityId>{200});  // Ada's AI drives her again
  CHECK(r.exp->cases().empty());                     // Ben is where he lives: nobody misses him
  r.cmd(PlayerDetach, 1);
  CHECK(r.hands.taken.empty());
  CHECK(r.me().body == 0);
  CHECK(r.players->returns().empty());  // nothing to walk home from
}

TEST_CASE("a pop with no body gets one from the pool, is absent, and the body goes back after the dwell (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kCop);
  CHECK(r.hands.embodies == 1);
  CHECK(r.me().body == 900);
  REQUIRE(r.exp->find(kCop));  // taken from his life
  REQUIRE(r.players->pooled().size() == 1);
  const std::int64_t since = r.players->pooled()[0].since;
  r.cmd(PlayerReturn, 1);  // a few ticks in: the body is kept until the dwell is over
  REQUIRE(r.players->pooled().size() == 1);
  CHECK(r.players->pooled()[0].release_at == since + r.hands.dwell_ticks());
  r.world->jump(since + r.hands.dwell_ticks() - 1);
  CHECK(r.hands.disembodies == 0);
  r.world->jump(since + r.hands.dwell_ticks() + 1);
  CHECK(r.hands.disembodies == 1);
  CHECK(r.players->pooled().empty());
}

TEST_CASE("taken again before the dwell is over, a pop gets the same pool body back (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kCop);
  r.cmd(PlayerReturn, 1);
  r.cmd(PlayerPossess, 1, kCop);
  CHECK(r.hands.embodies == 1);
  CHECK(r.me().body == 900);
  r.world->jump(r.world->clock.tick + 3 * kTicksPerGameHour);
  CHECK(r.hands.disembodies == 0);  // the player is in it
}

TEST_CASE("a full pool refuses the possession and leaves the player where it was (A1)") {
  Rig r;
  r.hands.slots = 0;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kCop);
  CHECK(r.last_refusal() == PlayerRefusal::NoBody);
  CHECK(r.me().in == kAda);
  CHECK(r.me().body == 100);
  CHECK(r.exp->cases().empty());
  r.join(kDee);  // a join that needs a pool body is refused as a whole
  CHECK(r.last_refusal() == PlayerRefusal::NoBody);
  CHECK(r.players->players().size() == 1);
}

TEST_CASE("a body that cannot be taken refuses; the map has one player's hands (A1)") {
  Rig r;
  r.join(kAda);
  r.cmd(PlayerPossess, 1, kWalker);
  CHECK(r.last_refusal() == PlayerRefusal::Away);
  r.join(kCop);  // a second player, in a pool body
  REQUIRE(r.players->players().size() == 2);
  r.cmd(PlayerPossess, 2, kBen);
  CHECK(r.last_refusal() == PlayerRefusal::MapTaken);
  r.cmd(PlayerDetach, 1);
  r.cmd(PlayerPossess, 2, kBen);
  CHECK(r.players->players()[1].body == 200);
}

TEST_CASE("a pool body held past its dwell is retried an hour later when it cannot go back (A1)") {
  Rig r;
  r.join(kCop);
  r.hands.refuse_disembody = true;
  r.cmd(PlayerDetach, 1);
  r.world->jump(r.world->clock.tick + r.hands.dwell_ticks() + 1);
  REQUIRE(r.players->pooled().size() == 1);
  r.hands.refuse_disembody = false;
  r.world->jump(r.world->clock.tick + kTicksPerGameHour + 1);
  CHECK(r.players->pooled().empty());
}

TEST_CASE("when the home dies the scenario's next pop becomes home and the player is in it at once (A1)") {
  Rig r;
  r.join(kAda);
  r.hands.next = kBen;
  r.dead.insert(kAda);
  r.world->run_ticks(1);  // a player in a body is looked at every tick
  CHECK(r.me().home == kBen);
  CHECK(r.me().in == kBen);
  CHECK(r.me().body == 200);
  CHECK(r.me().pins.front() == kBen);
  CHECK(r.hands.taken == std::set<EntityId>{200});
  bool chained = false;
  for (const Event& e : r.world->log.all())
    if (e.type == EventType::PlayerHomeSet) chained = r.world->log.get(e.cause)->type == EventType::PlayerBodyDied;
  CHECK(chained);
}

TEST_CASE("pool bodies and bodies held save and load, and jump as they step (A1)") {
  const auto drive = [](Rig& r) {
    r.join(kAda);
    r.cmd(PlayerPossess, 1, kCop);
    r.cmd(PlayerReturn, 1);
  };
  Rig a, b;
  drive(a);
  drive(b);
  const std::int64_t end = 3 * kTicksPerGameHour;
  a.world->jump(end);
  for (std::int64_t t = b.world->clock.tick; t < end; ++t) b.world->step();
  CHECK(a.world->state_hash() == b.world->state_hash());
  Rig c;
  drive(c);
  c.world->load(a.world->save());
  CHECK(c.players->pooled().size() == a.players->pooled().size());
  CHECK(c.me().body == a.me().body);
}

TEST_CASE("in the one world, an officer gets a pool body that lives, and a block survivor is controlled on the map (A1)" *
          doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  REQUIRE(t.players);
  REQUIRE(t.force.get() != nullptr);
  REQUIRE(t.hands.get() != nullptr);
  const PopId you = census_pop_id(static_cast<std::uint32_t>(*t.census->person_of(t.run->controlled())));
  t.world->submit({0, PlayerJoin, 0, static_cast<std::int64_t>(you), static_cast<std::int64_t>(DetachLevel::Free)});
  t.world->run_ticks(1);
  REQUIRE(t.players->players().size() == 1);
  CHECK(t.ai->possessed() == t.run->controlled());  // the player's hands are on the run's pop
  CHECK(t.expectations->find(you) == nullptr);
  const PopId officer = t.force->officers().front();
  t.world->submit({0, PlayerPossess, 1, static_cast<std::int64_t>(officer), 0});
  t.world->run_ticks(1);
  const EntityId body = t.players->players()[0].body;
  REQUIRE(body != 0);
  CHECK(t.pops->registry().embodied_body(officer) == body);
  CHECK(t.bodies->get(body).alive);
  CHECK(t.ai->possessed() == body);  // spec 68: the player's hands are on the officer, on the block
  REQUIRE(t.expectations->find(officer));
  t.world->jump(t.world->clock.tick + 2 * kTicksPerGameHour);
  CHECK(t.bodies->get(body).alive);  // two hours in the body simulation
  t.world->submit({0, PlayerReturn, 1, 0, 0});
  t.world->jump(t.world->clock.tick + 2 * kTicksPerGameHour);
  CHECK(t.pops->registry().embodied_body(officer) == 0);  // back in the registry, its body state kept
  CHECK(t.pops->registry().saved_body(officer) != nullptr);
  CHECK(t.ai->possessed() == t.run->controlled());
  int embodied = 0, disembodied = 0;
  for (const Event& e : t.world->log.all()) {
    embodied += e.type == EventType::PopEmbodied && e.subject == officer;
    disembodied += e.type == EventType::PopDisembodied && e.subject == officer;
  }
  CHECK(embodied == 1);
  CHECK(disembodied == 1);
}

// ---- Spec 68: a pool body on the map ---------------------------------------------------------------------------------

TEST_CASE("a pool body comes onto the map when taken and leaves it when the player goes (A1)") {
  Rig r;
  r.hands.on_map_pool = true;
  r.hands.on_map.clear();  // nobody has a map body: the home is a pool body too
  r.hands.slots = 2;
  r.join(kAda);
  CHECK(r.hands.arrived == std::vector<EntityId>{900});
  r.cmd(PlayerPossess, 1, kCop);
  CHECK(r.hands.arrived == std::vector<EntityId>{900, 901});
  CHECK(r.hands.left == std::vector<EntityId>{900});  // Ada's pool body left the map
  CHECK(r.hands.taken == std::set<EntityId>{901});
  r.cmd(PlayerReturn, 1);  // Ada's body is still held (dwell): the same body comes back onto the map
  CHECK(r.hands.embodies == 2);
  CHECK(r.hands.arrived == std::vector<EntityId>{900, 901, 900});
  CHECK(r.hands.left == std::vector<EntityId>{900, 901});
}

TEST_CASE("with pool bodies on the map, a pool body counts toward the map's one pair of hands (A1)") {
  Rig r;
  r.hands.on_map_pool = true;
  r.hands.slots = 2;
  r.join(kAda);  // a map body
  r.join(kCop);  // would be a second pair of hands on the map
  CHECK(r.last_refusal() == PlayerRefusal::MapTaken);
  CHECK(r.players->players().size() == 1);
  CHECK(r.hands.embodies == 0);  // refused before any body was taken from the pool
}

TEST_CASE("in the one world, the officer comes onto the block where the player starts, moves, and leaves it (A1)" * doctest::skip(World::kDebugBuild)) {
  tier0::Tier0 t = tier0::make_run(1);
  t.world->check_every_tick = false;
  const PopId you = census_pop_id(static_cast<std::uint32_t>(*t.census->person_of(t.run->controlled())));
  t.world->submit({0, PlayerJoin, 0, static_cast<std::int64_t>(you), static_cast<std::int64_t>(DetachLevel::Free)});
  const PopId officer = t.force->officers().front();
  t.world->submit({0, PlayerPossess, 1, static_cast<std::int64_t>(officer), 0});
  t.world->run_ticks(1);
  const EntityId body = t.players->players()[0].body;
  REQUIRE(body != 0);
  const Cell gate = *t.map->find_object("pop_start", "player");
  CHECK(t.actors->actor(body).pos == gate);
  CHECK(!t.bodies->get(body).away);
  CHECK(t.ai->possessed() == body);
  int arrived = 0;
  for (const Event& e : t.world->log.all()) arrived += e.type == EventType::ArrivedOnMap && e.subject == body && e.a == static_cast<std::int64_t>(officer);
  CHECK(arrived == 1);
  // He walks: a move to the yard gate, like any command for the body the player is in.
  const Cell start = *t.map->find_object("found", "stranger");
  t.world->submit({0, static_cast<std::uint16_t>(CommandType::MoveTo), body, pack_cell(start), 0});
  t.world->run_ticks(kTicksPerGameHour / 60);  // a minute of game time
  CHECK(!(t.actors->actor(body).pos == gate));
  t.world->submit({0, PlayerReturn, 1, 0, 0});
  t.world->run_ticks(1);
  CHECK(t.bodies->get(body).away);  // off the block at once
  CHECK(t.ai->possessed() == t.run->controlled());
  t.world->jump(t.world->clock.tick + 2 * kTicksPerGameHour);
  CHECK(t.pops->registry().embodied_body(officer) == 0);  // then back to the pool after the dwell
  CHECK(t.bodies->get(body).vacant);
}
