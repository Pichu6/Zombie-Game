#include "zs/events.h"

namespace zs {

EventId EventLog::append(std::int64_t tick, EventType type, EntityId subject, EventId cause, std::int64_t a,
                         std::int64_t b) {
  const EventId id = events_.size() + 1;
  events_.push_back({id, tick, type, subject, cause, a, b});
  return id;
}

const Event* EventLog::get(EventId id) const {
  if (id == 0 || id > events_.size()) return nullptr;
  return &events_[id - 1];
}

std::vector<Event> EventLog::cause_chain(EventId id) const {
  std::vector<Event> chain;
  for (const Event* e = get(id); e != nullptr; e = (e->cause < e->id) ? get(e->cause) : nullptr)
    chain.push_back(*e);
  return chain;
}

void Scheduler::schedule(const ScheduledEvent& e) { queue_.emplace(std::pair{e.tick, sequence_++}, e); }

std::vector<ScheduledEvent> Scheduler::pop_due(std::int64_t tick) {
  std::vector<ScheduledEvent> due;
  auto it = queue_.begin();
  while (it != queue_.end() && it->first.first <= tick) {
    due.push_back(it->second);
    it = queue_.erase(it);
  }
  return due;
}

}  // namespace zs
