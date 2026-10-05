#include "zs/demo.h"

#include <array>

#include "zs/hazard.h"

namespace zs::demo {

void GeneratorSystem::stop(World& w, EventType why, EventId cause) {
  running_ = false;
  w.emit(why, ids_.generator, cause);
}

void GeneratorSystem::apply_command(World& w, const Command& c, EventId cause) {
  switch (c.type) {
    case StartGenerator:
      if (!running_ && !broken_ && w.ledger.balance(ids_.tank, ids_.fuel_ml) > 0) {
        running_ = true;
        started_by_ = w.emit(EventType::DemoGeneratorStarted, ids_.generator, cause);
      }
      break;
    case StopGenerator:
      if (running_) stop(w, EventType::DemoGeneratorStopped, cause);
      break;
    case Refuel:
      if (!running_) w.ledger.transfer(w.clock.tick, ids_.cans, ids_.tank, ids_.fuel_ml, c.a, cause);
      break;
    case Repair:
      broken_ = false;
      break;
    default:
      break;
  }
}

void GeneratorSystem::on_scheduled(World& w, const ScheduledEvent& e) {
  if (e.type != EventType::DemoRefuelDelivery) return;
  const EventId id = w.emit(EventType::DemoRefuelDelivery, ids_.can_from_stash, e.cause);
  w.ledger.transfer(w.clock.tick, ids_.stash, ids_.cans, ids_.fuel_ml, e.a, id);
  w.locations.place(ids_.can_from_stash, ids_.building);
}

void GeneratorSystem::update(World& w) {
  if (!running_) return;
  // Exact integer flow: kFuelMlPerHour mL over kTicksPerGameHour ticks, remainder carried.
  burn_remainder_ += kFuelMlPerHour;
  const std::int64_t ml = burn_remainder_ / kTicksPerGameHour;
  burn_remainder_ %= kTicksPerGameHour;
  if (ml > 0 && w.ledger.convert(w.clock.tick, ids_.tank, ids_.house, ids_.burn, ml, started_by_) < ml) {
    stop(w, EventType::DemoOutOfFuel, started_by_);
    return;
  }
  const std::array<double, 2> rates = {kSparkPlugRate, kStallRate};
  if (auto which = resolve_exit_set(w.rng, ids_.generator, rates, kTickSeconds)) {
    broken_ = (*which == 0);
    stop(w, EventType::Breakdown, started_by_);
  }
}

void GeneratorSystem::save(ArchiveWriter& out) const {
  out.boolean(running_);
  out.boolean(broken_);
  out.i64(burn_remainder_);
  out.u64(started_by_);
}

void GeneratorSystem::load(ArchiveReader& in) {
  running_ = in.boolean();
  broken_ = in.boolean();
  burn_remainder_ = in.i64();
  started_by_ = in.u64();
}

std::unique_ptr<World> make_world(std::uint64_t seed) {
  auto w = std::make_unique<World>(seed);
  Ids ids{};
  ids.fuel_ml = w->ledger.add_resource("fuel_ml");
  ids.electric_kj = w->ledger.add_resource("electric_kj");
  ids.tank = w->ledger.add_owner("generator.tank");
  ids.cans = w->ledger.add_owner("building.cans");
  ids.stash = w->ledger.add_owner("house_b.stash");
  ids.house = w->ledger.add_owner("building.grid");
  ids.burn = w->ledger.add_conversion({ids.fuel_ml, ids.electric_kj, 1, 7});
  w->ledger.mint(ids.tank, ids.fuel_ml, 4000);
  w->ledger.mint(ids.cans, ids.fuel_ml, 10000);
  w->ledger.mint(ids.stash, ids.fuel_ml, 5000);
  w->locations.place(ids.generator, ids.building);
  w->add_system(std::make_unique<GeneratorSystem>(ids));
  w->close_setup();
  w->scheduler.schedule({2 * kTicksPerGameDay + 10 * kTicksPerGameHour, EventType::DemoRefuelDelivery,
                         ids.can_from_stash, 1, 5000, 0});
  return w;
}

Ids ids_of(const World& w) { return w.system<GeneratorSystem>()->ids(); }

void scripted_player(World& w) {
  const Ids ids = ids_of(w);
  const std::int64_t t = w.clock.tick % kTicksPerGameDay;
  auto* gen = w.system<GeneratorSystem>();
  if (t == 18 * kTicksPerGameHour && !gen->running()) {
    if (gen->broken()) w.submit({0, Repair, ids.generator, 0, 0});
    if (w.ledger.balance(ids.tank, ids.fuel_ml) < 3000) w.submit({0, Refuel, ids.generator, 3000, 0});
    w.submit({0, StartGenerator, ids.generator, 0, 0});
  }
  if (t == 23 * kTicksPerGameHour && gen->running()) w.submit({0, StopGenerator, ids.generator, 0, 0});
}

}  // namespace zs::demo
