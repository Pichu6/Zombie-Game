// Power network, fridge and light. Specs: docs/tier0/specs/17-power-network.md, 19-light-field.md
#include "zs/power.h"

#include <algorithm>
#include <stdexcept>

#include "toml.hpp"
#include "zs/detmath.h"
#include "zs/world.h"

namespace zs {
namespace {

std::optional<ResourceId> find_resource(const Ledger& l, const std::string& name) {
  const auto& names = l.resource_names();
  for (std::size_t i = 0; i < names.size(); ++i)
    if (names[i] == name) return static_cast<ResourceId>(i);
  return std::nullopt;
}

double log_lerp(double a, double b, double t) {
  return detmath::pow(10.0, detmath::log10(a) + (detmath::log10(b) - detmath::log10(a)) * t);
}

constexpr double kDt = kBodyStepTicks * kTickSeconds;  // fuel and fridge step (s)
constexpr double kIndoorDaylight = 0.02;     // share of outdoor light indoors (estimate; daylight factor ~1-5 %)
constexpr double kGlass = 0.85;              // clear single glazing transmission (product data)
constexpr double kCurtainCover = 0.97;       // a heavy curtain (estimate)
constexpr double kWindowArea_m2 = 1.2;       // the lit patch seen from outside
// A light stands out when what it throws at the observer beats 5 % of the ambient plus an absolute
// floor of dark-adapted vision (estimate; spec 19 rule 4).
constexpr double kContrast = 0.05, kDarkFloorLux = 0.0005;

}  // namespace

Power::Power(World& w, SiteMap& map, Assemblies& items, Actors& actors) : map_(map), items_(items), actors_(actors) {
  gasoline_ = w.ledger.add_resource("gasoline_ul");
  spoiled_ = w.ledger.add_resource("spoiled_food_j");
  burned_ = w.ledger.add_owner("sink:burned");
  const auto food = find_resource(w.ledger, "food_j");
  if (!food) throw std::logic_error("power: Bodies must register food_j first");
  food_ = *food;
  spoil_conv_ = w.ledger.add_conversion({food_, spoiled_, 1, 1});
}

void Power::load_file(const std::string& path) {
  const toml::table t = toml::parse_file(path);
  if (const auto* g = t["generator"].as_table()) {
    gen_.capacity_w = (*g)["capacity_w"].value_or(gen_.capacity_w);
    gen_.surge_w = (*g)["surge_w"].value_or(gen_.surge_w);
    gen_.idle_lph = (*g)["idle_lph"].value_or(gen_.idle_lph);
    gen_.full_lph = (*g)["full_lph"].value_or(gen_.full_lph);
    gen_.tank_l = (*g)["tank_l"].value_or(gen_.tank_l);
    gen_.noise_db_1m = (*g)["noise_db_1m"].value_or(gen_.noise_db_1m);
  }
  if (const auto* f = t["fridge"].as_table()) {
    fridge_.cold_c = (*f)["cold_c"].value_or(fridge_.cold_c);
    fridge_.room_c = (*f)["room_c"].value_or(fridge_.room_c);
    fridge_.cool_tau_h = (*f)["cool_tau_h"].value_or(fridge_.cool_tau_h);
    fridge_.warm_tau_h = (*f)["warm_tau_h"].value_or(fridge_.warm_tau_h);
    fridge_.life_days_at_4c = (*f)["life_days_at_4c"].value_or(fridge_.life_days_at_4c);
    fridge_.halving_c = (*f)["halving_c"].value_or(fridge_.halving_c);
  }
  if (const auto* a = t["circuit"].as_array())
    for (const auto& n : *a) {
      const auto& x = *n.as_table();
      circuits_.push_back({x["id"].value<std::string>().value(), x["rating_w"].value_or(1800.0)});
    }
  if (const auto* a = t["receiver"].as_array())
    for (const auto& n : *a) {
      const auto& x = *n.as_table();
      ReceiverDef d;
      d.kind = x["kind"].value<std::string>().value();
      d.demand_w = x["demand_w"].value_or(0.0);
      d.surge_w = x["surge_w"].value_or(d.demand_w);
      d.lumens = x["lumens"].value_or(0.0);
      d.on_at_start = x["on"].value_or(false);
      const std::string c = x["circuit"].value<std::string>().value();
      auto it = std::find_if(circuits_.begin(), circuits_.end(), [&](const CircuitDef& cd) { return cd.id == c; });
      if (it == circuits_.end()) throw std::runtime_error("power: unknown circuit " + c);
      d.circuit = static_cast<std::size_t>(it - circuits_.begin());
      rdefs_.push_back(d);
    }
  tripped_.assign(circuits_.size(), false);
}

void Power::setup(World& w) {
  const auto& objs = map_.objects();
  for (std::size_t i = 0; i < objs.size(); ++i) {
    const MapObject& o = objs[i];
    for (std::size_t d = 0; d < rdefs_.size(); ++d)
      if (rdefs_[d].kind == o.kind) receivers_.push_back({actors_.object_entity(i), d, o.at, rdefs_[d].on_at_start, false});
    if (o.kind == "generator") {
      gen_object_ = actors_.object_entity(i);
      gen_cell_ = o.at;
    } else if (o.kind == "fridge") {
      fridge_object_ = actors_.object_entity(i);
    } else if (o.kind == "power_panel") {
      panel_object_ = actors_.object_entity(i);
    }
  }
  if (!gen_object_ || !panel_object_) throw std::runtime_error("power: the map needs a generator and a power_panel");
  if (!actors_.store_of(gen_object_)) throw std::runtime_error("power: the generator needs a tank (gasoline_l)");
  fridge_c_ = fridge_.cold_c;
  build_spaces();
  // Grid failure: a scheduled world event in [day 2, day 5) (spec 17 rule 5).
  grid_fail_tick_ = 2 * kTicksPerGameDay +
                    static_cast<std::int64_t>(w.rng.uniform_int(0, Purpose::GridFailure, 3 * kTicksPerGameDay));
  w.scheduler.schedule({grid_fail_tick_, EventType::GridFailed, 0, 0, 0, 0});
  // The executor reaches power and light through hooks.
  actors_.power_check = [this](const World& wr, EntityId body, EntityId target, const std::string& how) {
    (void)body;
    if (target == gen_object_) {
      if (how == "start") return running_ ? Reason::AlreadyDone : fuel_ul(wr) <= 0 ? Reason::NoFuel : Reason::Ok;
      if (how == "stop") return running_ ? Reason::Ok : Reason::AlreadyDone;
      if (how == "refuel") return tank_ul() - fuel_ul(wr) < 1000 ? Reason::Full : Reason::Ok;
      return Reason::WrongTarget;
    }
    if (target == panel_object_) {
      if (how == "transfer_generator") return transfer_ == Transfer::Generator ? Reason::AlreadyDone : Reason::Ok;
      if (how == "transfer_mains") return transfer_ == Transfer::Mains ? Reason::AlreadyDone : Reason::Ok;
      if (how == "reset")
        return std::find(tripped_.begin(), tripped_.end(), true) == tripped_.end() ? Reason::AlreadyDone : Reason::Ok;
      return Reason::WrongTarget;
    }
    const int r = receiver_of(target);
    if (r < 0 || (how != "on" && how != "off")) return Reason::WrongTarget;
    return receivers_[static_cast<std::size_t>(r)].on == (how == "on") ? Reason::AlreadyDone : Reason::Ok;
  };
  actors_.power_apply = [this](World& wr, EntityId body, EntityId target, const std::string& how, EventId cause) {
    if (how == "start") start_generator(wr, cause);
    else if (how == "stop") stop_generator(wr, cause);
    else if (how == "refuel") refuel(wr, body, cause);
    else if (how == "transfer_generator") set_transfer(wr, Transfer::Generator, cause);
    else if (how == "transfer_mains") set_transfer(wr, Transfer::Mains, cause);
    else if (how == "reset") reset_breakers(wr, cause);
    else if (how == "on" || how == "off") set_on(wr, target, how == "on", cause);
  };
  actors_.light_lux = [this](const World& wr, const Cell& c) { return lux_at(wr, c); };
  solve(w, 0);
}

// ---- Network -----------------------------------------------------------------------------------------

int Power::receiver_of(EntityId object) const {
  for (std::size_t i = 0; i < receivers_.size(); ++i)
    if (receivers_[i].object == object) return static_cast<int>(i);
  return -1;
}

bool Power::powered(EntityId object) const {
  const int r = receiver_of(object);
  return r >= 0 && receivers_[static_cast<std::size_t>(r)].powered;
}

std::int64_t Power::fuel_ul(const World& w) const { return w.ledger.balance(*actors_.store_of(gen_object_), gasoline_); }

double Power::fuel_lph() const {
  if (!running_) return 0;
  return gen_.idle_lph + (gen_.full_lph - gen_.idle_lph) * std::min(1.0, load_w_ / gen_.capacity_w);
}

void Power::solve(World& w, EventId cause, int switched_on) {
  for (Receiver& r : receivers_) r.powered = false;
  load_w_ = 0;
  const bool on_generator = transfer_ == Transfer::Generator && running_;
  const bool live = (transfer_ == Transfer::Mains && mains_alive_) || on_generator;
  if (live) {
    std::vector<double> circuit_w(circuits_.size(), 0);
    for (const Receiver& r : receivers_)
      if (r.on) circuit_w[rdefs_[r.def].circuit] += rdefs_[r.def].demand_w;
    for (std::size_t c = 0; c < circuits_.size(); ++c) {
      if (tripped_[c]) continue;
      if (circuit_w[c] > circuits_[c].rating_w) {
        tripped_[c] = true;
        w.emit(EventType::BreakerTripped, panel_object_, cause, static_cast<std::int64_t>(c));
        continue;
      }
      load_w_ += circuit_w[c];
      for (Receiver& r : receivers_)
        if (r.on && rdefs_[r.def].circuit == c) r.powered = true;
    }
  }
  if (!on_generator) return;
  // Brownout and stall: over the running rating, or a motor start over the surge rating (rule 2).
  if (load_w_ > gen_.capacity_w) {
    stall(w, StallCause::Overload, cause);
  } else if (switched_on >= 0) {
    const Receiver& r = receivers_[static_cast<std::size_t>(switched_on)];
    const ReceiverDef& d = rdefs_[r.def];
    if (r.powered && load_w_ - d.demand_w + d.surge_w > gen_.surge_w) stall(w, StallCause::Surge, cause);
  }
}

void Power::stall(World& w, StallCause why, EventId cause) {
  running_ = false;
  w.emit(why == StallCause::NoFuel ? EventType::GeneratorOutOfFuel : EventType::GeneratorStalled, gen_object_, cause,
         static_cast<std::int64_t>(why));
  solve(w, cause);
}

bool Power::start_generator(World& w, EventId cause) {
  if (running_ || fuel_ul(w) <= 0) return false;
  running_ = true;
  solve(w, cause);
  return running_;
}

bool Power::stop_generator(World& w, EventId cause) {
  if (!running_) return false;
  running_ = false;
  solve(w, cause);
  return true;
}

bool Power::set_transfer(World& w, Transfer t, EventId cause) {
  if (transfer_ == t) return false;
  transfer_ = t;
  solve(w, cause);
  return true;
}

bool Power::set_on(World& w, EntityId object, bool on, EventId cause) {
  const int r = receiver_of(object);
  if (r < 0 || receivers_[static_cast<std::size_t>(r)].on == on) return false;
  receivers_[static_cast<std::size_t>(r)].on = on;
  solve(w, cause, on ? r : -1);
  return true;
}

bool Power::reset_breakers(World& w, EventId cause) {
  if (std::find(tripped_.begin(), tripped_.end(), true) == tripped_.end()) return false;
  tripped_.assign(circuits_.size(), false);
  solve(w, cause);
  return true;
}

std::int64_t Power::refuel(World& w, EntityId body, EventId cause) {
  const OwnerId tank = *actors_.store_of(gen_object_);
  std::int64_t moved = 0;
  for (EntityId c : actors_.carried(w, body)) {
    const std::int64_t room = tank_ul() - w.ledger.balance(tank, gasoline_);
    if (room <= 0) break;
    if (auto s = actors_.store_of(c)) moved += w.ledger.transfer(w.clock.tick, *s, tank, gasoline_, room, cause);
  }
  return moved;
}

void Power::fail_grid(World& w, EventId cause) {
  if (!mains_alive_) return;
  mains_alive_ = false;
  const EventId id = w.emit(EventType::GridFailed, 0, cause);
  solve(w, id);
}

void Power::on_scheduled(World& w, const ScheduledEvent& e) {
  if (e.type == EventType::GridFailed) fail_grid(w, e.cause);
}

// ---- Slow steps: fuel and fridge ------------------------------------------------------------------------

void Power::step_fuel(World& w) {
  if (!running_) return;
  const double ul = fuel_lph() * 1e6 * kDt / 3600.0 + burn_rest_ul_;
  const auto whole = static_cast<std::int64_t>(ul);
  burn_rest_ul_ = ul - static_cast<double>(whole);
  const OwnerId tank = *actors_.store_of(gen_object_);
  w.ledger.transfer(w.clock.tick, tank, burned_, gasoline_, std::min(whole, w.ledger.balance(tank, gasoline_)), 0);
  if (w.ledger.balance(tank, gasoline_) <= 0) {
    burn_rest_ul_ = 0;
    stall(w, StallCause::NoFuel, 0);
  }
}

void Power::step_fridge(World& w) {
  if (!fridge_object_) return;
  const bool cooling = powered(fridge_object_);
  const double target = cooling ? fridge_.cold_c : fridge_.room_c;
  const double tau_s = (cooling ? fridge_.cool_tau_h : fridge_.warm_tau_h) * 3600.0;
  fridge_c_ += (target - fridge_c_) * -detmath::expm1(-kDt / tau_s);
  const auto store = actors_.store_of(fridge_object_);
  const std::int64_t food = store ? w.ledger.balance(*store, food_) : 0;
  if (food <= 0) {
    spoil_ = 0;  // an empty fridge: the next batch starts fresh
    return;
  }
  // Shelf life halves for every `halving_c` above 4 C (a Q10-style rule; estimate).
  const double life_s =
      fridge_.life_days_at_4c * 86400.0 * detmath::pow(2.0, (4.0 - fridge_c_) / fridge_.halving_c);
  spoil_ += kDt / life_s;
  if (spoil_ >= 1) {
    w.ledger.convert(w.clock.tick, *store, *store, spoil_conv_, food, 0);
    w.emit(EventType::FoodSpoiled, fridge_object_, 0, food);
    spoil_ = 0;
  }
}

void Power::update(World& w) {
  if (!w.clock.is_body_step()) return;
  step_fuel(w);
  step_fridge(w);
}

// ---- Light ---------------------------------------------------------------------------------------------

double Power::ambient_lux(double hour) {
  // Night ~0.1 (moonlit), twilight ~10, day ~10 000 (IES tables); log-linear between.
  if (hour < 5 || hour >= 21) return 0.1;
  if (hour < 6) return log_lerp(0.1, 10, hour - 5);
  if (hour < 7) return log_lerp(10, 10000, hour - 6);
  if (hour < 19) return 10000;
  if (hour < 20) return log_lerp(10000, 10, hour - 19);
  return log_lerp(10, 0.1, hour - 20);
}

std::size_t Power::cell_index(const Cell& c) const {
  const auto& lv = map_.levels();
  const auto li = static_cast<std::size_t>(std::find(lv.begin(), lv.end(), c.z) - lv.begin());
  return (li * static_cast<std::size_t>(map_.height()) + static_cast<std::size_t>(c.y)) *
             static_cast<std::size_t>(map_.width()) + static_cast<std::size_t>(c.x);
}

// Walled spaces: cells joined by edges with nothing built on them. Light does not pass doors or
// windows, open or not (a Tier 0 simplification); walls don't change in Tier 0, so built once.
void Power::build_spaces() {
  const std::size_t n = map_.levels().size() * static_cast<std::size_t>(map_.width() * map_.height());
  spaces_.assign(n, -1);
  space_cells_.clear();
  for (int z : map_.levels())
    for (int y = 0; y < map_.height(); ++y)
      for (int x = 0; x < map_.width(); ++x) {
        const Cell start{z, x, y};
        if (!map_.walkable(start) || spaces_[cell_index(start)] >= 0) continue;
        const int id = static_cast<int>(space_cells_.size());
        int count = 0;
        std::vector<Cell> stack{start};
        spaces_[cell_index(start)] = id;
        while (!stack.empty()) {
          const Cell c = stack.back();
          stack.pop_back();
          ++count;
          for (const Cell& nb : {Cell{z, c.x + 1, c.y}, Cell{z, c.x - 1, c.y}, Cell{z, c.x, c.y + 1}, Cell{z, c.x, c.y - 1}}) {
            if (!map_.in_bounds(nb) || !map_.walkable(nb) || spaces_[cell_index(nb)] >= 0) continue;
            if (map_.edge(SiteMap::edge_between(c, nb)).kind != EdgeKind::None) continue;
            spaces_[cell_index(nb)] = id;
            stack.push_back(nb);
          }
        }
        space_cells_.push_back(count);
      }
}

int Power::space_of(const Cell& c) const { return map_.in_bounds(c) ? spaces_[cell_index(c)] : -1; }

double Power::lamp_lux(const Cell& c) const {
  const int s = space_of(c);
  if (s < 0) return 0;
  double lumens = 0;
  for (const Receiver& r : receivers_)
    if (r.powered && rdefs_[r.def].lumens > 0 && space_of(r.at) == s) lumens += rdefs_[r.def].lumens;
  // All the light lands on the floor, 1 m2 per cell (spec 19: lumens / floor area, simple).
  return lumens / static_cast<double>(space_cells_[static_cast<std::size_t>(s)]);
}

double Power::lux_at(const World& w, const Cell& c) const {
  const double ambient = ambient_lux(w.clock.hour());
  return (is_indoor(map_.floor(c)) ? kIndoorDaylight * ambient : ambient) + lamp_lux(c);
}

std::vector<WindowLeak> Power::window_leaks(const World& w) const {
  std::vector<WindowLeak> out;
  const double outside = ambient_lux(w.clock.hour());
  for (const auto& [id, ref] : actors_.edges()) {
    if (items_.def(items_.get(id).def).family != "window") continue;
    const Cell inner{ref.z, ref.x, ref.y};
    const Cell outer = ref.vertical ? Cell{ref.z, ref.x - 1, ref.y} : Cell{ref.z, ref.x, ref.y - 1};
    WindowLeak leak;
    leak.window = id;
    leak.edge = ref;
    for (const Cell& c : {inner, outer})
      if (map_.in_bounds(c) && is_indoor(map_.floor(c))) leak.room_lux = std::max(leak.room_lux, lamp_lux(c));
    const Edge& ed = map_.edge(ref);
    const double curtain = actors_.covered(w, id) ? kCurtainCover : 0;
    double boards = 0;
    if (ed.barricade != 0) {
      const Instance& bar = items_.get(ed.barricade);
      const ItemDef& d = items_.def(bar.def);
      int slots = 0, filled = 0;
      for (std::size_t i = 0; i < d.slots.size(); ++i)
        if (d.slots[i].accepts == "plank") {
          ++slots;
          filled += bar.slots[i] != 0;
        }
      if (slots > 0) boards = static_cast<double>(filled) / slots;
    }
    leak.cover = 1 - (1 - curtain) * (1 - boards);
    const double transmission = ed.open || ed.breached ? 1.0 : kGlass;
    leak.leak_lux = leak.room_lux * transmission * (1 - leak.cover);
    // The window as a small source: it throws leak * area / d^2 at the observer.
    const double d = detmath::sqrt(leak.leak_lux * kWindowArea_m2 / (kContrast * outside + kDarkFloorLux));
    leak.seen_m = d >= 1 ? d : 0;
    out.push_back(leak);
  }
  return out;
}

// ---- Persistence -----------------------------------------------------------------------------------------

void Power::save(ArchiveWriter& a) const {
  a.boolean(mains_alive_);
  a.u8(static_cast<std::uint8_t>(transfer_));
  a.boolean(running_);
  a.f64(load_w_);
  a.u64(tripped_.size());
  for (bool t : tripped_) a.boolean(t);
  a.u64(receivers_.size());
  for (const Receiver& r : receivers_) {
    a.boolean(r.on);
    a.boolean(r.powered);
  }
  a.f64(burn_rest_ul_);
  a.f64(fridge_c_);
  a.f64(spoil_);
  a.i64(grid_fail_tick_);
}

void Power::load(ArchiveReader& r) {
  mains_alive_ = r.boolean();
  transfer_ = static_cast<Transfer>(r.u8());
  running_ = r.boolean();
  load_w_ = r.f64();
  if (r.u64() != tripped_.size()) throw std::runtime_error("power: save does not match the circuits");
  for (std::size_t i = 0; i < tripped_.size(); ++i) tripped_[i] = r.boolean();
  if (r.u64() != receivers_.size()) throw std::runtime_error("power: save does not match the receivers");
  for (Receiver& rc : receivers_) {
    rc.on = r.boolean();
    rc.powered = r.boolean();
  }
  burn_rest_ul_ = r.f64();
  fridge_c_ = r.f64();
  spoil_ = r.f64();
  grid_fail_tick_ = r.i64();
}

}  // namespace zs
