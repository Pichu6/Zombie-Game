// Stage 4: doing things (specs 14, 15). A scripted pop drinks, eats, treats a wound, salvages a chair
// and barricades a window, through data processes that find tools and inputs by property.
#include <doctest.h>

#include "zs/tier0.h"

using namespace zs;
using namespace zs::tier0;

namespace {

std::int64_t total_def_mass(const Tier0& t) {
  std::int64_t g = 0;
  for (const auto& [id, inst] : t.items->all()) g += t.items->def(inst.def).mass_g;
  return g;
}

Command use(const Tier0& t, const char* process, EntityId target) {
  Command c;
  c.type = static_cast<std::uint16_t>(CommandType::Use);
  c.subject = t.player;
  c.a = static_cast<std::int64_t>(t.actors->find_process(process).value());
  c.b = static_cast<std::int64_t>(target);
  return c;
}

EntityId item_of(const Tier0& t, const char* def_id) {
  for (const auto& [id, inst] : t.items->all())
    if (t.items->def(inst.def).id == def_id) return id;
  return 0;
}

Tier0 run_script(bool as_ai, std::uint64_t seed = 1) {
  Tier0 t = make_world(seed);
  t.world->check_every_tick = true;
  wound_player(t);
  ScriptedPop script(as_ai);
  while (t.world->clock.tick < 4 * kTicksPerGameHour && script.drive(t)) t.world->step();
  return t;
}

}  // namespace

TEST_CASE("stage 4: the scripted pop eats, drinks, treats its wound, salvages and barricades") {
  const std::int64_t mass_before = total_def_mass(make_world(1));
  Tier0 t = run_script(false);
  const Body& b = t.bodies->get(t.player);
  REQUIRE(b.alive);
  const Wound& w = b.wounds.front();
  CHECK(w.cleaned == Cleaned::Disinfectant);
  CHECK(w.dressed);
  // The bandage is on the wound, not lost.
  CHECK(t.world->locations.parent_of(item_of(t, "bandage")) == w.id);
  // Food and water moved from the stores into the body.
  const auto pantry = t.actors->store_of(*t.actors->find_object("pantry")).value();
  CHECK(t.world->ledger.balance(pantry, t.bodies->ids().food_j) == 60'000'000 - 2'500'000);
  // The chair came apart; two planks and its nails are on the window.
  int done = 0, refused = 0;
  for (const Event& e : t.world->log.all()) {
    done += e.type == EventType::ActionDone;
    refused += e.type == EventType::ActionRefused || e.type == EventType::ActionInterrupted;
  }
  CHECK(done == 13);
  CHECK(refused == 0);
  EntityId bar = 0;
  for (const auto& [id, ref] : t.actors->edges())
    if (t.map->edge(ref).barricade) bar = t.map->edge(ref).barricade;
  REQUIRE(bar != 0);
  int parts = 0;
  for (EntityId p : t.items->get(bar).slots) parts += p != 0;
  CHECK(parts == 3);  // nails + two boards
  CHECK(t.items->works(bar));
  // Salvage and barricading move parts; they never create or lose mass.
  CHECK(total_def_mass(t) == mass_before);
}

TEST_CASE("stage 4: player-controlled and AI-controlled runs of the same intents hash the same (A1)") {
  Tier0 player = run_script(false);
  Tier0 ai = run_script(true);
  CHECK(player.world->clock.tick == ai.world->clock.tick);
  CHECK(player.world->world_hash() == ai.world->world_hash());
  CHECK(player.world->input_log().size() == 13);
  CHECK(ai.world->input_log().empty());
}

TEST_CASE("stage 4: interrupting a transfer at each phase leaves the item in exactly one place (A1)") {
  for (int phase = 0; phase < 3; ++phase) {
    CAPTURE(phase);
    Tier0 t = make_world(1);
    t.world->check_every_tick = true;
    const EntityId hammer = item_of(t, "hammer");
    const EntityId origin = t.world->locations.parent_of(hammer);
    t.world->submit(use(t, "take", hammer));
    // phase 0: cancel while walking; 1: cancel while it is in the hands; 2: let it finish.
    for (int i = 0; i < 20000; ++i) {
      t.world->step();
      const Action& act = t.actors->actor(t.player).action;
      if (phase == 0 && act.phase == Phase::Approach && i > 10) break;
      if (phase == 1 && act.phase == Phase::Work && act.held != 0) break;
      if (phase == 2 && act.phase == Phase::Idle) break;
    }
    if (phase < 2) {
      Command cancel;
      cancel.type = static_cast<std::uint16_t>(CommandType::Cancel);
      cancel.subject = t.player;
      t.world->submit(cancel);
      t.world->step();
    }
    int parents = 0;
    for (const auto& [e, p] : t.world->locations.all()) parents += e == hammer;
    CHECK(parents == 1);
    const EntityId at = t.world->locations.parent_of(hammer);
    if (phase == 0) CHECK(at == origin);
    if (phase == 1) CHECK(at == t.actors->floor_at(t.actors->actor(t.player).pos));
    if (phase == 2) CHECK(at == t.player);
    // Nothing stays reserved: someone else could take it now.
    CHECK(t.actors->preview(*t.world, t.player, *t.actors->find_process("take"), hammer).reason !=
          Reason::InUse);
  }
}

TEST_CASE("stage 4: knowledge hides offers; missing tools and supplies grey them with a reason (A1)") {
  Tier0 t = make_world(1);
  const EntityId wound = wound_player(t);
  auto names = [&](const std::vector<Offer>& offers) {
    std::vector<std::string> out;
    for (const Offer& o : offers) out.push_back(t.actors->processes()[o.process].id);
    return out;
  };
  // With first aid known, the wound offers disinfecting, greyed: no disinfectant in hand.
  auto offers = t.actors->offers_for(*t.world, t.player, wound);
  bool saw_disinfect = false;
  for (const Offer& o : offers)
    if (t.actors->processes()[o.process].id == "disinfect_wound") {
      saw_disinfect = true;
      CHECK_FALSE(o.enabled);
      CHECK(o.reason == Reason::MissingStock);
    }
  CHECK(saw_disinfect);
  // Without first aid knowledge, those offers are not shown at all.
  t.actors->actor(t.player).skill[static_cast<std::size_t>(Trade::FirstAid)] = -1;
  for (const std::string& id : names(t.actors->offers_for(*t.world, t.player, wound)))
    CHECK((id != "disinfect_wound" && id != "dress_wound"));
  // A window offers boarding, greyed for lack of a hammer.
  EntityId window = 0;
  for (const auto& [id, ref] : t.actors->edges())
    if (t.items->def(t.items->get(id).def).family == "window") { window = id; break; }
  const Offer board = t.actors->preview(*t.world, t.player, *t.actors->find_process("barricade"), window);
  CHECK(board.reason == Reason::NoTool);
}

TEST_CASE("stage 4: a crowbar pries faster than a screwdriver; both qualify by property") {
  Tier0 t = make_world(1);
  const std::size_t salvage = *t.actors->find_process("salvage_furniture");
  const EntityId chair = item_of(t, "chair");
  const EntityId crowbar = item_of(t, "crowbar"), driver = item_of(t, "screwdriver");
  t.world->locations.place(driver, t.player);
  const double with_driver = t.actors->preview(*t.world, t.player, salvage, chair).time_s;
  CHECK(t.actors->preview(*t.world, t.player, salvage, chair).enabled);
  t.world->locations.place(crowbar, t.player);
  const double with_crowbar = t.actors->preview(*t.world, t.player, salvage, chair).time_s;
  CHECK(with_crowbar < with_driver);
}

TEST_CASE("stage 4: a skilled pop with a good hammer builds a better barricade (A3)") {
  auto barricade_q = [](int skill, int hammer_q) {
    Tier0 t = make_world(1);
    t.actors->actor(t.player).skill[static_cast<std::size_t>(Trade::Carpentry)] = skill;
    t.items->get(item_of(t, "hammer")).quality = hammer_q;
    wound_player(t);
    ScriptedPop script(false);
    while (t.world->clock.tick < 4 * kTicksPerGameHour && script.drive(t)) t.world->step();
    for (const auto& [id, ref] : t.actors->edges())
      if (t.map->edge(ref).barricade) return t.items->get(t.map->edge(ref).barricade).quality;
    return -1;
  };
  const int novice = barricade_q(20, 50), skilled = barricade_q(80, 80);
  CHECK(novice >= 0);
  CHECK(skilled > novice);
}

TEST_CASE("stage 4: arriving is not working; save mid-action and load replays to the same hash") {
  Tier0 a = make_world(5);
  wound_player(a);
  ScriptedPop sa(false);
  bool checked_arrival = false;
  Phase last = Phase::Idle;
  for (int i = 0; i < 2500; ++i) {
    sa.drive(a);
    a.world->step();
    const Action& act = a.actors->actor(a.player).action;
    if (last == Phase::Approach && act.phase == Phase::Work) {
      CHECK(act.progress_s == 0);
      checked_arrival = true;
    }
    last = act.phase;
  }
  CHECK(checked_arrival);
  const auto bytes = a.world->save();
  Tier0 b = make_world(5);
  wound_player(b);
  b.world->load(bytes);
  ScriptedPop sb = sa;
  CHECK(b.world->state_hash() == a.world->state_hash());
  for (int i = 0; i < 3000; ++i) {
    sa.drive(a);
    sb.drive(b);
    a.world->step();
    b.world->step();
  }
  CHECK(b.world->state_hash() == a.world->state_hash());
}

TEST_CASE("stage 4: golden hash of the scripted pop (same on every compiler)") {
  Tier0 t = run_script(false);
  const std::uint64_t golden = 808867037775555810ULL;
  CHECK(t.world->world_hash() == golden);
  MESSAGE("stage 4 golden world hash: " << t.world->world_hash());
}

TEST_CASE("stage 4 review: walls, windows and built barricades can't be picked up") {
  Tier0 t = run_script(false);
  const std::size_t take = *t.actors->find_process("take");
  for (const auto& [id, ref] : t.actors->edges()) {
    CHECK(t.actors->preview(*t.world, t.player, take, id).reason == Reason::WrongTarget);
    if (const EntityId bar = t.map->edge(ref).barricade)
      CHECK(t.actors->preview(*t.world, t.player, take, bar).reason == Reason::WrongTarget);
  }
}

TEST_CASE("stage 4 review: a command submitted before a save survives the load") {
  Tier0 a = make_world(1);
  a.world->submit(use(a, "take", item_of(a, "hammer")));
  const auto bytes = a.world->save();
  Tier0 b = make_world(1);
  b.world->load(bytes);
  a.world->run_ticks(3000);
  b.world->run_ticks(3000);
  CHECK(a.world->locations.parent_of(item_of(a, "hammer")) == a.player);
  CHECK(b.world->state_hash() == a.world->state_hash());
}

TEST_CASE("stage 4 review: doors opened on the way are closed behind") {
  Tier0 t = run_script(false);
  int open_doors = 0;
  for (const auto& [id, ref] : t.actors->edges()) {
    const Edge& ed = t.map->edge(ref);
    open_doors += (ed.kind == EdgeKind::DoorInterior || ed.kind == EdgeKind::DoorExterior) && ed.open;
  }
  CHECK(open_doors == 0);
}
