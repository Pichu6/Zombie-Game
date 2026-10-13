// The hand-made Tier 0 block: cells, edges, rooms, portals, stairs and paths.
// Spec: docs/tier0/specs/09-site-map.md. Map legend: data/tier0/map/README.md
#pragma once
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "zs/assembly.h"

namespace zs {

enum class Floor : std::uint8_t { None, Wood, Concrete, Grass, Asphalt, Dirt };
bool is_indoor(Floor f);

enum class EdgeKind : std::uint8_t { None, WallInterior, WallExterior, DoorInterior, DoorExterior, Window, Fence, Gate };
bool is_openable(EdgeKind k);

struct Cell {
  int z = 0, x = 0, y = 0;
  bool operator==(const Cell&) const = default;
};

struct Edge {
  EdgeKind kind = EdgeKind::None;
  EntityId assembly = 0;   // the wall, door or window instance
  EntityId barricade = 0;  // boards in front of it, if any
  bool open = false;       // doors, windows, gates
  bool locked = false;
  bool breached = false;   // failed under attack: passable
};

// An edge address: horizontal edges sit on the top side of cell (x, y), vertical on the left side.
struct EdgeRef {
  int z = 0, x = 0, y = 0;
  bool vertical = false;
  bool operator==(const EdgeRef&) const = default;
};

struct Stairs {
  Cell a, b;
};

struct MapObject {
  std::string kind;  // pop_start, generator, fridge, lamp, stockpile, ...
  Cell at;
  std::string id;    // optional name
  std::vector<std::pair<std::string, double>> amounts;  // stocks and contents (e.g. food_kj = 60000)
};

struct Room {
  int z = 0;
  int cells = 0;
  bool indoor = false;
  bool touches_border = false;  // outdoor room joined to the off-map world
};

struct Portal {
  int room_a = 0, room_b = 0;
  std::optional<EdgeRef> edge;  // empty for stairs
  bool open = false;            // air and sound pass freely
};

class SiteMap : public System {
 public:
  const char* name() const override { return "site_map"; }
  void update(World&) override {}
  void save(ArchiveWriter& w) const override;
  void load(ArchiveReader& r) override;

  // Loads the TOML map and creates an assembly instance for every wall, door, window and fence.
  // Objects tagged `scenario = "..."` load only when that scenario is set here first ("" = none).
  void set_scenario(const std::string& s) { scenario_ = s; }
  void load_file(World& w, Assemblies& items, const std::string& path);

  // Spec 70: a fragment (another place, such as a town's police station) added beside the map as levels of its own, its
  // level z at z_base + z. No stairs join it to the map's levels, so nothing walks, sees or hears between them. Setup only,
  // after load_file and before anything indexes the map; the fragment must fit the map's width and height. Returns its
  // site number (1, 2, ...; the map itself is site 0).
  int add_site(World& w, Assemblies& items, const std::string& toml_text, int z_base);

  int width() const { return width_; }
  int height() const { return height_; }
  const std::vector<int>& levels() const { return levels_; }
  // Spec 70: which site a level belongs to (0: the map itself), and whether two levels are in the same place.
  int site_of(int z) const;
  bool same_site(int z1, int z2) const { return site_of(z1) == site_of(z2); }
  bool in_bounds(const Cell& c) const;
  Floor floor(const Cell& c) const;
  bool walkable(const Cell& c) const { return floor(c) != Floor::None; }
  // Walkable floor with no blocking object on it (spec 50 rule 3): where a body may end a step.
  bool passable(const Cell& c) const;
  bool blocked(const Cell& c) const;

  const Edge& edge(const EdgeRef& e) const;
  Edge& edge(const EdgeRef& e);
  // The edge between two 4-neighbour cells on one level.
  static EdgeRef edge_between(const Cell& a, const Cell& b);

  // Edge changes mark the room graph dirty; it is rebuilt on the next read.
  void set_open(const EdgeRef& e, bool open);
  void set_breached(const EdgeRef& e, bool breached);
  void set_locked(const EdgeRef& e, bool locked);
  void set_barricade(const EdgeRef& e, EntityId barricade);  // 0 removes it

  // Bumped by every edge change and by load: caches of derived fields key on it.
  std::uint64_t version() const { return version_; }
  // Something on an edge changed that its kind and flags don't show (planks added or taken off):
  // caches keyed on version() must recompute.
  void touch() { ++version_; }
  bool air_passes(const EdgeRef& e) const;   // sound and air pass freely
  bool can_walk(const EdgeRef& e) const;     // a body can cross (closed unlocked doors open on the way)

  // Room graph, rebuilt only when dirty.
  int room_of(const Cell& c) const;
  const std::vector<Room>& rooms() const;
  const std::vector<Portal>& portals() const;

  // Shortest walking path (Dijkstra, fixed neighbour order, 8 directions, spec 50). Empty if unreachable.
  std::vector<Cell> find_path(const Cell& from, const Cell& to) const;
  // A single step from `from` to an 8-neighbour on its level: the target is passable and the edge (straight) can be
  // walked, or the corner (diagonal) is open: both orthogonal cells passable, no edge of any kind around it.
  bool step_allowed(const Cell& from, const Cell& to) const;
  // Where a heading (dx, dy in -1..1) leads from `from` (spec 50 rule 6): its own neighbour, else for a diagonal the
  // passable axis, else nothing.
  std::optional<Cell> steer(const Cell& from, int dx, int dy) const;
  // Metres walked for one step of a path: 1 straight, sqrt 2 diagonal, 4 for a flight of stairs.
  static double step_length(const Cell& from, const Cell& to);

  const std::vector<Stairs>& stairs() const { return stairs_; }
  const std::vector<MapObject>& objects() const { return objects_; }
  std::optional<Cell> find_object(const std::string& kind, const std::string& id = "") const;

 private:
  int level_index(int z) const;
  std::size_t cell_index(const Cell& c) const;
  std::size_t edge_index(const EdgeRef& e) const;
  void rebuild_rooms() const;

  std::string scenario_;
  int width_ = 0, height_ = 0;
  std::vector<int> levels_;
  std::vector<int> level_sites_;  // spec 70: per level, its site (0: the map itself)
  int sites_ = 1;
  std::vector<Floor> floors_;
  std::vector<Edge> h_edges_;  // per level: (height + 1) rows x width
  std::vector<Edge> v_edges_;  // per level: height rows x (width + 1)
  std::vector<Stairs> stairs_;
  std::vector<MapObject> objects_;
  std::vector<std::string> blocking_kinds_;  // spec 50 rule 3
  std::vector<bool> blocked_;                // per cell, from objects_ and blocking_kinds_

  mutable bool dirty_ = true;
  std::uint64_t version_ = 0;
  mutable std::vector<int> room_ids_;
  mutable std::vector<Room> rooms_;
  mutable std::vector<Portal> portals_;
};

// Loads item definitions from data/tier0/items.toml into the assemblies store.
void load_item_defs(Assemblies& items, const std::string& path);

}  // namespace zs
