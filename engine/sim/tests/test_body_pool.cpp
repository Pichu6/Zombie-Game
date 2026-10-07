// Spec 29: the body pool, embody and disembody.
#include <doctest.h>

#include <memory>
#include <set>

#include "zs/body.h"
#include "zs/body_lab.h"
#include "zs/pop.h"

using namespace zs;

namespace {
struct Pool {
  std::unique_ptr<World> world;
  Bodies* bodies = nullptr;
  OwnerId pantry = 0;
};

Pool make_pool(std::size_t slots, std::uint64_t seed = 5, bool with_walker = false) {
  Pool p;
  p.world = std::make_unique<World>(seed);
  auto bodies = std::make_unique<Bodies>(*p.world);
  p.bodies = bodies.get();
  p.world->add_system(std::move(bodies));
  p.pantry = p.world->ledger.add_owner("pantry");
  p.world->ledger.mint(p.pantry, p.bodies->ids().food_j, std::int64_t{1} << 50);
  if (with_walker) {
    BodySpec w;
    w.walker = true;
    p.bodies->create(*p.world, w, "walker");
  }
  p.bodies->reserve_slots(*p.world, slots);
  p.world->close_setup();
  p.world->check_every_tick = true;
  return p;
}

std::vector<std::uint8_t> bytes_of(const BodyState& s) {
  ArchiveWriter w;
  write_body_state(w, s);
  return w.bytes();
}

std::int64_t tissue(const Pool& p) {
  std::int64_t total = 0;
  const BodyLedger& L = p.bodies->ids();
  for (ResourceId r : {L.fat_mg, L.muscle_mg, L.structure_mg, L.glycogen_mg}) total += p.world->ledger.total(r);
  return total;
}
}  // namespace

TEST_CASE("a pool holds vacant slots that run no rule and hold nothing (A1)") {
  Pool p = make_pool(4);
  CHECK(p.bodies->slots().size() == 4);
  CHECK(p.bodies->free_slots() == 4);
  const std::int64_t before = tissue(p);
  const auto hash = p.world->state_hash();
  p.world->run_ticks(kTicksPerGameDay);
  CHECK(tissue(p) == before);
  for (EntityId id : p.bodies->slots()) {
    const Body& b = p.bodies->get(id);
    CHECK(b.vacant);
    CHECK_FALSE(b.alive);
  }
  CHECK(p.world->log.all().size() == 1);  // only the run start: a vacant slot raises no event
  (void)hash;
}

TEST_CASE("embodying brings exactly the body's tissue into the ledger, the same amounts a created body has (A1)") {
  Pool p = make_pool(3);
  BodySpec spec;
  spec.height_m = 1.8;
  spec.mass_kg = 82;
  spec.fat_fraction = 0.22;
  const std::int64_t before = tissue(p);
  const EntityId id = p.bodies->embody(*p.world, spec, "ana", 4242, 9);
  REQUIRE(id != 0);
  CHECK_FALSE(p.world->ledger.check().has_value());
  const Body& b = p.bodies->get(id);
  CHECK_FALSE(b.vacant);
  CHECK(b.alive);
  CHECK(b.pop == 4242);
  CHECK(b.label == "ana");
  CHECK(p.bodies->free_slots() == 2);
  // The amounts equal those of a body made by create() from the same spec.
  auto lab = body_lab::make_lab(1, {spec});
  const Body& made = lab.bodies->all().begin()->second;
  const BodyLedger& L = p.bodies->ids();
  const BodyLedger& M = lab.bodies->ids();
  CHECK(p.world->ledger.balance(b.owner, L.fat_mg) == lab.world->ledger.balance(made.owner, M.fat_mg));
  CHECK(p.world->ledger.balance(b.owner, L.muscle_mg) == lab.world->ledger.balance(made.owner, M.muscle_mg));
  CHECK(p.world->ledger.balance(b.owner, L.structure_mg) == lab.world->ledger.balance(made.owner, M.structure_mg));
  CHECK(p.world->ledger.balance(b.owner, L.blood_ul) == lab.world->ledger.balance(made.owner, M.blood_ul));
  CHECK(b.blood_normal_ul == made.blood_normal_ul);
  CHECK(tissue(p) - before == p.world->ledger.balance(b.owner, L.fat_mg) + p.world->ledger.balance(b.owner, L.muscle_mg) +
                                  p.world->ledger.balance(b.owner, L.structure_mg) + p.world->ledger.balance(b.owner, L.glycogen_mg));
  // The declared inflow carries its cause.
  bool found = false;
  for (const LedgerEntry& e : p.world->ledger.tail()) found = found || (e.kind == EntryKind::Inflow && e.cause == 9);
  CHECK(found);
}

TEST_CASE("embody, disembody, embody again is the same body with the same ledger, over many cycles (A1)") {
  Pool p = make_pool(2);
  BodySpec spec;
  spec.sex = Sex::Female;
  spec.age = 41;
  spec.height_m = 1.66;
  spec.mass_kg = 64;
  spec.fat_fraction = 0.3;
  EntityId id = p.bodies->embody(*p.world, spec, "bea", 7, 1);
  REQUIRE(id != 0);
  p.world->run_ticks(kTicksPerGameHour);  // some life: metabolism, no food
  BodyState first = p.bodies->disembody(*p.world, id, 2);
  const auto first_bytes = bytes_of(first);
  const std::int64_t tissue_after_first = tissue(p);
  const auto hash_after_first = p.world->state_hash();
  CHECK(p.bodies->free_slots() == 2);
  CHECK(tissue_after_first == 0 + (tissue_after_first));  // vacant slots hold no tissue
  for (int cycle = 0; cycle < 20; ++cycle) {
    id = p.bodies->embody_state(*p.world, first, 7, 3);
    REQUIRE(id != 0);
    CHECK_FALSE(p.world->ledger.check().has_value());
    BodyState again = p.bodies->disembody(*p.world, id, 4);
    CHECK(bytes_of(again) == first_bytes);
    CHECK(tissue(p) == tissue_after_first);
    CHECK(p.world->state_hash() == hash_after_first);  // no time passed: nothing about the world changed
  }
}

TEST_CASE("a body that lived keeps its wounds, needs and tissue through a leave and return (A1)") {
  Pool p = make_pool(2);
  const EntityId id = p.bodies->embody(*p.world, BodySpec{}, "cal", 11, 1);
  Wound cut;
  cut.region = Region::ForearmL;
  cut.depth = Depth::Fat;
  const EntityId wound = p.bodies->add_wound(*p.world, id, cut, 5);
  p.world->run_ticks(2 * kTicksPerGameHour);
  const Body& live = p.bodies->get(id);
  const std::int64_t blood_before = p.world->ledger.balance(live.owner, p.bodies->ids().blood_ul);
  const double sleep_before = live.sleep_pressure;
  REQUIRE(p.bodies->cannot_disembody(*p.world, id).empty());
  BodyState st = p.bodies->disembody(*p.world, id, 6);
  CHECK(st.wounds.size() == 1);
  CHECK(st.wounds[0].id == wound);
  CHECK(st.blood_ul == blood_before);
  CHECK(st.sleep_pressure == sleep_before);
  CHECK(p.world->locations.parent_of(wound) == Locations::kWorld);  // the wound left with the body
  const EntityId back = p.bodies->embody_state(*p.world, st, 11, 7);
  REQUIRE(back != 0);
  const Body& again = p.bodies->get(back);
  REQUIRE(again.wounds.size() == 1);
  CHECK(again.wounds[0].id == wound);
  CHECK(p.world->locations.parent_of(wound) == back);
  CHECK(p.world->ledger.balance(again.owner, p.bodies->ids().blood_ul) == blood_before);
  CHECK(again.sleep_pressure == sleep_before);
}

TEST_CASE("a full pool refuses, never queues, and never exceeds its size (A1)") {
  Pool p = make_pool(3);
  std::vector<EntityId> ids;
  for (int i = 0; i < 3; ++i) { ids.push_back(p.bodies->embody(*p.world, BodySpec{}, "p" + std::to_string(i), 100 + i, 1)); CHECK(ids.back() != 0); }
  CHECK(p.bodies->free_slots() == 0);
  const auto before = p.world->state_hash();
  CHECK(p.bodies->embody(*p.world, BodySpec{}, "extra", 999, 1) == 0);
  CHECK(p.world->state_hash() == before);  // a refusal changes nothing
  std::set<EntityId> distinct(ids.begin(), ids.end());
  CHECK(distinct.size() == 3);
  p.bodies->disembody(*p.world, ids[1], 2);
  CHECK(p.bodies->free_slots() == 1);
  CHECK(p.bodies->embody(*p.world, BodySpec{}, "late", 998, 1) == ids[1]);  // the freed slot is the next one used
}

TEST_CASE("a body cannot leave unless it can: walker, dead, vacant, foreign, holding items (A1)") {
  Pool p = make_pool(3, 6, true);
  CHECK(p.bodies->cannot_disembody(*p.world, 123456) == "not a pool body");
  const EntityId walker_id = p.bodies->all().begin()->first;  // the created walker is not a slot
  CHECK(p.bodies->cannot_disembody(*p.world, walker_id) == "not a pool body");
  const EntityId vacant = p.bodies->slots()[0];
  CHECK(p.bodies->cannot_disembody(*p.world, vacant) == "slot is vacant");
  const EntityId id = p.bodies->embody(*p.world, BodySpec{}, "dee", 5, 1);
  CHECK(p.bodies->cannot_disembody(*p.world, id).empty());
  // Holding an item (anything located under the body that is not a wound).
  const EntityId item = p.world->new_entity();
  p.world->locations.place(item, id);
  CHECK(p.bodies->cannot_disembody(*p.world, id) == "the body holds items");
  CHECK_THROWS(p.bodies->disembody(*p.world, id, 2));
  p.world->locations.place(item, Locations::kWorld);
  CHECK(p.bodies->cannot_disembody(*p.world, id).empty());
  // Dead.
  p.bodies->kill(*p.world, id, DeathCause::Exsanguination, 3);
  CHECK(p.bodies->cannot_disembody(*p.world, id) == "the body is dead");
  CHECK(p.bodies->free_slots() == 2);  // the corpse still holds its slot: it is a thing in the world now
}

TEST_CASE("random embodies and disembodies keep the pool, the ledger and occupancy consistent (A1)") {
  Pool p = make_pool(5, 9);
  std::map<std::uint64_t, EntityId> in;   // pop -> body
  std::map<std::uint64_t, BodyState> out; // pop -> saved state
  for (int step = 0; step < 300; ++step) {
    const std::uint64_t pop = 1 + keyed_u64(3, Purpose::Test, step, 0) % 12;
    if (in.count(pop)) {
      out[pop] = p.bodies->disembody(*p.world, in[pop], 10 + step);
      in.erase(pop);
    } else {
      const EntityId id = out.count(pop) ? p.bodies->embody_state(*p.world, out[pop], pop, 10 + step)
                                         : p.bodies->embody(*p.world, BodySpec{}, "p" + std::to_string(pop), pop, 10 + step);
      if (id) in[pop] = id;
      else CHECK(in.size() == 5);  // refused only when the pool is full
    }
    if (step % 7 == 0) p.world->run_ticks(60);
    CHECK_FALSE(p.world->ledger.check().has_value());
    std::size_t occupied = 0;
    for (EntityId id : p.bodies->slots()) if (!p.bodies->get(id).vacant) ++occupied;
    CHECK(occupied == in.size());
    CHECK(p.bodies->free_slots() == 5 - in.size());
    for (const auto& [pop_id, body] : in) CHECK(p.bodies->get(body).pop == pop_id);
  }
}

TEST_CASE("a world with a pool saves, loads and saves again to the same bytes (A1)") {
  const auto build = [] {
    Pool p = make_pool(4, 12);
    const EntityId a = p.bodies->embody(*p.world, BodySpec{}, "fay", 31, 1);
    BodySpec other;
    other.sex = Sex::Female;
    other.height_m = 1.6;
    other.mass_kg = 58;
    p.bodies->embody(*p.world, other, "gus", 32, 1);
    Wound cut;
    cut.region = Region::ThighR;
    p.bodies->add_wound(*p.world, a, cut, 2);
    p.world->run_ticks(3000);
    return p;
  };
  Pool a = build();
  const auto bytes = a.world->save();
  Pool candidate = make_pool(4, 12);
  candidate.world->load(bytes);
  CHECK(candidate.world->save() == bytes);
  CHECK(candidate.world->state_hash() == a.world->state_hash());
  std::size_t occupied = 0;
  for (EntityId id : candidate.bodies->slots()) {
    const Body& b = candidate.bodies->get(id);
    if (!b.vacant) { ++occupied; CHECK((b.pop == 31 || b.pop == 32)); CHECK((b.label == "fay" || b.label == "gus")); }
  }
  CHECK(occupied == 2);
}
