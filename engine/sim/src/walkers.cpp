// Walkers: brain, pushes on edges, bites, turning, strikes.
// Specs: docs/tier0/specs/23-walker-brain.md, 10-fortification.md, 20-damage.md, 21-infection.md
#include <algorithm>
#include <cmath>
#include <queue>
#include <stdexcept>
#include <tuple>

#include "zs/dead.h"
#include "zs/detmath.h"

namespace zs {
namespace {

constexpr std::int64_t kThinkTicks = 10;          // perceive and decide every 5 s, staggered
constexpr std::int64_t kAttackTicks = 6;          // a grab or bite attempt every 3 s
// A bite is a clamp, not a peck: the walker worries the same wound before it can bite again
// (estimate; keeps a held victim from collecting a new wound every few seconds).
constexpr std::int64_t kChewTicks = 60;
// Room around one person: only so many bodies can get hands and teeth on them at once; the rest
// crowd behind (estimate).
constexpr int kMaxOnOne = 3;
constexpr double kCoordination = 0.5;             // walker speed = body walk speed x this (tuned "mostly slow")
constexpr double kPursuitBoost = 1.2;             // a little faster with a target in sight
constexpr double kFadeDbPerS = 0.02;              // a 20 dB memory fades in ~17 min (estimate)
constexpr double kSightStrength = 40, kWindowStrength = 10;
constexpr double kMoanDb = 65;  // decision 0064 rule 2: a moan at 1 m, an estimate to tune
constexpr double kPushNoiseDb = 75;               // shoving and pounding on a door (estimate)
constexpr double kEdgeCost = 25;                  // path cost of a closed door or window: push through it
constexpr double kGrabChance = 0.5, kBiteChance = 0.4, kEscapeBase = 0.2;
// Fortification (spec 10 rule 2): a part loses condition at 1e6 ppm per minute x (load/hold - 0.5)^2.
constexpr double kPushRatePpmPerS = 1e6 / 60.0, kPushFloor = 0.5;
// Strikes (spec 20 rule 1, 4): energy = 25 J x strength x (1 + 3 x tool), strength = push force / 500 N.
// A hammer or crowbar in an average adult's hands delivers ~100 J; skull fracture ~60 J (impact
// biomechanics, e.g. Yoganandan et al.; tune).
constexpr double kStrikeBaseJ = 25, kSkullJ = 60, kHeadHitChance = 0.6;
constexpr double kCrackShare = 0.3, kSkullSumJ = 2 * kSkullJ;  // blows that add up, and how far

using EdgeKey = std::tuple<int, int, int, bool>;
EdgeKey key_of(const EdgeRef& e) { return {e.z, e.x, e.y, e.vertical}; }

// Where bites land on a standing person facing a grab: hands and forearms first (estimates from
// bite-injury literature on defensive wounds).
struct RegionWeight {
  Region region;
  double weight;
};
constexpr RegionWeight kBiteRegions[] = {
    {Region::ForearmL, 0.2}, {Region::ForearmR, 0.2}, {Region::HandL, 0.1},  {Region::HandR, 0.1},
    {Region::UpperArmL, 0.07}, {Region::UpperArmR, 0.07}, {Region::Neck, 0.08}, {Region::Chest, 0.06},
    {Region::ShinL, 0.06}, {Region::ShinR, 0.06}};

bool openable_or_weak(EdgeKind k) {
  return k == EdgeKind::DoorInterior || k == EdgeKind::DoorExterior || k == EdgeKind::Window || k == EdgeKind::Fence ||
         k == EdgeKind::Gate;
}

}  // namespace

const char* to_string(Mode m) {
  switch (m) {
    case Mode::Idle: return "idle";
    case Mode::Investigate: return "investigating";
    case Mode::Pursue: return "pursuing";
    case Mode::Bash: return "pushing a door";
    case Mode::Attack: return "attacking";
    case Mode::Feed: return "feeding";
  }
  return "?";
}

Walkers::Walkers(World&, SiteMap& map, Assemblies& items, Bodies& bodies, Actors& actors, Power& power, Census& census)
    : map_(map), items_(items), bodies_(bodies), actors_(actors), power_(power), census_(census) {}

void Walkers::setup(World&) {
  actors_.body_cell = [this](EntityId body) { return cell_of(body); };
  actors_.held = [this](EntityId body) { return held(body); };
  actors_.hit = [this](World& w, EntityId attacker, EntityId target, EntityId tool, EventId cause) {
    strike(w, attacker, target, tool, cause);
  };
}

void Walkers::spawn(World& w, EntityId body, const Cell& at) {
  WalkerState z;
  z.body = body;
  z.pos = at;
  z.goal = at;
  z.next_think = w.clock.tick + static_cast<std::int64_t>(body % kThinkTicks);
  walkers_[body] = z;
}

std::optional<Cell> Walkers::cell_of(EntityId body) const {
  auto it = walkers_.find(body);
  if (it == walkers_.end()) return std::nullopt;
  return it->second.pos;
}

std::optional<Turning> Walkers::turning_of(EntityId body) const {
  for (const Turning& t : turnings_)
    if (t.body == body) return t;
  return std::nullopt;
}

// ---- Senses ------------------------------------------------------------------------------------------

double Walkers::sight_range_m(double lux) {
  // Spec 19 rule 2, compressed: ~3 m by moonlight, ~11 m in a dim lamp-lit room, ~35 m by day (estimate).
  return std::min(40.0, 3.0 + 8.0 * detmath::log10(1.0 + std::max(0.0, lux)));
}

bool Walkers::line_of_sight(const World& w, const Cell& a, const Cell& b) const {
  if (a.z != b.z) return false;
  auto see_through = [&](const Cell& p, const Cell& q) {
    const EdgeRef e = SiteMap::edge_between(p, q);
    const Edge& ed = map_.edge(e);
    if (ed.kind == EdgeKind::None || ed.breached || ed.kind == EdgeKind::Fence || ed.kind == EdgeKind::Gate) return true;
    if (ed.barricade != 0) return false;
    if (ed.kind == EdgeKind::Window) return !actors_.covered(w, ed.assembly);  // glass is clear; a curtain isn't
    return is_openable(ed.kind) && ed.open;
  };
  int x = a.x, y = a.y;
  const int dx = std::abs(b.x - a.x), dy = std::abs(b.y - a.y), sx = b.x > a.x ? 1 : -1, sy = b.y > a.y ? 1 : -1;
  int err = dx - dy;
  while (x != b.x || y != b.y) {
    const int e2 = 2 * err;
    const Cell from{a.z, x, y};
    if (e2 > -dy && e2 < dx) {  // a diagonal step: clear if either corner route is clear
      const Cell via1{a.z, x + sx, y}, via2{a.z, x, y + sy}, to{a.z, x + sx, y + sy};
      const bool r1 = map_.in_bounds(via1) && see_through(from, via1) && see_through(via1, to);
      const bool r2 = map_.in_bounds(via2) && see_through(from, via2) && see_through(via2, to);
      if (!r1 && !r2) return false;
      err += -dy + dx;
      x += sx;
      y += sy;
    } else if (e2 > -dy) {
      if (!see_through(from, {a.z, x + sx, y})) return false;
      err -= dy;
      x += sx;
    } else {
      if (!see_through(from, {a.z, x, y + sy})) return false;
      err += dx;
      y += sy;
    }
  }
  return true;
}

double Walkers::heard_db(const World&, const NoiseSource& s, const Cell& at, double) {
  if (cache_version_ != map_.version()) {
    loss_cache_.clear();
    cache_version_ = map_.version();
  }
  const std::size_t key = cell_index(map_, s.at);
  auto it = loss_cache_.find(key);
  if (it == loss_cache_.end()) it = loss_cache_.emplace(key, path_loss(map_, items_, s.at)).first;
  const double loss = it->second[cell_index(map_, at)];
  if (loss >= 1e299) return kSilentDb;
  return s.db_1m - 20 * detmath::log10(std::max(1.0, distance_m(s.at, at))) - loss;
}

std::vector<NoiseSource> Walkers::sources(const World&) const {
  std::vector<NoiseSource> out;
  for (const auto& [id, z] : walkers_) {
    if (z.pushing) out.push_back({z.pos, kPushNoiseDb});
    // Decision 0064 rule 2: a walker that has something moans, and the moan carries to other walkers, so a horde turns
    // as a wave and relays the sound from walker to walker. The level is an estimate to tune.
    if (z.mode == Mode::Pursue || z.mode == Mode::Attack || z.mode == Mode::Bash) out.push_back({z.pos, kMoanDb});
  }
  return out;
}

// A cell up to `steps` walkable steps from `from` in the direction of `to` (the same storey): where a noticed sound
// sends a walker (decision 0064), not the spot itself.
Cell Walkers::toward(const Cell& from, const Cell& to, int steps) const {
  Cell c = from;
  for (int i = 0; i < steps; ++i) {
    const int dx = (to.x > c.x) - (to.x < c.x), dy = (to.y > c.y) - (to.y < c.y);
    if (!dx && !dy) break;
    Cell nb{c.z, c.x + dx, c.y + dy};
    if (dx && dy) nb = std::abs(to.x - c.x) >= std::abs(to.y - c.y) ? Cell{c.z, c.x + dx, c.y} : Cell{c.z, c.x, c.y + dy};
    if (!map_.passable(nb) || !walker_passes(SiteMap::edge_between(c, nb))) break;
    c = nb;
  }
  return c;
}

// ---- Brain ---------------------------------------------------------------------------------------------

bool Walkers::in_reach(const Cell& a, const Cell& b) const {
  if (a == b) return true;
  return a.z == b.z && std::abs(a.x - b.x) + std::abs(a.y - b.y) == 1 && walker_passes(SiteMap::edge_between(a, b));
}

std::vector<EntityId> Walkers::visible_to(const World& w, EntityId viewer) const {
  std::vector<std::pair<double, EntityId>> seen;
  const Body& vb = bodies_.get(viewer);
  if (!vb.alive || vb.activity.asleep) return {};
  const Cell from = actors_.actor(viewer).pos;
  const double sight = bodies_.capabilities(vb, w.clock.tick).sight;
  for (const auto& [id, z] : walkers_) {
    if (!bodies_.get(id).alive || z.pos.z != from.z) continue;
    const double d = distance_m(from, z.pos);
    if (d <= sight_range_m(power_.lux_at(w, z.pos)) * sight && line_of_sight(w, from, z.pos)) seen.emplace_back(d, id);
  }
  std::sort(seen.begin(), seen.end());
  std::vector<EntityId> out;
  for (const auto& s : seen) out.push_back(s.second);
  return out;
}

// The senses' report for fear (spec 11): held 1, a walker in reach 0.9, one in sight up to 0.8
// falling to 0 at 20 m.
void Walkers::sense_threats(World& w) {
  for (const auto& [id, a] : actors_.actors()) {
    Body& b = bodies_.get(id);
    if (!b.alive || b.spec.walker || b.away) continue;
    double threat = held_.contains(id) ? 1.0 : 0.0;
    for (EntityId zid : visible_to(w, id)) {
      const Cell zp = walkers_.at(zid).pos;
      threat = std::max(threat, in_reach(a.pos, zp) ? 0.9 : 0.8 * std::max(0.0, 1 - distance_m(a.pos, zp) / 20));
    }
    b.threat = threat;
  }
}

bool Walkers::walker_passes(const EdgeRef& e) const {
  const Edge& ed = map_.edge(e);
  if (ed.kind == EdgeKind::None || ed.breached) return true;
  return is_openable(ed.kind) && ed.open && ed.barricade == 0;  // walkers can't open doors, only push them in
}

void Walkers::plan_path(WalkerState& z) {
  z.path.clear();
  z.next = 0;
  z.step_m = 0;
  if (z.goal == z.pos || !map_.in_bounds(z.goal) || !map_.walkable(z.goal)) return;
  // A goal on furniture (a noisy generator) is reached at a free neighbour with nothing between (spec 50 rule 4).
  const bool goal_blocked = map_.blocked(z.goal);
  auto beside_goal = [&](const Cell& c) {
    return c.z == z.goal.z && std::abs(c.x - z.goal.x) + std::abs(c.y - z.goal.y) == 1 &&
           walker_passes(SiteMap::edge_between(c, z.goal));
  };
  const std::size_t n = map_.levels().size() * static_cast<std::size_t>(map_.width() * map_.height());
  std::vector<double> cost(n, 1e300);
  std::vector<std::size_t> prev(n, n);
  std::vector<Cell> cells(n);
  using Key = std::tuple<double, std::size_t>;
  std::priority_queue<Key, std::vector<Key>, std::greater<Key>> open;
  const std::size_t start = cell_index(map_, z.pos);
  std::size_t goal = cell_index(map_, z.goal);
  cost[start] = 0;
  cells[start] = z.pos;
  open.emplace(0.0, start);
  while (!open.empty()) {
    const auto [c, i] = open.top();
    open.pop();
    if (c > cost[i]) continue;
    if (i == goal) break;
    if (goal_blocked && i != start && beside_goal(cells[i])) {
      goal = i;
      break;
    }
    const Cell at = cells[i];
    auto relax = [&](const Cell& nb, double step) {
      if (!map_.passable(nb)) return;
      const std::size_t j = cell_index(map_, nb);
      if (c + step < cost[j]) {
        cost[j] = c + step;
        prev[j] = i;
        cells[j] = nb;
        open.emplace(cost[j], j);
      }
    };
    for (const Cell& nb : {Cell{at.z, at.x + 1, at.y}, Cell{at.z, at.x - 1, at.y}, Cell{at.z, at.x, at.y + 1}, Cell{at.z, at.x, at.y - 1}}) {
      if (!map_.in_bounds(nb)) continue;
      const EdgeRef e = SiteMap::edge_between(at, nb);
      if (walker_passes(e)) relax(nb, 1);
      else if (openable_or_weak(map_.edge(e).kind)) relax(nb, 1 + kEdgeCost);
    }
    // Diagonals across open corners, as pops walk (spec 50 rule 1).
    for (const Cell& nb : {Cell{at.z, at.x + 1, at.y - 1}, Cell{at.z, at.x + 1, at.y + 1}, Cell{at.z, at.x - 1, at.y + 1}, Cell{at.z, at.x - 1, at.y - 1}})
      if (map_.in_bounds(nb) && map_.step_allowed(at, nb)) relax(nb, 1.4142135623730951);
    for (const Stairs& s : map_.stairs()) {
      if (s.a == at) relax(s.b, 4);
      if (s.b == at) relax(s.a, 4);
    }
  }
  if (cost[goal] >= 1e299 || (goal_blocked && goal == cell_index(map_, z.goal))) return;
  for (std::size_t i = goal; i != start; i = prev[i]) z.path.push_back(cells[i]);
  std::reverse(z.path.begin(), z.path.end());
}

void Walkers::think(World& w, WalkerState& z) {
  const std::int64_t now = w.clock.tick;
  const Body& b = bodies_.get(z.body);
  const Cell old_goal = z.goal;
  // Sight: the nearest living person in line of sight and light (rule 3).
  EntityId seen = 0;
  double seen_d = 1e300;
  for (const auto& [id, a] : actors_.actors()) {
    const Body& pb = bodies_.get(id);
    if (!pb.alive || pb.spec.walker || pb.away || a.pos.z != z.pos.z) continue;
    const double d = distance_m(z.pos, a.pos);
    if (d > sight_range_m(power_.lux_at(w, a.pos)) * bodies_.capabilities(b, now).sight) continue;
    if (d < seen_d && line_of_sight(w, z.pos, a.pos)) {
      seen = id;
      seen_d = d;
    }
  }
  const double faded = z.memory_db - kFadeDbPerS * static_cast<double>(now - z.memory_tick) * kTickSeconds;
  if (seen) {
    z.target = seen;
    z.goal = actors_.actor(seen).pos;
    z.memory_db = kSightStrength;
    z.memory_tick = now;
    z.mode = in_reach(z.pos, z.goal) ? Mode::Attack : Mode::Pursue;
  } else {
    // Hearing (rule 2): the strongest sound above the background replaces a weaker memory.
    const double bg = background_db(power_.mains_alive(), w.clock.hour());
    double best = std::max(0.0, faded);
    std::optional<Cell> heard;
    std::vector<NoiseSource> srcs = noise_sources(w, actors_, power_);
    for (const NoiseSource& s : sources(w))
      if (!(s.at == z.pos)) srcs.push_back(s);
    for (const NoiseSource& s : srcs) {
      // Decision 0064: noticed at 0 dB over the background; placed at +10 dB. A noticed sound sends the walker a few
      // steps toward it; a placed one sends it to the spot.
      const double margin = (heard_db(w, s, z.pos, bg) - bg - kNoticeDb) * bodies_.capabilities(b, now).hearing;
      if (margin > best + 1) {  // a clearly stronger sound
        best = margin;
        heard = margin >= kPlaceDb ? s.at : toward(z.pos, s.at, 6);
      }
    }
    // A lit window standing out against the night (spec 19 rule 4).
    if (!is_indoor(map_.floor(z.pos)) && Power::ambient_lux(w.clock.hour()) < 1000) {  // by day a lit window does not stand out
      if (leaks_tick_ != now / kBodyStepTicks) {  // lit windows, read once per game minute
        leaks_ = power_.window_leaks(w);
        leaks_tick_ = now / kBodyStepTicks;
      }
      for (const WindowLeak& l : leaks_) {
        const Cell inner{l.edge.z, l.edge.x, l.edge.y};
        if (l.seen_m <= 0 || inner.z != z.pos.z || distance_m(inner, z.pos) > l.seen_m) continue;
        if (kWindowStrength > best + 1) {
          best = kWindowStrength;
          const Cell outer = l.edge.vertical ? Cell{inner.z, inner.x - 1, inner.y} : Cell{inner.z, inner.x, inner.y - 1};
          heard = is_indoor(map_.floor(inner)) ? inner : outer;
        }
      }
    }
    if (heard) {
      z.goal = *heard;
      z.memory_db = best;
      z.memory_tick = now;
      z.mode = Mode::Investigate;
      z.target = 0;
    } else if (z.mode == Mode::Feed && z.target && now - z.memory_tick < 2 * kTicksPerGameHour) {
      // Keeps feeding: distant noise doesn't distract it (rule 6).
    } else if (faded <= 0) {
      z.mode = Mode::Idle;
      z.target = 0;
      z.pushing.reset();
      // Drift: now and then one step somewhere (rule 1).
      if (w.rng.uniform01(z.body, Purpose::WalkerMind) < 0.1) {
        const int dir = static_cast<int>(w.rng.uniform_int(z.body, Purpose::WalkerMind, 4));
        const Cell nb{z.pos.z, z.pos.x + (dir == 0) - (dir == 1), z.pos.y + (dir == 2) - (dir == 3)};
        if (map_.passable(nb) && walker_passes(SiteMap::edge_between(z.pos, nb))) z.goal = nb;
      }
    }
  }
  if (z.mode == Mode::Pursue && z.target) {
    // In reach: same cell, or next to it with nothing between.
    if (in_reach(z.pos, actors_.actor(z.target).pos)) z.mode = Mode::Attack;
  }
  if (!(z.goal == old_goal) || (z.path.empty() && !(z.goal == z.pos)) || z.next >= z.path.size()) plan_path(z);
}

void Walkers::move(World& w, WalkerState& z) {
  Body& b = bodies_.get(z.body);
  b.activity = Activity{};
  if (w.buggify.fire(FaultSite::WalkerStumbles, w.clock.tick, z.body, 100000)) return;  // 10 %: no step this tick
  if (z.next >= z.path.size()) {
    z.pushing.reset();
    return;
  }
  const Cell to = z.path[z.next];
  const bool stairs = to.z != z.pos.z;
  const bool diagonal = !stairs && to.x != z.pos.x && to.y != z.pos.y;
  if (diagonal && !map_.step_allowed(z.pos, to)) {  // the corner closed: plan again
    plan_path(z);
    return;
  }
  if (!stairs && !diagonal) {
    const EdgeRef e = SiteMap::edge_between(z.pos, to);
    if (!walker_passes(e)) {
      // Rule 4: blocked between it and a live stimulus: push.
      z.pushing = e;
      z.mode = Mode::Bash;
      b.activity.met = 4;  // shoving (estimate)
      return;
    }
  }
  z.pushing.reset();
  if (z.mode == Mode::Bash) z.mode = Mode::Investigate;
  if (z.walk_mps < 0) z.walk_mps = bodies_.walk_speed(b, w.clock.tick);
  const double speed = z.walk_mps * kCoordination * (z.mode == Mode::Pursue ? kPursuitBoost : 1.0);
  b.activity.speed_mps = speed;
  b.activity.grade_pct = stairs ? 50 : 0;
  z.step_m += speed * kTickSeconds;
  const double length = SiteMap::step_length(z.pos, to);
  if (z.step_m >= length) {
    z.step_m -= length;
    z.pos = to;
    ++z.next;
  }
}

void Walkers::attack(World& w, WalkerState& z) {
  Body& b = bodies_.get(z.body);
  b.activity = Activity{};
  b.activity.met = 3;
  const std::int64_t now = w.clock.tick;
  if (now < z.next_attack || !z.target) return;
  z.next_attack = now + kAttackTicks;
  const Body& victim = bodies_.get(z.target);
  if (!victim.alive) {
    z.mode = Mode::Feed;
    z.memory_tick = now;
    held_.erase(z.target);
    return;
  }
  if (!in_reach(z.pos, actors_.actor(z.target).pos)) {
    if (auto it = held_.find(z.target); it != held_.end() && it->second == z.body) held_.erase(it);
    z.mode = Mode::Pursue;
    return;
  }
  if (crowded_.contains(z.body)) return;  // crowding behind the ones on it
  auto it = held_.find(z.target);
  if (it == held_.end()) {
    if (w.rng.uniform01(z.target, Purpose::BiteHit) < kGrabChance) {
      held_[z.target] = z.body;
      w.emit(EventType::Grabbed, z.body, 0, static_cast<std::int64_t>(z.target));
    }
    return;
  }
  if (it->second != z.body) {  // someone else holds it: bite anyway, less often
    if (w.rng.uniform01(z.target, Purpose::BiteHit) < kBiteChance / 2) {
      bite(w, z.body, z.target, 0);
      z.next_attack = now + kChewTicks;
    }
    return;
  }
  // The victim fights to get free: strength against strength.
  const double ratio = bodies_.push_force_n(victim, now) / std::max(1.0, bodies_.push_force_n(b, now));
  if (w.rng.uniform01(z.target, Purpose::BiteHit) < kEscapeBase * std::clamp(ratio, 0.2, 3.0)) {
    held_.erase(it);
    return;
  }
  if (w.rng.uniform01(z.target, Purpose::BiteHit) < kBiteChance) {
    bite(w, z.body, z.target, 0);
    z.next_attack = now + kChewTicks;
  }
}

// ---- Damage (spec 20) ------------------------------------------------------------------------------------

void Walkers::bite(World& w, EntityId walker, EntityId victim, EventId cause) {
  double total = 0;
  for (const RegionWeight& r : kBiteRegions) total += r.weight;
  double u = w.rng.uniform01(victim, Purpose::BiteHit) * total;
  Region region = kBiteRegions[0].region;
  for (const RegionWeight& r : kBiteRegions) {
    if (u < r.weight) {
      region = r.region;
      break;
    }
    u -= r.weight;
  }
  const double d = w.rng.uniform01(victim, Purpose::BiteHit);
  Wound wd;
  wd.region = region;
  wd.type = WoundType::Bite;
  wd.depth = d < 0.3 ? Depth::Skin : d < 0.65 ? Depth::Fat : Depth::Muscle;
  wd.length_mm = 25 + 20 * w.rng.uniform01(victim, Purpose::BiteHit);
  wd.contamination = 0.9;
  wd.pathogen_dose = 1;
  const EventId ev = w.emit(EventType::Bite, walker, cause, static_cast<std::int64_t>(victim), static_cast<std::int64_t>(region));
  bodies_.add_wound(w, victim, wd, ev);
  Body& vb = bodies_.get(victim);
  if (vb.activity.asleep) {  // nobody sleeps through a bite
    vb.activity.asleep = false;
    w.emit(EventType::WokeUp, victim, ev);
  }
  // Spec 21 rules 1-2: infection is established by route and depth, drawn once; times drawn up front.
  if (bodies_.get(victim).spec.walker || turning_of(victim)) return;
  const double p = wd.depth == Depth::Skin ? 0.5 : 0.9;  // estimate (default preset)
  if (w.rng.uniform01(victim, Purpose::Turning) >= p) return;
  Turning t;
  t.body = victim;
  t.cause = ev;
  const std::int64_t now = w.clock.tick;
  t.prodrome_tick = now + static_cast<std::int64_t>((6 + 6 * w.rng.uniform01(victim, Purpose::Turning)) * kTicksPerGameHour);
  t.death_tick = now + static_cast<std::int64_t>((24 + 24 * w.rng.uniform01(victim, Purpose::Turning)) * kTicksPerGameHour);
  t.rise_after = static_cast<std::int64_t>((10 + 50 * w.rng.uniform01(victim, Purpose::Turning)) * kTicksPerGameHour / 60);
  turnings_.push_back(t);
}

double Walkers::strike_energy_j(const World& w, EntityId attacker, EntityId tool) const {
  const double strength = bodies_.push_force_n(bodies_.get(attacker), w.clock.tick) / 500.0;
  double t = 0;
  if (tool && items_.exists(tool))
    t = std::max({items_.prop(tool, Prop::Blunt), items_.prop(tool, Prop::Lever) * 0.9, items_.prop(tool, Prop::Edge) * 0.6});
  return kStrikeBaseJ * strength * (1 + 3 * t);
}

bool Walkers::strike(World& w, EntityId attacker, EntityId target, EntityId tool, EventId cause) {
  auto it = walkers_.find(target);
  const auto from = actors_.cell_of(w, attacker);
  const Body& tb = bodies_.get(target);
  // A still corpse (not yet risen, not put down) can be struck too: every blow lands on the head.
  const bool corpse = it == walkers_.end() && !tb.alive && !put_down_.contains(target);
  const auto at = corpse ? actors_.cell_of(w, target) : (it != walkers_.end() ? std::optional<Cell>(it->second.pos) : std::nullopt);
  bool destroyed = false;
  if (at && from && in_reach(*from, *at)) {
    const Body& a = bodies_.get(attacker);
    const double aim = corpse ? 1.0 : kHeadHitChance * (1 - 0.5 * a.fear) * detmath::sqrt(std::max(0.05, a.exertion));
    Wound wd;
    wd.type = WoundType::Contusion;
    wd.depth = Depth::Fat;
    wd.contamination = 0.1;
    if (w.rng.uniform01(target, Purpose::Strike) < aim) {
      const double delivered = strike_energy_j(w, attacker, tool) * (0.6 + 0.6 * w.rng.uniform01(target, Purpose::Strike));
      // One hard blow breaks the skull; weaker blows that still crack it add up (two skulls' worth).
      double& sum = head_j_[target];
      if (delivered >= kSkullJ * kCrackShare) sum += delivered;
      destroyed = delivered >= kSkullJ || sum >= kSkullSumJ;
      wd.region = Region::Head;
      if (delivered >= kSkullJ * 0.6) wd.type = WoundType::Fracture;
    } else {
      wd.region = Region::Chest;
    }
    const EventId ev = w.emit(EventType::Hit, attacker, cause, static_cast<std::int64_t>(target), destroyed ? 1 : 0);
    if (!corpse) bodies_.add_wound(w, target, wd, ev);
    if (destroyed) {
      head_j_.erase(target);
      if (corpse) {
        put_down_.insert(target);
      } else {
        bodies_.kill(w, target, DeathCause::BrainDestroyed, ev);
        for (auto h = held_.begin(); h != held_.end();) h = h->second == target ? held_.erase(h) : std::next(h);
        if (auto p = census_.person_of(target)) census_.set_fate(w, *p, Fate::Destroyed);
        walkers_.erase(it);
      }
    }
  } else {
    w.emit(EventType::Hit, attacker, cause, static_cast<std::int64_t>(target), 0);  // out of reach: a miss
  }
  return destroyed;
}

// ---- Fortification (spec 10) ---------------------------------------------------------------------------

bool Walkers::push_edge(World& w, const EdgeRef& e, double load_n, double dt_s, EventId cause) {
  Edge& ed = map_.edge(e);
  if (ed.breached || ed.assembly == 0) return ed.breached;
  // The first intact part in chain order takes the load: the barricade's chain, then the edge's own.
  auto first_working = [&](EntityId holder) -> EntityId {
    const Instance& inst = items_.get(holder);
    for (const std::string& slot : items_.def(inst.def).failure_chain)
      if (auto i = items_.slot_index(holder, slot)) {
        const EntityId part = inst.slots[*i];
        if (part && items_.works(part)) return part;
      }
    return 0;
  };
  const EntityId holder = ed.barricade ? ed.barricade : ed.assembly;
  const EntityId part = first_working(holder);
  if (part) {
    Instance& p = items_.get(part);
    const double hold = std::max(100.0, items_.def(p.def).hold_force_n * (0.5 + p.quality / 100.0));
    const double over = std::max(0.0, load_n / hold - kPushFloor);
    p.condition.damage(static_cast<std::int64_t>(kPushRatePpmPerS * over * over * dt_s));
    if (!items_.works(part)) w.emit(EventType::PartFailed, part, cause, static_cast<std::int64_t>(ed.assembly));
  }
  if (ed.barricade && !items_.works(ed.barricade)) {
    map_.set_barricade(e, 0);  // its loose boards stay where they were
    return false;
  }
  if (!ed.barricade && !items_.works(ed.assembly)) {
    map_.set_breached(e, true);
    w.emit(EventType::EdgeBreached, ed.assembly, cause);
    return true;
  }
  return false;
}

// ---- Turning (spec 21) ------------------------------------------------------------------------------------

void Walkers::rise(World& w, Turning& t) {
  t.risen = true;
  bodies_.reanimate(w, t.body, t.cause);
  if (auto p = census_.person_of(t.body)) census_.set_fate(w, *p, Fate::WalkerOnMap);
  held_.erase(t.body);
  const auto at = actors_.cell_of(w, t.body);
  if (at) spawn(w, t.body, *at);
}

void Walkers::step_turnings(World& w) {
  const std::int64_t now = w.clock.tick;
  for (Turning& t : turnings_) {
    if (t.risen) continue;
    const Body& b = bodies_.get(t.body);
    if (b.alive && now >= t.death_tick) bodies_.kill(w, t.body, DeathCause::Pathogen, t.cause);
    if (!b.alive && t.died_tick < 0) t.died_tick = now;
    if (t.died_tick >= 0 && now >= t.died_tick + t.rise_after && b.death_cause != DeathCause::BrainDestroyed &&
        !put_down_.contains(t.body))
      rise(w, t);
  }
  // The census follows the group's dead: dead for good, or dead until they rise.
  for (const Person& p : census_.people())
    if ((p.fate == Fate::Pop || (p.fate == Fate::Survivor && p.body)) && !bodies_.get(p.body).alive) census_.set_fate(w, *census_.person_of(p.body), Fate::Dead);
}

// ---- Update ------------------------------------------------------------------------------------------------

void Walkers::update(World& w) {
  const std::int64_t now = w.clock.tick;
  std::map<EdgeKey, std::pair<EdgeRef, double>> pushes;
  // The first kMaxOnOne walkers on a victim (in body order) get at it; the rest crowd behind.
  crowded_.clear();
  std::map<EntityId, int> on_victim;
  for (const auto& [id, z] : walkers_)
    if (z.mode == Mode::Attack && z.target && ++on_victim[z.target] > kMaxOnOne) crowded_.insert(id);
  for (auto& [id, z] : walkers_) {
    const Body& b = bodies_.get(id);
    if (!b.alive) continue;
    if (now >= z.next_think) {
      z.walk_mps = bodies_.walk_speed(b, now);  // wounds change slowly: once per think is enough
      think(w, z);
      z.next_think = now + kThinkTicks;
    }
    if (z.mode == Mode::Attack) {
      attack(w, z);
    } else {
      move(w, z);
      if (z.pushing) {
        auto& p = pushes[key_of(*z.pushing)];
        p.first = *z.pushing;
        p.second += bodies_.push_force_n(b, now);  // several walkers at one door add up (rule 4)
      }
    }
  }
  for (const auto& [k, p] : pushes)
    if (push_edge(w, p.first, p.second, kTickSeconds, 0))
      for (auto& [id, z] : walkers_)
        if (z.pushing && key_of(*z.pushing) == k) z.pushing.reset();
  // Holds end when the holder is gone or has let go (turned to something else), or the victim died.
  for (auto it = held_.begin(); it != held_.end();) {
    const auto holder = walkers_.find(it->second);
    const bool gone = holder == walkers_.end() || holder->second.mode != Mode::Attack ||
                      holder->second.target != it->first || !bodies_.get(it->first).alive;
    it = gone ? held_.erase(it) : std::next(it);
  }
  if (now % kThinkTicks == 0) sense_threats(w);
  if (w.clock.is_body_step()) step_turnings(w);
}

// ---- Persistence -------------------------------------------------------------------------------------------

namespace {
void put_cell(ArchiveWriter& a, const Cell& c) {
  a.i64(c.z);
  a.i64(c.x);
  a.i64(c.y);
}
Cell get_cell(ArchiveReader& r) {
  Cell c;
  c.z = static_cast<int>(r.i64());
  c.x = static_cast<int>(r.i64());
  c.y = static_cast<int>(r.i64());
  return c;
}
}  // namespace

void Walkers::save(ArchiveWriter& a) const {
  a.u64(walkers_.size());
  for (const auto& [id, z] : walkers_) {
    a.u64(id);
    put_cell(a, z.pos);
    a.u8(static_cast<std::uint8_t>(z.mode));
    put_cell(a, z.goal);
    a.f64(z.memory_db);
    a.i64(z.memory_tick);
    a.u64(z.target);
    a.u64(z.path.size());
    for (const Cell& c : z.path) put_cell(a, c);
    a.u64(z.next);
    a.f64(z.step_m);
    a.f64(z.walk_mps);
    a.boolean(z.pushing.has_value());
    if (z.pushing) {
      put_cell(a, {z.pushing->z, z.pushing->x, z.pushing->y});
      a.boolean(z.pushing->vertical);
    }
    a.i64(z.next_think);
    a.i64(z.next_attack);
  }
  a.u64(held_.size());
  for (const auto& [v, h] : held_) {
    a.u64(v);
    a.u64(h);
  }
  a.u64(turnings_.size());
  for (const Turning& t : turnings_) {
    a.u64(t.body);
    a.u64(t.cause);
    a.i64(t.prodrome_tick);
    a.i64(t.death_tick);
    a.i64(t.rise_after);
    a.i64(t.died_tick);
    a.boolean(t.risen);
  }
  a.u64(head_j_.size());
  for (const auto& [b, j] : head_j_) {
    a.u64(b);
    a.f64(j);
  }
  a.u64(put_down_.size());
  for (EntityId b : put_down_) a.u64(b);
  // The minute's lit windows are kept with the save: a load mid-minute must see what the run saw.
  a.i64(leaks_tick_);
  a.u64(leaks_.size());
  for (const WindowLeak& l : leaks_) {
    a.i64(l.edge.z);
    a.i64(l.edge.x);
    a.i64(l.edge.y);
    a.boolean(l.edge.vertical);
    a.f64(l.seen_m);
  }
}

void Walkers::load(ArchiveReader& r) {
  walkers_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    WalkerState z;
    z.body = r.u64();
    z.pos = get_cell(r);
    z.mode = static_cast<Mode>(r.u8());
    z.goal = get_cell(r);
    z.memory_db = r.f64();
    z.memory_tick = r.i64();
    z.target = r.u64();
    for (std::uint64_t k = r.u64(); k > 0; --k) z.path.push_back(get_cell(r));
    z.next = r.u64();
    z.step_m = r.f64();
    z.walk_mps = r.f64();
    if (r.boolean()) {
      const Cell c = get_cell(r);
      z.pushing = EdgeRef{c.z, c.x, c.y, r.boolean()};
    }
    z.next_think = r.i64();
    z.next_attack = r.i64();
    walkers_[z.body] = z;
  }
  held_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const EntityId v = r.u64();
    held_[v] = r.u64();
  }
  turnings_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    Turning t;
    t.body = r.u64();
    t.cause = r.u64();
    t.prodrome_tick = r.i64();
    t.death_tick = r.i64();
    t.rise_after = r.i64();
    t.died_tick = r.i64();
    t.risen = r.boolean();
    turnings_.push_back(t);
  }
  head_j_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    const EntityId b = r.u64();
    head_j_[b] = r.f64();
  }
  put_down_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) put_down_.insert(r.u64());
  loss_cache_.clear();
  cache_version_ = ~std::uint64_t{0};
  leaks_tick_ = r.i64();
  leaks_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    WindowLeak l;
    l.edge.z = static_cast<int>(r.i64());
    l.edge.x = static_cast<int>(r.i64());
    l.edge.y = static_cast<int>(r.i64());
    l.edge.vertical = r.boolean();
    l.seen_m = r.f64();
    leaks_.push_back(l);
  }
}

}  // namespace zs
