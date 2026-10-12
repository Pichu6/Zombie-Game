// Expectations: broken commitments are noticed. Spec: docs/tier0/specs/59-expectations.md (decision 0070)
#include "zs/expectations.h"

#include <algorithm>
#include <stdexcept>

#include "toml.hpp"

namespace zs {
namespace {

constexpr std::int64_t kTicksPerMinute = kTicksPerGameHour / 60;

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

const Expectations::Case* Expectations::find(PopId pop) const {
  for (const Case& c : cases_)
    if (c.pop == pop) return &c;
  return nullptr;
}

void Expectations::refuse(World& w, const Command& c, EventId cause, std::int64_t why) {
  w.emit(EventType::ExpectationRefused, c.subject, cause, c.type, why);
}

void Expectations::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type == AbsenceBegin) {
    if (find(c.subject)) return refuse(w, c, cause, 1);
    Case k;
    k.pop = c.subject;
    k.began = w.clock.tick;
    k.cause = c.a >= 0 && c.a < static_cast<std::int64_t>(AbsenceCause::kCount) ? static_cast<AbsenceCause>(c.a) : AbsenceCause::Unknown;
    const bool phoned_in = c.b != 0;
    k.begin_event = w.emit(EventType::AbsenceBegan, c.subject, cause, static_cast<std::int64_t>(k.cause), phoned_in ? 1 : 0);
    k.last_event = k.begin_event;
    k.next_tick = kNever;
    std::optional<Commitment> due;
    if (!phoned_in && next_) due = next_(c.subject, w.clock.tick);
    if (due && !due->holders.empty()) {  // somebody expects the pop somewhere: the ladder starts if it is still absent then
      k.stage = AbsenceStage::Waiting;
      k.kind = due->kind;
      k.holders = due->holders;
      const LadderTimes& lt = params_.times[static_cast<int>(due->kind)];
      k.broken_tick = std::max(w.clock.tick, due->due_tick + minutes(lt.tolerance_min));
      k.next_tick = k.broken_tick;
    }  // else Ordinary: phoned in, or nobody expects the pop (a hermit's absence goes unnoticed)
    cases_.push_back(std::move(k));
  } else if (c.type == AbsenceEnd) {
    const auto it = std::find_if(cases_.begin(), cases_.end(), [&](const Case& k) { return k.pop == c.subject; });
    if (it == cases_.end()) return refuse(w, c, cause, 2);
    const std::int64_t gone = w.clock.tick - it->began;
    const EventId ended = w.emit(EventType::AbsenceEnded, it->pop, it->last_event, static_cast<std::int64_t>(it->stage), gone);
    if (it->stage >= AbsenceStage::Reported) w.emit(EventType::CaseClosed, it->pop, ended, w.clock.tick - it->broken_tick);
    cases_.erase(it);
  }
}

std::int64_t Expectations::next_wake(const World&, std::int64_t now) const {
  std::int64_t next = kNever;
  for (const Case& c : cases_) next = std::min(next, std::max(now, c.next_tick));
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
          c.next_tick = c.next_tick + minutes(lt.search_after_min);
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
  }
}

void Expectations::load(ArchiveReader& r) {
  cases_.clear();
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
    cases_.push_back(std::move(c));
  }
}

}  // namespace zs
