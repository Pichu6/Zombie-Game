// Players and possession. Specs: docs/tier0/specs/66-players-and-possession.md, 67-the-body-of-possession.md
// (decisions 0066, 0068, 0070)
#include "zs/players.h"

#include <algorithm>
#include <stdexcept>

#include "toml.hpp"
#include "zs/expectations.h"

namespace zs {
namespace {

constexpr std::int64_t kTicksPerMinute = kTicksPerGameHour / 60;
constexpr PlayerRefusal kNone{0};

}  // namespace

const char* player_mode_name(PlayerMode m) {
  switch (m) {
    case PlayerMode::Embodied: return "embodied";
    case PlayerMode::Detached: return "detached";
    case PlayerMode::Observer: return "observer";
    default: return "?";
  }
}

const char* detach_level_name(DetachLevel l) {
  switch (l) {
    case DetachLevel::Free: return "free";
    case DetachLevel::Knowledge: return "knowledge";
    case DetachLevel::Off: return "off";
    default: return "?";
  }
}

PossessionParams load_possession_params(const std::string& path) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  PossessionParams p;
  const auto m = t["release"]["return_home_min"].value<std::int64_t>();
  if (!m || *m < 0) throw std::runtime_error(path + ": [release] needs a non-negative return_home_min");
  p.return_home_min = static_cast<int>(*m);
  const auto slots = t["body"]["slots"].value<std::int64_t>();
  if (!slots || *slots < 0) throw std::runtime_error(path + ": [body] needs a non-negative slots");
  p.body_slots = static_cast<int>(*slots);
  return p;
}

const Players::Player* Players::find(std::uint64_t player) const {
  for (const Player& p : players_)
    if (p.id == player) return &p;
  return nullptr;
}

Players::Player* Players::get(std::uint64_t player) { return const_cast<Player*>(find(player)); }

std::uint64_t Players::holder_of(PopId pop) const {
  if (pop == kNoPop) return 0;
  for (const Player& p : players_)
    if (p.in == pop || (p.home == pop && !p.home_dead)) return p.id;
  return 0;
}

bool Players::can_take(const Player& p, PopId pop) const {
  const std::uint64_t holder = holder_of(pop);
  return holder == 0 || holder == p.id;
}

// Spec 67: the map has one player's hands (the pop AI leaves one body to the player).
bool Players::on_map_held_by_other(const Player& p) const {
  for (const Player& q : players_)
    if (q.id != p.id && q.body != 0 && std::none_of(pooled_.begin(), pooled_.end(), [&](const Pooled& x) { return x.body == q.body; }))
      return true;
  return false;
}

void Players::refuse(World& w, const Command& c, EventId cause, PlayerRefusal why) {
  w.emit(EventType::PlayerRefused, c.subject, cause, c.type, static_cast<std::int64_t>(why));
}

// Rule 4: leaving a pop. One that was taken from its life heads home and is back after the getting-home time; the home
// simply goes on living (its own AI and standing orders). Spec 67: a body on the map goes back to its AI; a pool body is
// kept until its dwell is over, then goes back to the pool.
void Players::release(World& w, Player& p, EventId cause) {
  if (p.in == kNoPop) return;
  const PopId pop = p.in;
  if (p.body != 0 && hands_) {
    const auto pooled = std::find_if(pooled_.begin(), pooled_.end(), [&](const Pooled& x) { return x.body == p.body; });
    if (pooled != pooled_.end()) pooled->release_at = std::max(w.clock.tick, pooled->since + hands_->dwell_ticks());
    else hands_->give_back(w, p.body);
  }
  p.body = 0;
  p.in = kNoPop;
  p.mode = p.home != kNoPop ? PlayerMode::Detached : PlayerMode::Observer;
  const Expectations::Case* open = exp_ ? exp_->find(pop) : nullptr;
  const bool taken = open && open->cause == AbsenceCause::Possessed;
  const std::int64_t home_at = taken ? w.clock.tick + static_cast<std::int64_t>(params_.return_home_min) * kTicksPerMinute : 0;
  const EventId left = w.emit(EventType::PlayerLeftBody, p.id, cause, static_cast<std::int64_t>(pop), home_at);
  if (taken) returns_.push_back({pop, home_at, left});
}

// Rule 3: entering a pop. A pop that is not the player's home is taken from its life: an absence begins, caused by the
// possession. A pop on its way home is taken again before it arrives: its absence goes on. Spec 67: a pop whose body is
// on the map is controlled where it is and is not absent; a pop with no body gets one from the pool. Everything that can
// refuse is checked before anything changes.
PlayerRefusal Players::possess(World& w, Player& p, PopId pop, EventId cause) {
  if (!alive_ || !alive_(pop, w.clock.tick)) return PlayerRefusal::NotAlive;
  if (p.in == pop) return PlayerRefusal::AlreadyThere;
  if (!can_take(p, pop)) return PlayerRefusal::Reserved;
  const bool home = pop == p.home && !p.home_dead;
  const auto heading_home = [&](const Return& r) { return r.pop == pop; };
  const bool on_way = std::any_of(returns_.begin(), returns_.end(), heading_home);
  const Expectations::Case* open = exp_ ? exp_->find(pop) : nullptr;
  if (open && !on_way && !home) return PlayerRefusal::Away;
  Hands::Found found;
  EntityId body = 0;
  if (hands_) {
    found = hands_->body_of(w, pop);
    if (found.where == Hands::Where::Unavailable) return PlayerRefusal::Away;
    if (found.where == Hands::Where::OnMap) {
      if (on_map_held_by_other(p)) return PlayerRefusal::MapTaken;
      body = found.body;
    } else {
      const auto kept = std::find_if(pooled_.begin(), pooled_.end(), [&](const Pooled& x) { return x.pop == pop; });
      if (kept != pooled_.end()) {  // its pool body is still held from an earlier possession: the same body again
        kept->release_at = kNever;
        body = kept->body;
      } else {
        body = hands_->embody(w, pop, cause);
        if (body == 0) return PlayerRefusal::NoBody;
        pooled_.push_back({pop, body, w.clock.tick, kNever, cause});
      }
    }
  }
  release(w, p, cause);  // may add a return: no iterator into returns_ survives it
  const bool on_map = hands_ && found.where == Hands::Where::OnMap;
  const bool begins = !home && !on_way && !on_map && exp_ != nullptr;
  returns_.erase(std::remove_if(returns_.begin(), returns_.end(), heading_home), returns_.end());
  if (on_map) hands_->take(w, body);
  p.in = pop;
  p.body = body;
  p.mode = PlayerMode::Embodied;
  const EventId took = w.emit(EventType::PlayerPossessed, p.id, cause, static_cast<std::int64_t>(pop), begins ? 1 : 0);
  if (begins) exp_->begin_absence(w, pop, AbsenceCause::Possessed, false, took);
  return kNone;
}

void Players::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type < PlayerJoin || c.type > PlayerSetLevel) return;
  if (c.type == PlayerJoin) {
    const PopId pop = static_cast<PopId>(c.a);
    const DetachLevel level = c.b >= 0 && c.b < static_cast<std::int64_t>(DetachLevel::kCount) ? static_cast<DetachLevel>(c.b) : DetachLevel::Knowledge;
    if (pop == kNoPop && level != DetachLevel::Free) return refuse(w, c, cause, PlayerRefusal::Level);  // 0068 rule 5: free cam needs free
    if (pop != kNoPop && (!alive_ || !alive_(pop, w.clock.tick))) return refuse(w, c, cause, PlayerRefusal::NotAlive);
    if (pop != kNoPop && holder_of(pop) != 0) return refuse(w, c, cause, PlayerRefusal::Reserved);
    if (pop != kNoPop && exp_ && exp_->find(pop)) return refuse(w, c, cause, PlayerRefusal::Away);
    Player p;
    p.id = players_.empty() ? 1 : players_.back().id + 1;
    p.level = level;
    if (pop != kNoPop) {
      p.home = pop;
      p.pins.push_back(pop);
    }
    players_.push_back(p);
    if (pop != kNoPop) {
      if (const PlayerRefusal why = possess(w, players_.back(), pop, cause); why != kNone) {  // no body for it: no join
        players_.pop_back();
        return refuse(w, c, cause, why);
      }
    }
    w.emit(EventType::PlayerJoined, p.id, cause, static_cast<std::int64_t>(pop), static_cast<std::int64_t>(level));
    return;
  }
  Player* p = get(c.subject);
  if (!p) return refuse(w, c, cause, PlayerRefusal::NoSuchPlayer);
  const bool free = p->level == DetachLevel::Free;
  const bool homeless = p->home == kNoPop || p->home_dead;  // choosing a new home is open at every level (0068 rule 5)
  switch (c.type) {
    case PlayerPossess: {
      const PopId pop = static_cast<PopId>(c.a);
      if (!free && !homeless && pop != p->home) return refuse(w, c, cause, PlayerRefusal::Level);
      if (const PlayerRefusal why = possess(w, *p, pop, cause); why != kNone) refuse(w, c, cause, why);
      return;
    }
    case PlayerDetach:
      if (p->in == kNoPop) return refuse(w, c, cause, PlayerRefusal::NotEmbodied);
      if (p->level == DetachLevel::Off && !homeless) return refuse(w, c, cause, PlayerRefusal::Level);  // off cannot leave the body
      return release(w, *p, cause);
    case PlayerReturn:
      if (homeless) return refuse(w, c, cause, PlayerRefusal::NoHome);
      if (p->in == p->home) return refuse(w, c, cause, PlayerRefusal::AlreadyThere);
      if (const PlayerRefusal why = possess(w, *p, p->home, cause); why != kNone) refuse(w, c, cause, why);
      return;
    case PlayerMakeHome: {
      if (p->in == kNoPop) return refuse(w, c, cause, PlayerRefusal::NotEmbodied);
      if (p->in == p->home && !p->home_dead) return refuse(w, c, cause, PlayerRefusal::AlreadyThere);
      const PopId old = p->home;
      p->home = p->in;
      p->home_dead = false;
      p->pins.erase(std::remove(p->pins.begin(), p->pins.end(), p->home), p->pins.end());
      p->pins.insert(p->pins.begin(), p->home);  // the old home stays as an ordinary pin
      w.emit(EventType::PlayerHomeSet, p->id, cause, static_cast<std::int64_t>(p->home), static_cast<std::int64_t>(old));
      return;
    }
    case PlayerPin: {
      const PopId pop = static_cast<PopId>(c.a);
      if (!alive_ || !alive_(pop, w.clock.tick)) return refuse(w, c, cause, PlayerRefusal::NotAlive);
      if (std::find(p->pins.begin(), p->pins.end(), pop) != p->pins.end()) return refuse(w, c, cause, PlayerRefusal::AlreadyThere);
      p->pins.push_back(pop);
      w.emit(EventType::PlayerPinned, p->id, cause, static_cast<std::int64_t>(pop), 1);
      return;
    }
    case PlayerUnpin: {
      const PopId pop = static_cast<PopId>(c.a);
      const auto it = std::find(p->pins.begin(), p->pins.end(), pop);
      if (it == p->pins.end()) return refuse(w, c, cause, PlayerRefusal::NotPinned);
      if (pop == p->home && !p->home_dead) return refuse(w, c, cause, PlayerRefusal::HomeLocked);
      p->pins.erase(it);
      w.emit(EventType::PlayerPinned, p->id, cause, static_cast<std::int64_t>(pop), 0);
      return;
    }
    case PlayerCallIn: {
      if (p->in == kNoPop) return refuse(w, c, cause, PlayerRefusal::NotEmbodied);
      const Expectations::CallIn r = exp_ ? exp_->phone_in(w, p->in, cause) : Expectations::CallIn::NoCase;
      if (r == Expectations::CallIn::TooLate) return refuse(w, c, cause, PlayerRefusal::TooLate);
      if (r != Expectations::CallIn::Done) return refuse(w, c, cause, PlayerRefusal::NothingToCall);
      return;
    }
    case PlayerSetLevel: {
      if (c.a < 0 || c.a >= static_cast<std::int64_t>(DetachLevel::kCount)) return refuse(w, c, cause, PlayerRefusal::Level);
      const DetachLevel to = static_cast<DetachLevel>(c.a);
      if (to == p->level) return refuse(w, c, cause, PlayerRefusal::AlreadyThere);
      if (to == DetachLevel::Off && p->in == kNoPop) return refuse(w, c, cause, PlayerRefusal::NotEmbodied);  // off lives in a body
      if (to == DetachLevel::Knowledge && p->home == kNoPop && p->in == kNoPop) return refuse(w, c, cause, PlayerRefusal::NoHome);  // no side to know through
      const bool raised = to < p->level;
      p->level = to;
      p->marked = p->marked || raised;
      w.emit(EventType::PlayerLevelSet, p->id, cause, static_cast<std::int64_t>(to), raised ? 1 : 0);
      return;
    }
    default:
      return;
  }
}

// Rule 8: the bodies are looked at, every tick while a player is in a body and once an hour while one only has a home.
// A home that died leaves its player out of the body above it, unless the scenario hands control on (spec 67: the run's
// next member becomes the home); a possessed pop that died leaves its player out too (its absence stays open: nobody
// knows where it is).
void Players::body_check(World& w) {
  const std::int64_t now = w.clock.tick;
  for (Player& p : players_) {
    if (p.in != kNoPop && !alive_(p.in, now)) {
      const PopId dead = p.in;
      const bool was_home = dead == p.home;
      if (was_home) p.home_dead = true;
      if (p.body != 0 && hands_) {
        const auto pooled = std::find_if(pooled_.begin(), pooled_.end(), [&](const Pooled& x) { return x.body == p.body; });
        if (pooled != pooled_.end()) pooled_.erase(pooled);  // the pop system freed the slot with the death
        else hands_->give_back(w, p.body);
      }
      p.in = kNoPop;
      p.body = 0;
      p.mode = p.home != kNoPop ? PlayerMode::Detached : PlayerMode::Observer;  // out of the body above it, home or not (0068 rule 5)
      const EventId died = w.emit(EventType::PlayerBodyDied, p.id, 0, static_cast<std::int64_t>(dead), was_home ? 1 : 0);
      const PopId next = was_home && hands_ ? hands_->successor(w, dead) : kNoPop;
      if (next != kNoPop && alive_(next, now) && can_take(p, next)) {
        const PopId old = p.home;
        p.home = next;
        p.home_dead = false;
        p.pins.erase(std::remove(p.pins.begin(), p.pins.end(), next), p.pins.end());
        p.pins.insert(p.pins.begin(), next);
        const EventId set = w.emit(EventType::PlayerHomeSet, p.id, died, static_cast<std::int64_t>(next), static_cast<std::int64_t>(old));
        possess(w, p, next, set);  // a refusal leaves the player out of the body above its new home
      }
    }
    if (p.home != kNoPop && !p.home_dead && !alive_(p.home, now)) {
      p.home_dead = true;
      w.emit(EventType::PlayerBodyDied, p.id, 0, static_cast<std::int64_t>(p.home), 1);
    }
  }
}

// Spec 67: pool bodies whose dwell is over go back to the pool (a refusal, such as a body holding something, retries in an
// hour). A pop that died in its pool body was already released by the pop system.
void Players::pool_step(World& w) {
  const std::int64_t now = w.clock.tick;
  for (auto it = pooled_.begin(); it != pooled_.end();) {
    if (it->release_at > now) {
      ++it;
      continue;
    }
    if (!alive_(it->pop, now) || hands_->disembody(w, it->pop, it->cause)) {
      it = pooled_.erase(it);
    } else {
      it->release_at = now + kTicksPerGameHour;
      ++it;
    }
  }
}

std::int64_t Players::next_wake(const World&, std::int64_t now) const {
  std::int64_t next = kNever;
  for (const Return& r : returns_) next = std::min(next, std::max(now, r.at));
  for (const Pooled& b : pooled_) next = std::min(next, std::max(now, b.release_at));
  for (const Player& p : players_) {
    if (p.body != 0) return now;  // a body can die any tick
    if (p.in != kNoPop || (p.home != kNoPop && !p.home_dead))
      next = std::min(next, (now + kTicksPerGameHour - 1) / kTicksPerGameHour * kTicksPerGameHour);  // the next whole hour
  }
  return next;
}

void Players::update(World& w) {
  const std::int64_t now = w.clock.tick;
  // Rule 4: released pops reach home in the order they were released.
  for (auto it = returns_.begin(); it != returns_.end();) {
    if (it->at > now) {
      ++it;
      continue;
    }
    if (exp_) exp_->end_absence(w, it->pop);
    it = returns_.erase(it);
  }
  if (hands_ && alive_) pool_step(w);
  if (!alive_) return;
  const bool in_body = std::any_of(players_.begin(), players_.end(), [](const Player& p) { return p.body != 0; });
  if (in_body || now % kTicksPerGameHour == 0) body_check(w);
}

void Players::save(ArchiveWriter& w) const {
  w.u64(players_.size());
  for (const Player& p : players_) {
    w.u64(p.id);
    w.u8(static_cast<std::uint8_t>(p.mode));
    w.u8(static_cast<std::uint8_t>(p.level));
    w.boolean(p.marked);
    w.u64(p.home);
    w.boolean(p.home_dead);
    w.u64(p.in);
    w.u64(p.body);
    w.u64(p.pins.size());
    for (PopId pin : p.pins) w.u64(pin);
  }
  w.u64(returns_.size());
  for (const Return& r : returns_) {
    w.u64(r.pop);
    w.i64(r.at);
    w.u64(r.cause);
  }
  w.u64(pooled_.size());
  for (const Pooled& b : pooled_) {
    w.u64(b.pop);
    w.u64(b.body);
    w.i64(b.since);
    w.i64(b.release_at);
    w.u64(b.cause);
  }
}

void Players::load(ArchiveReader& r) {
  players_.clear();
  returns_.clear();
  pooled_.clear();
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    Player p;
    p.id = r.u64();
    p.mode = static_cast<PlayerMode>(r.u8());
    p.level = static_cast<DetachLevel>(r.u8());
    p.marked = r.boolean();
    p.home = r.u64();
    p.home_dead = r.boolean();
    p.in = r.u64();
    p.body = r.u64();
    const std::uint64_t pins = r.u64();
    for (std::uint64_t j = 0; j < pins; ++j) p.pins.push_back(r.u64());
    players_.push_back(std::move(p));
  }
  const std::uint64_t returns = r.u64();
  for (std::uint64_t i = 0; i < returns; ++i) {
    Return x;
    x.pop = r.u64();
    x.at = r.i64();
    x.cause = r.u64();
    returns_.push_back(x);
  }
  const std::uint64_t pooled = r.u64();
  for (std::uint64_t i = 0; i < pooled; ++i) {
    Pooled b;
    b.pop = r.u64();
    b.body = r.u64();
    b.since = r.i64();
    b.release_at = r.i64();
    b.cause = r.u64();
    pooled_.push_back(b);
  }
}

}  // namespace zs
