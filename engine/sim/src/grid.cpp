// Spec: docs/tier0/specs/35-networks.md
#include "zs/grid.h"

#include <algorithm>
#include <numeric>
#include <stdexcept>

namespace zs {

GridId GridTree::add_grid(std::string name, std::int64_t parent) {
  if (parent >= static_cast<std::int64_t>(grids_.size())) throw std::invalid_argument("GridTree::add_grid: unknown parent");
  Grid g;
  g.name = std::move(name);
  g.parent = parent;
  grids_.push_back(std::move(g));
  return static_cast<GridId>(grids_.size() - 1);
}

std::size_t GridTree::add_node(GridId g, GridNode n) {
  if (n.capacity_w < 0 || n.demand_w < 0) throw std::invalid_argument("GridTree::add_node: watts must not be negative");
  Grid& grid = grids_.at(g);
  grid.nodes.push_back(std::move(n));
  return grid.nodes.size() - 1;
}

void GridTree::add_edge(GridId g, std::size_t a, std::size_t b) {
  Grid& grid = grids_.at(g);
  if (a >= grid.nodes.size() || b >= grid.nodes.size() || a == b) throw std::invalid_argument("GridTree::add_edge: bad nodes");
  grid.edges.emplace_back(a, b);
}

void GridTree::attach(GridId parent, std::size_t load, GridId child, std::size_t feed) {
  Grid& p = grids_.at(parent);
  Grid& c = grids_.at(child);
  if (c.parent != static_cast<std::int64_t>(parent)) throw std::invalid_argument("GridTree::attach: not its parent");
  if (load >= p.nodes.size() || p.nodes[load].kind != NodeKind::Load) throw std::invalid_argument("GridTree::attach: the feed must be a Load of the parent");
  if (feed >= c.nodes.size() || c.nodes[feed].kind == NodeKind::Load) throw std::invalid_argument("GridTree::attach: the entry must not be a Load");
  p.nodes[load].child = static_cast<std::int64_t>(child);
  c.feed = feed;
}

bool GridTree::valid(NodeId id) const {
  const auto g = static_cast<std::size_t>(id >> 32);
  const auto n = static_cast<std::size_t>(id & 0xffffffff);
  return g < grids_.size() && n < grids_[g].nodes.size();
}

GridNode& GridTree::node(NodeId id) {
  if (!valid(id)) throw std::out_of_range("GridTree::node");
  return grids_[static_cast<std::size_t>(id >> 32)].nodes[static_cast<std::size_t>(id & 0xffffffff)];
}

const GridNode& GridTree::node(NodeId id) const {
  if (!valid(id)) throw std::out_of_range("GridTree::node");
  return grids_[static_cast<std::size_t>(id >> 32)].nodes[static_cast<std::size_t>(id & 0xffffffff)];
}

// Union-find over closed, sound nodes.
std::vector<int> GridTree::islands(const Grid& g) const {
  const std::size_t n = g.nodes.size();
  std::vector<std::size_t> up(n);
  std::iota(up.begin(), up.end(), 0);
  auto find = [&](std::size_t x) {
    while (up[x] != x) { up[x] = up[up[x]]; x = up[x]; }
    return x;
  };
  auto passes = [&](std::size_t i) {
    const GridNode& nd = g.nodes[i];
    return nd.sound && (nd.kind != NodeKind::Switch || nd.closed);
  };
  for (const auto& [a, b] : g.edges) {
    if (!passes(a) || !passes(b)) continue;
    const std::size_t ra = find(a), rb = find(b);
    if (ra != rb) up[std::max(ra, rb)] = std::min(ra, rb);
  }
  std::vector<int> island(n, -1);
  for (std::size_t i = 0; i < n; ++i) if (passes(i)) island[i] = static_cast<int>(find(i));
  return island;
}

// Bottom-up: what the grid would draw through its feed (or, on a root, what its loads want): the leaf loads and
// child grids reachable from the feed over closed, sound nodes. Writes each child-grid load's demand.
std::int64_t GridTree::connected_demand(GridId gi) {
  Grid& g = grids_.at(gi);
  // Children first, so a child-grid load carries the child's own connected demand.
  for (GridNode& nd : g.nodes)
    if (nd.kind == NodeKind::Load && nd.child >= 0) nd.demand_w = connected_demand(static_cast<GridId>(nd.child));
  const std::vector<int> island = islands(g);
  std::int64_t sum = 0;
  if (g.parent >= 0) {
    const int feed_island = g.feed < g.nodes.size() ? island[g.feed] : -1;
    if (feed_island < 0) { g.demand_w = 0; return 0; }
    for (std::size_t i = 0; i < g.nodes.size(); ++i)
      if (g.nodes[i].kind == NodeKind::Load && island[i] == feed_island) sum += g.nodes[i].demand_w;
  } else {
    for (std::size_t i = 0; i < g.nodes.size(); ++i)
      if (g.nodes[i].kind == NodeKind::Load && island[i] >= 0) sum += g.nodes[i].demand_w;
  }
  g.demand_w = sum;
  return sum;
}

// Top-down: each island's supply (sound sources in it, plus the feed's allotment if the feed is in it) is given to
// its loads in node order until it runs out; a child-grid load passes what it got down as its feed.
void GridTree::serve(GridId gi, std::int64_t feed_w) {
  Grid& g = grids_.at(gi);
  const std::vector<int> island = islands(g);
  const int feed_island = (g.parent >= 0 && g.feed < g.nodes.size()) ? island[g.feed] : -1;
  for (GridNode& nd : g.nodes) { nd.connected = false; nd.served_w = 0; }
  std::vector<std::int64_t> supply(g.nodes.size(), 0);  // per island root
  for (std::size_t i = 0; i < g.nodes.size(); ++i)
    if (island[i] >= 0 && g.nodes[i].kind == NodeKind::Source) supply[island[i]] += g.nodes[i].capacity_w;
  if (feed_island >= 0) supply[feed_island] += feed_w;
  g.capacity_w = 0;
  for (std::size_t i = 0; i < g.nodes.size(); ++i)
    if (island[i] >= 0 && g.nodes[i].kind == NodeKind::Source) g.capacity_w += g.nodes[i].capacity_w;
  if (feed_island >= 0) g.capacity_w += feed_w;
  std::vector<std::int64_t> left = supply;
  std::vector<std::int64_t> given(g.nodes.size(), 0);  // per island: what its sources gave
  g.served_w = 0;
  for (std::size_t i = 0; i < g.nodes.size(); ++i) {
    GridNode& nd = g.nodes[i];
    if (island[i] < 0) continue;
    nd.connected = supply[island[i]] > 0;  // the feed counts only through what it was allotted
    if (nd.kind != NodeKind::Load) continue;
    const std::int64_t got = std::min(nd.demand_w, left[island[i]]);
    left[island[i]] -= got;
    given[island[i]] += got;
    nd.served_w = got;
    g.served_w += got;
    if (nd.child >= 0) serve(static_cast<GridId>(nd.child), got);
  }
  // Sources report what they gave: the feed's allotment counts first, then the sources in node order.
  if (feed_island >= 0) given[feed_island] -= std::min(feed_w, given[feed_island]);
  for (std::size_t i = 0; i < g.nodes.size(); ++i) {
    if (island[i] < 0 || g.nodes[i].kind != NodeKind::Source) continue;
    std::int64_t& g_island = given[island[i]];
    const std::int64_t gave = std::min(g.nodes[i].capacity_w, g_island);
    g.nodes[i].served_w = gave;
    g_island -= gave;
  }
  // A child grid that is not reached at all is served nothing.
  for (std::size_t i = 0; i < g.nodes.size(); ++i)
    if (g.nodes[i].kind == NodeKind::Load && g.nodes[i].child >= 0 && island[i] < 0) serve(static_cast<GridId>(g.nodes[i].child), 0);
}

std::vector<GridId> GridTree::solve() {
  std::vector<std::pair<std::int64_t, std::int64_t>> before;
  for (const Grid& g : grids_) before.emplace_back(g.served_w, g.demand_w - g.served_w);
  for (std::size_t i = 0; i < grids_.size(); ++i)
    if (grids_[i].parent < 0) connected_demand(static_cast<GridId>(i));
  for (std::size_t i = 0; i < grids_.size(); ++i)
    if (grids_[i].parent < 0) serve(static_cast<GridId>(i), 0);
  std::vector<GridId> changed;
  for (std::size_t i = 0; i < grids_.size(); ++i)
    if (before[i] != std::make_pair(grids_[i].served_w, grids_[i].demand_w - grids_[i].served_w)) changed.push_back(static_cast<GridId>(i));
  return changed;
}

GridTree::Summary GridTree::summary(GridId gi) const {
  const Grid& g = grids_.at(gi);
  Summary s{0, 0, 0, 0};
  for (const GridNode& nd : g.nodes) {
    if (nd.kind == NodeKind::Source && nd.sound) s.capacity_w += nd.capacity_w;
    if (nd.kind == NodeKind::Load && nd.child < 0) {
      s.demand_w += nd.demand_w;
      s.served_w += nd.served_w;
    }
    if (nd.kind == NodeKind::Load && nd.child >= 0) {
      const Summary c = summary(static_cast<GridId>(nd.child));
      s.capacity_w += c.capacity_w;
      s.demand_w += c.demand_w;
      s.served_w += c.served_w;
    }
  }
  s.unserved_w = s.demand_w - s.served_w;
  return s;
}

void GridTree::save(ArchiveWriter& w) const {
  w.u64(grids_.size());
  for (const Grid& g : grids_) {
    w.u64(g.nodes.size());
    for (const GridNode& nd : g.nodes) {
      w.boolean(nd.closed); w.boolean(nd.sound); w.i64(nd.capacity_w);
      w.i64(nd.child < 0 ? nd.demand_w : 0);
    }
  }
}

void GridTree::load(ArchiveReader& r) {
  const std::uint64_t n = r.u64();
  if (n != grids_.size()) throw std::runtime_error("GridTree::load: the save has a different number of grids");
  for (Grid& g : grids_) {
    const std::uint64_t m = r.u64();
    if (m != g.nodes.size()) throw std::runtime_error("GridTree::load: the save has a different number of nodes");
    for (GridNode& nd : g.nodes) {
      nd.closed = r.boolean(); nd.sound = r.boolean(); nd.capacity_w = r.i64();
      const std::int64_t demand = r.i64();
      if (nd.child < 0) nd.demand_w = demand;
    }
  }
  solve();
}

}  // namespace zs
