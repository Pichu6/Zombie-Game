// The grid tree as a world system: commands open and close switches, fault and repair nodes, set a plant's
// capacity or a leaf load's demand; a solve follows each change and reports the grids whose service changed.
// Spec: docs/tier0/specs/35-networks.md
#pragma once
#include <cstdint>

#include "zs/grid.h"
#include "zs/world.h"

namespace zs {

enum GridCommand : std::uint16_t {
  GridSwitch = 0x3001,       // subject = node id, a = 1 close, 0 open (Switch nodes)
  GridFault = 0x3002,        // subject = node id, a = 1 sound, 0 faulted (any node)
  GridSetCapacity = 0x3003,  // subject = node id, a = watts (Source nodes)
  GridSetDemand = 0x3004,    // subject = node id, a = watts (leaf Load nodes)
};

enum class GridRefusal : std::int64_t {
  None = 0,
  UnknownNode = 1,
  WrongKind = 2,  // a switch command on a load, a demand on a child grid's load...
  Nothing = 3,    // already in that state
  BadValue = 4,
};

class GridSystem : public System {
 public:
  const char* name() const override { return "grids"; }
  GridTree& tree() { return tree_; }  // setup: shape and starting state; then commands only
  const GridTree& tree() const { return tree_; }
  void start() { tree_.solve(); }  // once at the end of setup

  void apply_command(World& w, const Command& c, EventId cause) override;
  void update(World&) override {}
  void save(ArchiveWriter& w) const override { tree_.save(w); }
  void load(ArchiveReader& r) override { tree_.load(r); }

 private:
  void refuse(World& w, const Command& c, EventId cause, GridRefusal why);
  void solved(World& w, EventId cause);
  GridTree tree_;
};

}  // namespace zs
