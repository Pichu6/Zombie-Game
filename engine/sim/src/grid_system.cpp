// Spec: docs/tier0/specs/35-networks.md
#include "zs/grid_system.h"

namespace zs {

void GridSystem::refuse(World& w, const Command& c, EventId cause, GridRefusal why) {
  w.emit(EventType::GridRefused, c.subject, cause, c.type, static_cast<std::int64_t>(why));
}

void GridSystem::solved(World& w, EventId cause) {
  for (GridId g : tree_.solve()) {
    const Grid& grid = tree_.grid(g);
    w.emit(EventType::GridChanged, g, cause, grid.served_w, grid.demand_w - grid.served_w);
  }
  if (on_solved) on_solved(w, cause);
}

void GridSystem::apply_command(World& w, const Command& c, EventId cause) {
  if (c.type < GridSwitch || c.type > GridSetDemand) return;
  if (!tree_.valid(c.subject)) return refuse(w, c, cause, GridRefusal::UnknownNode);
  GridNode& nd = tree_.node(c.subject);
  switch (c.type) {
    case GridSwitch: {
      if (nd.kind != NodeKind::Switch) return refuse(w, c, cause, GridRefusal::WrongKind);
      const bool closed = c.a != 0;
      if (nd.closed == closed) return refuse(w, c, cause, GridRefusal::Nothing);
      nd.closed = closed;
      break;
    }
    case GridFault: {
      const bool sound = c.a != 0;
      if (nd.sound == sound) return refuse(w, c, cause, GridRefusal::Nothing);
      nd.sound = sound;
      break;
    }
    case GridSetCapacity: {
      if (nd.kind != NodeKind::Source) return refuse(w, c, cause, GridRefusal::WrongKind);
      if (c.a < 0) return refuse(w, c, cause, GridRefusal::BadValue);
      if (nd.capacity_w == c.a) return refuse(w, c, cause, GridRefusal::Nothing);
      nd.capacity_w = c.a;
      break;
    }
    case GridSetDemand: {
      if (nd.kind != NodeKind::Load || nd.child >= 0) return refuse(w, c, cause, GridRefusal::WrongKind);
      if (c.a < 0) return refuse(w, c, cause, GridRefusal::BadValue);
      if (nd.demand_w == c.a) return refuse(w, c, cause, GridRefusal::Nothing);
      nd.demand_w = c.a;
      break;
    }
    default: return;
  }
  solved(w, cause);
}

}  // namespace zs
