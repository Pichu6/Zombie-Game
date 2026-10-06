// The run. Spec: docs/tier0/specs/26-run.md (rules for the headless run).
#include "zs/run.h"

#include <algorithm>
#include <stdexcept>

#include "zs/pop_ai.h"

namespace zs {

const char* to_string(RunOutcome o) {
  switch (o) {
    case RunOutcome::Running: return "running";
    case RunOutcome::Survived: return "survived to day 14";
    case RunOutcome::Rescued: return "survived to day 14 and heard where the pickup is";
    case RunOutcome::AllDead: return "nobody left";
  }
  return "?";
}

const char* to_string(Role r) {
  switch (r) {
    case Role::Neighbour: return "neighbour";
    case Role::Technician: return "technician";
    case Role::Stranger: return "stranger";
  }
  return "?";
}

Run::Run(World&, Bodies& bodies, Actors& actors, Power& power, Census& census, Walkers& walkers, PopAI& ai)
    : bodies_(bodies), actors_(actors), power_(power), census_(census), walkers_(walkers), ai_(ai) {}

void Run::start(World&, EntityId player, EntityId radio, std::int64_t stranger_tick, bool stranger_bitten,
                std::int64_t broadcast_tick, std::int64_t end_tick) {
  radio_ = radio;
  stranger_tick_ = stranger_tick;
  stranger_bitten_ = stranger_bitten;
  broadcast_tick_ = broadcast_tick;
  end_tick_ = end_tick;
  controlled_ = player;
  group_ = {player};
  actors_.story_check = [this](const World&, EntityId, EntityId t, const std::string& how) { return check(t, how); };
  actors_.story_apply = [this](World& w, EntityId b, EntityId t, const std::string& how, EventId c) { apply(w, b, t, how, c); };
}

bool Run::waiting(EntityId body) const {
  for (const Found& f : found_)
    if (f.body == body) return !f.joined && !bodies_.get(body).away && bodies_.get(body).alive;
  return false;
}

Reason Run::check(EntityId target, const std::string& how) const {
  if (how == "invite") {
    for (const Found& f : found_)
      if (f.body == target) return f.joined ? Reason::AlreadyDone : waiting(target) ? Reason::Ok : Reason::WrongTarget;
    return Reason::WrongTarget;  // already one of the group, or nobody we know
  }
  if (target != radio_) return Reason::WrongTarget;
  if (how == "repair_radio") return radio_repaired_ ? Reason::AlreadyDone : Reason::Ok;
  if (how == "listen_radio") {
    if (!radio_repaired_) return Reason::Broken;
    return power_.powered(radio_) ? Reason::Ok : Reason::NoPower;
  }
  return Reason::WrongTarget;
}

void Run::apply(World& w, EntityId body, EntityId target, const std::string& how, EventId cause) {
  if (how == "invite") {
    for (Found& f : found_) {
      if (f.body != target || f.joined) continue;
      f.joined = true;
      group_.push_back(target);
      census_.set_fate(w, *census_.person_of(target), Fate::Pop);
      ai_.add_pop(w, target);
      if (f.role == Role::Stranger && stranger_bitten_) ai_.set_honest(target, false);  // hides it (spec 26 rule 2)
      w.emit(EventType::Joined, target, cause, static_cast<std::int64_t>(body));
    }
  } else if (how == "repair_radio") {
    radio_repaired_ = true;
    w.emit(EventType::RadioRepaired, body, cause);
  } else if (how == "listen_radio") {
    last_listen_ = w.clock.tick;
    if (w.clock.tick >= broadcast_tick_) {
      if (!heard_) w.emit(EventType::RadioHeard, body, cause);
      heard_ = true;
    } else {
      w.emit(EventType::RadioStatic, body, cause);
    }
  }
}

void Run::update(World& w) {
  if (outcome_ != RunOutcome::Running) return;
  const std::int64_t now = w.clock.tick;
  // The stranger comes to the yard gate (rule 2).
  for (const Found& f : found_) {
    Body& b = bodies_.get(f.body);
    if (f.role == Role::Stranger && b.away && now >= stranger_tick_) {
      b.away = false;
      const EventId ev = w.emit(EventType::StrangerArrived, f.body, 0);
      // The hidden bite happened on the way here: one of the dead still out on the streets.
      if (stranger_bitten_) {
        EntityId biter = 0;
        for (const Person& p : census_.people())
          if (!biter && p.body && (p.fate == Fate::WalkerOffMap || p.fate == Fate::WalkerOnMap) && bodies_.get(p.body).alive)
            biter = p.body;
        for (int i = 0; biter && i < 20 && !walkers_.turning_of(f.body); ++i) walkers_.bite(w, biter, f.body, ev);
        stranger_bitten_ = walkers_.turning_of(f.body).has_value();  // nobody left out there to bite him: he comes clean
      }
    }
  }
  auto living = [&](EntityId id) { const Body& b = bodies_.get(id); return b.alive && !b.spec.walker; };
  // Control passes to the member who joined first (rule 6).
  if (!living(controlled_)) {
    const auto next = std::find_if(group_.begin(), group_.end(), living);
    if (next != group_.end()) {
      w.emit(EventType::ControlPassed, *next, 0, static_cast<std::int64_t>(controlled_));
      if (ai_.possessed() == controlled_) ai_.possess(*next);  // the player's hands move to the next member
      controlled_ = *next;
    }
  }
  // The end (rule 7).
  if (std::none_of(group_.begin(), group_.end(), living)) outcome_ = RunOutcome::AllDead;
  else if (now >= end_tick_) outcome_ = heard_ ? RunOutcome::Rescued : RunOutcome::Survived;
  if (outcome_ != RunOutcome::Running) w.emit(EventType::RunEnded, controlled_, 0, static_cast<std::int64_t>(outcome_));
}

void Run::save(ArchiveWriter& a) const {
  a.u64(radio_);
  a.i64(stranger_tick_);
  a.i64(broadcast_tick_);
  a.i64(end_tick_);
  a.boolean(stranger_bitten_);
  a.u64(found_.size());
  for (const Found& f : found_) {
    a.u8(static_cast<std::uint8_t>(f.role));
    a.u64(f.body);
    a.boolean(f.joined);
  }
  a.u64(group_.size());
  for (EntityId b : group_) a.u64(b);
  a.u64(controlled_);
  a.boolean(radio_repaired_);
  a.boolean(heard_);
  a.i64(last_listen_);
  a.u8(static_cast<std::uint8_t>(outcome_));
}

void Run::load(ArchiveReader& r) {
  radio_ = r.u64();
  stranger_tick_ = r.i64();
  broadcast_tick_ = r.i64();
  end_tick_ = r.i64();
  stranger_bitten_ = r.boolean();
  found_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) {
    Found f;
    f.role = static_cast<Role>(r.u8());
    f.body = r.u64();
    f.joined = r.boolean();
    found_.push_back(f);
  }
  group_.clear();
  for (std::uint64_t n = r.u64(); n > 0; --n) group_.push_back(r.u64());
  controlled_ = r.u64();
  radio_repaired_ = r.boolean();
  heard_ = r.boolean();
  last_listen_ = r.i64();
  outcome_ = static_cast<RunOutcome>(r.u8());
}

}  // namespace zs
