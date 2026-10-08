// Networks above building scale: a tree of grids (nation, district, building) with one node record at every
// level. A child grid is a load of its parent, fed through one of its own nodes. Demand is summed bottom-up over
// the parts that are closed and sound; supply is allotted top-down in a fixed order; a fault below is unserved
// load above. Solved on change; there is no time in it. Spec: docs/tier0/specs/35-networks.md
#pragma once
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "zs/archive.h"

namespace zs {

using GridId = std::uint32_t;
using NodeId = std::uint64_t;  // (grid << 32) | node index: one id space for commands

enum class NodeKind : std::uint8_t { Source, Switch, Load, Hub };

struct GridNode {
  std::string name;
  NodeKind kind;
  std::int64_t capacity_w = 0;  // Source: what it can give when sound
  std::int64_t demand_w = 0;    // Load: what it draws when connected (a child grid's load: its connected demand)
  bool closed = true;           // Switch: open breaks the path
  bool sound = true;            // any node: a fault takes it out of the graph
  std::int64_t child = -1;      // Load: the grid it feeds (-1 = a leaf load)
  // Results of the last solve.
  bool connected = false;       // reachable from a running source or the feed
  std::int64_t served_w = 0;    // Load: what it got; Source: what it gave
};

struct Grid {
  std::string name;
  std::int64_t parent = -1;     // grid index (-1 = root)
  std::size_t feed = 0;         // the node through which the parent's supply enters (a Hub or Source); unused on a root
  std::vector<GridNode> nodes;
  std::vector<std::pair<std::size_t, std::size_t>> edges;
  // Results of the last solve.
  std::int64_t demand_w = 0;    // connected demand of its leaf loads and child grids (what it would draw)
  std::int64_t served_w = 0;
  std::int64_t capacity_w = 0;  // sound sources inside it, plus what its feed was allotted
};

class GridTree {
 public:
  GridId add_grid(std::string name, std::int64_t parent = -1);
  std::size_t add_node(GridId g, GridNode n);
  void add_edge(GridId g, std::size_t a, std::size_t b);
  // Makes node `load` of grid `parent` the feed of `child` (which must have `parent` as its parent) entering
  // `child` at its node `feed`.
  void attach(GridId parent, std::size_t load, GridId child, std::size_t feed);

  static NodeId node_id(GridId g, std::size_t node) { return (static_cast<NodeId>(g) << 32) | node; }
  bool valid(NodeId id) const;
  GridNode& node(NodeId id);
  const GridNode& node(NodeId id) const;
  const Grid& grid(GridId g) const { return grids_.at(g); }
  Grid& grid(GridId g) { return grids_.at(g); }
  std::size_t size() const { return grids_.size(); }

  // Solves the whole tree from its roots: demand up, supply down. Returns the grids whose served or unserved
  // load changed.
  std::vector<GridId> solve();

  struct Summary {
    std::int64_t capacity_w, demand_w, served_w, unserved_w;
  };
  // Readouts for a grid, including everything under it (enumeration: sums over leaf loads and sound sources).
  Summary summary(GridId g) const;

  // Persistence: the state a command can change (closed, sound, capacity, demand of leaf loads); the shape is setup.
  void save(ArchiveWriter& w) const;
  void load(ArchiveReader& r);

 private:
  std::int64_t connected_demand(GridId g);
  void serve(GridId g, std::int64_t feed_w);
  std::vector<int> islands(const Grid& g) const;  // per node: island index, -1 if unsound
  std::vector<Grid> grids_;
};

}  // namespace zs
