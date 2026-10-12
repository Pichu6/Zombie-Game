// Expectations: broken commitments are noticed, and the employer counts the shifts missed.
// Specs: docs/tier0/specs/59-expectations.md, 61-employer-absence-rule.md (decision 0070)
#include "zs/expectations.h"

#include <algorithm>
#include <stdexcept>

#include "toml.hpp"

namespace zs {
namespace {

constexpr std::int64_t kTicksPerMinute = kTicksPerGameHour / 60;
constexpr int kShiftLookups = 8;  // how many commitments are skipped looking for the next shift start

std::int64_t minutes(int m) { return static_cast<std::int64_t>(m) * kTicksPerMinute; }

LadderTimes load_times(const toml::table& t, const char* key, const std::string& path) {
  const toml::table* k = t[key].as_table();
  if (!k) throw std::runtime_error(path + ": missing table [" + key + "]");
  LadderTimes out;
  const auto get = [&](const char* name) {
    const auto v = (*k)[name].value<std::int64_t>();
    if (!v || *v < 0) throw std::runtime_error(path + ": [" + key + "] needs a non-negative " + name);
    return static_cast<int>(*v);
  };
  out.tolerance_min = get("tolerance_min");
  out.ask_after_min = get("ask_after_min");
  out.report_after_min = get("report_after_min");
  out.search_after_min = get("search_after_min");
  if (out.report_after_min < out.ask_after_min)
    throw std::runtime_error(path + ": [" + key + "] reports before asking around");
  return out;
}

}  // namespace

const char* commitment_name(CommitmentKind k) {
  switch (k) {
    case CommitmentKind::ShiftStart: return "shift start";
    case CommitmentKind::ArriveHome: return "arriving home";
    default: return "?";
  }
}

ExpectationParams load_expectation_params(const std::string& path) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  ExpectationParams p;
  p.times[static_cast<int>(CommitmentKind::ShiftStart)] = load_times(t, "shift_start", path);
  p.times[static_cast<int>(CommitmentKind::ArriveHome)] = load_times(t, "arrive_home", path);
  return p;
}

EmployerPolicy load_employer_policy(const std::string& path, const std::string& country_profile, const std::string& income) {
  toml::table t;
  try {
    t = toml::parse_file(path);
  } catch (const toml::parse_error& e) {
    throw std::runtime_error(path + ": " + std::string(e.description()));
  }
  std::string id = country_profile;
  if (id.empty() || !t[id].is_table()) {
    const auto named = t["index"][income].value<std::string>();
    if (!named) throw std::runtime_error(path + ": [index] has no table for " + income);
    id = *named;
  }
  const toml::table* c = t[id].as_table();
  if (!c) throw std::runtime_error(path + ": no table [" + id + "]");
  EmployerPolicy p;
  const auto days = (*c)["abandonment_days"].value<std::int64_t>();
  if (!days || *days < 1) throw std::runtime_error(path + ": [" + id + "] needs abandonment_days of at least 1");
  p.abandonment_days = static_cast<int>(*days);
  const auto flag = [&](const char* key) {
    const auto v = (*c)[key].value<bool>();
    if (!v) throw std::runtime_error(path + ": [" + id + "] needs " + key + " (true or false)");
    return *v;
  };
  p.automatic_separation = flag("automatic_separation");
  p.pause_on_report = flag("pause_on_report");
  p.police_conduct_case = flag("police_conduct_case");
  return p;
}

const Expectations::Case* Expectations::find(PopId pop) const {
  for (const Case& c : cases_)
    if (c.pop == pop) return &c;
  return nullptr;
}

bool Expectations::lost_job(PopId pop) const { return std::find(lost_jobs_.begin(), lost_jobs_.end(), pop) != lost_jobs_.end(); }

void Expectations::refuse(World& w, const Command& c, EventId cause, std::int64_t why) {
  w.emit(EventType::ExpectationRefused, c.subject, cause, c.type, why);
}

std::optional<Commitment> next_shift_of(const NextCommitment& next, PopId pop, std::int64_t from) {
  if (!next) return std::nullopt;
  for (int i = 0; i < kShiftLookups; ++i) {
    const std::optional<Commitment> c = next(pop, from);
    if (!c) return std::nullopt;
    if (c->kind == CommitmentKind::ShiftStart && c->employer != EmployerKind::None) return c;
    from = c->due_tick + 1;
  }
  return std::nullopt;
}

// The next shift start at or after `from` (the employer expects shifts only).
std::optional<Commitment> Expectations::next_shift(PopId pop, std::int64_t from) const { return next_shift_of(next_, pop, from); }

void Expectations::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type == AbsenceBegin) {
    if (find(c.subject)) return refuse(w, c, cause, 1);
    Case k;
    k.pop = c.subject;
    k.began = w.clock.tick;
    k.cause = c.a >= 0 && c.a < static_cast<std::int64_t>(AbsenceCause::kCount) ? static_cast<AbsenceCause>(c.a) : AbsenceCause::Unknown;
    const bool phoned_in = c.b != 0;
    k.begin_event = w.emit(EventType::AbsenceBegan, c.subject, cause, static_cast<std::int64_t>(k.cause), phoned_in ? 1 : 0);
    k.last_event = k.employer_event = k.begin_event;
    k.next_tick = kNever;
    k.shift_check = kNever;
    if (!phoned_in && next_) {
      const std::optional<Commitment> due = next_(c.subject, w.clock.tick);
      if (due && !due->holders.empty()) {  // somebody expects the pop somewhere: the ladder starts if it is still absent then
        k.stage = AbsenceStage::Waiting;
        k.kind = due->kind;
        k.holders = due->holders;
        const LadderTimes& lt = params_.times[static_cast<int>(due->kind)];
        k.broken_tick = std::max(w.clock.tick, due->due_tick + minutes(lt.tolerance_min));
        k.next_tick = k.broken_tick;
      }  // else no ladder: phoned in, or nobody is told (a hermit's absence goes unnoticed by the household)
      // The employer counts shifts missed without contact: a phone call from the pop would have ended it (AB-7).
      if (const std::optional<Commitment> shift = next_shift(c.subject, w.clock.tick)) {
        k.employer = shift->employer;
        k.shift_due = shift->due_tick;
        k.shift_check = std::max(w.clock.tick, shift->due_tick + minutes(params_.times[static_cast<int>(CommitmentKind::ShiftStart)].tolerance_min));
      }
    }
    cases_.push_back(std::move(k));
  } else if (c.type == AbsenceEnd) {
    const auto it = std::find_if(cases_.begin(), cases_.end(), [&](const Case& k) { return k.pop == c.subject; });
    if (it == cases_.end()) return refuse(w, c, cause, 2);
    const std::int64_t gone = w.clock.tick - it->began;
    const EventId ended = w.emit(EventType::AbsenceEnded, it->pop, it->last_event, static_cast<std::int64_t>(it->stage), gone);
    if (it->stage >= AbsenceStage::Reported) w.emit(EventType::CaseClosed, it->pop, ended, w.clock.tick - it->broken_tick);
    meet_employer(w, *it, ended);
    for (Listener* l : listeners_) l->on_absence_ended(w, *it, ended);
    cases_.erase(it);
  }
}

// On return the employer meets the pop (AB-7): a conduct case for a police employer, a forgiven absence if the pop had been
// reported missing, a warning for a no-call, a disciplinary process where the count had reached the limit without a
// separation. Nothing if no shift was ever counted, and nothing for a pop already separated.
void Expectations::meet_employer(World& w, const Case& c, EventId after) {
  if (c.employer == EmployerKind::None || c.missed == 0 || c.separated) return;
  if (c.employer == EmployerKind::Police && policy_.police_conduct_case) {
    w.emit(EventType::ConductCaseOpened, c.pop, after, c.missed);
    return;
  }
  const ReturnOutcome outcome = c.paused ? ReturnOutcome::Forgiven : c.disciplined ? ReturnOutcome::Disciplinary : ReturnOutcome::Warning;
  w.emit(EventType::ReturnedToWork, c.pop, after, static_cast<std::int64_t>(outcome), c.missed);
}

// One scheduled day missed without contact is counted; at the limit the employer separates, or opens a process, or (a
// police employer) waits for the return.
void Expectations::employer_step(World& w, Case& c) {
  ++c.missed;
  c.employer_event = w.emit(EventType::ShiftMissed, c.pop, c.employer_event, c.missed, static_cast<std::int64_t>(c.employer));
  if (c.missed >= policy_.abandonment_days) {
    if (c.employer == EmployerKind::Police && policy_.police_conduct_case) {
      c.shift_check = kNever;  // a conduct case follows the return; nothing is recorded now
    } else if (policy_.automatic_separation) {
      c.employer_event = w.emit(EventType::Separated, c.pop, c.employer_event, c.missed);
      for (Listener* l : listeners_) l->on_separated(w, c, c.employer_event);
      c.separated = true;
      lost_jobs_.push_back(c.pop);
      c.shift_check = kNever;
    } else {
      c.employer_event = w.emit(EventType::DisciplinaryOpened, c.pop, c.employer_event, c.missed);
      c.disciplined = true;
      c.shift_check = kNever;
    }
    return;
  }
  if (const std::optional<Commitment> next = next_shift(c.pop, c.shift_due + 1)) {
    c.shift_due = next->due_tick;
    c.shift_check = next->due_tick + minutes(params_.times[static_cast<int>(CommitmentKind::ShiftStart)].tolerance_min);
  } else {
    c.shift_check = kNever;
  }
}

// The police have staff for the search of a reported pop (spec 64): the search opens now, caused by the last step.
void Expectations::start_search(World& w, PopId pop) {
  for (Case& c : cases_) {
    if (c.pop != pop || c.stage != AbsenceStage::Reported) continue;
    c.last_event = w.emit(EventType::SearchStarted, c.pop, c.last_event);
    c.stage = AbsenceStage::Searching;
    c.next_tick = kNever;
    return;
  }
}

std::int64_t Expectations::next_wake(const World&, std::int64_t now) const {
  std::int64_t next = kNever;
  for (const Case& c : cases_) next = std::min({next, std::max(now, c.next_tick), std::max(now, c.shift_check)});
  return next;
}

void Expectations::update(World& w) {
  const std::int64_t now = w.clock.tick;
  for (Case& c : cases_) {
    const LadderTimes& lt = params_.times[static_cast<int>(c.kind)];
    while (c.next_tick <= now) {
      switch (c.stage) {
        case AbsenceStage::Waiting: {  // the commitment fell due and the pop is still away: broken, and the first holder phones
          const EventId broken = w.emit(EventType::CommitmentBroken, c.pop, c.begin_event, static_cast<std::int64_t>(c.kind),
                                        static_cast<std::int64_t>(c.holders.front()));
          c.last_event = w.emit(EventType::HolderPhoned, c.holders.front(), broken, static_cast<std::int64_t>(c.pop), 0);
          c.stage = AbsenceStage::Broken;
          c.broken_tick = c.next_tick;
          c.next_tick = c.broken_tick + minutes(lt.ask_after_min);
          break;
        }
        case AbsenceStage::Broken:  // no answer: friends, colleagues and hospitals
          c.last_event = w.emit(EventType::HolderAskedAround, c.holders.front(), c.last_event, static_cast<std::int64_t>(c.pop),
                                static_cast<std::int64_t>(c.holders.size()));
          c.stage = AbsenceStage::Asked;
          c.next_tick = c.broken_tick + minutes(lt.report_after_min);
          break;
        case AbsenceStage::Asked:  // nobody has seen the pop: reported missing (no waiting period applies)
          c.last_event = w.emit(EventType::MissingReported, c.pop, c.last_event, static_cast<std::int64_t>(c.holders.back()));
          c.stage = AbsenceStage::Reported;
          // The ladder opens the search an hour on, unless the police own it (spec 64): then start_search does, when staffed.
          c.next_tick = external_search_ ? kNever : c.next_tick + minutes(lt.search_after_min);
          // A report explains the absence: the employer's count pauses (AB-7), if its rule says so.
          if (policy_.pause_on_report && c.employer != EmployerKind::None && !c.separated) {
            c.paused = true;
            c.shift_check = kNever;
          }
          for (Listener* l : listeners_) l->on_reported(w, c, c.last_event);
          break;
        case AbsenceStage::Reported:  // the police open a search
          c.last_event = w.emit(EventType::SearchStarted, c.pop, c.last_event);
          c.stage = AbsenceStage::Searching;
          c.next_tick = kNever;
          break;
        default:
          c.next_tick = kNever;
          break;
      }
    }
    // The employer counts a shift unless the count is paused or finished. A report found at the same tick pauses first.
    while (c.shift_check <= now && !c.paused) employer_step(w, c);
  }
}

void Expectations::save(ArchiveWriter& w) const {
  w.u64(cases_.size());
  for (const Case& c : cases_) {
    w.u64(c.pop);
    w.i64(c.began);
    w.u8(static_cast<std::uint8_t>(c.cause));
    w.u8(static_cast<std::uint8_t>(c.stage));
    w.u8(static_cast<std::uint8_t>(c.kind));
    w.i64(c.broken_tick);
    w.i64(c.next_tick);
    w.u64(c.holders.size());
    for (PopId h : c.holders) w.u64(h);
    w.u64(c.begin_event);
    w.u64(c.last_event);
    w.u8(static_cast<std::uint8_t>(c.employer));
    w.i64(c.shift_due);
    w.i64(c.shift_check);
    w.i64(c.missed);
    w.boolean(c.paused);
    w.boolean(c.separated);
    w.boolean(c.disciplined);
    w.u64(c.employer_event);
  }
  w.u64(lost_jobs_.size());
  for (PopId p : lost_jobs_) w.u64(p);
}

void Expectations::load(ArchiveReader& r) {
  cases_.clear();
  lost_jobs_.clear();
  const std::uint64_t n = r.u64();
  for (std::uint64_t i = 0; i < n; ++i) {
    Case c;
    c.pop = r.u64();
    c.began = r.i64();
    c.cause = static_cast<AbsenceCause>(r.u8());
    c.stage = static_cast<AbsenceStage>(r.u8());
    c.kind = static_cast<CommitmentKind>(r.u8());
    c.broken_tick = r.i64();
    c.next_tick = r.i64();
    const std::uint64_t holders = r.u64();
    for (std::uint64_t j = 0; j < holders; ++j) c.holders.push_back(r.u64());
    c.begin_event = r.u64();
    c.last_event = r.u64();
    c.employer = static_cast<EmployerKind>(r.u8());
    c.shift_due = r.i64();
    c.shift_check = r.i64();
    c.missed = static_cast<int>(r.i64());
    c.paused = r.boolean();
    c.separated = r.boolean();
    c.disciplined = r.boolean();
    c.employer_event = r.u64();
    cases_.push_back(std::move(c));
  }
  const std::uint64_t lost = r.u64();
  for (std::uint64_t i = 0; i < lost; ++i) lost_jobs_.push_back(r.u64());
}

}  // namespace zs
