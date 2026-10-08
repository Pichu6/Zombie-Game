// Orders and laws across levels: an edict is a record (kind, target, number, reason, signer) that takes effect only
// through the ordinary commands of the systems below it, each application carrying the edict as its cause, so every
// transfer under it can be traced back ("why is the hospital short of staff?") and its effects are sums of what
// moved. Spec: docs/tier0/specs/39-orders-and-belief.md
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "zs/archive.h"
#include "zs/economy.h"
#include "zs/events.h"
#include "zs/roster.h"

namespace zs {

// What an edict may change (world simulation §7): a rate (a levy or tax share), an allocation (where it goes), a
// roster effect (a layoff or recall by region). Append only.
enum class EdictKind : std::uint8_t {
  Levy = 0,     // each day, `per_mille` of a good's bulk from every Building-scope owner under `scope` to `destination`
  Tax = 1,      // each day, `per_mille` of the money of every Building-scope owner under `scope` to `destination`, as Tax
  Staffing = 2, // on signing, `delta` members of the region's workplace rosters are laid off (negative: recalled)
};

struct EdictDef {
  std::string name;
  EdictKind kind;
  OwnerId scope;          // the region (or country) it bites in
  OwnerId destination;    // Levy, Tax: who receives
  GoodId good = 0;        // Levy
  std::int64_t per_mille = 0;  // Levy, Tax: the share per day
  std::int64_t delta = 0;      // Staffing
  std::uint32_t pop_region = 0;  // Staffing: the registry's region id
};

struct Edict {
  std::uint32_t def;
  OwnerId signer;
  std::int64_t signed_tick;
  std::int64_t repealed_tick = -1;
  EventId signed_event = 0;  // the cause every application descends from
  std::int64_t applications = 0;
  std::int64_t moved = 0;    // bulk, cents or members moved under it, ever
  bool in_force() const { return repealed_tick < 0; }
};

class Edicts {
 public:
  std::uint32_t add_def(EdictDef d);
  const EdictDef& def(std::uint32_t i) const { return defs_.at(i); }
  std::size_t defs() const { return defs_.size(); }

  std::size_t sign(std::uint32_t def, OwnerId signer, std::int64_t tick, EventId signed_event);
  bool repeal(std::size_t edict, std::int64_t tick);
  const Edict& at(std::size_t i) const { return all_.at(i); }
  Edict& at(std::size_t i) { return all_.at(i); }
  std::size_t size() const { return all_.size(); }
  const std::vector<Edict>& all() const { return all_; }

  // The edict behind an event, by walking its causes to a signed-edict event (nullopt if none).
  std::optional<std::size_t> behind(const EventLog& log, EventId event) const;
  // The chain of causes from an event up to its root, nearest first.
  static std::vector<EventId> trace(const EventLog& log, EventId event);

  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r);

 private:
  std::vector<EdictDef> defs_;
  std::vector<Edict> all_;
};

}  // namespace zs
