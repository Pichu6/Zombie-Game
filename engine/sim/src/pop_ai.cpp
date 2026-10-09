// Pop AI. Spec: docs/tier0/specs/24-pop-ai.md
#include "zs/pop_ai.h"

#include <algorithm>
#include <stdexcept>

#include "zs/detmath.h"

namespace zs {
namespace {

constexpr std::int64_t kDecideTicks = 10;                      // a decision step every 5 s
constexpr std::int64_t kRetryTicks = 5 * 60 * 2;               // a failed want waits 5 game min
constexpr double kPanicFear = 0.7, kPanicFearBrave = 0.9;      // fear that drops everything (rule 5)
constexpr double kOrderPushFear = 0.5;                         // an insisted order works below this (rule 4)
constexpr double kDarkLux = 1;                                 // a room this dark is refused (rule 4)
constexpr double kThirst = 0.4, kHunger = 0.5;                 // needs that win over orders and jobs
constexpr double kTiredDay = 0.75, kTiredNight = 0.35;
constexpr double kRefuelBelow = 0.3;                           // of the tank
constexpr double kEngageM = 4;                                 // an armed pop steps up to a walker this close indoors
constexpr double kAvoidM = 8;                                  // an unarmed pop backs off from one this close
constexpr double kInviteM = 25;                                // a found survivor this near is worth the walk
constexpr double kHelpM = 15;                                  // a struggle this close is heard and answered

int rank(Doing d) {
  switch (d) {
    case Doing::Fight:
    case Doing::Flee: return 0;
    case Doing::Treat:
    case Doing::PutDown: return 1;
    case Doing::Drink: return 2;
    case Doing::Eat: return 3;
    case Doing::Sleep: return 4;
    case Doing::Order: return 5;
    case Doing::Arm:
    case Doing::Invite: return 6;
    case Doing::Refuel:
    case Doing::Barricade:
    case Doing::Guard:
    case Doing::Haul:
    case Doing::Search:
    case Doing::Radio: return 7;
    case Doing::Idle: return 9;
  }
  return 9;
}

double weapon_score(const Assemblies& items, EntityId id) {
  return std::max(items.prop(id, Prop::Blunt), 0.9 * items.prop(id, Prop::Lever));
}

}  // namespace

const char* to_string(Job j) {
  constexpr const char* names[] = {"haul", "refuel", "repair", "cook", "guard", "barricade", "search"};
  return names[static_cast<std::size_t>(j)];
}

const char* to_string(Refusal r) {
  switch (r) {
    case Refusal::None: return "none";
    case Refusal::Dark: return "too dark in there";
    case Refusal::OpenGround: return "walkers out there";
    case Refusal::LiveCircuit: return "the wiring is live";
  }
  return "?";
}

const char* to_string(Doing d) {
  constexpr const char* names[] = {"idle",  "fighting", "running", "treating a wound", "drinking", "eating",
                                   "sleeping", "following orders", "getting a weapon", "keeping the generator going",
                                   "boarding windows", "on watch", "hauling", "putting down the dead",
                                   "searching a house", "asking someone to join", "at the radio"};
  return names[static_cast<std::size_t>(d)];
}

PopAI::PopAI(World&, SiteMap& map, Assemblies& items, Bodies& bodies, Actors& actors, Power& power, Walkers& walkers)
    : map_(map), items_(items), bodies_(bodies), actors_(actors), power_(power), walkers_(walkers) {}

void PopAI::add_pop(World& w, EntityId body) {
  PopMind m;
  m.body = body;
  // Default job board (rule 2): keep the power going and board up first, then watch, then haul.
  m.priority = {3, 1, 2, 0, 2, 1, 2};
  m.brave = w.rng.uniform01(body, Purpose::PopTrait) < 0.3;
  m.honest = w.rng.uniform01(body, Purpose::PopTrait) < 0.7;
  m.next_decide = w.clock.tick + static_cast<std::int64_t>(body % kDecideTicks);
  minds_[body] = m;
}

int PopAI::process(const char* id) const {
  const auto p = actors_.find_process(id);
  if (!p) throw std::runtime_error(std::string("pop ai: no process ") + id);
  return static_cast<int>(*p);
}

bool PopAI::empty_can(const World& w, EntityId item) const {
  if (items_.def(items_.get(item).def).id != "fuel_can") return false;
  const auto store = actors_.store_of(item);
  return !store || w.ledger.balance(*store, power_.gasoline()) <= 0;
}

bool PopAI::carries(const World& w, EntityId body, const std::string& def_id) const {
  for (EntityId id : actors_.carried(w, body))
    if (items_.exists(id) && items_.def(items_.get(id).def).id == def_id && !empty_can(w, id)) return true;
  return false;
}

bool PopAI::armed(const World& w, EntityId body) const {
  for (EntityId id : actors_.carried(w, body))
    if (items_.exists(id) && weapon_score(items_, id) >= 0.3) return true;
  return false;
}

std::optional<EntityId> PopAI::loose(const World& w, const std::string& def_id, const Cell& near, bool with_parts) const {
  std::optional<EntityId> best;
  double best_d = 1e300;
  for (const auto& [id, inst] : items_.all()) {
    if (items_.def(inst.def).id != def_id || empty_can(w, id)) continue;
    if (with_parts && std::none_of(inst.slots.begin(), inst.slots.end(), [](EntityId s) { return s != 0; })) continue;
    const EntityId parent = w.locations.parent_of(id);
    const auto at = actors_.floor_cell(parent);
    if (!at) continue;  // carried, mounted, or on a wound
    const double d = distance_m(near, *at) + 10.0 * std::abs(near.z - at->z);
    if (d < best_d) {
      best = id;
      best_d = d;
    }
  }
  return best;
}

std::optional<EntityId> PopAI::loose_weapon(const World& w, const Cell& near) const {
  std::optional<EntityId> best;
  double best_s = 0;
  for (const auto& [id, inst] : items_.all()) {
    const auto at = actors_.floor_cell(w.locations.parent_of(id));
    if (!at) continue;
    double s = weapon_score(items_, id);
    if (items_.def(inst.def).id == "plank") s -= 0.2;  // a plank only if nothing better: they go on windows
    if (s < 0.2) continue;
    const double score = s - 0.01 * (distance_m(near, *at) + 10.0 * std::abs(near.z - at->z));
    if (!best || score > best_s) {
      best = id;
      best_s = score;
    }
  }
  return best;
}

std::optional<Cell> PopAI::safe_cell(const World& w, EntityId body, const std::vector<EntityId>& threats) const {
  const Cell from = actors_.actor(body).pos;
  // One cell per indoor room (the first in scan order), scored by distance to the nearest threat.
  std::map<int, Cell> rep;
  for (int z : map_.levels())
    for (int y = 0; y < map_.height(); ++y)
      for (int x = 0; x < map_.width(); ++x) {
        const Cell c{z, x, y};
        const int r = map_.room_of(c);
        if (r < 0 || !map_.passable(c) || !map_.rooms()[static_cast<std::size_t>(r)].indoor) continue;
        rep.try_emplace(r, c);
      }
  std::vector<std::pair<double, Cell>> scored;
  for (const auto& [r, c] : rep) {
    double s = 1e9;
    for (EntityId t : threats)
      if (auto tc = walkers_.cell_of(t)) s = std::min(s, distance_m(c, *tc) + 10.0 * std::abs(c.z - tc->z));
    if (threats.empty()) s = -distance_m(from, c) - 10.0 * std::abs(from.z - c.z);  // nearest indoors
    scored.emplace_back(-s, c);
  }
  std::stable_sort(scored.begin(), scored.end(), [](const auto& a, const auto& b) { return a.first < b.first; });
  (void)w;
  for (const auto& [s, c] : scored)
    if (c == from || !map_.find_path(from, c).empty()) return c;
  return std::nullopt;
}

std::optional<EntityId> PopAI::window_to_board(const World& w, const Cell& from, EntityId me) const {
  (void)w;
  // Steps from the pop over indoor cells on its level (one flood fill, no stairs): only windows of
  // the house it is in count.
  const int width = map_.width();
  std::vector<int> steps(static_cast<std::size_t>(width * map_.height()), -1);
  auto at = [&](const Cell& c) -> int& { return steps[static_cast<std::size_t>(c.y * width + c.x)]; };
  if (is_indoor(map_.floor(from))) {
    std::vector<Cell> open{from};
    at(from) = 0;
    for (std::size_t i = 0; i < open.size(); ++i) {
      const Cell c = open[i];
      for (const Cell& nb : {Cell{c.z, c.x + 1, c.y}, Cell{c.z, c.x - 1, c.y}, Cell{c.z, c.x, c.y + 1}, Cell{c.z, c.x, c.y - 1}}) {
        if (!map_.in_bounds(nb) || at(nb) >= 0 || !map_.passable(nb) || !is_indoor(map_.floor(nb)) ||
            !map_.can_walk(SiteMap::edge_between(c, nb)))
          continue;
        at(nb) = at(c) + 1;
        open.push_back(nb);
      }
    }
  }
  std::optional<EntityId> best;
  int best_len = 0;
  for (const auto& [id, ref] : actors_.edges()) {
    if (ref.z != from.z || items_.def(items_.get(id).def).family != "window") continue;
    const Edge& e = map_.edge(ref);
    if (e.breached || others_doing(me, Doing::Barricade, id) > 0) continue;  // someone is on this one
    if (e.barricade) {  // already full?
      bool room = false;
      const Instance& bar = items_.get(e.barricade);
      const ItemDef& d = items_.def(bar.def);
      for (std::size_t i = 0; i < d.slots.size(); ++i) room |= d.slots[i].accepts == "plank" && bar.slots[i] == 0;
      if (!room) continue;
    }
    const Cell inner{ref.z, ref.x, ref.y};
    const Cell outer = ref.vertical ? Cell{ref.z, ref.x - 1, ref.y} : Cell{ref.z, ref.x, ref.y - 1};
    for (const Cell& c : {inner, outer}) {
      if (!map_.in_bounds(c) || at(c) < 0) continue;
      if (!best || at(c) < best_len) {
        best = id;
        best_len = at(c);
      }
    }
  }
  return best;
}

std::optional<EntityId> PopAI::treatable_wound(const Body& b) const {
  for (const Wound& wd : b.wounds) {
    if (wd.dressed || wd.type == WoundType::Contusion || wd.type == WoundType::Fracture) continue;
    if (bodies_.wound_bleed_ml_per_min(b, wd) > 0.2 || (wd.contamination >= 0.3 && wd.cleaned == Cleaned::No))
      return wd.id;
  }
  return std::nullopt;
}

int PopAI::others_doing(EntityId me, Doing d, EntityId target) const {
  int n = 0;
  for (const auto& [id, m] : minds_) {
    if (id == me || m.doing != d || !bodies_.get(id).alive) continue;
    if (!target) {
      ++n;
      continue;
    }
    const Action& act = actors_.actor(id).action;
    bool on = act.target == target;
    for (const auto& step : m.plan) on |= step.second == target;
    n += on;
  }
  return n;
}

bool PopAI::generator_hours(const World& w) const {
  const double h = w.clock.hour();
  return careless_ || (h >= 10 && h < 16);  // spec 24 / 26 rule 8: the quietest useful hours
}

bool PopAI::night(const World& w) const {
  const double h = w.clock.hour();
  return h >= 22 || h < 6;
}

EntityId PopAI::guard_on_duty(const World& w) const {
  if (!night(w)) return 0;
  EntityId best = 0;
  int best_p = 99;
  for (const auto& [id, m] : minds_) {
    const Body& b = bodies_.get(id);
    const int p = m.priority[static_cast<std::size_t>(Job::Guard)];
    if (!b.alive || b.spec.walker || p == 0 || id == possessed_) continue;
    if (p < best_p) {
      best = id;
      best_p = p;
    }
  }
  return best;
}

Refusal PopAI::refusal(const World& w, EntityId body, const Cell& to) const {
  const Cell from = actors_.actor(body).pos;
  // Open ground with walkers in sight: leaving the indoors, never coming back in.
  if (is_indoor(map_.floor(from)) && !walkers_.visible_to(w, body).empty()) {
    const auto path = map_.find_path(from, to);
    if (std::any_of(path.begin(), path.end(), [&](const Cell& c) { return !is_indoor(map_.floor(c)); }))
      return Refusal::OpenGround;
  }
  // A dark room it isn't already in.
  if (is_indoor(map_.floor(to)) && map_.room_of(to) != map_.room_of(from) && power_.lux_at(w, to) < kDarkLux)
    return Refusal::Dark;
  return Refusal::None;
}

Refusal PopAI::give_order(World& w, EntityId body, const Order& order) {
  PopMind& m = minds_.at(body);
  Cell to = order.at;
  if (order.type == OrderType::Follow || order.type == OrderType::Attack) {
    const auto c = actors_.cell_of(w, order.target);
    if (!c) return Refusal::None;
    to = *c;
  }
  Refusal r = order.type == OrderType::Hold || order.type == OrderType::None ? Refusal::None : refusal(w, body, to);
  if (r != Refusal::None && order.insist && bodies_.get(body).fear < kOrderPushFear) r = Refusal::None;
  if (r != Refusal::None) {
    w.emit(EventType::PopRefused, body, 0, static_cast<std::int64_t>(r));
    return r;
  }
  m.order = order;
  if (m.doing != Doing::Fight && m.doing != Doing::Flee) cancel(w, m);  // obey now
  return Refusal::None;
}

void PopAI::cancel(World& w, PopMind& m) {
  m.plan.clear();
  m.doing = Doing::Idle;
  if (!actors_.idle(m.body)) {
    Command c;
    c.type = static_cast<std::uint16_t>(CommandType::Cancel);
    c.subject = m.body;
    w.queue_intent(c);
  }
}

bool PopAI::plan_want(World& w, PopMind& m, Doing want) {
  const EntityId me = m.body;
  const Cell pos = actors_.actor(me).pos;
  const Body& b = bodies_.get(me);
  auto& plan = m.plan;
  auto use = [&](const char* p, EntityId target) { plan.emplace_back(process(p), target); };
  auto move = [&](const Cell& c) { plan.emplace_back(-1, static_cast<EntityId>(pack_cell(c))); };
  auto take = [&](const char* def) {
    if (carries(w, me, def)) return true;
    const auto it = loose(w, def, pos);
    if (!it) return false;
    use("take", *it);
    return true;
  };
  plan.clear();
  const auto threats = walkers_.visible_to(w, me);
  switch (want) {
    case Doing::Fight: {
      const EntityId t = m.target ? m.target : (threats.empty() ? 0 : threats.front());
      if (!t) return false;
      use("strike_walker", t);
      break;
    }
    case Doing::PutDown: {
      if (!m.target) return false;
      use("put_down", m.target);
      break;
    }
    case Doing::Flee: {
      const auto c = safe_cell(w, me, threats);
      if (!c || *c == pos) return false;
      move(*c);
      break;
    }
    case Doing::Treat: {
      const auto wound = treatable_wound(b);
      if (!wound) return false;
      const bool bandage = take("bandage");
      const Wound* wd = nullptr;
      for (const Wound& x : b.wounds) if (x.id == *wound) wd = &x;
      if (wd->cleaned == Cleaned::No && take("disinfectant_bottle")) use("disinfect_wound", *wound);
      if (bandage) use("dress_wound", *wound);
      if (plan.empty()) return false;
      break;
    }
    case Doing::Drink: {
      const auto sink = actors_.find_object("sink");
      if (!sink) return false;
      use("drink_tap", *sink);
      break;
    }
    case Doing::Eat: {
      const auto fridge = actors_.find_object("fridge");
      const auto pantry = actors_.find_object("pantry");
      if (fridge && actors_.preview(w, me, static_cast<std::size_t>(process("eat_fresh")), *fridge).enabled)
        use("eat_fresh", *fridge);
      else if (pantry && actors_.preview(w, me, static_cast<std::size_t>(process("eat")), *pantry).enabled) use("eat", *pantry);
      else return false;
      break;
    }
    case Doing::Sleep: {
      // Sleep upstairs when there is an upstairs: walkers have to break in and climb first.
      std::optional<Cell> bed;
      for (int z : map_.levels())
        for (int y = 0; y < map_.height() && !bed; ++y)
          for (int x = 0; x < map_.width() && !bed; ++x) {
            const Cell c{z, x, y};
            if (z > pos.z && map_.passable(c) && is_indoor(map_.floor(c)) && !map_.find_path(pos, c).empty()) bed = c;
          }
      if (bed) move(*bed);
      use("sleep", me);
      break;
    }
    case Doing::Order: {
      const Order& o = m.order;
      if (o.type == OrderType::Go) {
        if (pos == o.at) {
          m.order = Order{};  // arrived
          return false;
        }
        move(o.at);
      } else if (o.type == OrderType::Follow) {
        const auto c = actors_.cell_of(w, o.target);
        if (!c || (c->z == pos.z && distance_m(*c, pos) <= 2)) return false;
        move(*c);
      } else if (o.type == OrderType::Attack) {
        if (!bodies_.get(o.target).alive) {
          m.order = Order{};
          return false;
        }
        use("strike_walker", o.target);
      } else {
        return false;  // hold: stay put
      }
      break;
    }
    case Doing::Arm: {
      const auto weapon = loose_weapon(w, pos);
      if (!weapon) return false;
      use("take", *weapon);
      break;
    }
    case Doing::Refuel: {
      const EntityId gen = power_.generator_object();
      if (!generator_hours(w)) {
        use("stop_generator", gen);
        break;
      }
      const auto panel = actors_.find_object("power_panel");
      if (power_.transfer() != Transfer::Generator && panel) use("switch_to_generator", *panel);
      const double fuel = static_cast<double>(power_.fuel_ul(w)) / static_cast<double>(power_.tank_ul());
      if (fuel < kRefuelBelow && take("fuel_can")) use("refuel_generator", gen);
      if (fuel <= 0 && plan.empty()) return false;
      if (!power_.generator_running()) use("start_generator", gen);
      break;
    }
    case Doing::Barricade: {
      const auto window = window_to_board(w, pos, me);
      if (!window) return false;
      if (!take("hammer") || !take("nail_set")) return false;
      if (!carries(w, me, "plank")) {
        if (auto plank = loose(w, "plank", pos)) use("take", *plank);
        else if (auto chair = loose(w, "chair", pos, true)) {
          use("salvage_furniture", *chair);
          break;  // the planks lie on the floor next time
        } else return false;
      }
      use("barricade", *window);
      break;
    }
    case Doing::Guard: use("shout", me); break;
    case Doing::Idle: {
      if (is_indoor(map_.floor(pos))) return false;
      const auto c = safe_cell(w, me, threats);  // go back inside
      if (!c) return false;
      move(*c);
      break;
    }
    case Doing::Haul: return false;  // gap: no haul process in Tier 0 yet
    case Doing::Invite: {
      if (!m.target) return false;
      use("invite", m.target);
      break;
    }
    case Doing::Search: {
      const auto home = home_to_search(me);
      if (!home) return false;
      const Cell at = home->second;
      if (at.z == pos.z && distance_m(at, pos) <= 2) {
        // Here: the house is searched; bring back the fuel lying in it (spec 26 rule 4).
        searched_.insert(home->first);
        w.emit(EventType::Searched, me, 0, static_cast<std::int64_t>(home->first));
        for (const Found& f : run_ ? run_->found() : std::vector<Found>{}) {
          const Cell fc = actors_.actor(f.body).pos;
          if (fc.z == at.z && distance_m(fc, at) <= 12) known_.insert(f.body);  // calls out, or is heard
        }
        const auto can = loose(w, "fuel_can", pos);
        if (!can || map_.room_of(*actors_.floor_cell(w.locations.parent_of(*can))) == -1) return false;
        const Cell cc = *actors_.floor_cell(w.locations.parent_of(*can));
        if (distance_m(cc, at) > 10) return false;
        use("take", *can);
        break;
      }
      move(at);
      break;
    }
    case Doing::Radio: {
      if (!run_) return false;
      const EntityId radio = run_->radio();
      if (!run_->radio_repaired()) {
        if (!take("screwdriver")) return false;
        use("repair_radio", radio);
        break;
      }
      if (!radio_on()) use("switch_on", radio);
      use("listen_radio", radio);
      use("switch_off", radio);
      break;
    }
  }
  m.doing = want;
  return !plan.empty();
}

void PopAI::send_next(World& w, PopMind& m) {
  const auto [proc, target] = m.plan.front();
  m.plan.pop_front();
  const EntityId me = m.body;
  // Refusals (rule 4) for everything but fighting and running: where is this step going?
  if (m.doing != Doing::Fight && m.doing != Doing::Flee && m.doing != Doing::Treat && m.doing != Doing::Drink &&
      m.doing != Doing::Eat && m.doing != Doing::Sleep && m.doing != Doing::Idle && m.doing != Doing::PutDown) {
    std::optional<Cell> to = proc < 0 ? std::optional<Cell>(unpack_cell(static_cast<std::int64_t>(target)))
                                      : actors_.cell_of(w, target);
    if (to) {
      Refusal r = refusal(w, me, *to);
      if (r != Refusal::None && m.doing == Doing::Order && m.order.insist && bodies_.get(me).fear < kOrderPushFear)
        r = Refusal::None;
      if (r != Refusal::None) {
        w.emit(EventType::PopRefused, me, 0, static_cast<std::int64_t>(r), static_cast<std::int64_t>(m.doing));
        if (m.doing == Doing::Order) m.order = Order{};
        m.retry_after[m.doing] = w.clock.tick + kRetryTicks;
        m.plan.clear();
        m.doing = Doing::Idle;
        return;
      }
    }
  }
  Command c;
  c.subject = me;
  if (proc < 0) {
    c.type = static_cast<std::uint16_t>(CommandType::MoveTo);
    c.a = static_cast<std::int64_t>(target);
  } else {
    if (!actors_.preview(w, me, static_cast<std::size_t>(proc), target).enabled) {
      m.retry_after[m.doing] = w.clock.tick + kRetryTicks;
      m.plan.clear();
      m.doing = Doing::Idle;
      return;
    }
    c.type = static_cast<std::uint16_t>(CommandType::Use);
    c.a = proc;
    c.b = static_cast<std::int64_t>(target);
  }
  w.queue_intent(c);
}

std::optional<EntityId> PopAI::to_invite(EntityId me) {
  if (!run_) return std::nullopt;
  const Cell pos = actors_.actor(me).pos;
  std::optional<EntityId> best;
  std::size_t best_len = 0;
  for (const Found& f : run_->found()) {
    if (!run_->waiting(f.body)) continue;
    const Cell at = actors_.actor(f.body).pos;
    // Only someone the group knows about: found in a search, met in the same room, or calling at the gate.
    if (map_.room_of(at) == map_.room_of(pos)) known_.insert(f.body);  // met: not forgotten after leaving the room
    if (f.role != Role::Stranger && !known_.contains(f.body)) continue;
    if (distance_m(pos, at) + 10.0 * std::abs(pos.z - at.z) > kInviteM) continue;
    const auto path = map_.find_path(pos, at);
    if (path.empty() && !(at == pos)) continue;
    if (!best || path.size() < best_len) {
      best = f.body;
      best_len = path.size();
    }
  }
  return best;
}

std::optional<std::pair<EntityId, Cell>> PopAI::home_to_search(EntityId me) const {
  const auto& objs = map_.objects();
  for (std::size_t i = 0; i < objs.size(); ++i) {
    if (objs[i].kind != "home") continue;
    const EntityId e = actors_.object_entity(i);
    if (searched_.contains(e)) continue;
    if (others_doing(me, Doing::Search, static_cast<EntityId>(pack_cell(objs[i].at))) > 0) continue;
    return std::make_pair(e, objs[i].at);
  }
  return std::nullopt;
}

bool PopAI::food_left(const World& w, EntityId me) const {
  for (const auto& [kind, proc] : {std::pair{"fridge", "eat_fresh"}, std::pair{"pantry", "eat"}})
    if (const auto o = actors_.find_object(kind); o && actors_.preview(w, me, static_cast<std::size_t>(process(proc)), *o).enabled)
      return true;
  return false;
}

bool PopAI::radio_on() const {
  for (const Receiver& r : power_.receivers())
    if (run_ && r.object == run_->radio()) return r.on;
  return false;
}

std::optional<EntityId> PopAI::ally_in_trouble(const World& w, EntityId me) const {
  const Cell pos = actors_.actor(me).pos;
  std::optional<EntityId> best;
  double best_d = kHelpM;
  for (const auto& [id, o] : minds_) {
    if (id == me || !bodies_.get(id).alive || bodies_.get(id).spec.walker) continue;
    const Cell at = actors_.actor(id).pos;
    if (at.z != pos.z) continue;
    for (const EntityId z : walkers_.visible_to(w, id)) {
      const Cell zc = *walkers_.cell_of(z);
      if (!walkers_.in_reach(at, zc)) continue;
      const double d = distance_m(pos, zc);
      if (d <= best_d) {
        best = z;
        best_d = d;
      }
    }
  }
  return best;
}

std::optional<EntityId> PopAI::dead_to_put_down(const World&, EntityId me) const {
  const Cell pos = actors_.actor(me).pos;
  std::optional<EntityId> best;
  double best_d = 1e300;
  for (const auto& [id, o] : minds_) {
    const Body& ob = bodies_.get(id);
    if (id == me || ob.alive || ob.spec.walker || walkers_.put_down(id)) continue;
    if (ob.death_cause == DeathCause::BrainDestroyed) continue;
    const Cell at = actors_.actor(id).pos;
    const double d = distance_m(pos, at) + 10.0 * std::abs(pos.z - at.z);
    if (others_doing(me, Doing::PutDown, id) == 0 && d < best_d) {
      best = id;
      best_d = d;
    }
  }
  return best;
}

void PopAI::decide(World& w, PopMind& m) {
  const EntityId me = m.body;
  const Body& b = bodies_.get(me);
  const std::int64_t now = w.clock.tick;
  // Rule 7: an honest pop tells the group about a bite; a secretive one keeps it to itself.
  if (!m.reported && m.honest)
    for (const Wound& wd : b.wounds)
      if (wd.type == WoundType::Bite) {
        m.reported = true;
        w.emit(EventType::BiteReported, me, 0, static_cast<std::int64_t>(wd.id));
        break;
      }
  if (b.activity.asleep) return;
  const Cell pos = actors_.actor(me).pos;
  const auto threats = walkers_.visible_to(w, me);
  const bool has_weapon = armed(w, me);
  Doing want = Doing::Idle;
  // Fight or flee.
  if (!threats.empty()) {
    const Cell t = *walkers_.cell_of(threats.front());
    const double d = distance_m(pos, t);
    const bool reach = walkers_.in_reach(pos, t);
    // A walker beyond a wall or window can't get at it until something gives: no stepping up, no backing off.
    const bool same_space = is_indoor(map_.floor(pos)) ? map_.room_of(pos) == map_.room_of(t) : !is_indoor(map_.floor(t));
    if (b.fear >= (m.brave ? kPanicFearBrave : kPanicFear) && !(reach && has_weapon)) {
      if (!m.panicked) w.emit(EventType::Panicked, me, 0);
      m.panicked = true;
      want = Doing::Flee;
    } else if (reach) {
      want = has_weapon ? Doing::Fight : Doing::Flee;
    } else if (has_weapon && d <= kEngageM && is_indoor(map_.floor(pos)) && same_space && (m.brave || b.fear < 0.5)) {
      want = Doing::Fight;
    } else if (!has_weapon && d <= kAvoidM && same_space) {
      want = Doing::Flee;
    }
  }
  if (m.panicked && want != Doing::Flee && b.fear < (m.brave ? kPanicFearBrave : kPanicFear) - 0.1) m.panicked = false;
  m.target = 0;
  // Help a pop at grips with a walker, armed and not panicking.
  if ((want == Doing::Idle || want == Doing::Fight) && has_weapon && !m.panicked) {
    if (want == Doing::Idle || !walkers_.in_reach(pos, *walkers_.cell_of(threats.front())))
      if (const auto z = ally_in_trouble(w, me)) {
        want = Doing::Fight;
        m.target = *z;
      }
  }
  auto ready = [&](Doing d) { auto it = m.retry_after.find(d); return it == m.retry_after.end() || now >= it->second; };
  if (want == Doing::Idle) {
    const Needs n = bodies_.needs(b);
    const EntityId guard = guard_on_duty(w);
    const double tired = night(w) ? kTiredNight : kTiredDay;
    bool weapon_near = false;
    if (!has_weapon && ready(Doing::Arm)) {
      weapon_near = loose_weapon(w, pos).has_value();
      if (!weapon_near) m.retry_after[Doing::Arm] = now + kRetryTicks;  // nothing lying about: look later
    }
    const auto corpse = has_weapon && ready(Doing::PutDown) ? dead_to_put_down(w, me) : std::nullopt;
    if (corpse) {
      want = Doing::PutDown;
      m.target = *corpse;
    } else if (treatable_wound(b) && ready(Doing::Treat)) want = Doing::Treat;
    else if (n.thirst > kThirst && ready(Doing::Drink)) want = Doing::Drink;
    else if (n.hunger > kHunger && ready(Doing::Eat) && food_left(w, me)) want = Doing::Eat;
    else if (n.tiredness > tired && (guard != me || n.tiredness > 0.95) && ready(Doing::Sleep)) want = Doing::Sleep;
    else if (m.order.type != OrderType::None && m.order.type != OrderType::Hold && ready(Doing::Order)) want = Doing::Order;
    else if (m.order.type == OrderType::Hold) want = Doing::Idle;
    else if (const auto who = threats.empty() && ready(Doing::Invite) ? to_invite(me) : std::nullopt) {
      want = Doing::Invite;
      m.target = *who;
    } else if (weapon_near) want = Doing::Arm;
    else {
      for (int p = 1; p <= 3 && want == Doing::Idle; ++p)
        for (std::size_t j = 0; j < kJobCount && want == Doing::Idle; ++j) {
          if (m.priority[j] != p) continue;
          switch (static_cast<Job>(j)) {
            case Job::Refuel: {
              // Cautious default: the generator runs by day for the fridge and is stopped at night,
              // when its noise carries farthest (spec 18) and nothing masks it.
              const double fuel = static_cast<double>(power_.fuel_ul(w)) / static_cast<double>(power_.tank_ul());
              const bool day = generator_hours(w);
              const bool needed = !power_.mains_alive() &&
                                  (day ? power_.transfer() != Transfer::Generator || !power_.generator_running() || fuel < kRefuelBelow
                                       : power_.generator_running());
              const bool can = fuel > 0 || loose(w, "fuel_can", pos) || carries(w, me, "fuel_can");
              if (needed && can && ready(Doing::Refuel) && others_doing(me, Doing::Refuel) == 0) want = Doing::Refuel;
              break;
            }
            case Job::Barricade:
              if (!careless_ && ready(Doing::Barricade) && window_to_board(w, pos, me)) want = Doing::Barricade;
              break;
            case Job::Guard: {
              bool sleeper = false;
              for (const auto& [id, o] : minds_) sleeper |= id != me && bodies_.get(id).alive && bodies_.get(id).activity.asleep;
              if (!careless_ && guard == me && !threats.empty() && sleeper && ready(Doing::Guard)) want = Doing::Guard;
              break;
            }
            case Job::Search:
              if (run_ && threats.empty() && !night(w) && w.clock.hour() >= 8 && w.clock.hour() < 17 && ready(Doing::Search) &&
                  home_to_search(me))
                want = Doing::Search;
              break;
            case Job::Repair: {
              if (!run_ || !ready(Doing::Radio)) break;
              const bool tech = actors_.actor(me).skill[static_cast<std::size_t>(Trade::Electronics)] >= 0;
              const double h = w.clock.hour();
              const bool power = power_.mains_alive() ? power_.transfer() == Transfer::Mains
                                                      : power_.generator_running() && power_.transfer() == Transfer::Generator;
              const bool listen = run_->radio_repaired() && !run_->heard() && h >= 12 && h < 14 && power &&
                                  run_->last_listen_tick() / kTicksPerGameDay != w.clock.tick / kTicksPerGameDay;
              if (((tech && !run_->radio_repaired()) || listen) && others_doing(me, Doing::Radio) == 0) want = Doing::Radio;
              break;
            }
            default: break;  // haul, cook: no Tier 0 processes yet
          }
        }
    }
  }
  // Preempt what it is doing if the new want is more urgent.
  const bool busy = !actors_.idle(me) || !m.plan.empty();
  if (busy && want != m.doing && rank(want) < rank(m.doing)) cancel(w, m);
  if (!actors_.idle(me)) return;
  if (m.plan.empty()) {
    if (!plan_want(w, m, want)) {
      // A followed pop that is close needs no plan now, but must be followed again as soon as it moves.
      const bool following = want == Doing::Order && m.order.type == OrderType::Follow;
      if (want != Doing::Idle && !following) m.retry_after[want] = now + kRetryTicks;
      m.doing = Doing::Idle;
      return;
    }
  }
  send_next(w, m);
}

void PopAI::update(World& w) {
  const std::int64_t now = w.clock.tick;
  for (auto& [id, m] : minds_) {
    const Body& b = bodies_.get(id);
    if (!b.alive || b.spec.walker) {
      m.plan.clear();
      m.doing = Doing::Idle;
      continue;
    }
    if (id == possessed_ || now < m.next_decide) continue;
    m.next_decide = now + kDecideTicks;
    decide(w, m);
  }
}

void PopAI::save(ArchiveWriter& a) const {
  a.u64(possessed_);
  a.boolean(careless_);
  a.u64(searched_.size());
  for (EntityId e : searched_) a.u64(e);
  a.u64(known_.size());
  for (EntityId e : known_) a.u64(e);
  a.u64(minds_.size());
  for (const auto& [id, m] : minds_) {
    a.u64(id);
    for (int p : m.priority) a.u8(static_cast<std::uint8_t>(p));
    a.u8(static_cast<std::uint8_t>(m.order.type));
    a.i64(pack_cell(m.order.at));
    a.u64(m.order.target);
    a.boolean(m.order.insist);
    a.boolean(m.brave);
    a.boolean(m.honest);
    a.boolean(m.reported);
    a.boolean(m.panicked);
    a.u64(m.target);
    a.u8(static_cast<std::uint8_t>(m.doing));
    a.u64(m.plan.size());
    for (const auto& [p, t] : m.plan) {
      a.i64(p);
      a.u64(t);
    }
    a.i64(m.next_decide);
    a.u64(m.retry_after.size());
    for (const auto& [d, t] : m.retry_after) {
      a.u8(static_cast<std::uint8_t>(d));
      a.i64(t);
    }
  }
}

void PopAI::load(ArchiveReader& r) {
  possessed_ = r.u64();
  careless_ = r.boolean();
  searched_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) searched_.insert(r.u64());
  known_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) known_.insert(r.u64());
  // Pops join during a run (spec 26), so the minds are rebuilt from the save.
  minds_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const EntityId id = r.u64();
    PopMind& m = minds_[id];
    m.body = id;
    for (int& p : m.priority) p = r.u8();
    m.order.type = static_cast<OrderType>(r.u8());
    m.order.at = unpack_cell(r.i64());
    m.order.target = r.u64();
    m.order.insist = r.boolean();
    m.brave = r.boolean();
    m.honest = r.boolean();
    m.reported = r.boolean();
    m.panicked = r.boolean();
    m.target = r.u64();
    m.doing = static_cast<Doing>(r.u8());
    m.plan.clear();
    for (std::uint64_t k = r.u64(); k > 0; --k) {
      const auto p = static_cast<int>(r.i64());
      m.plan.emplace_back(p, r.u64());
    }
    m.next_decide = r.i64();
    m.retry_after.clear();
    for (std::uint64_t k = r.u64(); k > 0; --k) {
      const auto d = static_cast<Doing>(r.u8());
      m.retry_after[d] = r.i64();
    }
  }
}

}  // namespace zs
