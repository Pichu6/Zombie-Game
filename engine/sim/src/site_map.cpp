#include "zs/site_map.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <sstream>
#include <stdexcept>

#include "toml.hpp"

namespace zs {

bool is_indoor(Floor f) { return f == Floor::Wood || f == Floor::Concrete; }

bool is_openable(EdgeKind k) {
  return k == EdgeKind::DoorInterior || k == EdgeKind::DoorExterior || k == EdgeKind::Window || k == EdgeKind::Gate;
}

namespace {

Floor floor_from_char(char c) {
  switch (c) {
    case ' ': return Floor::None;
    case '.': return Floor::Wood;
    case ',': return Floor::Concrete;
    case '"': return Floor::Grass;
    case '_': return Floor::Asphalt;
    case ':': return Floor::Dirt;
    default: throw std::runtime_error(std::string("map: unknown floor '") + c + "'");
  }
}

EdgeKind edge_from_char(char c) {
  switch (c) {
    case ' ': return EdgeKind::None;
    case '-': case '|': return EdgeKind::WallInterior;
    case '#': return EdgeKind::WallExterior;
    case 'D': return EdgeKind::DoorInterior;
    case 'E': return EdgeKind::DoorExterior;
    case 'W': return EdgeKind::Window;
    case '=': return EdgeKind::Fence;
    case 'G': return EdgeKind::Gate;
    default: throw std::runtime_error(std::string("map: unknown edge '") + c + "'");
  }
}

const char* def_for(EdgeKind k) {
  switch (k) {
    case EdgeKind::WallInterior: return "wall_interior";
    case EdgeKind::WallExterior: return "wall_exterior";
    case EdgeKind::DoorInterior: return "door_interior";
    case EdgeKind::DoorExterior: return "door_exterior";
    case EdgeKind::Window: return "window";
    case EdgeKind::Fence: return "fence";
    case EdgeKind::Gate: return "gate";
    default: return "";
  }
}

std::vector<std::string> split_lines(const std::string& text) {
  std::vector<std::string> lines;
  std::istringstream in(text);
  for (std::string line; std::getline(in, line);) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
    lines.push_back(line);
  }
  while (!lines.empty() && lines.front().empty()) lines.erase(lines.begin());
  while (!lines.empty() && lines.back().empty()) lines.pop_back();
  return lines;
}

Cell cell_from(const toml::array* a) {
  if (!a || a->size() != 3) throw std::runtime_error("map: a cell is [level, x, y]");
  return {static_cast<int>(a->get(0)->value<std::int64_t>().value()),
          static_cast<int>(a->get(1)->value<std::int64_t>().value()),
          static_cast<int>(a->get(2)->value<std::int64_t>().value())};
}

}  // namespace

int SiteMap::level_index(int z) const {
  for (std::size_t i = 0; i < levels_.size(); ++i)
    if (levels_[i] == z) return static_cast<int>(i);
  return -1;
}

bool SiteMap::in_bounds(const Cell& c) const {
  return level_index(c.z) >= 0 && c.x >= 0 && c.y >= 0 && c.x < width_ && c.y < height_;
}

std::size_t SiteMap::cell_index(const Cell& c) const {
  return (static_cast<std::size_t>(level_index(c.z)) * height_ + c.y) * width_ + c.x;
}

Floor SiteMap::floor(const Cell& c) const { return in_bounds(c) ? floors_[cell_index(c)] : Floor::None; }

std::size_t SiteMap::edge_index(const EdgeRef& e) const {
  const bool ok = e.vertical ? (e.x >= 0 && e.x <= width_ && e.y >= 0 && e.y < height_)
                             : (e.x >= 0 && e.x < width_ && e.y >= 0 && e.y <= height_);
  if (!ok) throw std::out_of_range("map: edge out of range");
  const auto lv = static_cast<std::size_t>(level_index(e.z));
  if (e.vertical) return (lv * height_ + e.y) * (width_ + 1) + e.x;
  return (lv * (height_ + 1) + e.y) * width_ + e.x;
}

const Edge& SiteMap::edge(const EdgeRef& e) const { return e.vertical ? v_edges_.at(edge_index(e)) : h_edges_.at(edge_index(e)); }
Edge& SiteMap::edge(const EdgeRef& e) { return e.vertical ? v_edges_.at(edge_index(e)) : h_edges_.at(edge_index(e)); }

EdgeRef SiteMap::edge_between(const Cell& a, const Cell& b) {
  if (a.x == b.x) return {a.z, a.x, std::max(a.y, b.y), false};
  return {a.z, std::max(a.x, b.x), a.y, true};
}

void SiteMap::load_file(World& w, Assemblies& items, const std::string& path) {
  const toml::table t = toml::parse_file(path);
  width_ = t["map"]["width"].value<int>().value();
  height_ = t["map"]["height"].value<int>().value();
  const auto* levels = t["level"].as_array();
  if (!levels) throw std::runtime_error("map: no [[level]] tables");
  levels_.clear();
  for (const auto& lv : *levels) levels_.push_back(lv.as_table()->get("z")->value<int>().value());
  const std::size_t n_levels = levels_.size();
  floors_.assign(n_levels * height_ * width_, Floor::None);
  h_edges_.assign(n_levels * (height_ + 1) * width_, Edge{});
  v_edges_.assign(n_levels * height_ * (width_ + 1), Edge{});

  for (const auto& lv : *levels) {
    const auto& tab = *lv.as_table();
    const int z = tab["z"].value<int>().value();
    const auto lines = split_lines(tab["grid"].value<std::string>().value());
    if (static_cast<int>(lines.size()) != 2 * height_ + 1) throw std::runtime_error("map: level " + std::to_string(z) + " has the wrong number of rows");
    auto at = [&](int row, int col) -> char {
      const std::string& s = lines[row];
      return col < static_cast<int>(s.size()) ? s[col] : ' ';
    };
    for (int y = 0; y < height_; ++y)
      for (int x = 0; x < width_; ++x) floors_[cell_index({z, x, y})] = floor_from_char(at(2 * y + 1, 2 * x + 1));
    for (int y = 0; y <= height_; ++y)
      for (int x = 0; x < width_; ++x) edge(EdgeRef{z, x, y, false}).kind = edge_from_char(at(2 * y, 2 * x + 1));
    for (int y = 0; y < height_; ++y)
      for (int x = 0; x <= width_; ++x) edge(EdgeRef{z, x, y, true}).kind = edge_from_char(at(2 * y + 1, 2 * x));
  }

  // One assembly instance per wall, door, window and fence segment, in a fixed order.
  const int quality = t["map"]["build_quality"].value_or(50);
  auto make = [&](std::vector<Edge>& edges) {
    for (Edge& e : edges) {
      if (e.kind == EdgeKind::None) continue;
      const auto def = items.find_def(def_for(e.kind));
      if (!def) throw std::runtime_error(std::string("map: missing item def ") + def_for(e.kind));
      e.assembly = items.create(w, *def, quality, Locations::kWorld);
    }
  };
  make(h_edges_);
  make(v_edges_);

  stairs_.clear();
  if (const auto* st = t["stairs"].as_array())
    for (const auto& s : *st) stairs_.push_back({cell_from(s.as_table()->get("a")->as_array()), cell_from(s.as_table()->get("b")->as_array())});
  objects_.clear();
  if (const auto* objs = t["object"].as_array())
    for (const auto& o : *objs) {
      const auto& ot = *o.as_table();
      MapObject mo{ot["kind"].value<std::string>().value(), cell_from(ot["at"].as_array()), ot["id"].value_or(std::string{}), {}};
      // Any other number is an amount (a store's stock, a container's contents), kept in key order.
      for (const auto& [key, value] : ot)
        if (auto v = value.value<double>()) mo.amounts.emplace_back(std::string(key.str()), *v);
      if (const auto* contents = ot["contents"].as_table())
        for (const auto& [key, value] : *contents)
          if (auto v = value.value<double>()) mo.amounts.emplace_back(std::string(key.str()), *v);
      objects_.push_back(std::move(mo));
    }
  dirty_ = true;
}

void SiteMap::set_open(const EdgeRef& e, bool open) {
  Edge& ed = edge(e);
  if (!is_openable(ed.kind) || ed.open == open) return;
  ed.open = open;
  dirty_ = true;
}

void SiteMap::set_breached(const EdgeRef& e, bool breached) {
  edge(e).breached = breached;
  dirty_ = true;
}

void SiteMap::set_locked(const EdgeRef& e, bool locked) { edge(e).locked = locked; }

void SiteMap::set_barricade(const EdgeRef& e, EntityId barricade) {
  edge(e).barricade = barricade;
  dirty_ = true;
}

bool SiteMap::air_passes(const EdgeRef& e) const {
  const Edge& ed = edge(e);
  if (ed.kind == EdgeKind::None || ed.breached) return true;
  return is_openable(ed.kind) && ed.open && ed.barricade == 0;
}

bool SiteMap::can_walk(const EdgeRef& e) const {
  const Edge& ed = edge(e);
  if (ed.kind == EdgeKind::None || ed.breached) return true;
  if (ed.kind != EdgeKind::DoorInterior && ed.kind != EdgeKind::DoorExterior && ed.kind != EdgeKind::Gate) return false;
  if (ed.barricade != 0) return false;
  return ed.open || !ed.locked;
}

void SiteMap::rebuild_rooms() const {
  room_ids_.assign(floors_.size(), -1);
  rooms_.clear();
  portals_.clear();
  const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};  // N, E, S, W
  for (int z : levels_)
    for (int y = 0; y < height_; ++y)
      for (int x = 0; x < width_; ++x) {
        const Cell start{z, x, y};
        if (!walkable(start) || room_ids_[cell_index(start)] >= 0) continue;
        const int id = static_cast<int>(rooms_.size());
        Room room{z, 0, is_indoor(floor(start)), false};
        std::vector<Cell> stack{start};
        room_ids_[cell_index(start)] = id;
        while (!stack.empty()) {
          const Cell c = stack.back();
          stack.pop_back();
          ++room.cells;
          if (!is_indoor(floor(c)) && (c.x == 0 || c.y == 0 || c.x == width_ - 1 || c.y == height_ - 1))
            room.touches_border = true;
          for (int d = 0; d < 4; ++d) {
            const Cell n{z, c.x + dx[d], c.y + dy[d]};
            if (!walkable(n) || room_ids_[cell_index(n)] >= 0 || !air_passes(edge_between(c, n))) continue;
            room_ids_[cell_index(n)] = id;
            stack.push_back(n);
          }
        }
        rooms_.push_back(room);
      }
  // Portals: every edge between two walkable cells of different rooms, then stairs.
  for (int z : levels_)
    for (int y = 0; y < height_; ++y)
      for (int x = 0; x < width_; ++x) {
        const Cell c{z, x, y};
        if (!walkable(c)) continue;
        for (const Cell n : {Cell{z, x + 1, y}, Cell{z, x, y + 1}}) {
          if (!walkable(n)) continue;
          const int a = room_ids_[cell_index(c)], b = room_ids_[cell_index(n)];
          if (a == b) continue;
          const EdgeRef e = edge_between(c, n);
          portals_.push_back({a, b, e, air_passes(e)});
        }
      }
  for (const Stairs& s : stairs_)
    portals_.push_back({room_ids_[cell_index(s.a)], room_ids_[cell_index(s.b)], std::nullopt, true});
  dirty_ = false;
}

int SiteMap::room_of(const Cell& c) const {
  if (dirty_) rebuild_rooms();
  return walkable(c) ? room_ids_[cell_index(c)] : -1;
}

const std::vector<Room>& SiteMap::rooms() const {
  if (dirty_) rebuild_rooms();
  return rooms_;
}

const std::vector<Portal>& SiteMap::portals() const {
  if (dirty_) rebuild_rooms();
  return portals_;
}

std::vector<Cell> SiteMap::find_path(const Cell& from, const Cell& to) const {
  if (!walkable(from) || !walkable(to)) return {};
  constexpr int kStep = 10, kDoor = 20, kStairs = 20;
  const std::size_t n = floors_.size();
  std::vector<int> cost(n, INT32_MAX);
  std::vector<std::size_t> prev(n, SIZE_MAX);
  std::vector<Cell> cells(n);
  for (int z : levels_)
    for (int y = 0; y < height_; ++y)
      for (int x = 0; x < width_; ++x) cells[cell_index({z, x, y})] = {z, x, y};
  using Item = std::pair<int, std::size_t>;  // (cost, cell index): ties broken by index
  std::priority_queue<Item, std::vector<Item>, std::greater<Item>> open;
  const std::size_t s = cell_index(from), goal = cell_index(to);
  cost[s] = 0;
  open.push({0, s});
  const int dx[4] = {0, 1, 0, -1}, dy[4] = {-1, 0, 1, 0};
  while (!open.empty()) {
    const auto [c, i] = open.top();
    open.pop();
    if (c != cost[i]) continue;
    if (i == goal) break;
    const Cell cur = cells[i];
    auto relax = [&](const Cell& nb, int step) {
      const std::size_t j = cell_index(nb);
      if (c + step < cost[j]) { cost[j] = c + step; prev[j] = i; open.push({cost[j], j}); }
    };
    for (int d = 0; d < 4; ++d) {
      const Cell nb{cur.z, cur.x + dx[d], cur.y + dy[d]};
      if (!walkable(nb)) continue;
      const EdgeRef e = edge_between(cur, nb);
      if (!can_walk(e)) continue;
      const Edge& ed = edge(e);
      relax(nb, kStep + ((is_openable(ed.kind) && !ed.open && !ed.breached) ? kDoor : 0));
    }
    for (const Stairs& st : stairs_) {
      if (st.a == cur) relax(st.b, kStairs);
      else if (st.b == cur) relax(st.a, kStairs);
    }
  }
  if (cost[goal] == INT32_MAX) return {};
  std::vector<Cell> path;
  for (std::size_t i = goal; i != SIZE_MAX; i = prev[i]) path.push_back(cells[i]);
  std::reverse(path.begin(), path.end());
  return path;
}

std::optional<Cell> SiteMap::find_object(const std::string& kind, const std::string& id) const {
  for (const MapObject& o : objects_)
    if (o.kind == kind && (id.empty() || o.id == id)) return o.at;
  return std::nullopt;
}

void SiteMap::save(ArchiveWriter& w) const {
  // The layout comes from the map file; the save holds what can change: edge states.
  for (const auto* edges : {&h_edges_, &v_edges_}) {
    w.u64(edges->size());
    for (const Edge& e : *edges) {
      w.u64(e.assembly);
      w.u64(e.barricade);
      w.u8(static_cast<std::uint8_t>((e.open ? 1 : 0) | (e.locked ? 2 : 0) | (e.breached ? 4 : 0)));
    }
  }
}

void SiteMap::load(ArchiveReader& r) {
  for (auto* edges : {&h_edges_, &v_edges_}) {
    if (r.u64() != edges->size()) throw std::runtime_error("save: map size differs");
    for (Edge& e : *edges) {
      e.assembly = r.u64();
      e.barricade = r.u64();
      const std::uint8_t f = r.u8();
      e.open = f & 1;
      e.locked = f & 2;
      e.breached = f & 4;
    }
  }
  dirty_ = true;
}

namespace {
Joint joint_from(const std::string& s) {
  if (s == "loose") return Joint::Loose;
  if (s == "screwed") return Joint::Screwed;
  if (s == "nailed") return Joint::Nailed;
  if (s == "welded") return Joint::Welded;
  if (s == "glued") return Joint::Glued;
  throw std::runtime_error("items: unknown joint " + s);
}
}  // namespace

void load_item_defs(Assemblies& items, const std::string& path) {
  const toml::table t = toml::parse_file(path);
  if (const auto* mats = t["material"].as_array())
    for (const auto& node : *mats) {
      const auto& m = *node.as_table();
      items.add_material({m["id"].value<std::string>().value(), m["density_kg_m3"].value_or(0.0),
                          m["hardness"].value_or(0.0), m["source"].value_or(std::string{})});
    }
  const auto* list = t["item"].as_array();
  if (!list) throw std::runtime_error("items: no [[item]] tables");
  for (const auto& node : *list) {
    const auto& it = *node.as_table();
    ItemDef d;
    // Inheritance: start from the parent (defined earlier in the file), then override.
    if (auto parent = it["parent"].value<std::string>()) {
      const auto pid = items.find_def(*parent);
      if (!pid) throw std::runtime_error("items: parent " + *parent + " must come first");
      d = items.def(*pid);
      d.name.clear();
    }
    d.id = it["id"].value<std::string>().value();
    d.name = it["name"].value_or(d.id);
    d.family = it["family"].value_or(d.family.empty() ? d.id : d.family);
    d.mass_g = it["mass_g"].value_or(d.mass_g);
    d.failure_threshold = it["failure_threshold_ppm"].value_or(d.failure_threshold);
    d.hold_force_n = it["hold_force_n"].value_or(d.hold_force_n);
    d.sound_loss_db = it["sound_loss_db"].value_or(d.sound_loss_db);
    d.source = it["source"].value_or(d.source);
    d.material = it["material"].value_or(d.material);
    if (!d.material.empty() && !items.find_material(d.material))
      throw std::runtime_error("items: unknown material " + d.material + " in " + d.id);
    if (const auto* chain = it["failure_chain"].as_array()) {
      d.failure_chain.clear();
      for (const auto& s : *chain) d.failure_chain.push_back(s.value<std::string>().value());
    }
    if (const auto* slots = it["slot"].as_array()) {
      d.slots.clear();
      for (const auto& sn : *slots) {
        const auto& st = *sn.as_table();
        d.slots.push_back({st["name"].value<std::string>().value(), st["accepts"].value<std::string>().value(),
                           st["required"].value_or(true), joint_from(st["joint"].value_or(std::string("screwed"))),
                           st["default"].value_or(std::string{})});
      }
    }
    if (const auto* props = it["props"].as_table())
      for (const auto& [key, value] : *props) {
        const auto p = prop_from(std::string(key.str()));
        if (!p) throw std::runtime_error("items: unknown property " + std::string(key.str()) + " in " + d.id);
        d.props[static_cast<std::size_t>(*p)] = value.value<double>().value();
      }
    items.add_def(std::move(d));
  }
}

}  // namespace zs
